//==============================================================================
// SMatrix.hpp
// Created 4/28/12.
//==============================================================================

//==============================================================================
/*
 * Defines classes for representing symmetric, Hermtian, lower triangular,
 * and upper triangular matrices.
 * Should add a skew-symmetric matrix class when needed.
 *
 * All types are derived from SMatrixBase, which stores only the elements on
 * the diagonal and below.
 */
//==============================================================================

#ifndef ESTDLIB_SMATRIX
#define ESTDLIB_SMATRIX

#include <iostream>
#include <iomanip>
#include "Matrix.hpp"


//==============================================================================
// Base Classes and Template Functions
//==============================================================================

//------------------------------------------------------------------------------
// Base class from which all matrix classes defined in this file are derived.
// It only stores the lower triangular portion of a square matrix.
template <class T>
class SMatrixBase {
private:
   T* _data;               // matrix data, holds diagonal and below
   unsigned _d;            // number of rows and columns
   unsigned _maxD;         // allocated space

public:
   static const T _zero;   // used when a const& to zero is needed
   
public:
   // Assignment Iterator
   class AItr {
   private:
      T* _ptr, _end;
      unsigned _r, _c;
   public:
      AItr (SMatrixBase<T>& m): _ptr(m._data), _end(_ptr + entries(m.dim())), _r(0), _c(0) {}
      AItr& operator++ () {
         ++_ptr;
         if (_c == _r) { _c = 0; ++_r; } else { ++_c; }
         return *this;
      }
      bool valid () const { return _ptr < _end; }
      T& ref () { return *_ptr; }
      unsigned r () const { return _r; }
      unsigned c () const { return _c; }
   };

public:
   SMatrixBase (unsigned d);
   SMatrixBase (SMatrixBase<T> const& m);
   ~SMatrixBase ();
   
   T const& entry (unsigned r, unsigned c) const { return _data[entries(r)+c]; }
   T&       entry (unsigned r, unsigned c)       { return _data[entries(r)+c]; }
   
   unsigned dim () const { return _d; }
   unsigned rows () const { return _d; }
   unsigned cols () const { return _d; }
   unsigned maxRows () const { return _maxD; }
   unsigned maxCols () const { return _maxD; }
   
   void zero () { memset(_data, 0, sizeof(T)*entries(_d)); }
   
   SMatrixBase& operator= (SMatrixBase<T> const& m);
   
private:
   // Returns the number of free entries in an r*r symmetric matrix
   // (ie number of entries on diagonal or below),
   // or equivalently, the index of the first entry in column r.
   // (This value is sum_{i=1}^r i == r(r+1)/2.)
   inline unsigned entries (unsigned r) const { return (r*(r+1))>>1; }
};

//------------------------------------------------------------------------------
template <class M>
std::ostream& print (std::ostream& os, M const& m);

//------------------------------------------------------------------------------
template <class M0, class M1, class M2>
M0 multiply (M1 const& m1, M2 const& m2);

//------------------------------------------------------------------------------
template <class M0, class M1, class M2>
M0 add (M1 const& m1, M2 const& m2);


//==============================================================================
// SMatrixBase<T> and MatrixPrint<M> Method Definitions
//==============================================================================

//------------------------------------------------------------------------------
template <class T>
SMatrixBase<T>::SMatrixBase (unsigned d): _d(d), _maxD(d) {
   _data = new T[entries(_d)];
}

//------------------------------------------------------------------------------
template <class T>
SMatrixBase<T>::SMatrixBase (SMatrixBase<T> const& m): _d(m.dim()), _maxD(m.dim()) {
   _data = new T[entries(_d)];
   memcpy(_data, m._data, sizeof(T)*entries(_d));
}

//------------------------------------------------------------------------------
template <class T>
SMatrixBase<T>::~SMatrixBase () {
   delete[] _data;
}

//------------------------------------------------------------------------------
template <class T>
SMatrixBase<T>& SMatrixBase<T>::operator= (SMatrixBase<T> const& m) {
   if (_maxD < m.dim()) {
      delete[] _data;
      _maxD = m.dim();
      _data = new T[entries(_d)];
   }
   _d = m.dim();
   memcpy(_data, m._data, sizeof(T)*entries(_d));
}

