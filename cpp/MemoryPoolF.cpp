//==============================================================================
/// \file MemoryPoolF.cpp
// created on July 1 2012 by editing MemoryPool.cpp
//==============================================================================

#include "MemoryPoolF.h"
#include <iostream>

using namespace std;

/** \class MemoryPoolF
 *
 * MemoryPoolF contains a linked list of large blocks of memory. Each block
 * has a MemoryBlock object at its beginning that contains the size of the block
 * and a pointer to the next block. Each time the user calls MemoryPoolF::alloc,
 * the MemoryPoolF returns a pointer to at least the requested number of bytes of
 * free memory. It gets these smaller chunks of memory from the first block
 * in its list of large blocks. When this block runs out of room, it allocates
 * a new large block and adds it to the top of the list. (All blocks but the
 * first are full.)
 *
 * MemoryPoolF also has a list of reserve blocks. These can be given to the
 * MemoryPoolF with the MemoryPoolF::donate method. New blocks will not be
 * allocated until all the reserve blocks are filled.
 *
 * To delete the items in the pool, one can use MemoryPoolF::clear, which
 * turns all active memory into reserve memory, or MemoryPoolF::releaseAll,
 * which returns all the active and reserve memory it has to the operating system.
 * (Items must be deleted all at once; there is no free list.)
 *
 * MemoryPoolF will also ensure that the objects it allocates adhere to a
 * specific alignment. It does this by placing appropriate padding around all
 * the items.
 *
 * Technical Note: MemoryPoolF's definition of being aligned is that there
 * is an integer multiple of alignment bytes between the pointer given to the
 * pool by malloc and every pointer given to the user by MemoryPoolF::alloc.
 * This means that the actual alignment of MemoryPoolF cannot be greater than
 * that of malloc. (That is, you can tell MemoryPoolF to align to 16 bytes,
 * and it will space your items accordingly, but if malloc hands it memory aligned
 * to 8 bytes then half the time your items will actually be aligned to odd
 * multiples of 8.) Malloc is required to hand back memory aligned to fit
 * any fundamental type, though, so only in extremely bizarre and specific
 * circumstances should this be an issue.
 *
 */


//==============================================================================
// MemoryBlockRecord Method Definitions
//==============================================================================

//------------------------------------------------------------------------------
void MemoryPoolF::MemoryBlockRecord::attach (void* ptr, unsigned blockSize) {
   _start = static_cast<char*>(ptr);
   _end   = _start + blockSize;
   _firstFree = 0;
}

//------------------------------------------------------------------------------
unsigned MemoryPoolF::MemoryBlockRecord::partition (unsigned itemSize) {
   unsigned blockSize = _end - _start;
   _freeItems = blockSize / itemSize;
   _occupied.resize(_freeItems);
   _occupied.zero();
   _firstFree = 0;  // is _firstFree ever not zero when this is called?
   return _freeItems;
}

//------------------------------------------------------------------------------
void* MemoryPoolF::MemoryBlockRecord::alloc (unsigned itemSize) {
   unsigned memIndex = _firstFree;
   _occupied.set(_firstFree);
   if (--_freeItems > 0) {
      BitField::Itr itr = BitField::Itr(_occupied, _firstFree);
      itr.nextUnset();
      _firstFree = itr.i();
      // this might leave _firstFree invalid; this is intentional
   }
   return &_start[itemSize * memIndex];
}

//------------------------------------------------------------------------------
void MemoryPoolF::MemoryBlockRecord::free (unsigned index) {
   _occupied.unset(index);
   ++_freeItems;
   if (index < _firstFree)
      _firstFree = index;
}

//------------------------------------------------------------------------------
void MemoryPoolF::MemoryBlockRecord::clear () {
   _occupied.zero();
   _freeItems = 0;
   _firstFree = 0;
}

//------------------------------------------------------------------------------
void MemoryPoolF::MemoryBlockRecord::operator= (MemoryBlockRecord && mbr) {
   _start = mbr._start;
   _end = mbr._end;
   _occupied = std::move(mbr._occupied);
   _freeItems = mbr._freeItems;
   _firstFree = mbr._firstFree;
   mbr._start = nullptr;
   mbr._end = nullptr;
}


//==============================================================================
// MemoryPoolF Method Definitions
//==============================================================================

//------------------------------------------------------------------------------
MemoryPoolF::MemoryPoolF ()
: _block(nullptr), _blocks(0), _maxBlocks(0), _itemSize(0), _minFree(5),
_nextBlockSize(64), _minDonationSize(0), _allocs(0), _frees(0),
_capacityItems(0), _capacityBytes(0)
{}

//------------------------------------------------------------------------------
unsigned MemoryPoolF::setItemSize (unsigned itemSize, unsigned alignment) {
   // only set _itemSize if the pool is empty
   if (_allocs - _frees == 0) {
      _itemSize = alignment * ( (itemSize + alignment - 1) / alignment );
      _capacityItems = 0;
      for (unsigned i=0; i<_blocks; ++i) {
         _capacityItems += _block[i].partition(_itemSize);
      }
   }
   return _itemSize;
}

//------------------------------------------------------------------------------
MemoryPoolF::~MemoryPoolF () {
   releaseAll();
   std::free(_block);
}

