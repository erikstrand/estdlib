//==============================================================================
// main.cpp
// created July 16 2012
//==============================================================================

/*
 * This code adds a million random numbers to a hash set, then searches for
 * a million random numbers.
 * Just for fun it records the number of lookups that succeed (this isn't
 * an effective test for uniform randomness).
 */


#include <iostream>
#include <iomanip>
#include <math.h>
#include "MemoryPoolF.h"
#include "HashSet.hpp"
#include "Random.h"

using namespace std;


//------------------------------------------------------------------------------
// extends unsigned with the methods required by HashSet
struct HUnsigned {
   unsigned _n;
   HUnsigned () {}
   HUnsigned (unsigned n): _n(n) {}
   unsigned hash () const { return _n; }
   bool operator== (HUnsigned hu) const { return _n == hu._n; }
};


//------------------------------------------------------------------------------
int main (int argc, char * const argv[]) {

   const unsigned n = 1000000;
   const unsigned m = 1000000;
   HashSet<HUnsigned, MemoryPoolF> set(n);
   XorShift32 rand(0xdefceedll);

   cout << "Adding " << n << " random numbers to the HashSet...\n";
   do {
      set.add(rand.u32());
   } while (set.size() <= n);

   cout << "Searching for " << m << " random numbers in the HashSet...\n";
   unsigned hits = 0;
   HUnsigned temp;
   for (unsigned i=0; i<m; ++i) {
      temp = rand.u32();
      if (set.find(temp)) {
         ++hits;
      }
   }

   cout << "Found " << hits << " hits.\n";
   cout << "(We expect " << (float)n * (float)m / pow(2, 32) << " hits if the random number generator is uniformly distributed).";
   cout << '\n';

   return 0;
}