//------------------------------------------------------------------------------
template <class M>
std::ostream& print (std::ostream& os, M const& m) {
   os << std::right;
   for (unsigned r=0; r<m.dim(); ++r) {
      for (unsigned c=0; c<m.dim(); ++c) {
         os << std::setw(10) << m.entry(r,c);
      }
      os << '\n';
   }
   os << '\n';
   return os;
}


//==============================================================================
// Derived Class Definitions (SMatrix, LMatrix, UMatrix)
//==============================================================================

//------------------------------------------------------------------------------
// A general symmetric matrix.
template <class T>
class SMatrix: public SMatrixBase<T> {
public:
   class AItr : public SMatrixBase<T>::AItr {
      AItr (SMatrix<T>& m): SMatrixBase<T>::AItr(m) {}
   };

   SMatrix (unsigned d): SMatrixBase<T>(d) {}
   SMatrix (SMatrix<T> const& m): SMatrixBase<T>(m) {}
   SMatrix& operator= (SMatrix<T> const& m) { SMatrixBase<T>::operator=(m); }
   
   T const& entry (unsigned r, unsigned c) const {
      return (r >= c)? SMatrixBase<T>::entry(r, c) : SMatrixBase<T>::entry(c, r);
   }
   T& entry (unsigned r, unsigned c) {
      return (r >= c)? SMatrixBase<T>::entry(r, c) : SMatrixBase<T>::entry(c, r);
   }
};

//------------------------------------------------------------------------------
// A general lower triangular matrix.
template <class T>
class LMatrix : public SMatrixBase<T> {
public:
   class AItr : public SMatrixBase<T>::AItr {
      AItr (LMatrix<T>& m): SMatrixBase<T>::AItr(m) {}
   };
   
   LMatrix (unsigned d): SMatrixBase<T>(d) {}
   LMatrix (LMatrix<T> const& m): SMatrixBase<T>(m) {}
   LMatrix (SMatrixBase<T> const& b): SMatrixBase<T>(b) {}
   LMatrix& operator= (LMatrix<T> const& m) { SMatrixBase<T>::operator=(m); }

   T const& entry (unsigned r, unsigned c) const {
      return (r >= c)? SMatrixBase<T>::entry(r, c) : SMatrixBase<T>::_zero;
   }
   T& entry (unsigned r, unsigned c) {
      if (r < c) {
         std::cout << "Cannot return a reference to an element "
                   << "in the upper half of a lower triangular matrix!\n";
         exit(1);
      }
      return SMatrixBase<T>::entry(r, c);
   }
   
   SMatrix<T> square () const;
};

template <typename T>
SMatrix<T> LMatrix<T>::square () const {
   SMatrix<T> s(this->dim());
   for (unsigned i=0; i<this->dim(); ++i) {
      for (unsigned j=0; j<=i; ++j) {
         s.entry(i,j) = SMatrixBase<T>::_zero;
         for (unsigned k=0; k<=j; ++k) {
            s.entry(i,j) += this->entry(i,k) * this->entry(j,k);
         }
      }
   }
   return s;
}

//------------------------------------------------------------------------------
// A general upper triangular matrix.
template <class T>
class UMatrix : public SMatrixBase<T> {
public:
   class AItr : public SMatrixBase<T>::AItr {
      AItr (UMatrix<T>& m): SMatrixBase<T>::AItr(m) {}
      unsigned r () const { return SMatrixBase<T>::AItr::c(); }
      unsigned c () const { return SMatrixBase<T>::AItr::r(); }
   };
   
   UMatrix (unsigned d): SMatrixBase<T>(d) {}
   UMatrix (UMatrix<T> const& m): SMatrixBase<T>(m) {}
   UMatrix (SMatrixBase<T> const& b): SMatrixBase<T>(b) {}
   UMatrix& operator= (UMatrix<T> const& m) { SMatrixBase<T>::operator=(m); }
   
   T const& entry (unsigned r, unsigned c) const {
      return (r <= c)? SMatrixBase<T>::entry(c, r) : SMatrixBase<T>::_zero;
   }
   T& entry (unsigned r, unsigned c) {
      if (r > c) {
         std::cout << "Cannot return a reference to an element "
                  << "in the lower half of an upper triangular matrix!\n";
         exit(1);
      }
      return SMatrixBase<T>::entry(c, r);
   }   
};

