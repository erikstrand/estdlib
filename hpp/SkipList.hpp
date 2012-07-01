#ifndef ESTDLIB_SKIPLIST
#define ESTDLIB_SKIPLIST

#include <iostream>
#include <cstdlib>
#include <string.h>
#include "MemoryPool.h"
#include "Wrap.hpp"
#include "Random.h"


//==============================================================================
// A skiplist.
//==============================================================================
/**
 * Need description of memory layout!
 */

template<typename ITEM>
class SkipList {
// SubClasses
private:
   typedef Wrap<ITEM> W;

   /// The links that form the list.
   struct Link {
      W item;
      Link* next; // this is really the first item in an array of pointers to Links
      // express lanes go here
      
      Link (typename W::Ex item): item(item) {}   // next is always set after construction
      Link*& nextInLane (unsigned lane) { return (&next)[lane]; }
      Link* const& nextInLane (unsigned lane) const { return (&next)[lane]; }
      // Returns the amount of extra memory required for a Link with the specified number of lanes.
      // This much memory must be placed immediately after the Link itself.
      static unsigned footprint (unsigned lanes) { return sizeof(Link) + (lanes-1)*sizeof(Link*); }
   };

public:
   /// Cautiously iterates over the SkipList.
   class ConstIterator {
   protected:
      Link const* _current;
   public:
      ConstIterator (): _current(0) {}
      ConstIterator (Link const* link): _current(link) {}
      bool valid () const { return _current; }
      typename W::CRef cref () const { return _current->item.cref(); }
      typename W::CPtr cptr () const { return _current->item.cptr(); }
      typename W::Ex   ex   () const { return _current->item.ex(); }
      // All the following methods will crash if called on an invalid Iterator.
      ConstIterator& operator++ () { _current = _current->next; return *this; }
      bool operator<  (ConstIterator itr) const { return this->cref() <  itr.cref(); }
      bool operator<= (ConstIterator itr) const { return this->cref() <= itr.cref(); }
      bool operator>  (ConstIterator itr) const { return this->cref() >  itr.cref(); }
      bool operator>= (ConstIterator itr) const { return this->cref() >= itr.cref(); }
      /*
      template<typename KEY> bool operator<= (KEY const& key) const { return this->cref() <= key; }
      template<typename KEY> bool operator<  (KEY const& key) const { return this->cref() <  key; }
      template<typename KEY> bool operator>= (KEY const& key) const { return this->cref() >= key; }
      template<typename KEY> bool operator>  (KEY const& key) const { return this->cref() >  key; }
      */
   };
 
   /// Iterates over the SkipList.
   class Iterator : public ConstIterator {
   public:
      Iterator (): ConstIterator() {}
      Iterator (Link* link): ConstIterator(link) {}
      typename W::Ref ref () const { return const_cast<Link*>(this->_current)->item.ref(); }
      typename W::Ptr ptr () const { return const_cast<Link*>(this->_current)->item.ptr(); }
      Iterator& operator++ () { ConstIterator::operator++(); return *this; }
      bool operator<  (Iterator itr) const { return this->cref() <  itr.cref(); }
      bool operator<= (Iterator itr) const { return this->cref() <= itr.cref(); }
      bool operator>  (Iterator itr) const { return this->cref() >  itr.cref(); }
      bool operator>= (Iterator itr) const { return this->cref() >= itr.cref(); }
   };

// Member data
private:
   MemoryPool* _pool;   ///< memory pool where links are created
   XorShift32* _rand;   ///< random number generator
   bool _sharedPool;    ///< true if the MemoryPool is shared (and thus should not be deleted)
   bool _sharedRand;    ///< true if the random number generator is shared (and thus should not be deleted)
   unsigned _lanes;     ///< Number of lanes (starts at 1). Every Link links to lane 0.
   Link* _head;         ///< Points to dummy link at head of the list.
   /// number of items in each lane (_lanes long array)
   /** Note that _items[0] is the total number of items in the SkipList. */
   unsigned* _items;
   float _linkProb;    ///< probability of new entry linking to lane n is _linkProb^n
   float _trigger;     ///< new lane is added when _items[0] exceeds _trigger == 1/(_linkProb^_lanes)
   
   // Working Memory
   /// Points to last Links visited in each lane. Used for insertions.
   /** This should be pulled off the stack when needed, not stored here. */
   Link** _lastStops;
   
// Interface
public:
   SkipList (unsigned initialCapacity, float linkProb = 0.25, MemoryPool* pool = 0, XorShift32* rand = 0);
   ~SkipList ();
   
   /// Adds a new ITEM to the SkipList.
   Iterator add (typename W::Ex item);

