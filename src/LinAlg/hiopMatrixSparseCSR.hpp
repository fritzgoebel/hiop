#ifndef HIOP_SPARSE_MATRIX_CSR
#define HIOP_SPARSE_MATRIX_CSR

#include "hiopVector.hpp"
#include "hiopMatrixDense.hpp"
#include "hiopMatrixSparse.hpp"
#include "hiopMatrixSparseTriplet.hpp"

#include <cassert>
#include <unordered_map>

namespace hiop
{

/**
 * @brief Sparse matrix of doubles in compressed row format for use on CPU/host. Data
 * is not (memory, MPI) distributed.
 *
 * Note: most of the methods are not implemented (TODO) as this is work in progress.
 */
class hiopMatrixSparseCSR : public hiopMatrixSparse
{
public:
  hiopMatrixSparseCSR(int num_rows, int num_cols, int nnz);
  virtual ~hiopMatrixSparseCSR();

  virtual void setToZero();
  virtual void setToConstant(double c);
  virtual void copyFrom(const hiopMatrixSparse& dm);
  virtual void copy_to(index_type* irow, index_type* jcol, double* val);
  virtual void copy_to(hiopMatrixDense& W);

  virtual void copyRowsFrom(const hiopMatrix& src, const index_type* rows_idxs, size_type n_rows);

  virtual void timesVec(double beta,  hiopVector& y, double alpha, const hiopVector& x) const;
  virtual void timesVec(double beta,  double* y, double alpha, const double* x) const;

  virtual void transTimesVec(double beta,   hiopVector& y, double alpha, const hiopVector& x) const;
  virtual void transTimesVec(double beta,   double* y, double alpha, const double* x) const;

  virtual void timesMat(double beta, hiopMatrix& W, double alpha, const hiopMatrix& X) const;

  virtual void transTimesMat(double beta, hiopMatrix& W, double alpha, const hiopMatrix& X) const;

  virtual void timesMatTrans(double beta, hiopMatrix& W, double alpha, const hiopMatrix& X) const;

  virtual void addDiagonal(const double& alpha, const hiopVector& d_);
  virtual void addDiagonal(const double& value);
  virtual void addSubDiagonal(const double& alpha, index_type start, const hiopVector& d_);
  
  /* add to the diagonal of 'this' (destination) starting at 'start_on_dest_diag' elements of
   * 'd_' (source) starting at index 'start_on_src_vec'. The number of elements added is 'num_elems'
   * when num_elems>=0, or the remaining elems on 'd_' starting at 'start_on_src_vec'. */
  virtual void addSubDiagonal(index_type start_on_dest_diag,
                              const double& alpha,
                              const hiopVector& d_,
                              index_type start_on_src_vec,
                              int num_elems=-1)
  {
    assert(false && "not needed / implemented");
  }
  virtual void addSubDiagonal(index_type start_on_dest_diag, int num_elems, const double& c)
  {
    assert(false && "not needed / implemented");
  }

  /* Add to the diagonal of 'this' (destination) starting at 'start_on_dest_diag' elements of
  * 'd_' (source) starting at index 'start_on_src_vec'. The number of elements added is 'num_elems', 
  * scaled by 'scal'
  */
  virtual void copySubDiagonalFrom(const index_type& start_on_dest_diag,
                                   const size_type& num_elems,
                                   const hiopVector& d_,
                                   const index_type& start_on_nnz_idx,
                                   double scal=1.0);

  /* add constant 'c' to the diagonal of 'this' (destination) starting at 'start_on_dest_diag' elements.
  * The number of elements added is 'num_elems'
  */
  virtual void setSubDiagonalTo(const index_type& start_on_dest_diag,
                                const size_type& num_elems,
                                const double& c,
                                const index_type& start_on_nnz_idx);

  virtual void addMatrix(double alpha, const hiopMatrix& X);

