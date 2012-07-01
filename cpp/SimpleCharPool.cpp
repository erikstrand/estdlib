/*
 *  SimpleCharPool.cpp
 *  Created 7/18/11.
 */

#include "SimpleCharPool.h"
#include <cstring>

//------------------------------------------------------------------------------
// Member Function Definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
SimpleCharPool::SimpleCharPool (unsigned initialChars) {
	beginning = static_cast<char*> (malloc(initialChars+1));
	beginning[0] = 0;	// not really sure why I do this, but it seems nice
	write = beginning;
	end = beginning + initialChars;
}

//------------------------------------------------------------------------------
unsigned SimpleCharPool::alloc (unsigned size) {
	if (write + size > end) resize(size);
	unsigned index = write - beginning;
	write += size;
	return index;
}

//------------------------------------------------------------------------------
unsigned SimpleCharPool::addString (char const* s) { 
   unsigned length = strlen(s)+1;   // we want to count the null terminator
   unsigned index = alloc(length);
	memcpy(&beginning[index], s, length);
	return index;
}

//------------------------------------------------------------------------------
/*
 * Doubles the capacity of SimpleCharPool, or adds minChange additional characters
 * (whichever makes it larger).
 */
void SimpleCharPool::resize (unsigned minChange) {
	unsigned size = end - beginning;
	unsigned usedsize = write - beginning;
	unsigned newsize = size > minChange ? (size<<1) : size + minChange;
   
	char* newbeginning = static_cast<char*>(malloc(newsize));
	memcpy(newbeginning, beginning, usedsize);
	free(beginning);
   
	beginning = newbeginning;
	write = beginning + usedsize;
	end = beginning + newsize;
}
