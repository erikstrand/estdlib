/*
 *  Vector.cpp
 *  Created by Erik Strand on 7/10/11.
 */

#ifndef ESS_Vector
#define ESS_Vector

//==============================================================================
// Class Vector<T>
//==============================================================================

template <class T>
class Vector {
private:
   T* _data;
   unsigned _length;       // used length
   unsigned _maxLength;    // allocated length
   
public:
   inline explicit Vector (unsigned length = 0);
   
   T const& operator[] (unsigned n) const { return _data[n]; }
   T&       operator[] (unsigned n)       { return _data[n]; }
   unsigned length () const { return _length; }
   unsigned maxLength () const { return _maxLength; }
   void resize (unsigned n);
   
   void operator= (Vector const& v);
};

//------------------------------------------------------------------------------
// Member Function Definitions

template <class T>
Vector<T>::Vector (unsigned length)
: _data(length? new T[length] : 0), _length(length), _maxLength(length)
{}

template <class T>
void Vector<T>::resize (unsigned newMaxLength) {
   T* newData = new T[newMaxLength];
   memcpy(newData, _data, sizeof(T)*_length);
   _maxLength = newMaxLength;
}

template <class T>
void Vector<T>::operator= (Vector const& v)
{
   if (_maxLength < v.length()) {
      delete[] _data;
      _data = new T[v.length()];
      _maxLength = v.length();
   }
   _length = v.length();
   
   memcpy(_data, v._data, sizeof(T)*_length);
}

#endif // ESS_Vector
