/** @file
  STM memory management

  Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmInit.h"
#include "PeStm.h"

void HeapList(int id);
//#define HEAPCHECK

/**

  This function allocate pages in MSEG.

  @param Pages the requested pages number

  @return pages address

**/

VOID *
AllocatePages (
               IN UINTN Pages
               )
{
    UINT64  Address;
    HEAP_HEADER * BlockHeader;
    HEAP_HEADER * PrevBlock;
    HEAP_HEADER * NewBlock;
    BOOLEAN foundBlock;
    BOOLEAN endList;

    // implements a first fit algorithm

    // find the first block that fits

    AcquireSpinLock (&mHostContextCommon.MemoryLock);

    Address = mHostContextCommon.HeapFree;     // get begining of freelist

    PrevBlock = (HEAP_HEADER *) NULL;
    BlockHeader = (HEAP_HEADER *)(UINTN)Address;

    foundBlock = FALSE;
	if(BlockHeader == 0)
	{
		endList = TRUE;   // we are totally out of space
	}
	else
	{
		endList = FALSE;
	}
    while(!endList)
    {
        if(BlockHeader->BlockLength >= Pages)
        {
            foundBlock = TRUE;
            Address = (UINT64) (UINTN) BlockHeader;  // begining of block to return
            break;
        }

        if(BlockHeader->NextBlock == 0)
        {
            endList = TRUE;
            break;
        }
        PrevBlock = BlockHeader;  // remember the previous block;
        BlockHeader  = BlockHeader->NextBlock;
    }

    if(endList)
    {
        DEBUG((EFI_D_ERROR, "AllocatePages(0x%x) fail - no freeblock of the correct size\n", Pages));
#ifdef HEAPCHECK
       // ReleaseSpinLock (&mHostContextCommon.MemoryLock);
        HeapList(1);
#endif
        ReleaseSpinLock (&mHostContextCommon.MemoryLock);
        return NULL;
    }

    // found a block that fits - now need to make adjustments

    // cases 1 - first in list == change HeapFree
    //       2 - middle of list == change previous pointer
    //       3 - released block at end of list == change previous pointer
    // subcases - block is completely consumed - need to change pointer in previous block
    //          - block has leftover space

    //if (mHostContextCommon.HeapBottom + STM_PAGES_TO_SIZE(Pages) > mHostContextCommon.HeapTop) {
    //    DEBUG ((EFI_D_ERROR, "AllocatePages(%x) fail\n", Pages));
    //    ReleaseSpinLock (&mHostContextCommon.MemoryLock);
    //    //CpuDeadLoop ();
    //    return NULL;
    //}

    // (1) breakup block (if necessary)
    // (2) point HeapFree to the new block (or to the next block in the event the current block is consumed

    if(BlockHeader->BlockLength == Pages)
    {
        NewBlock = (BlockHeader->NextBlock);  // get next block in the list
    }
    else  // need to break the block up
    {
        NewBlock = (HEAP_HEADER *)(UINTN)(Address + ((UINT64)(UINTN)STM_PAGES_TO_SIZE(Pages))); // second half of block
        NewBlock->NextBlock = BlockHeader->NextBlock;
        NewBlock->BlockLength = BlockHeader->BlockLength - Pages;
    }

    if(BlockHeader == (HEAP_HEADER *)(UINTN) mHostContextCommon.HeapFree)
    {
        mHostContextCommon.HeapFree = (UINT64) NewBlock;
    }
    else
    {
        PrevBlock->NextBlock = NewBlock;
    }

    //Address = mHostContextCommon.HeapTop - STM_PAGES_TO_SIZE(Pages);
    //mHostContextCommon.HeapTop = Address;
#ifdef HEAPCHECK   
    HeapList(2);
#endif
    ReleaseSpinLock (&mHostContextCommon.MemoryLock);
    ZeroMem ((VOID *)(UINTN)Address, STM_PAGES_TO_SIZE (Pages));
#ifdef HEAPCHECK
    DEBUG((EFI_D_ERROR, "****Allocating 0x%x pages at 0x%016llx - %d Cleared\n", Pages, Address, STM_PAGES_TO_SIZE (Pages)));
#endif
    return (VOID *)(UINTN)Address;
}