  /* block of W += alpha*transpose(this), where W is dense */
  virtual void transAddToSymDenseMatrixUpperTriangle(index_type row_dest_start,
                                                     index_type col_dest_start,
                                                     double alpha,
                                                     hiopMatrixDense& W) const;
  virtual void addUpperTriangleToSymDenseMatrixUpperTriangle(index_type diag_start,
                                                             double alpha,
                                                             hiopMatrixDense& W) const
  {
    assert(false && "not yet implemented");
  }

  virtual void addUpperTriangleToSymSparseMatrixUpperTriangle(index_type diag_start,
                                                              double alpha,
                                                              hiopMatrixSparse& W) const
  {
    assert(false && "not yet implemented");
  }

  /* Diag block of W += alpha * M * D^{-1} * transpose(M), where M=this
   *
   * Only the upper triangular entries of W are updated.
   */
  virtual void addMDinvMtransToDiagBlockOfSymDeMatUTri(index_type rowCol_dest_start,
                                                       const double& alpha,
						       const hiopVector& D,
                                                       hiopMatrixDense& W) const;

  /* Block of W += alpha * M * D^{-1} * transpose(N), where M=this
   *
   * Warning: The product matrix M * D^{-1} * transpose(N) with start offsets 'row_dest_start' and
   * 'col_dest_start' needs to fit completely in the upper triangle of W. If this is NOT the
   * case, the method will assert(false) in debug; in release, the method will issue a
   * warning with HIOP_DEEPCHECKS (otherwise NO warning will be issue) and will silently update
   * the (strictly) lower triangular  elements (these are ignored later on since only the upper
   * triangular part of W will be accessed)
   */
  virtual void addMDinvNtransToSymDeMatUTri(index_type row_dest_start,
                                            index_type col_dest_start,
                                            const double& alpha,
                                            const hiopVector& D,
                                            const hiopMatrixSparse& N,
                                            hiopMatrixDense& W) const;

  virtual void copyRowsBlockFrom(const hiopMatrix& src_gen,
                                 const index_type& rows_src_idx_st,
                                 const size_type& n_rows,
                                 const index_type& rows_dest_idx_st,
                                 const size_type& dest_nnz_st);

  /**
  * @brief Copy matrix 'src_gen', into 'this' as a submatrix from corner 'dest_row_st' and 'dest_col_st'
  * The non-zero elements start from 'dest_nnz_st' will be replaced by the new elements. 
  * When `offdiag_only` is set to true, only the off-diagonal part of `src_gen` is copied.
  *
  * @pre 'this' must have enough rows and cols after row 'dest_row_st' and col 'dest_col_st'
  * @pre 'dest_nnz_st' + the number of non-zeros in the copied matrix must be less or equal to 
  * this->numOfNumbers()
  * @pre User must know the nonzero pattern of src and dest matrices. The method assumes 
  * that non-zero patterns does not change between calls and that 'src_gen' is a valid
  *  submatrix of 'this'
  * @pre This function does NOT preserve the sorted row/col indices. USE WITH CAUTION!
  */
  virtual void copySubmatrixFrom(const hiopMatrix& src_gen,
                                 const index_type& dest_row_st,
                                 const index_type& dest_col_st,
                                 const size_type& dest_nnz_st,
                                 const bool offdiag_only = false);
  
  /**
  * @brief Copy the transpose of matrix 'src_gen', into 'this' as a submatrix from corner 
  * 'dest_row_st' and 'dest_col_st'.
  * The non-zero elements start from 'dest_nnz_st' will be replaced by the new elements.
  * When `offdiag_only` is set to true, only the off-diagonal part of `src_gen` is copied.
  * 
  * @pre This function does NOT preserve the sorted row/col indices. USE WITH CAUTION!
  */
  virtual void copySubmatrixFromTrans(const hiopMatrix& src_gen,
                                      const index_type& dest_row_st,
                                      const index_type& dest_col_st,
                                      const size_type& dest_nnz_st,
                                      const bool offdiag_only = false);

