//==============================================================================
/// \file MemoryPoolF.h
// created on July 1 2012 by editing MemoryPool.h
//==============================================================================

#ifndef ESTLIB_MEMORY_POOL_F
#define ESTLIB_MEMORY_POOL_F


//==============================================================================
/// A memory pool that hands out fixed sized pieces.
//==============================================================================

class MemoryPoolF {
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
   struct MemoryBlockRecord {
      char* start;
      char* end;
      BitField occupied;
      unsigned freeItems;
      unsigned firstFree;

      MemoryBlockRecord (): start(nullptr), end(nullptr), occupied(), freeItems(0) {}
      void attach (void* ptr, unsigned blockSize, unsigned itemSize);
      bool operator>  (MemoryBlockRecord const& mbr) { return freeItems > mbr.freeItems; }
      bool contains (void* ptr) { return (start <= static_cast<char*>(ptr) and static_cast<char*>(ptr) < end); }
   };
   
// Members
private:
   MemoryBlockRecord* _block; ///< array of MemoryBlockRecords
   unsigned _blocks;          ///< number of MemoryBlockRecords currently being used
   unsigned _maxBlocks;       ///< can fit _maxBlocks MemoryBlockRecords in _block
   unsigned _itemSize;        ///< size of chunks that MemoryPoolFF will hand out
   unsigned _nextBlockSize;   ///< the number of items we intend to fit in the next block we allocate
   /// if a donated block's capacity is less than _minimumDonationSize, it is tossed
   unsigned _minimumDonationSize;

   // These members are for profiling and debugging purposes only.
   unsigned _requestedPieces; ///< number of times MemoryPoolF::alloc has been called
   unsigned _activeSize;      ///< total size of all active blocks (including size of MemoryBlocks themselves)
   unsigned _blocks;          ///< number of blocks in active chain

// Methods
public:
   /// Constructor
   MemoryPoolF ();
   bool setItemSize (unsigned itemSize, unsigned alignment = itemSize);
   void setNextBlockSize (unsigned nextBlockSize);
   ~MemoryPoolF ();        ///< Destructor

   void donate (void* start, unsigned size); ///< Adds a memory block to the list of reserve blocks.
   void* alloc ();            ///< returns _itemSize bytes of memory
   void  free  ();

   void clear ();             ///< Empties the pool, but does not return the memory to the operating system.
   void releaseAll ();        ///< Returns all memory to the operating system.

   
   // All the remaining methods are purely for profiling and/or debugging purposes.
   unsigned blocks              () const { return _blocks;              }
   unsigned itemSize            () const { return _itemSize;            }
   unsigned nextBlockSize       () const { return _nextBlockSize;       }
   unsigned mimimumDonationSize () const { return _minimumDonationSize; }
   unsigned requestedPieces     () const { return _requestedPieces;     }
   unsigned activeBytes         () const { return _activeBytes;         }
   unsigned memoryBlockSize     () const { return sizeof(MemoryBlock);  }
   unsigned freeItems           () const { return _block[0].freeItems;  }

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
