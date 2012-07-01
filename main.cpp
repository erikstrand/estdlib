#include <iostream>
#include <iomanip>
#include "Random.h"
#include "SMatrix.hpp"
#include "Cholesky.hpp"

using namespace std;

template <>
const float SMatrixBase<float>::_zero = 0.0;

class Histogram {
private:
   double _low;
   double _delta;
   unsigned _bins;
   unsigned* _bin;

public:
   Histogram (double low, double delta, unsigned bins): _low(low), _delta(delta), _bins(bins) {
      _bin = new unsigned[_bins+2];
   }
   ~Histogram () {
      delete[] _bin;
   }
   void add (double value) {
      int bin = static_cast<unsigned> ((value - _low) / _delta);
      if (bin < 0) {
//         cout << value << " is less than " << low(0) << '\n';
         ++_bin[_bins];
         return;
      }
      if (bin >= _bins) {
//         cout << value << " is greater than " << low(_bins) << '\n';
         ++_bin[_bins+1];
         return;
      }
//      cout << value << " is between " << low(bin) << " and " << high(bin) << '\n';
      ++_bin[bin];
   }
   unsigned underflow () const { return _bin[_bins]; }
   unsigned overflow () const { return _bin[_bins+1]; }
   double low  (unsigned bin) const { return _low + bin*_delta; }
   double high (unsigned bin) const { return _low + (bin+1)*_delta; }
   void print () const {
      cout << left << setw(6) << "-1" << underflow() << '\n';
      for (unsigned i=0; i<_bins; ++i) {
         cout << left << setw(6) << i << '(' << setw(5) << low(i) << ", " << setw(5) << high(i) << ")" << right << setw(6) << _bin[i] << '\n';
      }
      cout << left << setw(6) << _bins << overflow() << '\n';
   }
};


int main (int argc, char * const argv[]) {
   
   checkSizes();
   
//   XorShift32 rand(245452, 832652);
//   NormalGen normGen(&rand, 0.0, 3.0);
//   Histogram hist(-10.0, 1.0, 20);
//
//   for (unsigned i=0; i<100000; ++i) {
//      hist.add(normGen.next());
//   }
//   
//   hist.print();
   
   Matrix<float> m(3,3);
   m.entry(0, 0) = 1.2;
   m.entry(1, 2) = -1;

   LMatrix<float> l(3);
   l.entry(0, 0) = 1;
   l.entry(1, 1) = 1;
   l.entry(2, 2) = 2;
   l.entry(1, 0) = 0.32;
   l.entry(2, 0) = 3.3;
   
   SMatrix<float> s = l.square();

   cout << m;
   cout << l;
   cout << s;
   
   cout << l + m;
   cout << s + m;

//   Cholesky<float> cholesky(s);
//   if (cholesky.decompose()) {
//      cout << cholesky.l;
//      cout << cholesky.l.square();
//   } else {
//      cout << "Failure!\n";
//   }
   

   
   
   return 0;
}
