//==============================================================================
// LinkedList.hpp
// Created 1/28/12.
//==============================================================================

#ifndef ESTDLIB_LINKED_LIST
#define ESTDLIB_LINKED_LIST

#include <new>
#include "Wrap.hpp"
#include "MemoryPool.h"


//==============================================================================
// Struct LinkedList<ITEM>
//==============================================================================

template<typename ITEM>
class LinkedList {
// SubClasses
public:
   struct Link {
      Link* _next;
      Wrap<ITEM> _item;
      Link (Link* next, typename Wrap<ITEM>::Ex item): _next(next), _item(item) {}
   };
   
// Iterators
public:
   class Itr {
   private:
      Link* _link;
      
   public:
      Itr (): _link(0) {}
      Itr (Link* start): _link(start) {}
      bool valid () const { return _link; }
      bool last  () const { return !(_link->_next); }
      Itr& operator++ () { _link = _link->_next; return *this; }
      typename Wrap<ITEM>::Ref  ref  () const { return _link->_item.ref(); }
      typename Wrap<ITEM>::CRef cref () const { return _link->_item.cref(); }
      typename Wrap<ITEM>::Ptr  ptr  () const { return _link->_item.ptr(); }
      typename Wrap<ITEM>::CPtr cptr () const { return _link->_item.cptr(); }
      Link* link () const { return _link; }
   };
   friend class Itr;

   class CItr {
   private:
      Link* _link;
      
   public:
      CItr (): _link(0) {}
      CItr (Link* start): _link(start) {}
      bool valid () const { return _link; }
      bool last  () const { return !(_link->_next); }
      CItr& operator++ () { _link = _link->_next; return *this; }
      typename Wrap<ITEM>::CRef cref () const { return _link->_item.cref(); }
      typename Wrap<ITEM>::CPtr cptr () const { return _link->_item.cptr(); }
      Link const* link () const { return _link; }
   };
   friend class CItr;

// Members
private:
   Link* _first;     // Must be first, so it looks like a Link 
   unsigned _items;

// Interface
public:
   // Constructor
   LinkedList (): _first(0), _items(0) {}

   // Destructor
   ~LinkedList ();

   // Returns the number of items in the LinkedList.
   unsigned size () const { return _items; }
   // Returns the first item.
   typename Wrap<ITEM>::T first () { return _first->_item.t(); }

   // Iterators
   Itr itr () { return Itr(_first); }
   CItr citr () const { return CItr(_first); }
   // Returns a dummy Itr that can be used to add elements to the head of the list.
   // Use with caution, as accessing the Itr's ITEM in any way is undefined!
   Itr dummyItr () { return Itr(reinterpret_cast<Link*>(this)); }

   // Adds an ITEM
   inline void add (typename Wrap<ITEM>::Ex item);
   // Adds an ITEM, using the provided MemoryPool as an allocator.
   inline void add (typename Wrap<ITEM>::Ex item, MemoryPool& pool);
   // Adds an ITEM after the specified Link.
   inline Itr add (Itr itr, typename Wrap<ITEM>::Ex item);
   // Adds a Link that you have constructed yourself.
   inline void addLink (Link* link);

   // Removes the link following parent.
   inline typename Wrap<ITEM>::T removeNext (Link* parent);
   // Removes the first link.
   inline typename Wrap<ITEM>::T removeFirst ();
   
   // Leaks the link following parent.
   inline void leakNext (Link* parent);
   inline void leakNext (Itr const& itr) { leakNext(itr.link()); }
   // Leaks the first link.
   inline void leakFirst ();
   // Leaks all the links in the LinkedList.
   inline void leakAll ();
};


//==============================================================================
// Inline Method Definitions
//==============================================================================

//------------------------------------------------------------------------------
// Destructor
template<typename ITEM>
LinkedList<ITEM>::~LinkedList () {
   Link* kill = _first;
   Link* temp;
   while (kill) {
      temp = kill->_next;
      delete kill;
      kill = temp;
   }
}

//------------------------------------------------------------------------------
// Adds an ITEM
template<typename ITEM>
void LinkedList<ITEM>::add (typename Wrap<ITEM>::Ex item) {
   _first = new Link(_first, item);
   ++_items;
}

//------------------------------------------------------------------------------
// Adds an ITEM, using the provided MemoryPool as an allocator.
template<typename ITEM>
void LinkedList<ITEM>::add (typename Wrap<ITEM>::Ex item, MemoryPool& pool) {
   _first = new(pool.alloc(sizeof(Link))) Link(_first, item);
   ++_items;
}

//------------------------------------------------------------------------------
template<typename ITEM>
typename LinkedList<ITEM>::Itr LinkedList<ITEM>::add (Itr itr, typename Wrap<ITEM>::Ex item) {
   ++_items;
   return Itr(itr.link()->_next = new Link(itr.link()->_next, item));
}

//------------------------------------------------------------------------------
// Adds a Link that you have constructed yourself.
template<typename ITEM>
void LinkedList<ITEM>::addLink (Link* link) {
   link->_next = _first;
   _first = link;
   ++_items;
}

//------------------------------------------------------------------------------
// Removes the link following parent
template<typename ITEM>
typename Wrap<ITEM>::T LinkedList<ITEM>::removeNext (Link* parent) {
   Link* temp = parent->_next;
   typename Wrap<ITEM>::T item = temp->_item.t();
   parent->_next = temp->_next;
   delete temp;
   --_items;
   return item;
}

//------------------------------------------------------------------------------
template<typename ITEM>
typename Wrap<ITEM>::T LinkedList<ITEM>::removeFirst () {
   return removeNext(reinterpret_cast<Link*> (this));
}

//------------------------------------------------------------------------------
// Leaks the link following parent, but doesn't delete it.
// This causes a memory leak, unless everything is in a MemoryPool.
template<typename ITEM>
void LinkedList<ITEM>::leakNext (Link* parent) {
   parent->_next = parent->_next->_next;
   --_items;
}

//------------------------------------------------------------------------------
template<typename ITEM>
void LinkedList<ITEM>::leakFirst () {
   leakNext(reinterpret_cast<Link*> (this));
}

//------------------------------------------------------------------------------
template<typename ITEM>
void LinkedList<ITEM>::leakAll () {
   _first = 0;
   _items = 0;
}


//==============================================================================
// Method Definitions
//==============================================================================


#endif // ESTDLIB_LINKED_LIST