/**

This function free pages in MSEG.

@param Address pages address
@param Pages   pages number

**/
VOID
FreePages (
           IN VOID  *Address,
           IN UINTN Pages
           )
{
    HEAP_HEADER * CurrentBlock;
    HEAP_HEADER * PreviousBlock;
#ifdef HEAPCHECK
    DEBUG((EFI_D_ERROR, "****Freeing 0x%x pages at 0x%016llx\n", Pages, (UINTN) Address));
#endif
    AcquireSpinLock (&mHostContextCommon.MemoryLock);

    // (1) Set header
    // (2) find place in buffer chain
    // (3) coalese(sp?)

	Address = (void *)((UINTN) Address & ~0xfff);     // mask out the lower 12 bits

    ((HEAP_HEADER *)Address)->NextBlock = 0L;
    ((HEAP_HEADER *)Address)->BlockLength = Pages;

#ifdef HEAPCHECK
	DEBUG((EFI_D_ERROR, "Address->NextBlock: 0x%016llx Address->BlockLength: 0x%016llx\n",
		((HEAP_HEADER *)Address)->NextBlock,
		((HEAP_HEADER *)Address)->BlockLength));
#endif
    PreviousBlock = 0L;
    CurrentBlock = (HEAP_HEADER *)(UINTN) mHostContextCommon.HeapFree;

    // find where it belongs
    while( CurrentBlock != 0L)
    {
        if((UINTN)CurrentBlock > (UINTN)Address)
        {
            break;
        }

        PreviousBlock = CurrentBlock;
        CurrentBlock = CurrentBlock->NextBlock;
    }

    //link it in
    if(PreviousBlock == 0L)
    {
        // at beginning of list
        ((HEAP_HEADER *)Address)->NextBlock = CurrentBlock;
        mHostContextCommon.HeapFree = (UINT64)Address;
    }
    else
    {
        // somewhere in list
        ((HEAP_HEADER *)Address)->NextBlock = CurrentBlock;
        PreviousBlock->NextBlock = (HEAP_HEADER *)Address;
    }
#ifdef HEAPCHECK
	DEBUG((EFI_D_ERROR, "Address->NextBlock: 0x%016llx Address->BlockLength: 0x%016llx\n",
		((HEAP_HEADER *)Address)->NextBlock,
		((HEAP_HEADER *)Address)->BlockLength));
#endif
    // coalesce

    // First check the block after
    if(CurrentBlock != 0L)
    {
        if(((UINT64)Address + STM_PAGES_TO_SIZE(Pages)) == (UINT64)(UINTN)CurrentBlock)
        {
#ifdef HEAPCHECK
			DEBUG((EFI_D_ERROR, "Combined with block after\n"));
#endif         
			((HEAP_HEADER *)Address)->NextBlock = CurrentBlock->NextBlock;
            ((HEAP_HEADER *)Address)->BlockLength = ((HEAP_HEADER *)Address)->BlockLength + CurrentBlock->BlockLength;
#ifdef HEAPCHECK			
			DEBUG((EFI_D_ERROR, "Address->NextBlock: 0x%016llx Address->BlockLength: 0x%016llx\n",
		((HEAP_HEADER *)Address)->NextBlock,
		((HEAP_HEADER *)Address)->BlockLength));
#endif
		}
    }

    // then then block before
    if(PreviousBlock != 0L)
    {
		if(((UINT64)PreviousBlock + STM_PAGES_TO_SIZE((UINT64)PreviousBlock->BlockLength)) == (UINT64)Address)
        {
#ifdef HEAPCHECK
			DEBUG((EFI_D_ERROR, "Combined with block before\n"));

			DEBUG((EFI_D_ERROR, "PreviousBlock: 0x%016llx BlockLength: 0x%016llx Add: 0x%016llx\n",
		PreviousBlock,
		STM_PAGES_TO_SIZE((UINT64)PreviousBlock->BlockLength),
		((HEAP_HEADER*) Address)));
#endif
            PreviousBlock->NextBlock = ((HEAP_HEADER *)Address)->NextBlock;
			PreviousBlock->BlockLength += ((HEAP_HEADER *) Address)->BlockLength;
        }
    }

 //   if ((UINT64)(UINTN)Address == mHostContextCommon.HeapTop) {
 //       mHostContextCommon.HeapTop += STM_PAGES_TO_SIZE(Pages);
 //   }
#ifdef HEAPCHECK
    HeapList(3);
#endif
    ReleaseSpinLock (&mHostContextCommon.MemoryLock);
    return ;
}

void HeapList(int id)
{
    HEAP_HEADER * CurrentBlock = (HEAP_HEADER *)(UINTN) mHostContextCommon.HeapFree;

    DEBUG((EFI_D_ERROR, " ***HeapList %d Start***\n", id));

    while(CurrentBlock != 0L)
    {
        DEBUG((EFI_D_ERROR, "  Block: 0x%llx  BlockLength: 0x%x NextBlock: 0x%llx\n", CurrentBlock, CurrentBlock->BlockLength, CurrentBlock->NextBlock));
        CurrentBlock = CurrentBlock->NextBlock;
    }

    DEBUG((EFI_D_ERROR, " ***HeapList %d Done***\n", id));
}
