//==============================================================================
// SimpleCharPool.h
// Created 7/18/11.
//==============================================================================

#ifndef ESS_SIMPLE_CHAR_POOL
#define ESS_SIMPLE_CHAR_POOL

#include <cstdlib>


//==============================================================================
// SimpleCharPool
//==============================================================================

/*
 * Stores many C strings contiguously. (Each is null terminated.)
 * Hands out unsigned indeces into itself, that the user must keep track of.
 * Strings cannot be modified once they are stored (if you changed the length
 * you could write over the next string).
 */

class SimpleCharPool {
private:
	char* beginning;	// first character in the SimpleCharPool
	char* write;		// where next string will start
	char* end;			// one past the last character
   
public:
	SimpleCharPool (unsigned initialChars);
	inline ~SimpleCharPool ();
   unsigned alloc (unsigned size);
	unsigned addString (char const* s);
	void resize (unsigned minChange);
	char const* operator[] (unsigned index) const { return beginning + index; }
   // be very careful you don't overwrite things!
   char* edit (unsigned index) { return beginning + index; }
};


//------------------------------------------------------------------------------
// Inline Member Function Definitions
//------------------------------------------------------------------------------

SimpleCharPool::~SimpleCharPool () {
   free(beginning);
}

#endif // ESS_SIMPLE_CHAR_POOL