   /// Attempts to find an ITEM in the SkipList with the specified KEY.
   template<typename KEY> Iterator find (KEY const& key);
   /// Returns an Iterator to (one of) the largest ITEM less than or equal to the specified KEY.
   template<typename KEY> Iterator findLow (KEY const& key);
   /// Returns an Iterator to (one of) the smallest ITEM greater than or equal to the specified KEY.
   template<typename KEY> Iterator findHigh (KEY const& key);
   
   /// Attempts to find an ITEM in the SkipList with the specified KEY.
   template<typename KEY> ConstIterator find (KEY const& key) const;
   /// Returns an Iterator to (one of) the largest ITEM less than or equal to the specified KEY.
   template<typename KEY> ConstIterator findLow (KEY const& key) const;
   /// Returns an Iterator to (one of) the smallest ITEM greater than or equal to the specified KEY.
   template<typename KEY> ConstIterator findHigh (KEY const& key) const;

   ///< Returns an Iterator to the first ITEM in the SkipList.
   Iterator iterator () { return Iterator(_head->next); }
   /// Returns a ConstIterator to the first ITEM in the SkipList.
   ConstIterator constIterator () const { return ConstIterator(_head->next); }
   
   /// Returns the number of items in the SkipList. 
   unsigned size () const { return _items[0]; } 
   
// Private Methods
private:
   void resize ();
   unsigned chooseNewLanes () const;

// Debug Methods
public:
   void printStats () const;
   void print () const;
};


//==============================================================================
// Method Definitions
//==============================================================================

//------------------------------------------------------------------------------
// Constructor
template<typename ITEM>
SkipList<ITEM>::SkipList (unsigned initialCapacity, float linkProb, MemoryPool* pool, XorShift32* rand)
   : _lanes(1), _linkProb(linkProb), _trigger(1.0/linkProb)
{
   // Ensure linkProb is valid.
   if (0.0 >= _linkProb or _linkProb >= 1.0) {
      throw("Error from SkipList constructor: linkProb must be between 0 and 1.\n");
   }

   // Find the right number of lanes to start out with,
   // and calculate how much memory we expect these lanes to take up.
   float inverseLinkProb = _trigger;
   float compoundedLinkProb = _linkProb; // prob of linking to fastest lane
   // The values calculated in this loop could be done in closed form
   // (logs for lanes and trigger, powers for compounded)
   // but the performance loss should not be great since lanes is usually small.
   // (Even with 100,000 items and p = 1/4, lanes will only be 8.)
   while (initialCapacity > static_cast<unsigned>(_trigger)) {
      ++_lanes;
      _trigger *= inverseLinkProb;
      compoundedLinkProb *= _linkProb;
   }
   float probSum = (1.0 - compoundedLinkProb) / (1.0 - _linkProb);

   // Set up MemoryPool
   if (pool) {
      _sharedPool = true;
      _pool = pool;
   } else {
      _sharedPool = false;
      unsigned linkMemory = static_cast<unsigned>( static_cast<float>(initialCapacity)*probSum ) * sizeof(Link*);
      _pool = new MemoryPool(initialCapacity*sizeof(W) + linkMemory, sizeof(ITEM*));
   }

   // Set up random number generator
   if (rand) {
      _sharedRand = true;
      _rand = rand;
   } else {
      _sharedRand = false;
      _rand = new XorShift32(0xadefceed);
   }

   // Allocate other members.
   _head      = static_cast<Link*>     (calloc(1, Link::footprint(_lanes)));
   _lastStops = static_cast<Link**>    (calloc(_lanes, sizeof(Link*)));
   _items     = static_cast<unsigned*> (calloc(_lanes, sizeof(unsigned)));
}

//------------------------------------------------------------------------------
// Destructor
template<typename ITEM>
inline SkipList<ITEM>::~SkipList ()
{
   free(_head);
   free(_lastStops);
   free(_items);
   if (!_sharedPool)
      delete _pool;
}

