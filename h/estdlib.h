//==============================================================================
// estdlib.h
// Created 8/3/2011.
//==============================================================================

#ifndef ESTDLIB
#define ESTDLIB

#include <iosfwd>


//==============================================================================
// String Manipulation
//==============================================================================

// advances the char* until it no longer points to whitespace
inline void eatws (char*& pos);
unsigned eatBlankLines (std::istream& is, char* line, unsigned bufferSize);
// advances the char* until it reaches the terminator or whitespace
inline void findws (char*& pos);
// advnaces the char* until it reaches the terminator or specified delimiter
inline void findchar (char*& pos, char const delim);

// puts a zero after the next word
// makes nextWord point to the next non whitespace character after the word
// returns the number of characters in the word (not including the null terminator)
unsigned separateWord (char*& start, char*& nextWord);


//------------------------------------------------------------------------------
// Inline Definitions
//------------------------------------------------------------------------------

#include <cctype> // defines isspace(int_c)

//------------------------------------------------------------------------------
void eatws (char*& pos) {
   while (isspace(*pos)) {
      ++pos;
   }
}

//------------------------------------------------------------------------------
void findws (char*& pos) {
   while (*pos and !isspace(*pos)) {
      ++pos;
   }
}

//------------------------------------------------------------------------------
void findchar (char*& pos, char const delim) {
   while (*pos and *pos != delim) {
      ++pos;
   }
}


//==============================================================================
// Endianness Swappers
//==============================================================================

inline void swapEndianness (char* byte, unsigned bytes) {
   char temp;
   for (unsigned i=0; i<(bytes>>1); ++i) {
      temp = byte[i];
      byte[i] = byte[bytes-1-i];
      byte[bytes-1-i] = temp;
   }
}   

template <class Type>
Type& swapEndianness (Type& t) {
   swapEndianness(reinterpret_cast<char*> (&t), sizeof(Type));
   return t;
}


#endif // ESTDLIB
