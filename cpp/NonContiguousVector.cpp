//------------------------------------------------------------------------------
/// \file MemoryPoolFixed.cpp
// created on Wed July 21 by erik
//
// Copyright:
//    Copyright (C) 2010, LBNL, MIT
//    See COPYING file that comes with this distribution.
//
// Description:
//    Implementation of class MemoryPoolFixed
//------------------------------------------------------------------------------

#include "MemoryPoolFixed.h"
#include <cstdlib>

namespace Geneva {

/** \class MemoryPoolFixed
 *
 * Items must be deleted all at once; there is no free list.
 * Memory given to MemoryPoolFixed with MemoryPoolFixed::donate goes in a list
 * of reserve memory blocks that it will use up before allocating new
 * memory itself.
 *
 */

//------------------------------------------------------------------------------
// Returns a pointer to the next available piece of memory.
/**
 * If it is out of room and malloc fails, it returns a null pointer.
 */
void* MemoryPoolFixed::alloc ()
{
   // Make sure that the item will fit in the active block.
   // Note that this test fails when _insertPoint == _endOfBlock == 0,
   // as is the case after construction.
   void* writeHere = (void*) _insertPoint;
   _insertPoint += _itemSize;
   if (_insertPoint <= _endOfBlock) {
      ++_items;
      return writeHere;
   }
   
   // If the item won't fit, we need another MemoryBlock.
   MemoryBlock* newBlock;
   if (_reserveBlock) {              // Use reserve blocks, if there are any.
      newBlock = _reserveBlock;
      _reserveBlock = _reserveBlock->_next;
      _reserveSize -= _reserveBlock->_size;
   } else {                          // Otherwise allocate a new block.
      unsigned newBlockSize = _blockCapacity + sizeof(MemoryBlock);
      // calloc is not necessary, but it can be nice to get zeroed memory
      newBlock = (MemoryBlock*) calloc(1, newBlockSize);
      if (!newBlock)                 // If calloc fails...
         return 0;
      newBlock->_size = newBlockSize;

      // increase _blockCapacity (for next time we need to reallocate)
      _blockCapacity <<= 1;
   }

   // Hook up the new MemoryBlock.
   ++_items;
   ++_activeBlocks;
   _activeSize += newBlock->_size;
   newBlock->_next = _activeBlock;
   _activeBlock = newBlock;

   _endOfBlock  = (char*)_activeBlock + _activeBlock->_size;
   _insertPoint = (char*)_activeBlock + sizeof(MemoryBlock) + _itemSize;
   return (void*)((char*)_activeBlock + sizeof(MemoryBlock));
}

//------------------------------------------------------------------------------
// Clears all memory, but does not hand it back to the operating system.
void MemoryPoolFixed::clear () {
   MemoryBlock* nextBlock;
   while (_activeBlock) {
      nextBlock = _activeBlock->_next;
      donate(_activeBlock, _activeBlock->_size);
      _activeBlock = nextBlock;
   }

   _insertPoint  = 0;
   _endOfBlock   = 0;

   _items        = 0;
   _activeSize   = 0;
   _activeBlocks = 0;
}

//------------------------------------------------------------------------------
// Returns all memory to the operating system.
void MemoryPoolFixed::releaseAll () {
   releaseReserve();
   
   MemoryBlock* nextBlock;
   while (_activeBlock) {
      nextBlock = _activeBlock->_next;
      free(_activeBlock);
      _activeBlock = nextBlock;
   }

   _insertPoint   = 0;
   _endOfBlock    = 0;

   _items         = 0;
   _activeSize    = 0;
   _activeBlocks  = 0;
}

//------------------------------------------------------------------------------
// Returns all reserve memory to the operating system.
void MemoryPoolFixed::releaseReserve () {
   MemoryBlock* nextBlock;
   while (_reserveBlock) {
      nextBlock = _reserveBlock->_next;
      free(_reserveBlock);
      _reserveBlock = nextBlock;
   }

   _reserveSize   = 0;
   _reserveBlocks = 0;
}

//------------------------------------------------------------------------------
// adds a memory block to the list of free blocks
/**
 * The block starts at start, and is size bytes long.
 */
void MemoryPoolFixed::donate (void* start, unsigned size) {
   // If the donated block is too small, it's not worth keeping it around
   // Currently we discard a donated block if is smaller than the last
   // MemoryBlock we allocated.
   if (size < (_blockCapacity >> 1)) {
      free(start);
      return;
   }
   MemoryBlock* newBlock= (MemoryBlock*) start;
   newBlock->_size= size;
   newBlock->_next= _reserveBlock;
   _reserveBlock= newBlock;

   ++_reserveBlocks;
   _reserveSize += size;
}

} // namespace Geneva
