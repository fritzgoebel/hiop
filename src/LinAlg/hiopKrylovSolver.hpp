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

/* implements the Krylov iterative solver
* @file hiopKrylovSolver.hpp
* @ingroup LinearSolvers
* @author Nai-Yuan Chiang <chiang7@lnnl.gov>, LNNL
* @author Cosmin G. Petra <petra1@lnnl.gov>, LNNL
*/

#ifndef HIOP_KrylovSolver
#define HIOP_KrylovSolver

#include "hiopMatrix.hpp"
#include "hiopMatrixDense.hpp"
#include "hiopVector.hpp"

#include "hiop_blasdefs.hpp"

#include "hiopCppStdUtils.hpp"

#include "hiopLinearOperator.hpp"
#include "hiopNlpFormulation.hpp"

namespace hiop
{

/**
 * Base class for Krylov Solvers used by HiOp
 */

class hiopKrylovSolver
{
public:
  hiopKrylovSolver(int n,
                   const std::string& mem_space,
                   hiopLinearOperator* A_opr,
                   hiopLinearOperator* Mleft_opr = nullptr,
                   hiopLinearOperator* Mright_opr = nullptr,
                   const hiopVector* x0 = nullptr);
  virtual ~hiopKrylovSolver();

  /** Solves a linear system.
   * param 'x' is on entry the right hand side(s) of the system to be solved. On
   * exit is contains the solution(s).
   */
  virtual bool solve(hiopVector& x) = 0;

  /** Set the initial guess for the Krylov solver */
  virtual void set_x0(const hiopVector& x0);

  /** Set the iterate to a constant value */
  virtual void set_x0(double xval);

  ///
  virtual void set_max_num_iter(int num_iter);

  /** Return statistic convergence stats */
  virtual double get_sol_errrel();

  ///
  virtual double get_sol_errabs();

  ///
  virtual double get_num_iter();

  ///
  virtual std::string get_convergence_info() = 0;

  /**
   * Convergence flag: 0 for success, the other codes depending on the Krylov method
   * used. Concrete message about the convergence can be obtained from 
   * get_convergence_info.
   */
  virtual int get_convergence_flag() = 0;
protected:
  void 
protected:

  /// Memory space
  std::string mem_space_;
  
  /// Linear operator to apply the linear system matrix to a residual/vector
  hiopLinearOperator* A_opr_;

  /// Left preconditioner
  hiopLinearOperator* ML_opr_;
  
  /// Right preconditioners
  hiopLinearOperator* MR_opr_;

  hiopVector* x0_;

  hiopNlpFormulation* nlp_;
};

/** 
 * a Krylov solver class implementing the PCG framework
 */
class hiopPCGSolver : public hiopKrylovSolver
{
public:
  /** initialization constructor */
  hiopPCGSolver(hiopLinearOperator* A_opr,
                hiopLinearOperator* Mleft_opr = nullptr,
                hiopLinearOperator* Mright_opr = nullptr,
                const hiopVector* x0 = nullptr,
                hiopNlpFormulation* nlp);
  virtual ~hiopPCGSolver();

  /** Solves a linear system.
   * param 'x' is on entry the right hand side(s) of the system to be solved. On
   * exit is contains the solution(s).
   */
  virtual bool solve(hiopVector& x);

protected:
  const double tol_;
  size_type maxit_;
  double iter_;
  int flag_;

  hiopVector* xmin_;
  hiopVector* res_;
  hiopVector* yk_;
  hiopVector* zk_;
  hiopVector* pk_;
  hiopVector* qk_;
};


} //end namespace

#endif
