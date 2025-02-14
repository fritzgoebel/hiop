
# Set headers to be installed as part of the hiop interface
set(hiopLinAlg_INTERFACE_HEADERS
  hiop_blasdefs.hpp
  hiopLinAlgFactory.hpp
  hiopLinSolver.hpp
  hiopLinSolverIndefDenseLapack.hpp
  hiopLinSolverIndefDenseMagma.hpp
  hiopLinSolverIndefSparseMA57.hpp
  hiopLinSolverMA86Z.hpp
  hiopLinSolverSparseSTRUMPACK.hpp
  hiopLinSolverSparsePARDISO.hpp
  hiopLinSolverSparseCUSOLVER.hpp
  hiopLinSolverUMFPACKZ.hpp
  hiopLinSolverCholCuSparse.hpp
  hiopMatrix.hpp
  hiopMatrixComplexDense.hpp
  hiopMatrixComplexSparseTriplet.hpp
  hiopMatrixDense.hpp
  hiopMatrixDenseRowMajor.hpp
  hiopMatrixMDS.hpp
  hiopMatrixRajaDense.hpp
  hiopMatrixRajaSparseTriplet.hpp
  hiopMatrixSparse.hpp
  hiopMatrixSparseTriplet.hpp
  hiopMatrixSparseTripletStorage.hpp
  hiopMatrixSparseCSR.hpp
  hiopMatrixSparseCSRSeq.hpp
  hiopVector.hpp
  hiopVectorInt.hpp
  hiopVectorIntRaja.hpp
  hiopVectorIntSeq.hpp
  hiopVectorPar.hpp
  hiopVectorRajaPar.hpp
  hiopLinearOperator.hpp
  hiopKrylovSolver.hpp
  )

# Set linear algebra common source files
set(hiopLinAlg_SRC
  hiopVectorPar.cpp
  hiopVectorIntSeq.cpp
  hiopMatrixDenseRowMajor.cpp
  hiopLinSolver.cpp
  hiopLinAlgFactory.cpp
  hiopMatrixMDS.cpp
  hiopMatrixComplexDense.cpp
  hiopMatrixSparseTripletStorage.cpp
  hiopMatrixSparseTriplet.cpp
  hiopMatrixComplexSparseTriplet.cpp
  hiopMatrixSparseCSRSeq.cpp
  hiopLinearOperator.cpp
  hiopKrylovSolver.cpp
)

set(hiopLinAlg_RAJA_SRC
  hiopVectorRajaPar.cpp
  hiopVectorIntRaja.cpp
  hiopMatrixRajaDense.cpp
  hiopMatrixRajaSparseTriplet.cpp
  )

set(hiopLinAlg_GPU_SRC
  hiopLinSolverIndefDenseMagma.cpp
  )

set(hiopLinAlg_MA57_SRC
  hiopLinSolverIndefSparseMA57.cpp
  )

set(hiopLinAlg_STRUMPACK_SRC
  hiopLinSolverSparseSTRUMPACK.cpp
  )

set(hiopLinAlg_PARDISO_SRC
  hiopLinSolverSparsePARDISO.cpp
)

set(hiopLinAlg_CUSOLVER_SRC
  hiopLinSolverSparseCUSOLVER.cpp
  hiopLinSolverCholCuSparse.cpp
  )

set(hiopLinAlg_KRON_REDUCTION_SRC
  hiopLinSolverUMFPACKZ.cpp
  )

# Add interfaces for sparse linear solvers when enabled
if(HIOP_SPARSE)
    if(HIOP_USE_COINHSL)
      list(APPEND hiopLinAlg_SRC ${hiopLinAlg_MA57_SRC})
    endif(HIOP_USE_COINHSL)      
    if(HIOP_USE_STRUMPACK)
      list(APPEND hiopLinAlg_SRC ${hiopLinAlg_STRUMPACK_SRC})
    endif(HIOP_USE_STRUMPACK)
    if(HIOP_USE_PARDISO)
      list(APPEND hiopLinAlg_SRC ${hiopLinAlg_PARDISO_SRC})
    endif(HIOP_USE_PARDISO)
    if(HIOP_USE_CUSOLVER)
      list(APPEND hiopLinAlg_SRC ${hiopLinAlg_CUSOLVER_SRC})
    endif(HIOP_USE_CUSOLVER)
    # TODO: Make KLU optional dependency of cuSOLVER LU factorization
    if(HIOP_USE_CUDA)
      list(APPEND hiopLinAlg_SRC hiopLinSolverCholCuSparse.cpp)
    endif(HIOP_USE_CUDA)
endif()

# Add RAJA/Umpire sources when enabled
if(HIOP_USE_RAJA)
  list(APPEND hiopLinAlg_INTERFACE_HEADERS hiop_raja_defs.hpp)
  list(APPEND hiopLinAlg_SRC ${hiopLinAlg_RAJA_SRC})
endif()

# If GPU support is enabled add Magma interface (CUDA version)
# Treat RAJA sources as CUDA (temporary, need more flexible solutions)
if(HIOP_USE_GPU)
  list(APPEND hiopLinAlg_SRC ${hiopLinAlg_GPU_SRC})
  if(HIOP_USE_CUDA)
    set_source_files_properties(${hiopLinAlg_RAJA_SRC} ${hiopLinAlg_CUSOLVER_SRC} PROPERTIES LANGUAGE CUDA)
  endif()
endif()

# Add interface to UMFPACK when Kron reduction is enabled
if(HIOP_WITH_KRON_REDUCTION)
  list(APPEND hiopLinAlg_SRC ${hiopLinAlg_KRON_REDUCTION_SRC})
endif()

# Build Kron reduction app
if(HIOP_WITH_KRON_REDUCTION)
  add_executable(test_hiopLinAlgComplex.exe test_hiopLinalgComplex.cpp)
  target_link_libraries(test_hiopLinAlgComplex.exe PRIVATE HiOp::HiOp)
endif(HIOP_WITH_KRON_REDUCTION)

install(
  FILES ${hiopLinAlg_INTERFACE_HEADERS}
  DESTINATION include
  )

add_library(hiopLinAlg OBJECT ${hiopLinAlg_SRC})
target_link_libraries(hiopLinAlg PRIVATE hiop_tpl)
