//------------------------------------------------------------------------------
// estdlib.cpp
// Created 8/3/2011.
//------------------------------------------------------------------------------

#include "estdlib.h"
#include <iostream>
using namespace std;

//------------------------------------------------------------------------------
unsigned eatBlankLines (istream& is, char* line, unsigned bufferSize) {
   char* pos;
   int lineStart(is.tellg());
   unsigned lines(0);
   while (true) {
      is.getline(line, bufferSize);
      pos = line;
      eatws(pos);
      if (*pos != 0 or !is.good()) {
         break;
      }
      lineStart = is.tellg();
      ++lines;
   }
   // what happens if we reach the end of the file?
   is.seekg(lineStart);
   return lines;
}

//------------------------------------------------------------------------------
unsigned separateWord (char*& start, char*& nextWord) {
   char* end = start+1;
   if (*start == '"') {
      ++start;
      findchar(end, '"');
      if (!*end) {
         cout << "Could not find closing quotation mark.\n";
         exit(1);
      }
      nextWord = end+1;    // character after the closing '"'
   } else {
      findws(end);
      nextWord = end;
   }
   eatws(nextWord);
   *end = 0;
   return end - start;
}
