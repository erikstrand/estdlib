/*
 *  SmartCharPool.cpp
 *  Created on 7/18/11.
 */

#include "SmartCharPool.h"
#include <cstring>

//------------------------------------------------------------------------------
// Member Function Definitions
//------------------------------------------------------------------------------

unsigned SmartCharPool::addString (char const* s) {
	if (storedIndeces == maxIndeces) resizeIndexTable();
	index[storedIndeces] = SimpleCharPool::addString(s);
	return storedIndeces++;
}

void SmartCharPool::resizeIndexTable () {
	maxIndeces = maxIndeces<<1;
	unsigned* newIndex = new unsigned[maxIndeces];
	memcpy(newIndex, index, sizeof(unsigned)*storedIndeces);
	delete[] index;
	index = newIndex;
}
