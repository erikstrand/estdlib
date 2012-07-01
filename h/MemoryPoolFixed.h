//------------------------------------------------------------------------------
/// \file MemoryPoolFixed.h
// created on Wed July 21 by erik
//
// Copyright:
//    Copyright (C) 2010, LBNL, MIT
//    See COPYING file that comes with this distribution.
//
// Description:
//    Interface of class MemoryPoolFixed
//------------------------------------------------------------------------------

#ifndef GENEVA_MEMORY_POOL_FIXED
#define GENEVA_MEMORY_POOL_FIXED

namespace Geneva {

//------------------------------------------------------------------------------
/// A memory pool that hands out memory in pieces of a fixed size.
//------------------------------------------------------------------------------

class MemoryPoolFixed  {
   private:
      /// The fundamental unit of storage of MemoryPoolFixed.
      /**
       * A MemoryBlock is a bookkeeping object placed at the beginning of every
       * block of memory that MemoryPoolFixed allocates. It has a pointer to the
       * next MemoryBlock (a MemoryPoolFixed is functionally a linked list of
       * MemoryBlocks), and the size of the block of memory that it is
       * at the head of.
       */
      struct MemoryBlock {
         MemoryBlock* _next;      ///< the next MemoryBlock
         unsigned _size;          ///< size in bytes of the MemoryBlock (including size of MemoryBlock itself)
         // (_size - sizeof(MemoryBlock)) bytes of memory go here
      };
      
      MemoryBlock* _activeBlock;  ///< current block, then chain of filled blocks
      MemoryBlock* _reserveBlock; ///< first block in chain of reserve (empty) blocks
      char* _insertPoint;         ///< points to where next item can be written
      char* _endOfBlock;          ///< points one byte past end of current block
      unsigned _itemSize;         ///< size of memory handouts (ie size of an item in the pool)
      unsigned _blockCapacity;    ///< the number of items we would like to fit in the next block we allocate

      // These members are for profiling and debugging purposes only.
      unsigned _items;            ///< number of items in the MemoryPoolFixed
      unsigned _activeSize;       ///< total size of all active blocks (including size of MemoryBlocks themselves)
      unsigned _activeBlocks;     ///< number of blocks in active chain
      unsigned _reserveSize;      ///< total size of all free blocks (including size of MemoryBlocks themselves)
      unsigned _reserveBlocks;    ///< number of blocks in reserve chain
      
   public:
      MemoryPoolFixed (unsigned itemSize, unsigned initialCapacity = 1024); ///< constructor
      ~MemoryPoolFixed ();        ///< destructor

      void* alloc ();             ///< Returns a pointer to the next available piece of memory.
      void clear ();              ///< Clears all memory, but does not hand it back to the operating system.
      void releaseAll ();         ///< Returns all memory to the operating system.
      void releaseReserve ();     ///< Returns all reserve memory to the operating system.
      void donate (void* start, unsigned size); ///< adds a memory block to the list of free blocks

      unsigned itemSize () const; ///< Returns the number of bytes you get when you call MemoryPoolFixed::alloc.
      unsigned memoryBlockSize () const; ///< Returns the size of one MemoryBlock object.

      // These functions are purely for profiling and/or debugging purposes.
      unsigned items () const;         ///< Returns the number of items in the MemoryPoolFixed object.
      unsigned activeBytes() const;    ///< Returns the total number of bytes being used for storage.
      unsigned activeBlocks () const;  ///< Returns the number of active MemoryBlock objects.
      unsigned reserveBytes () const;  ///< Returns the total number of bytes being held in reserve.
      unsigned reserveBlocks () const; ///< Returns the number of reserve MemoryBlock objects.

      /// Returns the number of bytes of the active MemoryBlock that are currently free.
      unsigned remainingBytesOfActiveBlock () const;
      /// Returns the maximum number of new items that can be stored in the active MemoryBlock.
      unsigned remainingCapacityOfActiveBlock () const;
};


//------------------------------------------------------------------------------
// Inline Declarations
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// constructor
inline MemoryPoolFixed::MemoryPoolFixed (unsigned itemSize, unsigned initialCapacity)
   : _activeBlock(0), _reserveBlock(0), _insertPoint(0), _endOfBlock(0), _itemSize(itemSize),
     _blockCapacity(itemSize * initialCapacity),
     _items(0), _activeSize(0), _activeBlocks(0), _reserveSize(0), _reserveBlocks(0)
{}

//------------------------------------------------------------------------------
// destructor
inline MemoryPoolFixed::~MemoryPoolFixed ()
{
   releaseAll();
}


//------------------------------------------------------------------------------
// Returns the number of bytes you get when you call MemoryPoolFixed::alloc.
inline unsigned MemoryPoolFixed::itemSize () const
{
   return _itemSize;
}

//------------------------------------------------------------------------------
// Returns the number of bytes you get when you call MemoryPoolFixed::alloc.
inline unsigned MemoryPoolFixed::memoryBlockSize () const
{
   return sizeof(MemoryBlock);
}


//------------------------------------------------------------------------------
// Returns the number of items in the MemoryPoolFixed object.
inline unsigned MemoryPoolFixed::items () const
{
   return _items;
}

//------------------------------------------------------------------------------
// Returns the total number of bytes being used for storage.
/**
 * This count includes the size of all active MemoryBlock objects themselves,
 * but not the size of the MemoryPoolFixed itself.
 */
inline unsigned MemoryPoolFixed::activeBytes () const
{
   return _activeSize;
}

//------------------------------------------------------------------------------
// Returns the total number of active MemoryBlock objects.
inline unsigned MemoryPoolFixed::activeBlocks () const
{
   return _activeBlocks;
}

//------------------------------------------------------------------------------
// Returns the total number of bytes being held in reserve.
/**
 * MemoryPoolFixed::reserveBytes / sizeof(item) is a slight overestimation
 * of the number of items that will fit in the reserve blocks. This is because
 * this memory must also be used to store MemoryBlock objects themselves,
 * and there could be space left at the end of some of the blocks after they
 * have been packed with all the items that will fit.
 */
inline unsigned MemoryPoolFixed::reserveBytes () const
{
   return _reserveSize;
}

//------------------------------------------------------------------------------
// Returns the total number of reserve MemoryBlock objects.
inline unsigned MemoryPoolFixed::reserveBlocks () const
{
   return _reserveBlocks;
}

//------------------------------------------------------------------------------
// Returns the number of bytes of the active MemoryBlock that are currently free.
inline unsigned MemoryPoolFixed::remainingBytesOfActiveBlock () const
{
   return _endOfBlock - _insertPoint;
}

//------------------------------------------------------------------------------
// Returns the maximum number of new items that can be stored in the active MemoryBlock.
inline unsigned MemoryPoolFixed::remainingCapacityOfActiveBlock () const
{
   return _itemSize % remainingBytesOfActiveBlock();
}


} // namespace Geneva

#endif GENEVA_MEMORY_POOL_FIXED
