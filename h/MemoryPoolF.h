//==============================================================================
/// \file MemoryPoolF.h
// created on July 1 2012 by editing MemoryPool.h
//==============================================================================

#ifndef ESTLIB_MEMORY_POOL_F
#define ESTLIB_MEMORY_POOL_F


//==============================================================================
/// A memory pool that hands out fixed sized pieces.
//==============================================================================

class MemoryPoolF  {
// SubClasses
private:
   /// The fundamental unit of storage of MemoryPoolF.
   /**
    * A MemoryBlock is a bookkeeping object placed at the beginning of every
    * block of memory that MemoryPoolF allocates. It has a pointer to the
    * next MemoryBlock (a MemoryPoolF is functionally a linked list of
    * MemoryBlocks), and the size of the block of memory that it is
    * at the head of.
    */
   struct MemoryBlock {
      MemoryBlock* _next;      ///< the next MemoryBlock
      unsigned _size;          ///< size in bytes of the MemoryBlock (including size of MemoryBlock itself)
      unsigned _items;         ///< total number of items we can fit in this block (depends on what the items are)
      // (_size - sizeof(MemoryBlock)) bytes of memory go here
   };
   
// Members
private:
   MemoryBlock* _activeBlock;  ///< current block, then chain of filled blocks
   MemoryBlock* _reserveBlock; ///< first block in chain of reserve (empty) blocks
   char* _activeMemory;        ///< points to the beginning of the writable portion of the active MemoryBlock
   unsigned _itemSize;         ///< size of chunks that MemoryPoolFF will hand out
   unsigned _pos;              ///< _activeMemory[_pos] is the first free byte of mem
   unsigned _end;              ///< _activeMemory[_end] is one past the last byte of the memory block

   unsigned _nextBlockSize;   ///< the number of items we intend to fit in the next block we allocate
   /// if a donated block's capacity is less than _minimumDonationSize, it is tossed
   unsigned _minimumDonationSize;

   // These members are for profiling and debugging purposes only.
   unsigned _requestedPieces;  ///< number of times MemoryPoolF::alloc has been called
   unsigned _activeSize;       ///< total size of all active blocks (including size of MemoryBlocks themselves)
   unsigned _activeBlocks;     ///< number of blocks in active chain
   unsigned _reserveSize;      ///< total size of all free blocks (including size of MemoryBlocks themselves)
   unsigned _reserveBlocks;    ///< number of blocks in reserve chain

// Methods
public:
   /// Constructor
   MemoryPoolF ();
   bool setItemSize (unsigned itemSize);
   void setNextBlockSize (unsigned nextBlockSize);
   ~MemoryPoolF ();        ///< Destructor

   /// Returns a pointer to a piece of memory with the specified size, with the default (max) alignment.
   void* alloc (unsigned size);

   void clear ();              ///< Empties the pool, but does not return the memory to the operating system.
   void releaseAll ();         ///< Returns all memory to the operating system.
   void releaseReserve ();     ///< Returns all reserve memory to the operating system.
   void donate (void* start, unsigned size); ///< Adds a memory block to the list of reserve blocks.

   
   // All the remaining methods are purely for profiling and/or debugging purposes.
   unsigned memoryBlockSize     () const { return sizeof(MemoryBlock);  }
   unsigned nextBlockSize       () const { return _nextBlockSize;       }
   unsigned mimimumDonationSize () const { return _minimumDonationSize; }
   unsigned requestedPieces     () const { return _requestedPieces;     }
   unsigned activeBytes         () const { return _activeBytes;         }
   unsigned activeBlocks        () const { return _activeBlocks;        }
   unsigned reserveBytes        () const { return _reserveBytes;        }
   unsigned reserveBlocks       () const { return _reserveBlocks;       }
   unsigned remainingBytesOfActiveBlock () const { return _end - _pos; }

   /*
   void printParameters () const;   ///< Prints data reflecting what the MemoryPoolF is set up to store.
   void printState () const;        ///< Prints data reflecting the current state of the MemoryPoolF.
   void print () const;             ///< Prints everything.
   */
};


//==============================================================================
// Inline Declarations
//==============================================================================

//------------------------------------------------------------------------------
// Destructor
inline MemoryPoolF::~MemoryPoolF ()
{
   releaseAll();
}


#endif // ESTLIB_MEMORY_POOL_F
