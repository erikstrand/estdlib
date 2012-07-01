//==============================================================================
// LUDecomposer.hpp
// created 7/10/11
//==============================================================================

#ifndef ESTDLIB_LU_DECOMPOSER
#define ESTDLIB_LU_DECOMPOSER

#include "Matrix.hpp"

//==============================================================================
/* Questions:
 * Does decompose return true/false for having a decomposition, or for being singular?
 *
 * Implementation Notes:
 * LUDecomposer is derived from matrix because it performs the decomposition in place.
 * Thus if you want to decompose an existing matrix you will have to copy it.
 * Rvalue copies will help reduce this overhead (not yet implemented), but be aware.
 * 
 * To solve a system of linear equations, feed LUDecomposer a matrix with more
 * columns than rows, where the extra columns contain the desired answers.
 * This must be done in advance because LUDecomposer permutes the rows during decomposition.
 */
//==============================================================================


//==============================================================================
// class LUDecomposer
//==============================================================================

template <class T>
class LUDecomposer : public Matrix<T> {
private:
   unsigned* _rowSwap;
   int _parity;         // used for determinant
   
public:
   LUDecomposer (Matrix<T> const& m);
   // performs the in place LU Decomposition
   bool decompose ();
   // copies out the L and U matrices
   void separate (Matrix<T>& l, Matrix<T>& u);
   // copies out the nth answer, or after solve has been run, nth solution
   void separate (Matrix<T>& solution, unsigned n);
   T determinant ();
   // performs in place solution where answers are in the (rows+n)th column
   void solve (unsigned n);   // solve (*this)*x = y, where y is column n
};


//==============================================================================
// Member Function Definitions
//==============================================================================

//------------------------------------------------------------------------------
template <class T>
LUDecomposer<T>::LUDecomposer (Matrix<T> const& m)
: Matrix<T>(m), _rowSwap(0), _parity(0) {}

//------------------------------------------------------------------------------
/* Returns true upon success, false upon failure.
 * Note on implementation: for very large matrices it would be more efficient
 * to change the order of the main for loops. See Numerical Recipes in C.
 * After running both the L and U matrices are stored in the LUDecomposer,
 * as it is understood that L has only ones on the diagonal.
 */
template <class T>
bool LUDecomposer<T>::decompose () {
   if (this->rows() > this->cols()) {
      return false;
   }
   
   // reset _rowSwap, in case decompose() is called twice
   delete[] _rowSwap;
   _rowSwap = new unsigned[Matrix<T>::rows()];
   _parity = 1;
   
   unsigned pivotRow;
   T maxAbs;
   T temp;
   
   // loop over columns
   for (unsigned c=0; c<Matrix<T>::rows(); ++c) {
      // assign upper entries
      for (unsigned r=0; r<=c; ++r) {
         for (unsigned k=0; k<r; ++k) {
            Matrix<T>::entry(r,c) -= Matrix<T>::entry(r,k) * Matrix<T>::entry(k,c);
         }
      }
      
      // assign lower entries, and find best pivot
      pivotRow = c;
      maxAbs = (Matrix<T>::entry(c,c) < 0) ? (-1)*Matrix<T>::entry(c,c) : Matrix<T>::entry(c,c);
      for (unsigned r=c+1; r<Matrix<T>::rows(); ++r) {
         for (unsigned k=0; k<c; ++k) {
            Matrix<T>::entry(r,c) -= Matrix<T>::entry(r,k) * Matrix<T>::entry(k,c);
         }
         temp = Matrix<T>::entry(r,c);
         if (temp < 0) temp *= -1;
         if (temp > maxAbs) {
            maxAbs = temp;
            pivotRow = r;
         }
      }
      
      if (maxAbs == 0) {
         // If we are already at the last row, then zero does not matter.
         // It means that the matrix was singular, but we still have an LU Decomposition.
         if (c+1 < Matrix<T>::rows()) {
            delete[] _rowSwap;
            _rowSwap = 0;
            _parity = 1;
         }
         return false;
      }
      
      // swap rows if necessary
      if (pivotRow != c) {
         Matrix<T>::swapRows(c,pivotRow);
         _parity *= -1;
      }
      _rowSwap[c] = pivotRow;
      
      temp = 1;
      temp /= Matrix<T>::entry(c,c);
      for (unsigned r=c+1; r<Matrix<T>::rows(); ++r) {
         Matrix<T>::entry(r,c) *= temp;
      }
   }
   return true;
}

//------------------------------------------------------------------------------
template <class T>
void LUDecomposer<T>::separate (Matrix<T>& l, Matrix<T>& u) {
   // todo: should check sizes of l and u
   
   l.zero();
   u.zero();
   
   unsigned copyLength;
   
   // copy l
   copyLength = sizeof(T);
   for (unsigned r=1; r<Matrix<T>::rows(); ++r) {
      memcpy(l[r], (*this)[r], copyLength);
      copyLength += sizeof(T);
   }
   for (unsigned i=0; i<Matrix<T>::rows(); ++i) {
      l[i][i] = 1;
   }
   
   // copy u
   copyLength = sizeof(T) * Matrix<T>::rows();
   for (unsigned r=0; r<Matrix<T>::rows(); ++r) {
      memcpy(&u[r][r], &Matrix<T>::entry(r,r), copyLength);
      copyLength -= sizeof(T);
   }
}

//------------------------------------------------------------------------------
template <class T>
void LUDecomposer<T>::separate (Matrix<T>& solution, unsigned n) {
   // todo: should check size of m
   for (unsigned r=0; r < Matrix<T>::rows(); ++r) {
      solution[r][0] = this->entry(r,n);
   }
}

//------------------------------------------------------------------------------
/* Returns 0 even if you haven't run decompose yet. */
template <class T>
T LUDecomposer<T>::determinant () {
   if (_rowSwap) {
      T det = _parity;
      for (unsigned i=0; i<this->rows(); ++i) {
         det *= (*this)[i][i];
      }
      return det;
   }
   return 0;
}

//------------------------------------------------------------------------------
/* Returns true if it finds a solution, false otherwise.
 * However, you should already know if there is no solution, because
 * your matrix is singular and decompose() returned false.
 */
template <class T>
bool LUDecomposer<T>::solve (unsigned n) {
   // solve L * b = y
   if (!_rowSwap) return false;
   for (unsigned r=1; r<Matrix<T>::rows(); ++r) {
      for (unsigned c=0; c<r; ++c) {
         this->entry(r,n) -= this->entry(r,c) * this->entry(c,n);
      }
   }
   
   // solve U * x = b
   for (int r=Matrix<T>::rows()-1; r>=0; --r) {
      for (unsigned c=Matrix<T>::rows()-1; c>r; --c) {
         this->entry(r,n) -= this->entry(r,c) * this->entry(c,n);
      }
      this->entry(r,n) /= this->entry(r,r);
   }
   return true;
}

#endif // ESTDLIB_LU_DECOMPOSER
