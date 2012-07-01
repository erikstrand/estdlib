#ifndef ESS_DOUBLE_SKIPLIST
#define ESS_DOUBLE_SKIPLIST

#include <iostream>
#include <cstdlib>
#include "MemoryPool.h"


//==============================================================================
/// A doubly linked skiplist.
//==============================================================================
/**
 * It stores pointers to arbitrary ITEMs.
 *
 * Need description of memory layout!
 *
 * Should be Wrapped!
 */

template<class ITEM>
class DoubleSkipList {
//------------------------------------------------------------------------------
// SubClasses
private:
   /// The links that form the list.
   struct Link {
      ITEM* item;
      Link* prev;
      Link* next; // this is really the first item in an array of pointers to Links
      // express lanes go here
      
      Link*& nextInLane (unsigned lane) { return (&next)[lane]; }
      Link* const& nextInLane (unsigned lane) const { return (&next)[lane]; }
      // Returns the amount of extra memory required for a Link with the specified number of lanes.
      // This much memory must be placed immediately after the Link itself.
      static unsigned footprint (unsigned lanes) { return sizeof(Link) + (lanes-1)*sizeof(Link*); }
   };

public:
   /// Iterates over the DoubleSkipList.
   class Iterator {
      friend class DoubleSkipList;
      private:
         Link* _current;

      private:
         Iterator (Link* link): _current(link) {}
      public:
         /// Tests if the Iterator is valid, and creates an automatic conversion.
         operator ITEM*   () const { return _current ? _current->item : 0; }
         // All the following methods will crash if called on an invalid Iterator.
         ITEM* operator-> () const { return _current->item; }
         ITEM& operator*  () const { return *(_current->item); }
         Iterator& operator++ () { _current = _current->next; return *this; }
         Iterator& operator-- () { _current = _current->prev; return *this; }
         template<class KEY> bool operator<= (KEY const& key) const { return (**this) <= key; }
         template<class KEY> bool operator<  (KEY const& key) const { return (**this) < key; }
         template<class KEY> bool operator>= (KEY const& key) const { return (**this) >= key; }
         template<class KEY> bool operator>  (KEY const& key) const { return (**this) > key; }
   };
   
   /// Cautiously iterates over the DoubleSkipList.
   class ConstIterator {
      friend class DoubleSkipList;
      private:
         Link const* _current;

      private:
         ConstIterator (Link const* link): _current(link) {}
      public:
         /// Tests if the Iterator is valid, and creates an automatic conversion.
         operator ITEM const*   () const { return _current ? _current->item : 0; }
         // All the following methods will crash if called on an invalid Iterator.
         ITEM const* operator-> () const { return _current->item; }
         ITEM const& operator*  () const { return *(_current->item); }
         ConstIterator& operator++ () { _current = _current->next; return *this; }
         ConstIterator& operator-- () { _current = _current->prev; return *this; }
         template<class KEY> bool operator<= (KEY const& key) const { return (**this) <= key; }
         template<class KEY> bool operator<  (KEY const& key) const { return (**this) < key; }
         template<class KEY> bool operator>= (KEY const& key) const { return (**this) >= key; }
         template<class KEY> bool operator>  (KEY const& key) const { return (**this) > key; }
   };
   
//------------------------------------------------------------------------------
// Member data
private:
   MemoryPool* _pool;   ///< memory pool where links are created
   unsigned _lanes;     ///< Number of lanes (starts at 1). Every Link links to lane 0.
   Link* _head;         ///< Points to dummy link at head of the list.
   /// number of items in each lane (_lanes long array)
   /** Note that _items[0] is the total number of items in the DoubleSkipList. */
   unsigned* _items;
   double _linkProb;    ///< probability of new entry linking to lane n is _linkProb^n
   double _trigger;     ///< new lane is added when _items[0] exceeds _trigger == 1/(_linkProb^_lanes)
   
   // Working Memory
   /// Points to last Links visited in each lane. Used for insertions.
   /** This should be pulled off the stack when needed, not stored here. */
   Link** _lastStops;
   
//------------------------------------------------------------------------------
// Interface
public:
   DoubleSkipList (unsigned initialCapacity, double linkProb = 0.25);
   ~DoubleSkipList ();
   
   /// Adds a new ITEM to the DoubleSkipList.
   Iterator add (ITEM& item);
   Iterator add (ITEM* item);

   /// Attempts to find an ITEM in the DoubleSkipList with the specified KEY.
   template<class KEY> Iterator find (KEY const& key);
   /// Returns an Iterator to (one of) the largest ITEM less than or equal to the specified KEY.
   template<class KEY> Iterator findLow (KEY const& key);
   /// Returns an Iterator to (one of) the smallest ITEM greater than or equal to the specified KEY.
   template<class KEY> Iterator findHigh (KEY const& key);
   
