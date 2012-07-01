/*
 *  SearchableCharPool.cpp
 *  Created 7/27/11.
 */

#include "SearchableCharPool.h"
#include <cstring>

//------------------------------------------------------------------------------
// Member Function Definitions
//------------------------------------------------------------------------------

// returns the index of the string, or -1 on failure
int SearchableCharPool::find (char const* string) const {
   for (unsigned i=0; i<strings(); ++i) {
      if (strcmp(string, getString(i)) == 0) {
         return i;
      }
   }
   return -1;
}
