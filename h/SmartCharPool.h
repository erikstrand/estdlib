/*
 *  SmartCharPool.h
 *  Created on 7/18/11.
 */

#ifndef ESS_SMART_CHAR_POOL
#define ESS_SMART_CHAR_POOL

#include "SimpleCharPool.h"

//==============================================================================
// SmartCharPool
//==============================================================================

/*
 * Just like a SimpleCharPool, but with 2 new features. First, a SmartCharPool
 * can return the nth string it contains (a SimpleCharPool can only return a
 * string when you give it the index of its first character). Second, it keeps
 * track of use statistics that SimpleCharPool does not.
 */

class SmartCharPool : public SimpleCharPool {
private:
	unsigned* index;		// indeces of the strings in the SmartCharPool
	unsigned maxIndeces;	// number of indeces we can fit in index array
	unsigned storedIndeces;	// number of C strings in the SmartCharPool
	
public:
	inline SmartCharPool (unsigned initialChars, unsigned initialStrings);
	inline ~SmartCharPool ();
	unsigned addString (char const* s);	// returns the number of the string you add
	void resizeIndexTable ();
	char const* getString (unsigned n) const { return (*this)[index[n]]; }
	
	unsigned strings () const { return storedIndeces; }
	// should add characters method
};


//------------------------------------------------------------------------------
// Inline Member Function Definitions
//------------------------------------------------------------------------------

SmartCharPool::SmartCharPool (unsigned initialChars, unsigned initialStrings)
: SimpleCharPool(initialChars), maxIndeces(initialStrings), storedIndeces(0)
{
	index = new unsigned[initialStrings];
}

SmartCharPool::~SmartCharPool () {
	delete[] index;
}


#endif // ESS_SMART_CHAR_POOL
