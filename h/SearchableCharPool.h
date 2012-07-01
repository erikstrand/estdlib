//==============================================================================
// SearchableCharPool.h
// Created 7/27/11.
//==============================================================================

#ifndef SEARCHABLE_CHAR_POOL
#define SEARCHABLE_CHAR_POOL

#include "SmartCharPool.h"


//==============================================================================
// SearchableCharPool
//==============================================================================
/*
 * Warning: the search is linear, so tends to get horrendously slow.
 */
class SearchableCharPool : public SmartCharPool {
public:
   inline SearchableCharPool  (unsigned initialChars, unsigned initialStrings);
   int find (char const* string) const;
};


//------------------------------------------------------------------------------
// Inline Member Function Definitions
//------------------------------------------------------------------------------

SearchableCharPool::SearchableCharPool (unsigned initialChars, unsigned initialStrings)
: SmartCharPool(initialChars, initialStrings)
{}

#endif // SEARCHABLE_CHAR_POOL