//------------------------------------------------------------------------------
void* MemoryPoolF::alloc () {
   // If there's free space in our first block, we will use it.
   // (The goal is to make this be the case as often as possible, ie nearly always.)
   // If not...
   if (!_blocks or _block[0].freeItems() == 0) {
      cout << "gotta sort\n";

      // sort the blocks by amount of free space
      sortBlocks();

      // If there's hardly any room in the block with the most free space,
      // we risk having to resort our blocks frequently.
      // (This will be less of a problem once sorting is implemented as a partial
      // quicksort, but for now this would be very bad.)
      if (!_blocks or _block[0].freeItems() < _minFree) {
         cout << "gotta make a new block" << '\n';
         print();
         allocBlock(_nextBlockSize);
      }
   }

   // at this point we definitely have space in our first block
   ++_allocs;
   return _block[0].alloc(_itemSize);
}

//------------------------------------------------------------------------------
void MemoryPoolF::free (void* item) {
   char* ptr = static_cast<char*> (item);
   for (unsigned i=0; i<_blocks; ++i) {
      if (_block[i].contains(ptr)) {
         unsigned index = _block[i].index(ptr, _itemSize);
         _block[i].free(index);
         ++_frees;
         return;
      }
   }
}

//------------------------------------------------------------------------------
// ToDo: put new block at front of list since it probably has most free space
unsigned MemoryPoolF::donate (void* start, unsigned size) {
   if (_blocks == _maxBlocks) {
      resizeBlockArray();
   } else {
      shiftBlockArray();
   }

   MemoryBlockRecord& block = _block[0];
   block.attach(start, size);
   unsigned addedCap = block.partition(_itemSize);
   _capacityItems += addedCap;
   _capacityBytes += block.capacityBytes();
   sortBlocks();
   return addedCap;
}

//------------------------------------------------------------------------------
unsigned MemoryPoolF::allocBlock (unsigned blockSize) {
   if (blockSize == 0) blockSize = _nextBlockSize;
   blockSize *= _itemSize;
   cout << "alloc " << blockSize << " bytes" << '\n';
   return donate(malloc(blockSize), blockSize);
}

//------------------------------------------------------------------------------
void MemoryPoolF::resizeBlockArray () {
   unsigned newMaxBlocks;
   if (_maxBlocks == 0) {
      newMaxBlocks = 8;
   } else {
      newMaxBlocks = _maxBlocks + (_maxBlocks >> 1);
   }
   auto newblock = static_cast<MemoryBlockRecord*> (malloc(newMaxBlocks*sizeof(MemoryBlockRecord)));

   for (unsigned i=0; i<_blocks; ++i) {
      newblock[i+1] = std::move(_block[i]);
   }
   ++_blocks;
   std::free(_block);
   _block = newblock;
   _maxBlocks = newMaxBlocks;
}

//------------------------------------------------------------------------------
void MemoryPoolF::shiftBlockArray () {
   for (unsigned i=_blocks; i>0; --i) {
      _block[i] = std::move(_block[i-1]);
   }
   ++_blocks;
}

//------------------------------------------------------------------------------
// Empties the pool, but does not return the memory to the operating system.
/**
 * The capacity of the MemoryPoolF thus stays the same. This method does
 * not zero the memory.
 */
void MemoryPoolF::clear () {
   for (unsigned i=0; i<_blocks; ++i) {
      _block[i].clear();
   }
   _allocs = 0;
   _frees = 0;
}

//------------------------------------------------------------------------------
// Returns all memory to the operating system.
void MemoryPoolF::releaseAll () {
   for (unsigned i=0; i<_blocks; ++i) {
      _block[i].release();
   }
   _blocks = 0;
   _allocs = 0;
   _frees = 0;
   _capacityItems = 0;
   _capacityBytes = 0;
}

//------------------------------------------------------------------------------
// Prints data reflecting what the MemoryPoolF is set up to store.
void MemoryPoolF::print () const
{
   std::cout << "===== MemoryPoolF Parameters =====\n";
   std::cout << "Blocks:          " << blocks() << '\n';
   std::cout << "MaxBlocks:       " << maxBlocks() << '\n';
   std::cout << "ItemSize:        " << itemSize() << '\n';
   std::cout << "MinFree:         " << minFree() << '\n';
   std::cout << "NextBlockSize:   " << nextBlockSize() << '\n';
   std::cout << "MinDonationSize: " << minDonationSize() << '\n';
   std::cout << "Allocs:          " << allocs() << '\n';
   std::cout << "Frees:           " << frees() << '\n';
   std::cout << "CapacityItems:   " << capacityItems() << '\n';
   std::cout << "CapacityBytes:   " << capacityBytes() << '\n';
   std::cout << "MemoryBlockSize: " << memoryBlockSize() << '\n';
   std::cout << "FreeItemsTotal:  " << freeItemsTotal() << '\n';
   std::cout << "FreeItemsBlock:  " << freeItemsBlock() << '\n';
   std::cout << '\n';
}


//------------------------------------------------------------------------------
// We should probably use a partial quicksort (ie only recurse on larger partitions)
// instead of insertion sort. However insertion sort isn't bad if:
// 1) list stays mostly sorted (deletes are evenly distribued over blocks)
// 2) sorting is done infrequently (make a new block when all are almost full)
void MemoryPoolF::sortBlocks () {
   MemoryBlockRecord temp;
   int j;
   for (unsigned i=1; i<_blocks; ++i) {
      if (_block[i] > _block[i-1]) {
         temp = std::move(_block[i]);
         _block[i] = std::move(_block[i-1]);
         j = i - 2;
         while (j >= 0 and temp > _block[j]) {
            --j;
         }
         ++j;
         _block[j] = std::move(temp);
      }
   }
}

