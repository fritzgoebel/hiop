// Copyright (c) 2017, Lawrence Livermore National Security, LLC.
// Produced at the Lawrence Livermore National Laboratory (LLNL).
// Written by Cosmin G. Petra, petra1@llnl.gov.
// LLNL-CODE-742473. All rights reserved.
//
// This file is part of HiOp. For details, see https://github.com/LLNL/hiop. HiOp 
// is released under the BSD 3-clause license (https://opensource.org/licenses/BSD-3-Clause). 
// Please also read “Additional BSD Notice” below.
//
// Redistribution and use in source and binary forms, with or without modification, 
// are permitted provided that the following conditions are met:
// i. Redistributions of source code must retain the above copyright notice, this list 
// of conditions and the disclaimer below.
// ii. Redistributions in binary form must reproduce the above copyright notice, 
// this list of conditions and the disclaimer (as noted below) in the documentation and/or 
// other materials provided with the distribution.
// iii. Neither the name of the LLNS/LLNL nor the names of its contributors may be used to 
// endorse or promote products derived from this software without specific prior written 
// permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
// SHALL LAWRENCE LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
// AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Additional BSD Notice
// 1. This notice is required to be provided under our contract with the U.S. Department 
// of Energy (DOE). This work was produced at Lawrence Livermore National Laboratory under 
// Contract No. DE-AC52-07NA27344 with the DOE.
// 2. Neither the United States Government nor Lawrence Livermore National Security, LLC 
// nor any of their employees, makes any warranty, express or implied, or assumes any 
// liability or responsibility for the accuracy, completeness, or usefulness of any 
// information, apparatus, product, or process disclosed, or represents that its use would
// not infringe privately-owned rights.
// 3. Also, reference herein to any specific commercial products, process, or services by 
// trade name, trademark, manufacturer or otherwise does not necessarily constitute or 
// imply its endorsement, recommendation, or favoring by the United States Government or 
// Lawrence Livermore National Security, LLC. The views and opinions of authors expressed 
// herein do not necessarily state or reflect those of the United States Government or 
// Lawrence Livermore National Security, LLC, and shall not be used for advertising or 
// product endorsement purposes.

/**
 * @file hiopVectorIntRaja.cpp
 *
 * @author Asher Mancinelli <asher.mancinelli@pnnl.gov>, PNNL
 *
 */

#include "hiopVectorIntRaja.hpp"
#include <umpire/Allocator.hpp>
#include <umpire/ResourceManager.hpp>

#include <RAJA/RAJA.hpp>
#include "hiop_raja_defs.hpp"

namespace hiop
{

hiopVectorIntRaja::hiopVectorIntRaja(size_type sz, std::string mem_space)
  : hiopVectorInt(sz),
    mem_space_(mem_space)
{
#ifndef HIOP_USE_GPU
  mem_space_ = "HOST";
#endif

  auto& resmgr = umpire::ResourceManager::getInstance();
  umpire::Allocator devalloc = resmgr.getAllocator(mem_space_);
  buf_ = static_cast<index_type*>(devalloc.allocate(sz_*sizeof(index_type)));
  if(mem_space_ == "DEVICE") {
    umpire::Allocator hostalloc = resmgr.getAllocator("HOST");
    buf_host_ = static_cast<index_type*>(hostalloc.allocate(sz_*sizeof(index_type)));
  } else {
    buf_host_ = buf_;
  }
}

hiopVectorIntRaja::~hiopVectorIntRaja()
{
  auto& resmgr = umpire::ResourceManager::getInstance();
  umpire::Allocator devalloc = resmgr.getAllocator(mem_space_);
  devalloc.deallocate(buf_);
  if (mem_space_ == "DEVICE") {
    umpire::Allocator hostalloc = resmgr.getAllocator("HOST");
    hostalloc.deallocate(buf_host_);
  }
  buf_host_ = nullptr;
  buf_ = nullptr;
}

void hiopVectorIntRaja::copy_from_dev()
{
  if (buf_ != buf_host_) {
    auto& resmgr = umpire::ResourceManager::getInstance();
    resmgr.copy(buf_host_, buf_);
  }
}

void hiopVectorIntRaja::copy_to_dev()
{
  if (buf_ != buf_host_) {
    auto& resmgr = umpire::ResourceManager::getInstance();
    resmgr.copy(buf_, buf_host_);
  }
}

void hiopVectorIntRaja::copy_from(const index_type* v_local)
{
  if(v_local) {
    auto& resmgr = umpire::ResourceManager::getInstance();
    index_type* data = const_cast<index_type*>(v_local);
    resmgr.copy(buf_, data, sz_*sizeof(index_type));
  }
}

void hiopVectorIntRaja::set_to_zero()
{
  auto& rm = umpire::ResourceManager::getInstance();
  rm.memset(buf_, 0);
}

/// Set all vector elements to constant c
void hiopVectorIntRaja::set_to_constant(const index_type c)
{
  index_type* data = buf_;
  RAJA::forall<hiop_raja_exec>(RAJA::RangeSegment(0, sz_),
    RAJA_LAMBDA(RAJA::Index_type i)
    {
      data[i] = c;
    });
}

/**
 * @brief Set the vector entries to be a linear space of starting at i0 containing evenly 
 * incremented integers up to i0+(n-1)di, when n is the length of this vector
 *
 * @pre The elements of the linear space should not overflow the index_type type
 *  
 * @param i0 the starting element in the linear space (entry 0 in vector)
 * @param di the increment for subsequent entries in the vector
 *
 */ 
void hiopVectorIntRaja::linspace(const index_type& i0, const index_type& di)
{
  index_type* data = buf_;
  RAJA::forall<hiop_raja_exec>(RAJA::RangeSegment(0, sz_),
    RAJA_LAMBDA(RAJA::Index_type i)
    {
      data[i] = i0+i*di;
    });
}
  
} // namespace hiop
