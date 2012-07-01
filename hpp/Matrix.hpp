//==============================================================================
// Matrix.hpp
// Created 7/10/11.
//==============================================================================

#ifndef ESTDLIB_MATRIX
#define ESTDLIB_MATRIX

#include <iostream>
#include <iomanip>
#include "Vector.hpp"


//==============================================================================
// Class Matrix<T>
//==============================================================================

template <class T>
class Matrix {
private:
   T** _row;            // pointers to rows in _data
   T* _data;            // all the entries in the matrix
   unsigned _rows;      // used rows
   unsigned _cols;      // used columns
   unsigned _maxRows;   // allocated rows
   unsigned _maxCols;   // allocated columns
   
public:
   // Assignment Iterator
   class AItr {
   private:
      Matrix<T>* _matrix;
      unsigned _r, _c;
   public:
      AItr (Matrix<T>& m): _matrix(&m), _r(0), _c(0) {}
      AItr& operator++ () {
         if (++_c == _matrix->cols()) { _c = 0; ++_r; }
         return *this;
      }
      bool valid () const { return _r < _matrix->rows(); }
      T& ref () { return _matrix->entry(_r, _c); }
      unsigned r () const { return _r; }
      unsigned c () const { return _c; }
   };
   
public:
   inline Matrix ();
   Matrix (unsigned rows, unsigned columns);
   Matrix (Matrix<T> const& m);
   ~Matrix ();
   void resize (unsigned rows, unsigned columns);
   
   T const* const operator[] (unsigned i) const { return _row[i]; }
   T*       const operator[] (unsigned i)       { return _row[i]; }
   T const* const row (unsigned i) const { return _row[i]; }
   T*       const row (unsigned i)       { return _row[i]; }
   T const& entry (unsigned i, unsigned j) const { return (*this)[i][j]; }
   T&       entry (unsigned i, unsigned j)       { return (*this)[i][j]; }
   
   unsigned rows () const { return _rows; }
   unsigned cols () const { return _cols; }
   unsigned maxRows () const { return _maxRows; }
   unsigned maxCols () const { return _maxCols; }
   
   // only use if memset is valid for T!
   void zero () { memset(_data, 0, sizeof(T)*_rows*_cols); }
   inline void swapRows (unsigned i, unsigned j);
   inline void downSize (unsigned r, unsigned c);
   
   void operator= (Matrix const& m);

   Matrix operator+ (Matrix const& rhm) const;
   Matrix operator* (Matrix const& rhm) const;
   
   Matrix& operator+= (Matrix const& rhm);
   Matrix& operator*= (Matrix const& rhm);
   
private:
   inline void initializeRowPointers ();
};

template <class T>
std::ostream& operator<< (std::ostream& os, Matrix<T> const& m);


//==============================================================================
// Inline Member Function Definitions
//==============================================================================

//------------------------------------------------------------------------------
template <class T>
Matrix<T>::Matrix ()
: _row(0), _data(0), _rows(0), _cols(0), _maxRows(0), _maxCols(0)
{}

//------------------------------------------------------------------------------
template <class T>
void Matrix<T>::swapRows(unsigned i, unsigned j) {
   T* temp = _row[i];
   _row[i] = _row[j];
   _row[j] = temp;
}

//------------------------------------------------------------------------------
template <class T>
void Matrix<T>::downSize (unsigned r, unsigned c) {
   _rows = r;
   _cols = c;
}


//==============================================================================
// Public Member Function Definitions
//==============================================================================

//------------------------------------------------------------------------------
template <class T>
Matrix<T>::Matrix (unsigned rows, unsigned columns)
: _rows(rows), _cols(columns)
{
   _row = new T*[_rows];
   _data = new T[_rows*_cols];
   initializeRowPointers();
}

//------------------------------------------------------------------------------
template <class T>
Matrix<T>::Matrix (Matrix<T> const& m)
: _rows(m.rows()), _cols(m.cols())
{
   _row = new T*[_rows];
   _data = new T[_rows*_cols];
   *this = m;   // initialization of row pointers is done in this assignment
}

