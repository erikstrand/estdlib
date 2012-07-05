#include <iostream>
#include <iomanip>
#include "MemoryPoolF.h"

using namespace std;


int main (int argc, char * const argv[]) {
   
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

   /*
   unsigned delIndex = 28;
   cout << ptr[delIndex] << ": " << *ptr[delIndex] << '\n';
   pool.free(ptr[delIndex]);
   ptr[delIndex] = static_cast<unsigned*> (pool.alloc());
   */

   //*
   for (unsigned i=0; i<n; ++i) {
      cout << ptr[i] << ": " << *ptr[i] << '\n';
   }
   //*/
   
   for (unsigned i=0; i<5; ++i) {
      pool.free(ptr[i]);
   }
   cout << pool.alloc() << '\n';


   pool.print();
   
   
   return 0;
}