  /**
  * @brief Copy selected columns of a diagonal matrix (a constant 'scalar' times identity),
  * into 'this' as a submatrix from corner 'dest_row_st' and 'dest_col_st'
  * The non-zero elements start from 'dest_nnz_st' will be replaced by the new elements.
  * @pre The diagonal entries in the destination need to be contiguous in the sparse triplet arrays of the destinations.
  * @pre this function does NOT preserve the sorted row/col indices. USE WITH CAUTION!
  */
  virtual void setSubmatrixToConstantDiag_w_colpattern(const double& scalar,
                                                       const index_type& dest_row_st,
                                                       const index_type& dest_col_st,
                                                       const size_type& dest_nnz_st,
                                                       const size_type& nnz_to_copy,
                                                       const hiopVector& ix);

  /**
  * @brief Copy selected rows of a diagonal matrix (a constant 'scalar' times identity),
  * into 'this' as a submatrix from corner 'dest_row_st' and 'dest_col_st'
  * The non-zero elements start from 'dest_nnz_st' will be replaced by the new elements.
  * @pre The diagonal entries in the destination need to be contiguous in the sparse triplet arrays of the destinations.
  * @pre this function does NOT preserve the sorted row/col indices. USE WITH CAUTION!
  */
  virtual void setSubmatrixToConstantDiag_w_rowpattern(const double& scalar,
                                                       const index_type& dest_row_st,
                                                       const index_type& dest_col_st,
                                                       const size_type& dest_nnz_st,
                                                       const size_type& nnz_to_copy,
                                                       const hiopVector& ix);

  /**
  * @brief Copy a diagonal matrix to destination.
  * This diagonal matrix is 'src_val'*identity matrix with size 'nnz_to_copy'x'nnz_to_copy'.
  * The destination is updated from the start row 'row_dest_st' and start column 'col_dest_st'. USE WITH CAUTION!
  */
  virtual void copyDiagMatrixToSubblock(const double& src_val,
                                        const index_type& dest_row_st,
                                        const index_type& dest_col_st,
                                        const size_type& dest_nnz_st,
                                        const size_type &nnz_to_copy);

  /** 
  * @brief same as @copyDiagMatrixToSubblock, but copies only diagonal entries specified by `pattern`.
  * At the destination, 'nnz_to_copy` nonzeros starting from index `dest_nnz_st` will be replaced.
  * @pre The added entries in the destination need to be contiguous in the sparse triplet arrays of the destinations.
  * @pre This function does NOT preserve the sorted row/col indices. USE WITH CAUTION!
  * @pre 'pattern' has same size as `x`. 
  * @pre 'pattern` has exactly `nnz_to_copy` nonzeros.
  */
  virtual void copyDiagMatrixToSubblock_w_pattern(const hiopVector& x,
                                                  const index_type& dest_row_st,
                                                  const index_type& dest_col_st,
                                                  const size_type& dest_nnz_st,
                                                  const size_type &nnz_to_copy,
                                                  const hiopVector& pattern);

  virtual double max_abs_value();

  virtual void row_max_abs_value(hiopVector &ret_vec);
  
  virtual void scale_row(hiopVector &vec_scal, const bool inv_scale=false);

  virtual bool isfinite() const;

  virtual void print(FILE* f=nullptr, const char* msg=nullptr, int maxRows=-1, int maxCols=-1, int rank=-1) const;

  virtual void startingAtAddSubDiagonalToStartingAt(index_type diag_src_start,
                                                    const double& alpha,
                                                    hiopVector& vec_dest,
                                                    index_type vec_start,
                                                    size_type num_elems=-1) const
  {
    assert(0 && "not implemented; should be used only for symmetric matrices.");
  }

  virtual size_type numberOfOffDiagNonzeros() const
  {
    assert(false && "not yet implemented");
    return 0;
  }

