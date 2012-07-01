//==============================================================================
// Cholesky.hpp
// Created 9/4/11.
//==============================================================================

#ifndef ESTDLIB_CHOLESKY
#define ESTDLIB_CHOLESKY

#include <cmath>
#include "SMatrix.hpp"


//==============================================================================
// Cholesky
//==============================================================================

template <typename T>
class Cholesky {
public:
   SMatrix<T> const* m;
   LMatrix<T> l;
   
   Cholesky (SMatrix<T> const& mm): m(&mm), l(m->dim()) {}
   bool decompose ();
};


//==============================================================================
// Member Function Definitions
//==============================================================================

template <typename T>
bool Cholesky<T>::decompose () {
   T sum;
   for (unsigned i=0; i<m->dim(); ++i) {
      for (unsigned j=0; j<i; ++j) {
         sum = SMatrixBase<T>::_zero;
         for (unsigned k=0; k<j; ++k) {
            sum += l.entry(i, k) * l.entry(j, k);
         }
         l.entry(i, j) = (m->entry(i, j) - sum) / l.entry(j, j);
      }
      
      // diagonal entry
      sum = SMatrixBase<T>::_zero;
      for (unsigned k=0; k<i; ++k) {
         sum += l.entry(i, k) * l.entry(i, k);
      }
      if ((l.entry(i, i) = m->entry(i, i) - sum) <= SMatrixBase<T>::_zero)
         return false;
      l.entry(i, i) = sqrt(l.entry(i, i));
   }
   return true;
}


//------------------------------------------------------------------------------

#endif // ESTDLIB_CHOLESKY