//------------------------------------------------------------------------------
// Adds an ITEM to the SkipList.
template<typename ITEM>
typename SkipList<ITEM>::Iterator SkipList<ITEM>::add (typename W::Ex item)
{
   unsigned newLanes;

   // If we will have more than _trigger items, resize and force the new entry to link to all lanes.
   if (++_items[0] > static_cast<unsigned>(_trigger)) {
      resize();
      newLanes = _lanes;
   } else {
      newLanes = chooseNewLanes(); // Otherwise choose a random number of express lanes to link to.
   }
   
   // Incremement lane counts for each express lane used by the new Link. (We already incremented _item[0].)
   for (unsigned i=1; i<newLanes; ++i)
      ++_items[i];
   
   // Find where to insert the new item, remembering where we have to change lanes (using _lastStops).
   // We need to remember where these stops are so we can update the relevant pointers later.
   unsigned lane = _lanes-1;
   _lastStops[lane] = _head;
   while (true) {
      while (_lastStops[lane]->nextInLane(lane) and _lastStops[lane]->nextInLane(lane)->item.cref() < item) {
         _lastStops[lane] = _lastStops[lane]->nextInLane(lane);
      }

      if (lane == 0) {
         break;
      } else {
         _lastStops[lane-1] = _lastStops[lane];
         --lane;
      }
   }
   
   // Allocate memory for new Link.
   Link* newLink = new( _pool->alloc(Link::footprint(newLanes)) ) Link(item);
   
   // Link in all forward pointing pointers of the new Link. (Note that lane == 0 at this point.)
   while (lane < newLanes) {
      newLink->nextInLane(lane) = _lastStops[lane]->nextInLane(lane);
      _lastStops[lane]->nextInLane(lane) = newLink;
      ++lane;
   }

   // Return an Iterator pointing to the new Link.
   return Iterator(newLink);
}

//------------------------------------------------------------------------------
// Attempts to find an ITEM in the SkipList with the specified KEY.
template<typename ITEM>
template<typename KEY>
typename SkipList<ITEM>::Iterator SkipList<ITEM>::find (KEY const& key)
{
   unsigned lane = _lanes-1;
   Link* currentLink = _head;
   while (true) {
      while (currentLink->nextInLane(lane) and currentLink->nextInLane(lane)->item.cref() < cref(key)) {
         currentLink = currentLink->nextInLane(lane);
      }
      if (lane == 0) {
         break;
      } else {
         --lane;
      }
   }
   
   return Iterator( currentLink->next and currentLink->next->item.cref() == cref(key) ? currentLink->next : 0 );
}

//------------------------------------------------------------------------------
// Returns an Iterator to (one of) the largest ITEMs less than or equal to the specified KEY.
template<typename ITEM>
template<typename KEY>
typename SkipList<ITEM>::Iterator SkipList<ITEM>::findLow (KEY const& key)
{
   unsigned lane = _lanes-1;
   Link* currentLink = _head;
   while (true) {
      while (currentLink->nextInLane(lane) and currentLink->nextInLane(lane)->item.cref() < cref(key)) {
         currentLink = currentLink->nextInLane(lane);
      }
      if (lane == 0) {
         break;
      } else {
         --lane;
      }
   }
   
   return Iterator( currentLink->next and currentLink->next->item.cref() == cref(key) ? currentLink->next : currentLink );
}

//------------------------------------------------------------------------------
// Returns an Iterator to (one of) the smallest ITEMs greater than or equal to the specified KEY.
template<typename ITEM>
template<typename KEY>
typename SkipList<ITEM>::Iterator SkipList<ITEM>::findHigh (KEY const& key)
{
   unsigned lane = _lanes-1;
   Link* currentLink = _head;
   while (true) {
      while (currentLink->nextInLane(lane) and currentLink->nextInLane(lane)->item.cref() < cref(key)) {
         currentLink = currentLink->nextInLane(lane);
      }
      if (lane == 0) {
         break;
      } else {
         --lane;
      }
   }
   
   return Iterator( currentLink->next );
}


//------------------------------------------------------------------------------
// Attempts to find an ITEM in the SkipList with the specified KEY.
template<typename ITEM>
template<typename KEY>
typename SkipList<ITEM>::ConstIterator SkipList<ITEM>::find (KEY const& key) const
{
   unsigned lane = _lanes-1;
   Link* currentLink = _head;
   while (true) {
      while (currentLink->nextInLane(lane) and currentLink->nextInLane(lane)->item.cref() < cref(key)) {
         currentLink = currentLink->nextInLane(lane);
      }
      if (lane == 0) {
         break;
      } else {
         --lane;
      }
   }
   
   return ConstIterator( (currentLink->next and currentLink->next->item.cref() == cref(key)) ? currentLink->next : 0 );
}

//------------------------------------------------------------------------------
// Returns an Iterator to (one of) the largest ITEMs less than or equal to the specified KEY.
template<typename ITEM>
template<typename KEY>
typename SkipList<ITEM>::ConstIterator SkipList<ITEM>::findLow (KEY const& key) const
{
   unsigned lane = _lanes-1;
   Link* currentLink = _head;
   while (true) {
      while (currentLink->nextInLane(lane) and currentLink->nextInLane(lane)->item.cref() < cref(key)) {
         currentLink = currentLink->nextInLane(lane);
      }
      if (lane == 0) {
         break;
      } else {
         --lane;
      }
   }
   
   return ConstIterator( currentLink->next and currentLink->next->item.cref() == cref(key) ? currentLink->next : currentLink );
}