  /// @brief extend base problem Jac to the Jac in feasibility problem
  virtual void set_Jac_FR(const hiopMatrixSparse& Jac_c,
                          const hiopMatrixSparse& Jac_d,
                          index_type* iJacS,
                          index_type* jJacS,
                          double* MJacS)
  {
    assert(false && "not yet implemented");
  }


  /// @brief extend base problem Hess to the Hess in feasibility problem
  virtual void set_Hess_FR(const hiopMatrixSparse& Hess,
                           index_type* iHSS,
                           index_type* jHSS,
                           double* MHSS,
                           const hiopVector& add_diag)
  {
    assert(false && "not yet implemented");
  }

  virtual hiopMatrixSparse* alloc_clone() const;
  virtual hiopMatrixSparse* new_copy() const;

  inline index_type* i_row()
  {
    return irowptr_;
  }
  inline index_type* j_col()
  {
    return jcolind_;
  }
  inline double* M()
  {
    return values_;
  }

  inline const index_type* i_row() const
  {
    return irowptr_;
  }
  inline const index_type* j_col() const
  {
    return jcolind_;
  }
  inline const double* M() const
  {
    return values_;
  }

#ifdef HIOP_DEEPCHECKS
  virtual bool assertSymmetry(double tol=1e-16) const
  {
    assert(false && "not yet implemented");
    return false;
  }
  virtual bool checkIndexesAreOrdered() const
  {
    return true;
  }
#endif

  
  //////////////////////////////////////////////////////////////////////
  // new methods that are needed by CSR
  /////////////////////////////////////////////////////////////////////
  // 1. should these be made available to the parent hiopMatrixSparse?
  // 2. 3-calls functionality?

  /**
   * @brief Extracts the diagonal entries of `this` matrix into the vector passed as argument
   *
   * @pre `this` matrix needs to be symmetric and of same size as `diag_out`
   */
  virtual void extract_diagonal(hiopVector& diag_out)
  {
    assert(false && "wip");
  }

  /// Notes:
  /// cusparseSpGEMM_workEstimation  cusparseSpGEMM_compute
  /// cusparseSpGEMM_copy
  /// cusparseSpGEMMreuse
  ///
  /// rocsparse_spgemm() 3 phases
  ///

  /**
   * Allocates a CSR matrix capable of storing the multiplication result of M = X*D*Y, where X 
   * is the calling matrix class (`this`) and D is a diagonal matrix specified by an input 
   * vector `diag`.
   *
   * @pre The dimensions of the matrices should be consistent with the multiplication.
   * 
   */
  hiopMatrixSparseCSR* times_diag_times_mat_alloc(const hiopVector& diag,
                                                 const hiopMatrixSparseCSR& Y)
  {
    assert(false && "wip");
  }  
  
  /**
   * Computes sparsity pattern and numerical values of M = X*D*Y, where X is the calling 
   * matrix class (`this`) and D is a diagonal matrix specified by an input vector `diag`.
   *
   * @note Both the sparsity pattern (row pointers and column indexes) and the nonzero values of
   * the input/output argument `M` are updated.
   *
   * @pre The dimensions of the matrices should be consistent with the multiplication.
   * @pre The internal arrays of `M` should have enough storage to hold the sparsity 
   * pattern (row pointers and column indexes) and values of the multiplication result. 
   * This preallocation can be done by calling `times_diag_times_mat_init` previously.
   * 
   */
  void times_diag_times_mat(const hiopVector& diag,
                            const hiopMatrixSparseCSR& Y,
                            hiopMatrixSparseCSR& M)
  {
    assert(false && "wip");
  }  

