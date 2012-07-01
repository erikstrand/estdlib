//==============================================================================
// Graph.cpp
// Created June 19 2012
//==============================================================================

#include "Graph.h"

using namespace std;


//==============================================================================
// Member Function Definitions
//==============================================================================

//------------------------------------------------------------------------------
void resize (unsigned nodes) {
   if (nodes != _nodes) {
      delete[] _neighbors;
      _neighbors = new BitField[_nodes];
   }
   for (unsigned i=0; i<nodes; ++i) {
      _neighbors[i].resize(nodes);
      _neighbors[i].zero();
   }
   _nodes = nodes;
}