//==============================================================================
// Addition Definitions
//==============================================================================

//------------------------------------------------------------------------------
template <class M0, class M1, class M2>
M0 add (M1 const& m1, M2 const& m2) {
   M0 result(m1.rows(), m1.cols());
   for (typename M0::AItr itr(result); itr.valid(); ++itr) {
      itr.ref() = m1.entry(itr.r(), itr.c()) + m2.entry(itr.r(), itr.c());
   }
   return result;
}


//==============================================================================
// Multiplcation Definitions
//==============================================================================

//------------------------------------------------------------------------------
// Default rule for multiplying all types of matrices.
// Works in all cases but often performs some unnecessary work.
template <class M0, class M1, class M2>
M0 multiply (M1 const& m1, M2 const& m2) {
   M0 result(m1.rows(), m2.cols());
   for (unsigned r=0; r<result.rows(); ++r) {
      for (unsigned c=0; c<result.cols(); ++c) {
         result.entry(r,c) = 0;
         for (unsigned k=0; k<m1.cols(); ++k) {
            result.entry(r,c) += m1.entry(r,k) * m2.entry(k,c);
         }
      }
   }
   return result;
}

//------------------------------------------------------------------------------
// Specialization for LMatrix * Matrix
template <class T>
Matrix<T> multiply (LMatrix<T> const& m1, Matrix<T> const& m2) {
   Matrix<T> result(m1.rows(), m2.cols());
   for (unsigned r=0; r<result.rows(); ++r) {
      for (unsigned c=0; c<result.cols(); ++c) {
         result.entry(r,c) = SMatrixBase<T>::_zero;
         for (unsigned k=0; k<=r; ++k) {
            result.entry(r,c) += m1.entry(r,k) * m2.entry(k,c);
         }
      }
   }
   return result;
}

//------------------------------------------------------------------------------
// Specialization for Matrix * LMatrix
template <class T>
Matrix<T> multiply (Matrix<T> const& m1, LMatrix<T> const& m2) {
   Matrix<T> result(m1.rows(), m2.cols());
   for (unsigned r=0; r<result.rows(); ++r) {
      for (unsigned c=0; c<result.cols(); ++c) {
         result.entry(r,c) = SMatrixBase<T>::_zero;
         for (unsigned k=c; k<m1.cols(); ++k) {
            result.entry(r,c) += m1.entry(r,k) * m2.entry(k,c);
         }
      }
   }
   return result;
}

//------------------------------------------------------------------------------
// Specialization for UMatrix * Matrix
template <class T>
Matrix<T> multiply (UMatrix<T> const& m1, Matrix<T> const& m2) {
   Matrix<T> result(m1.rows(), m2.cols());
   for (unsigned r=0; r<result.rows(); ++r) {
      for (unsigned c=0; c<result.cols(); ++c) {
         result.entry(r,c) = SMatrixBase<T>::_zero;
         for (unsigned k=r; k<m1.cols(); ++k) {
            result.entry(r,c) += m1.entry(r,k) * m2.entry(k,c);
         }
      }
   }
   return result;
}

//------------------------------------------------------------------------------
// Specialization for Matrix * UMatrix
template <class T>
Matrix<T> multiply (Matrix<T> const& m1, UMatrix<T> const& m2) {
   Matrix<T> result(m1.rows(), m2.cols());
   for (unsigned r=0; r<result.rows(); ++r) {
      for (unsigned c=0; c<result.cols(); ++c) {
         result.entry(r,c) = SMatrixBase<T>::_zero;
         for (unsigned k=0; k<=c; ++k) {
            result.entry(r,c) += m1.entry(r,k) * m2.entry(k,c);
         }
      }
   }
   return result;
}

//------------------------------------------------------------------------------
// Specialization for LMatrix * LMatrix
template <class T>
LMatrix<T> multiply (LMatrix<T> const& m1, LMatrix<T> const& m2) {
   LMatrix<T> result(m1.dim());
   for (unsigned r=0; r<result.rows(); ++r) {
      for (unsigned c=0; c<=r; ++c) {
         result.entry(r,c) = SMatrixBase<T>::_zero;
         for (unsigned k=c; k<=r; ++k) {
            result.entry(r,c) += m1.entry(r,k) * m2.entry(k,c);
         }
      }
   }
   return result;
}

