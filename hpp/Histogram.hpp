//==============================================================================
// Histogram.hpp
// Created 12/5/18
//==============================================================================

#ifndef ESTDLIB_HISTOGRAM
#define ESTDLIB_HISTOGRAM

#include <iostream>
#include <iomanip>


//==============================================================================
// Class Histogram
//==============================================================================

//------------------------------------------------------------------------------
template <class T>
class Histogram {
private:
   T _low;
   T _delta;
   T _min;
   T _max;
   unsigned _total;
   unsigned _bins;
   unsigned* _bin;

public:
   inline Histogram ();
   Histogram (T low, T delta, unsigned bins);
   ~Histogram () { delete[] _bin; }
   void add (T value);
   unsigned underflow () const { return _bin[_bins]; }
   unsigned overflow  () const { return _bin[_bins+1]; }
   T low  (unsigned bin) const { return _low + bin*_delta; }
   T high (unsigned bin) const { return _low + (bin+1)*_delta; }
   T lowest  () const { return low(0); }
   T highest () const { return high(_bins-1); }
   unsigned size () const { return _total; }
   T min () const { return _min; }
   T max () const { return _max; }
   void print () const;
};


//==============================================================================
// Member Function Definitions
//==============================================================================

//------------------------------------------------------------------------------
template <typename T>
Histogram<T>::Histogram (): _low(0), _delta(0), _min(0), _max(0), _total(0), _bins(0) {}

//------------------------------------------------------------------------------
template <typename T>
Histogram<T>::Histogram (T low, T delta, unsigned bins)
: _low(low), _delta(delta), _min(0), _max(0), _total(0), _bins(bins)
{
   _bin = new unsigned[_bins+2];
}

//------------------------------------------------------------------------------
template <typename T>
void Histogram<T>::add (T value) {
   // see if its the lowest or highest value
   if (size() == 0) _min = _max = value;
   else if (value < _min) _min = value;
   else if (value > _max) _max = value;

   // increment total
   ++_total;

   // increment the right bin
   int bini = static_cast<unsigned> ((value - _low) / _delta);
   if (bini < 0) {
      ++_bin[_bins];
      return;
   }
   unsigned bin = static_cast<unsigned>(bini);
   if (bin >= _bins) {
      ++_bin[_bins+1];
      return;
   }
   ++_bin[bin];
}

//------------------------------------------------------------------------------
template <typename T>
void Histogram<T>::print () const {
   std::cout << "Total: " << size() << ", " << "Min: " << _min << ", " <<  "Max: " << _max << '\n';
   std::cout << std::left << std::setw(6) << "-1" << '(' << std::setw(5)
             << lowest() << "-)      " << std::right << std::setw(6) << underflow() << '\n';
   for (unsigned i=0; i<_bins; ++i) {
      std::cout << std::left << std::setw(6) << i << '(' << std::setw(5)
                << low(i) << ", " << std::setw(5) << high(i) << ")"
                << std::right << std::setw(6) << _bin[i] << '\n';
   }
   std::cout << std::left << std::setw(6) << _bins << '(' << std::setw(5)
             << highest() << "+)     " << std::right << std::setw(6) << overflow() << '\n';
}


#endif // ESTDLIB_HISTOGRAM