//------------------------------------------------------------------------------
template <class T>
void Matrix<T>::resize (unsigned rows, unsigned columns) {
   _rows = rows;
   _cols = columns;
   unsigned oldMaxRows = _maxRows;
   unsigned oldMaxCols = _maxCols;
   unsigned mincols = (_cols < oldMaxCols)? _cols : oldMaxCols;
   unsigned minrows = (_rows < oldMaxRows)? _rows : oldMaxRows;

   T* olddata = _data;  // may not be modified
   T** oldrow = _row;
   bool dataChange = false;

   if (_rows*_cols > _maxRows*_maxCols) {
      dataChange = true;
      _data = new T[_rows*_cols];
      _maxRows = _rows;
      _maxCols = _cols;
   }

   if (dataChange or _rows > oldMaxRows) {
      _row = new T*[_rows];
      initializeRowPointers();

      if (olddata) {
         for (unsigned i=0; i<minrows; ++i) {
            memcpy(row(i), oldrow[i], mincols*sizeof(T));
         }
         delete[] oldrow;
         delete[] olddata;
      }
   }
}

//------------------------------------------------------------------------------
template <class T>
Matrix<T>::~Matrix () {
   delete[] _row;
   delete[] _data;
}

//------------------------------------------------------------------------------
template <class T>
void Matrix<T>::operator= (Matrix<T> const& m) {
   if (rows() != m.rows() or cols() != m.cols()) {
      delete[] _row;
      delete[] _data;
      _rows = m.rows();
      _cols = m.cols();
      _row = new T*[rows()];
      _data = new T[rows() * cols()];
   }
   initializeRowPointers();
   for (unsigned i=0; i<rows(); ++i) {
      memcpy(_row[i], m._row[i], sizeof(T)*cols());
   }
}

//------------------------------------------------------------------------------
template <class T>
Matrix<T> Matrix<T>::operator+ (Matrix<T> const& rhm) const {
   Matrix<T> result(rows(), rhm.cols());
   for (unsigned i=0; i<result.rows(); ++i) {
      for (unsigned j=0; j<result.cols(); ++j) {
         result.entry(i,j) = 0;
         for (unsigned k=0; k<cols(); ++k) {
            result.entry(i,j) += entry(i,k) * rhm.entry(k,j);
         }
      }
   }
   return result;
}

//------------------------------------------------------------------------------
template <class T>
Matrix<T> Matrix<T>::operator* (Matrix<T> const& rhm) const {
   Matrix<T> result(rows(), rhm.cols());
   for (unsigned i=0; i<result.rows(); ++i) {
      for (unsigned j=0; j<result.cols(); ++j) {
         result.entry(i,j) = 0;
         for (unsigned k=0; k<cols(); ++k) {
            result.entry(i,j) += entry(i,k) * rhm.entry(k,j);
         }
      }
   }
   return result;
}

//------------------------------------------------------------------------------
template <class T>
Matrix<T>& Matrix<T>::operator+= (Matrix<T> const& rhm) {
   for (unsigned i=0; i<rows(); ++i) {
      for (unsigned j=0; j<cols(); ++j) {
         entry(i, j) += rhm.entry(i, j);
      }
   }
   return *this;
}

//------------------------------------------------------------------------------
template <class T>
std::ostream& operator<< (std::ostream& os, Matrix<T> const& m) {
   os << std::right;
   for (unsigned i=0; i<m.rows(); ++i) {
      for (unsigned j=0; j<m.cols(); ++j) {
         os << std::setw(10) << m.entry(i,j);
      }
      os << '\n';
   }
   os << '\n';
   return os;
}


//==============================================================================
// Private Member Function Definitions
//==============================================================================

//------------------------------------------------------------------------------
template <class T>
void Matrix<T>::initializeRowPointers () {
   for (unsigned i=0; i<rows(); ++i) {
      _row[i] = & _data[i * _cols];
   }
}


#endif // ESTDLIB_MATRIX
