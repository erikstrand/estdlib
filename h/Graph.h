//==============================================================================
// Graph.h
// Created June 19 2012
//==============================================================================

#ifndef ESTDLIB_GRAPH
#define ESTDLIB_GRAPH

#include "LinkedList.hpp"


//==============================================================================
// Class Graph
//==============================================================================

//------------------------------------------------------------------------------
struct Edge {
   unsigned node1;
   unsigned node2;
};

//------------------------------------------------------------------------------
class BasicGraph {
private:
   unsigned _nodes;
   LinkedList<Edge> _edges;

public:
   unsigned nodes () const { return _nodes; }
   LinkedList<Edge> const& edges const () { return _edges; }
   // Edge& add (Edge const& edge) { }
};

//------------------------------------------------------------------------------
class Graph {
private:
   unsigned _nodes;
   BitField* _neighbors;

public:
   Graph (): _nodes(0);
   void resize (unsigned nodes);
   // Returns a CItr intended for iteration over neighbors of specified node.
   // (So it calls the constructor of BitField::CItr that initializes
   // to the first set bit instead of bit 0.)
   // Remember to use citr.nextSet() instead of ++citr!
   BitField::CItr citr (unsigned node) { return BitField::CItr(_neighbors[node], 1); }
};


//==============================================================================
// Inline Method Definitions
//==============================================================================


#endif // ESTLIB_GRAPH