//------------------------------------------------------------------------------
// Specialization for UMatrix * UMatrix
template <class T>
UMatrix<T> multiply (UMatrix<T> const& m1, UMatrix<T> const& m2) {
   UMatrix<T> result(m1.dim());
   for (unsigned r=0; r<result.rows(); ++r) {
      for (unsigned c=r; c<result.cols(); ++c) {
         result.entry(r,c) = SMatrixBase<T>::_zero;
         for (unsigned k=r; k<=c; ++k) {
            result.entry(r,c) += m1.entry(r,k) * m2.entry(k,c);
         }
      }
   }
   return result;
}


//==============================================================================
// Addition Operators
//==============================================================================
// Note: If + is commutative for T, then m2 + m1 can be inlined as m1 + m2;
// ie there is no need to instantiate two different template instances
// for each pair.
// In the interest of generality both versions are defined here;
// if executable size is important half of these can be changed
// (or agree on a standard order for each pair in the calling code).

//------------------------------------------------------------------------------
// regular + special
template <class T> Matrix<T> operator+ (Matrix<T> const& m1, SMatrix<T> const& m2) {
   return add< Matrix<T>, Matrix<T>, SMatrix<T> >(m1, m2);
}
template <class T> Matrix<T> operator+ (Matrix<T> const& m1, LMatrix<T> const& m2) {
   return add< Matrix<T>, Matrix<T>, LMatrix<T> >(m1, m2);
}
template <class T> Matrix<T> operator+ (Matrix<T> const& m1, UMatrix<T> const& m2) {
   return add< Matrix<T>, Matrix<T>, UMatrix<T> >(m1, m2);
}

//------------------------------------------------------------------------------
// special + regular
template <class T> Matrix<T> operator+ (SMatrix<T> const& m1, Matrix<T> const& m2) {
   return add< Matrix<T>, SMatrix<T>, Matrix<T> >(m1, m2);
}
template <class T> Matrix<T> operator+ (LMatrix<T> const& m1, Matrix<T> const& m2) {
   return add< Matrix<T>, LMatrix<T>, Matrix<T> >(m1, m2);
}
template <class T> Matrix<T> operator+ (UMatrix<T> const& m1, Matrix<T> const& m2) {
   return add< Matrix<T>, UMatrix<T>, Matrix<T> >(m1, m2);
}

//------------------------------------------------------------------------------
// SMatrix + special
template <class T> SMatrix<T> operator+ (SMatrix<T> const& m1, SMatrix<T> const& m2) {
   return add< SMatrix<T>, SMatrix<T>, SMatrix<T> >(m1, m2);
}
template <class T> Matrix<T> operator+ (SMatrix<T> const& m1, LMatrix<T> const& m2) {
   return add< Matrix<T>, SMatrix<T>, LMatrix<T> >(m1, m2);
}
template <class T> Matrix<T> operator+ (SMatrix<T> const& m1, UMatrix<T> const& m2) {
   return add< Matrix<T>, SMatrix<T>, UMatrix<T> >(m1, m2);
}

//------------------------------------------------------------------------------
// LMatrix + special
template <class T> Matrix<T> operator+ (LMatrix<T> const& m1, SMatrix<T> const& m2) {
   return add< Matrix<T>, LMatrix<T>, SMatrix<T> >(m1, m2);
}
template <class T> LMatrix<T> operator+ (LMatrix<T> const& m1, LMatrix<T> const& m2) {
   return add< LMatrix<T>, LMatrix<T>, LMatrix<T> >(m1, m2);
}
template <class T> Matrix<T> operator+ (LMatrix<T> const& m1, UMatrix<T> const& m2) {
   return add< Matrix<T>, LMatrix<T>, UMatrix<T> >(m1, m2);
}