  /**
   * Computes numerical values of M = X*D*Y, where X is the calling matrix class (`this`) 
   * and D is a diagonal matrix specified by an input vector `diag`.
   *
   * @note Only the nonzero values of the input/output argument `M` are updated since
   * the method assumes the index arrays (row pointers and column indexes) of `M` already
   * contain the sparsity pattern of the multiplication result.
   *
   * @pre The dimensions of the matrices should be consistent with the multiplication.
   * @pre The internal index arrays (row pointers and column indexes) of `M` should match the 
   * sparsity pattern of the multiplication result. This can be obtained by calling the method
   * `time_diag_times_mat` previously.
   * @pre The indexes arrays of `this`, `Y`, and `M` should not have changed since the 
   * last call to `times_diag_times_mat`.
   * 
   * Example of usage:
   * //initially allocate and compute M
   * auto* M = X.times_diag_times_mat_alloc(D, Y);
   * X.times_diag_times_mat(D, Y, M);
   * ... calculations ....
   * //if only nonzero entries in X, Y, and D have changed, call the fast multiplication routine
   * X.times_diag_times_numeric(D, Y, M);
   * 
   */
  void times_diag_times_mat_numeric(const hiopVector& diag,
                                    const hiopMatrixSparseCSR& Y,
                                    hiopMatrixSparseCSR& M)
  {
    assert(false && "wip");
  }  



  /**
   * Builds `this` as the CSR representation of the triplet matrix `M`.
   */

  //// note: cusparseXcoo2csr + on the device cudamemcpy
  void form_from(const hiopMatrixSparseTriplet& M)
  {
    assert(false && "wip");
  }
  //// note: only device cuda memcpy
  void form_from_numeric(const hiopMatrixSparseTriplet& M)
  {
    assert(false && "wip");
  }

  
  /**
   * Builds `this` as the CSR representation of the triplet matrix `M`.
   */
  //// note: cusparseCsr2cscEx2
  void form_transpose_from(const hiopMatrixSparseTriplet& M)
  {
    assert(false && "wip");
  }
  void form_transpose_from_numeric(const hiopMatrixSparseTriplet& M)
  {
    assert(false && "wip");
  }

  /**
   * Allocates and returns CSR matrix `M` capable of holding M = alpha*X+*beta*Y, 
   * where X is the calling matrix class (`this`) and Y is the argument passed 
   * to the method.
   */
  hiopMatrixSparseCSR* add_matrix_alloc(const hiopMatrixSparseCSR& Y)
  {
    assert(false && "wip");
  }

  /**
   * Performs matrix addition M = alpha*X+*beta*Y both symbolically (i.e., updates the
   * row pointers and column indexes arrays) and numerically.
   * 
   * 
   *
   */

  //// note cusparseXcsrgeam2
  void add_matrix(const double& alpha,
                  const hiopMatrixSparseCSR& Y,
                  const double& beta,
                  hiopMatrixSparseCSR& M)
  {
    assert(false && "wip");
  }

    /**
   * Performs matrix addition M = alpha*X+*beta*Y numerically
   * 
   * 
   *
   */
  void add_matrix_numeric(const double& alpha,
                          const hiopMatrixSparseCSR& Y,
                          const double& beta,
                          hiopMatrixSparseCSR& M)
  {
    assert(false && "wip");
  }

  /////////////////////////////////////////////////////////////////////
  // end of new CSR-specific methods
  /////////////////////////////////////////////////////////////////////
protected:

  //// inherits nrows_, ncols_, and nnz_ from parent hiopSparseMatrix
  
  /// Row pointers (starting indexes) in the column and values arrays
  index_type* irowptr_;

  /// Column indexes
  index_type* jcolind_;

  /// Nonzero values
  double* values_; 

private:
  hiopMatrixSparseCSR()
    : hiopMatrixSparse(0, 0, 0),
      irowptr_(nullptr),
      jcolind_(nullptr),
      values_(nullptr)
  {
  }
  hiopMatrixSparseCSR(const hiopMatrixSparseCSR&)
    : hiopMatrixSparse(0, 0, 0),
      irowptr_(nullptr),
      jcolind_(nullptr),
      values_(nullptr)
  {
    assert(false);
  }
};


} //end of namespace

#endif