   /// Attempts to find an ITEM in the DoubleSkipList with the specified KEY.
   template<class KEY> ConstIterator find (KEY const& key) const;
   /// Returns an Iterator to (one of) the largest ITEM less than or equal to the specified KEY.
   template<class KEY> ConstIterator findLow (KEY const& key) const;
   /// Returns an Iterator to (one of) the smallest ITEM greater than or equal to the specified KEY.
   template<class KEY> ConstIterator findHigh (KEY const& key) const;

   ///< Returns an Iterator to the first ITEM in the DoubleSkipList.
   Iterator begin ();
   /// Returns a ConstIterator to the first ITEM in the DoubleSkipList.
   ConstIterator constBegin () const;
//      Iterator end ();        ///< Returns an Iterator to the last ITEM in the DoubleSkipList.
   
   // Profiling methods
   unsigned size () const; ///< Returns the number of items in the DoubleSkipList.
   
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
// Inline Definitions
//==============================================================================

//------------------------------------------------------------------------------
// Constructor
template<class ITEM>
inline DoubleSkipList<ITEM>::DoubleSkipList (unsigned initialCapacity, double linkProb)
   : _lanes(1), _linkProb(linkProb), _trigger(1.0/linkProb)
{
   // Ensure linkProb is valid.
   if (0.0 >= _linkProb or _linkProb >= 1.0) {
      throw("Error from DoubleSkipList constructor: linkProb must be between 0 and 1.\n");
   }

   // Find the right number of lanes to start out with,
   // and calculate how much memory we expect these lanes to take up.
   double inverseLinkProb = 1.0/_linkProb;
   double compoundedLinkProb = _linkProb;
   double probSum = 0;
   while (initialCapacity > static_cast<unsigned>(_trigger)) {
      ++_lanes;
      _trigger *= inverseLinkProb;
      probSum += compoundedLinkProb;
      compoundedLinkProb *= _linkProb;
   }
   unsigned linkMemory = static_cast<unsigned>( static_cast<double>(initialCapacity)*probSum ) * sizeof(Link*);
   _pool = new MemoryPool(initialCapacity*sizeof(Link) + linkMemory, sizeof(ITEM*));

   // Allocate other members.
   _head      = static_cast<Link*>     (calloc(1, Link::footprint(_lanes)));
   _lastStops = static_cast<Link**>    (calloc(_lanes, sizeof(Link*)));
   _items     = static_cast<unsigned*> (calloc(_lanes, sizeof(unsigned)));
}

//------------------------------------------------------------------------------
// Destructor
template<class ITEM>
inline DoubleSkipList<ITEM>::~DoubleSkipList ()
{
   free(_head);
   free(_lastStops);
   free(_items);
   delete _pool;
}

//------------------------------------------------------------------------------
// Adds an item.
template<class ITEM>
inline typename DoubleSkipList<ITEM>::Iterator DoubleSkipList<ITEM>::add (ITEM* item)
{
   return add(*item);
}

//------------------------------------------------------------------------------
// Returns an Iterator to the first ITEM in the DoubleSkipList.
template<class ITEM>
inline typename DoubleSkipList<ITEM>::Iterator DoubleSkipList<ITEM>::begin ()
{
   return Iterator(_head->next);
}

//------------------------------------------------------------------------------
// Returns an Iterator to the first ITEM in the DoubleSkipList.
template<class ITEM>
inline typename DoubleSkipList<ITEM>::ConstIterator DoubleSkipList<ITEM>::constBegin () const
{
   return ConstIterator(_head->next);
}

//------------------------------------------------------------------------------
//Returns an Iterator to the last ITEM in the DoubleSkipList.
// Implement Me! //

//------------------------------------------------------------------------------
// Returns the number of items in the DoubleSkipList.
template<class ITEM>
inline unsigned DoubleSkipList<ITEM>::size () const
{
   return _items[0];
}


//==============================================================================
// Regular Definitions
//==============================================================================

//------------------------------------------------------------------------------
// Adds an ITEM to the DoubleSkipList.
template<class ITEM>
typename DoubleSkipList<ITEM>::Iterator DoubleSkipList<ITEM>::add (ITEM& item)
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
   unsigned lane = _lanes-1;
   _lastStops[lane] = _head;
   while (true) {
      while (_lastStops[lane]->nextInLane(lane) and *(_lastStops[lane]->nextInLane(lane)->item) < item) {
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
   Link* newLink = static_cast<Link*>( _pool->alloc(Link::footprint(newLanes)) );
   
   // Link in all forward pointing pointers of the new Link. (Note that lane == 0 at this point.)
   while (lane < newLanes) {
      newLink->nextInLane(lane) = _lastStops[lane]->nextInLane(lane);
      _lastStops[lane]->nextInLane(lane) = newLink;
      ++lane;
   }
   
   // Set prev and item pointers of the new Link.
   if (newLink->next)
      newLink->next->prev = newLink;
   newLink->prev = _lastStops[0];
   newLink->item = &item;
   
   // Return an Iterator pointing to the new Link.
   return Iterator(newLink);
}

//------------------------------------------------------------------------------
// Attempts to find an ITEM in the DoubleSkipList with the specified KEY.
template<class ITEM>
template<class KEY>
typename DoubleSkipList<ITEM>::Iterator DoubleSkipList<ITEM>::find (KEY const& key)
{
   unsigned lane = _lanes-1;
   Link* currentLink = _head;
   while (true) {
      while (currentLink->nextInLane(lane) and *(currentLink->nextInLane(lane)->item) < key) {
         currentLink = currentLink->nextInLane(lane);
      }
      if (lane == 0) {
         break;
      } else {
         --lane;
      }
   }
   
   return Iterator( (currentLink->next and *(currentLink->next->item) == key) ? currentLink->next : 0 );
}

//------------------------------------------------------------------------------
// Returns an Iterator to (one of) the largest ITEMs less than or equal to the specified KEY.
template<class ITEM>
template<class KEY>
typename DoubleSkipList<ITEM>::Iterator DoubleSkipList<ITEM>::findLow (KEY const& key)
{
   unsigned lane = _lanes-1;
   Link* currentLink = _head;
   while (true) {
      while (currentLink->nextInLane(lane) and *(currentLink->nextInLane(lane)->item) < key) {
         currentLink = currentLink->nextInLane(lane);
      }
      if (lane == 0) {
         break;
      } else {
         --lane;
      }
   }
   
   return Iterator( currentLink->next and *(currentLink->next->item) == key ? currentLink->next : currentLink );
}

//------------------------------------------------------------------------------
// Returns an Iterator to (one of) the smallest ITEMs greater than or equal to the specified KEY.
template<class ITEM>
template<class KEY>
typename DoubleSkipList<ITEM>::Iterator DoubleSkipList<ITEM>::findHigh (KEY const& key)
{
   unsigned lane = _lanes-1;
   Link* currentLink = _head;
   while (true) {
      while (currentLink->nextInLane(lane) and *(currentLink->nextInLane(lane)->item) < key) {
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
// Attempts to find an ITEM in the DoubleSkipList with the specified KEY.
template<class ITEM>
template<class KEY>
typename DoubleSkipList<ITEM>::ConstIterator DoubleSkipList<ITEM>::find (KEY const& key) const
{
   unsigned lane = _lanes-1;
   Link* currentLink = _head;
   while (true) {
      while (currentLink->nextInLane(lane) and *(currentLink->nextInLane(lane)->item) < key) {
         currentLink = currentLink->nextInLane(lane);
      }
      if (lane == 0) {
         break;
      } else {
         --lane;
      }
   }
   
   return ConstIterator( (currentLink->next and *(currentLink->next->item) == key) ? currentLink->next : 0 );
}

//------------------------------------------------------------------------------
// Returns an Iterator to (one of) the largest ITEMs less than or equal to the specified KEY.
template<class ITEM>
template<class KEY>
typename DoubleSkipList<ITEM>::ConstIterator DoubleSkipList<ITEM>::findLow (KEY const& key) const
{
   unsigned lane = _lanes-1;
   Link* currentLink = _head;
   while (true) {
      while (currentLink->nextInLane(lane) and *(currentLink->nextInLane(lane)->item) < key) {
         currentLink = currentLink->nextInLane(lane);
      }
      if (lane == 0) {
         break;
      } else {
         --lane;
      }
   }
   
   return ConstIterator( currentLink->next and *(currentLink->next->item) == key ? currentLink->next : currentLink );
}

//------------------------------------------------------------------------------
// Returns an Iterator to (one of) the smallest ITEMs greater than or equal to the specified KEY.
template<class ITEM>
template<class KEY>
typename DoubleSkipList<ITEM>::ConstIterator DoubleSkipList<ITEM>::findHigh (KEY const& key) const
{
   unsigned lane = _lanes-1;
   Link* currentLink = _head;
   while (true) {
      while (currentLink->nextInLane(lane) and *(currentLink->nextInLane(lane)->item) < key) {
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
template<class ITEM>
void DoubleSkipList<ITEM>::resize ()
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
template<class ITEM>
unsigned DoubleSkipList<ITEM>::chooseNewLanes () const
{
   double random(drand48());
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
// Debug and Profiling Method Definitions
//==============================================================================

//------------------------------------------------------------------------------
template<class ITEM>
void DoubleSkipList<ITEM>::printStats () const
{
   std::cout << "DoubleSkipList Stats\n";
   std::cout << "Items: " << _items[0] << "\nLanes: " << _lanes << '\n';
   for (unsigned i=0; i<_lanes; ++i)
      std::cout << "Links in Lane " << i << ": " << _items[i] << '\n';
   std::cout << '\n';
}

//------------------------------------------------------------------------------
template<class ITEM>
void DoubleSkipList<ITEM>::print () const
{
   std::cout << "Printing DoubleSkipList\n";
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


#endif // ESS_DOUBLE_SKIPLIST