//------------------------------------------------------------------------------
// Returns an Iterator to (one of) the smallest ITEMs greater than or equal to the specified KEY.
template<typename ITEM>
template<typename KEY>
typename SkipList<ITEM>::ConstIterator SkipList<ITEM>::findHigh (KEY const& key) const
{
   unsigned lane = _lanes-1;
   Link* currentLink = _head;
   while (true) {
      while (currentLink->nextInLane(lane) and currentLink->nextInLane(lane)->item.cref() < cref(key)) {
         currentLink = currentLink->nextInLane(lane);
      }
      if (lane == 0) {
         break;
      } else {
         --lane;
      }
   }
   
   return ConstIterator( currentLink->next );
}



//------------------------------------------------------------------------------
// Adds another express lane.
/**
 * No extra lanes are added to existing items; it simply becomes possible for
 * new links to be formed with the new lane.
 */
template<typename ITEM>
void SkipList<ITEM>::resize ()
{
   // Change state members.
   ++_lanes;
   _trigger *= 1.0/_linkProb;

   // Allocate new memory.
   Link*     newHead      = static_cast<Link*>     (malloc(Link::footprint(_lanes)));
   Link**    newLastStops = static_cast<Link**>    (malloc(_lanes * sizeof(Link*)));
   unsigned* newItems     = static_cast<unsigned*> (malloc(_lanes * sizeof(unsigned)));

   // Fill new memory.
   newHead->nextInLane(_lanes-1) = 0;
   memcpy(newHead, _head, Link::footprint(_lanes-1));
   newItems[_lanes-1] = 0;
   memcpy(newItems, _items, (_lanes-1)*sizeof(unsigned));

   // Free old memory, swap in new memory.
   free(_head);
   free(_lastStops);
   free(_items);
   _head = newHead;
   _lastStops = newLastStops;
   _items = newItems;
}

//------------------------------------------------------------------------------
template<typename ITEM>
unsigned SkipList<ITEM>::chooseNewLanes () const
{
   double random = _rand->f64();
   unsigned newLanes(1);
   double cutoff(_linkProb);
   for (unsigned i=1; i<_lanes; ++i) {
      if (random <= cutoff) {
         ++newLanes;
         cutoff *= _linkProb;
      } else {
         break;
      }
   }
   return newLanes;
}

//==============================================================================
// Comparison Operators
//==============================================================================

template <typename ITEM>
bool operator<= (typename Wrap<ITEM>::CRef item, typename SkipList<ITEM>::ConstIterator const& itr) { return item <= itr->cref(); }
template <typename ITEM>
bool operator<  (typename Wrap<ITEM>::CRef item, typename SkipList<ITEM>::ConstIterator const& itr) { return item <  itr->cref(); }
template <typename ITEM>
bool operator>= (typename Wrap<ITEM>::CRef item, typename SkipList<ITEM>::ConstIterator const& itr) { return item >= itr->cref(); }
template <typename ITEM>
bool operator>  (typename Wrap<ITEM>::CRef item, typename SkipList<ITEM>::ConstIterator const& itr) { return item >  itr->cref(); }

template <typename ITEM>
bool operator<= (typename Wrap<ITEM>::CRef item, typename SkipList<ITEM>::Iterator const& itr) { return item <= itr->cref(); }
template <typename ITEM>
bool operator<  (typename Wrap<ITEM>::CRef item, typename SkipList<ITEM>::Iterator const& itr) { return item <  itr->cref(); }
template <typename ITEM>
bool operator>= (typename Wrap<ITEM>::CRef item, typename SkipList<ITEM>::Iterator const& itr) { return item >= itr->cref(); }
template <typename ITEM>
bool operator>  (typename Wrap<ITEM>::CRef item, typename SkipList<ITEM>::Iterator const& itr) { return item >  itr->cref(); }


//==============================================================================
// Debug and Profiling Method Definitions
//==============================================================================

//------------------------------------------------------------------------------
template<typename ITEM>
void SkipList<ITEM>::printStats () const
{
   std::cout << "SkipList Stats\n";
   std::cout << "Items: " << _items[0] << "\nLanes: " << _lanes << '\n';
   for (unsigned i=0; i<_lanes; ++i)
      std::cout << "Links in Lane " << i << ": " << _items[i] << '\n';
   std::cout << '\n';
}

//------------------------------------------------------------------------------
template<typename ITEM>
void SkipList<ITEM>::print () const
{
   std::cout << "Printing SkipList\n";
   Iterator itr(_head);
   ++itr;
   if (!itr)
      std::cout << "The list is empty.\n";
   while(itr) {
      std::cout << *itr << '\n';
      ++itr;
   }
   std::cout << '\n';
}


#endif // ESTDLIB_SKIPLIST