//------------------------------------------------------------------------------
// UMatrix + special
template <class T> Matrix<T> operator+ (UMatrix<T> const& m1, SMatrix<T> const& m2) {
   return add< Matrix<T>, UMatrix<T>, SMatrix<T> >(m1, m2);
}
template <class T> Matrix<T> operator+ (UMatrix<T> const& m1, LMatrix<T> const& m2) {
   return add< Matrix<T>, UMatrix<T>, LMatrix<T> >(m1, m2);
}
template <class T> UMatrix<T> operator+ (UMatrix<T> const& m1, UMatrix<T> const& m2) {
   return add< UMatrix<T>, UMatrix<T>, UMatrix<T> >(m1, m2);
}


//==============================================================================
// Multiplication Operators
//==============================================================================

//------------------------------------------------------------------------------
// regular * special
template <class T> Matrix<T> operator* (Matrix<T> const& m1, SMatrix<T> const& m2) {
   return multiply(m1, m2);
}
template <class T> Matrix<T> operator* (Matrix<T> const& m1, LMatrix<T> const& m2) {
   return multiply(m1, m2);
}
template <class T> Matrix<T> operator* (Matrix<T> const& m1, UMatrix<T> const& m2) {
   return multiply(m1, m2);
}

//------------------------------------------------------------------------------
// special * regular
template <class T>
Matrix<T> operator* (SMatrix<T> const& m1, Matrix<T> const& m2) {
   return multiply(m1, m2);
}
template <class T>
Matrix<T> operator* (LMatrix<T> const& m1, Matrix<T> const& m2) {
   return multiply(m1, m2);
}
template <class T>
Matrix<T> operator* (UMatrix<T> const& m1, Matrix<T> const& m2) {
   return multiply(m1, m2);
}

//------------------------------------------------------------------------------
// SMatrix * special
template <class T> Matrix<T> operator* (SMatrix<T> const& m1, SMatrix<T> const& m2) {
   return multiply< Matrix<T>, SMatrix<T>, SMatrix<T> >(m1, m2);
}
template <class T> Matrix<T> operator* (SMatrix<T> const& m1, LMatrix<T> const& m2) {
   return multiply< Matrix<T>, SMatrix<T>, LMatrix<T> >(m1, m2);
}
template <class T> Matrix<T> operator* (SMatrix<T> const& m1, UMatrix<T> const& m2) {
   return multiply< Matrix<T>, SMatrix<T>, UMatrix<T> >(m1, m2);
}

//------------------------------------------------------------------------------
// LMatrix * special
template <class T> Matrix<T> operator* (LMatrix<T> const& m1, SMatrix<T> const& m2) {
   return multiply< Matrix<T>, LMatrix<T>, SMatrix<T> >(m1, m2);
}
template <class T> LMatrix<T> operator* (LMatrix<T> const& m1, LMatrix<T> const& m2) {
   return multiply< LMatrix<T>, LMatrix<T>, LMatrix<T> >(m1, m2);
}
template <class T> Matrix<T> operator* (LMatrix<T> const& m1, UMatrix<T> const& m2) {
   return multiply< Matrix<T>, LMatrix<T>, UMatrix<T> >(m1, m2);
}

//------------------------------------------------------------------------------
// UMatrix * special
template <class T> Matrix<T> operator* (UMatrix<T> const& m1, SMatrix<T> const& m2) {
   return multiply< Matrix<T>, UMatrix<T>, SMatrix<T> >(m1, m2);
}
template <class T> Matrix<T> operator* (UMatrix<T> const& m1, LMatrix<T> const& m2) {
   return multiply< Matrix<T>, UMatrix<T>, LMatrix<T> >(m1, m2);
}
template <class T> UMatrix<T> operator* (UMatrix<T> const& m1, UMatrix<T> const& m2) {
   return multiply< UMatrix<T>, UMatrix<T>, UMatrix<T> >(m1, m2);
}


//==============================================================================
// Insertion Operators
//==============================================================================

template <class T> std::ostream& operator<< (std::ostream& os, SMatrix<T> const& m) {
   return print< SMatrix<T> >(os, m);
}
template <class T> std::ostream& operator<< (std::ostream& os, LMatrix<T> const& m) {
   return print< LMatrix<T> >(os, m);
}
template <class T> std::ostream& operator<< (std::ostream& os, UMatrix<T> const& m) {
   return print< UMatrix<T> >(os, m);
}



#endif // ESTDLIB_SMATRIX
