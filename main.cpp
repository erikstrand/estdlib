#include <iostream>
#include <iomanip>
#include "MemoryPoolF.h"
#include "HashSet.hpp"

using namespace std;


struct HUnsigned {
   unsigned _n;
   HUnsigned (unsigned n): _n(n) {}
   unsigned hash () const { return _n; }
   bool operator== (HUnsigned hu) const { return _n == hu._n; }
};


int main (int argc, char * const argv[]) {


   HashSet<HUnsigned, MemoryPoolF> set(4);

   cout << "Adding...\n";
   set.add(3);
   set.add(4);
   set.add(7);
   if (set.find(HUnsigned(2))) { cout << "found 2\n"; }
   if (set.find(HUnsigned(3))) { cout << "found 3\n"; }
   if (set.find(HUnsigned(4))) { cout << "found 4\n"; }
   if (set.find(HUnsigned(5))) { cout << "found 5\n"; }
   if (set.find(HUnsigned(7))) { cout << "found 7\n"; }
   cout << '\n';

   cout << "Removing...\n";
   set.remove(HUnsigned(7));
   if (set.find(HUnsigned(2))) { cout << "found 2\n"; }
   if (set.find(HUnsigned(3))) { cout << "found 3\n"; }
   if (set.find(HUnsigned(4))) { cout << "found 4\n"; }
   if (set.find(HUnsigned(5))) { cout << "found 5\n"; }
   if (set.find(HUnsigned(7))) { cout << "found 7\n"; }
   cout << '\n';






   // tests MemPoolF
   /*
   MemoryPoolF pool;
   pool.setItemSize(sizeof(unsigned));
   pool.setNextBlockSize(16);
   unsigned n = 32;
   unsigned* ptr[n];
   for (unsigned i=0; i<n; ++i) {
      ptr[i] = static_cast<unsigned*> (pool.alloc());
      *ptr[i] = i;
   }
   cout << "Done adding things!\n";
   pool.print();

   unsigned delIndex = 28;
   cout << ptr[delIndex] << ": " << *ptr[delIndex] << '\n';
   pool.free(ptr[delIndex]);
   ptr[delIndex] = static_cast<unsigned*> (pool.alloc());

   for (unsigned i=0; i<n; ++i) {
      cout << ptr[i] << ": " << *ptr[i] << '\n';
   }
   
   for (unsigned i=0; i<5; ++i) {
      pool.free(ptr[i]);
   }
   cout << pool.alloc() << '\n';

   pool.print();
   */
   
   
   return 0;
}
