/*!
\file
Support for IA-32e memory paging

Provides support for translation of virtual to physical address for all modes of paging.
*/

#define PAGINGEPT_C 1                              //!< flag for include

//#include <stdio.h>
#include "StmRuntime.h"
#include "StmInit.h"
#include "PeStm.h"
#include "PeLoadVm.h"
#include "PeStmEpt.h"
//#define EPTCHECK

/*!
Setup VM/PE EPT tables.

\param[in] pStmVmm      Pointer to StmVmm structure

\retval   0         Function succeeds
\retval   !0        Error code

*/

extern UINT8 GetMemoryType (IN UINT64 BaseAddress);  // call to base EPT functionality - hope to eventually do more of this
extern UINT64 EndTimeStamp;

static void PeEptFreeL2(IN UINT64 Level2);
static void insertPhysAdd(EPT_ENTRY* L1PageTable, UINTN startAddress, UINTN StartIndex, UINTN size);
extern UINT32 PostPeVmProc(UINT32 rc, UINT32 CpuIndex, UINT32 mode);

extern VOID
EptCreatePageTable (
  OUT EPT_POINTER              *EptPointer,
  IN UINT32                    Xa
  );

#ifdef USENOBASE
UINT32 PeMapRegionEptHelper(UINT32 VmType,
	UINTN membase,
	UINTN memsize,
	UINTN physbase,   // when nonzero location to be mapped to guest physical
	BOOLEAN isRead,
	BOOLEAN isWrite,
	BOOLEAN isExec,
	BOOLEAN isRecurse,
	UINT32 CpuIndex);
#endif
// Initialization...

MRTT_INFO  mMtrrInfo;

#define L4_BITMASK            0x0000FF8000000000
#define L3_BITMASK            0x0000007FC0000000
#define L2_BITMASK            0x000000003FE00000
#define L1_BITMASK            0x00000000001FF000
#define OFFSET_BITMASK_4K     0x0000000000000FFF
#define BITMASK_PAGE_SIZE_2MB 0x00000000001FFFFF

#define PTE_COUNT             512

#define L4_POSITION          39
#define L3_POSITION          30
#define L2_POSITION          21
#define L1_POSITION          12

#ifdef HOLD_TO_MAKE_SURE
typedef struct {
	UINT32  Base;
	UINT32  Stepping;
} FIXED_MTRR_STRUCT_INFO;

FIXED_MTRR_STRUCT_INFO mFixedMtrrStructInfo[] = {
	{0x00000, SIZE_64KB},
	{0x80000, SIZE_16KB},
	{0xA0000, SIZE_16KB},
	{0xC0000, SIZE_4KB},
	{0xC8000, SIZE_4KB},
	{0xD0000, SIZE_4KB},
	{0xD8000, SIZE_4KB},
	{0xE0000, SIZE_4KB},
	{0xE8000, SIZE_4KB},
	{0xF0000, SIZE_4KB},
	{0xF8000, SIZE_4KB},
};
#endif

UINT8 GetMemoryTypeTest(UINT64 StartAddress)
{
	UINT8 Memory_Type = GetMemoryType(StartAddress);

	if((Memory_Type != 0) || (Memory_Type != 6))
		Memory_Type = 6;

	return Memory_Type;
}

/**

This function create EPT l4/L3 tables for VM/PE guest.
L2 and L1 are added during the VM/PE build

@param EptPointer EPT pointer
@param Xa         Execute access

**/

//EptCreatePageTable (&mGuestContextCommonSmm[VmType].EptPointer);
INT32 PeEptInit (OUT EPT_POINTER  *EptPointer)
{
	//EPT_ENTRY                *L4PageTable;
	//UINTN                    NumberOfPml4EntriesNeeded;
	//int EtmtUC = 0;          // UC memory type
	//int EtmtWB = 0;          // WB memory type
	//UINT64 Data64;
	//int MemoryType;


	EptCreatePageTable (EptPointer, 1);
#ifdef USENOBASE
	Data64 = AsmReadMsr64 (IA32_VMX_EPT_VPID_CAP_MSR_INDEX);

	if(Data64 & (1<<16))   // check for UC
		EtmtUC = 1;
	if(Data64 & (1<<14))   // check for WB
		EtmtWB = 1;

	DEBUG ((EFI_D_ERROR, "PeEptInit Entered\n"));

	if (mHostContextCommon.PhysicalAddressBits <= 39) {
		NumberOfPml4EntriesNeeded = 1;
		//NumberOfPdpEntriesNeeded = (UINTN)LShiftU64 (1, mHostContextCommon.PhysicalAddressBits - 30);
	} else {
		NumberOfPml4EntriesNeeded = (UINTN)LShiftU64 (1, mHostContextCommon.PhysicalAddressBits - 39);
		//NumberOfPdpEntriesNeeded = 512;
	}

	//SmrrBase = (UINT32)mMtrrInfo.SmrrBase & 0xFFFFF000;
	//SmrrLength = (UINT32)mMtrrInfo.SmrrMask & ~EFI_MSR_SMRR_PHYS_MASK_VALID;
	//SmrrLength = ~SmrrLength + 1;

	L4PageTable = (EPT_ENTRY *)AllocatePages (NumberOfPml4EntriesNeeded);  // the other levels will be allocated as needed

	if(L4PageTable == 0)
	{
		DEBUG((EFI_D_ERROR, "PeEptInit - failed to allocate L4 EPT Pagetable\n"));
		return -1;
	}
	EptPointer->Uint64 = (UINT64)(UINTN)L4PageTable;
	EptPointer->Bits32.Gaw = EPT_GAW_48BIT;
	//EptPointer->Bits32.Etmt = MEMORY_TYPE_WB;

	MemoryType = GetMemoryTypeTest((UINT64)L4PageTable);

	if( !(((MemoryType == MEMORY_TYPE_WB)&&(EtmtWB == 1)) || ((MemoryType == MEMORY_TYPE_UC)&&(EtmtUC == 1))
		))
	{
		// Invalid memory type try to default from the MSR
		if(EtmtWB == 1)
			MemoryType = MEMORY_TYPE_WB;
		else
			MemoryType = MEMORY_TYPE_UC;

		DEBUG((EFI_D_ERROR, "PeEptInit - Invalid EPT memory type EPT defaulting to %d\n", MemoryType));  
	}
	EptPointer->Bits32.Etmt = MemoryType;

	DEBUG ((EFI_D_ERROR, "PeEptInit EPTP: 0x%llx MemoryType: %x\n", EptPointer->Uint64, EptPointer->Bits32.Etmt));
#endif
	return 0;
}

/*! Serialize function to map a region of memory into the SMM page table

This function serializes requests to map a region of memory into the
SMM page table.

\param[in] VmType     Which VmType to handle
\param[in] membase    base address of memory to map
\param[in] memsize    size of memory region (in bytes) to map (max is 2G-1 bytes)
\param[in] isWrite    are writes allowed on this page?
\param[in] isPerm     is this a permanent mapping?
\param[in] isRecurse  is this a recursive call?

\retval   0       Function succeeds
\retval   !0      Error code
*/

#ifdef USENOBASE
UINT32 PeMapRegionEpt(UINT32 VmType,
	UINTN membase,
	UINTN memsize,
	UINTN physmem,
	BOOLEAN isRead,
	BOOLEAN isWrite,
	BOOLEAN isExec,
	UINT32 CpuIndex)
{
	UINT32 rc = STM_SUCCESS;
	BOOLEAN isRecurse = FALSE;

	//isExec = TRUE;    // for now allow execute access everywhere (later change to allow only TSEG execute)
#ifdef EPTCHECK
	DEBUG((EFI_D_ERROR, " %ld PeMapRegionEpt entered - membase: %x memsize: %x physbase: %x\n", 
		CpuIndex, membase, memsize, physmem));
#endif

	if (memsize <= 0)
	{
		rc = REGION_MEMSIZE_INVALID;
	}
	else
	{
		rc = PeMapRegionEptHelper(VmType,
			membase,
			memsize,
			physmem,
			isRead,
			isWrite,
			isExec,
			isRecurse,
			CpuIndex);
		//  release_mutex(MAP_MEMORY_MUTEX);
	}
	// DEBUG((EFI_D_ERROR, " %ld PeMapRegionEpt done - rc: %x \n", 
	//                 CpuIndex, rc));
	return rc;
}

/*! map a region of memory into the SMM page table

In order to allow SMI access to memory without page faulting, this routine
will allocate a page table and mark pages as PRESENT.

\param[in] membase    base address of memory to map
\param[in] memsize    size of memory region (in bytes) to map (max is 2G-1 bytes)
\param[in] isWrite    are writes allowed on this page?
\param[in] isPerm     is this a permanent mapping?
\param[in] isRecurse  is this a recursive call?
\param[in] StmVmm       Pointer to StmVmm structure

\retval   0       Function succeeds
\retval   !0      Error code
*/
UINT32 PeMapRegionEptHelper(UINT32 VmType,
	UINTN membase,
	UINTN memsize,
	UINTN physbase,   // when nonzero location to be mapped to guest physical
	BOOLEAN isRead,
	BOOLEAN isWrite,
	BOOLEAN isExec,
	BOOLEAN isRecurse,
	UINT32 CpuIndex)
{
	UINT32      rc = STM_SUCCESS;     // function return code
	UINT64      memend;                 // end of requested map region
	UINT64      Pages_2MB = 0x200000;
	UINT64 PhysAddr;
	UINT64 vPhysAddr;

	EPT_ENTRY                *L1PageTable = NULL;
	EPT_ENTRY                *L2PageTable = NULL;
	EPT_ENTRY                *L3PageTable;
	EPT_ENTRY                *L4PageTable;

	UINTN                    Index1;
	UINTN                    Index2;
	UINTN                    Index3;
	UINTN                    Index4;

	// calculate Indexes

	// Assume 4G
	Index4 = ((UINTN)RShiftU64 (membase, 39)) & 0x1ff;
#ifdef EPTCHECK
	DEBUG((EFI_D_ERROR, "%ld PeMapRegionEptHelper entered: membase: %llx memsize: %x physbase: %llx\n", 
		CpuIndex, membase, memsize, physbase));
#endif

	// Check parameters for sanity, should have already been checked
	if (memsize == 0)
	{
		DEBUG((EFI_D_ERROR, "%ld PeMapRegionEptHelper INVALID! size=0 (1)\n", CpuIndex));
		return INVALID_RESOURCE;
	}

	memend = membase + memsize - 1ULL;

	//Index3 = (membase & L3_BITMASK) >> L3_POSITION; // get page directory page table index
	//Index2 = (membase & L2_BITMASK) >> L2_POSITION;  // get page directory index
	//Index1 = (membase & L1_BITMASK) >> L1_POSITION; // get page table index

	//Index4 = ((UINTN)RShiftU64 (membase, 39)) & 0x1ff;
	//Index3 = ((UINTN)membase >> 30) & 0x1ff;
	Index3 = ((UINTN) RShiftU64(membase, 30)) & 0x1ff;
	Index2 = ((UINTN)membase >> 21) & 0x1ff;
	Index1 = ((UINTN)membase >> 12) & 0x1ff;
	//Offset = ((UINTN)membase & 0xFFF);

	vPhysAddr = membase;

#ifdef EPTCHECK
	DEBUG((EFI_D_ERROR, "%ld PeMapRegionEptHelper base=%llx size=0x%llx physbase=%llx %s%s%s \n",
		CpuIndex,
		membase,
		memsize,
		physbase,
		isRead ? "R" : "",
		isWrite ? "W" : "",
		isExec ? "X" : "" ));

	DEBUG((EFI_D_ERROR, "%ld PeMapRegionEptHelper I4: %d I3: %d I2: %d I1: %d\n", CpuIndex, Index4, Index3, Index2, Index1));
#endif

	L4PageTable = (EPT_ENTRY *)((UINTN)mGuestContextCommonSmm[VmType].EptPointer.Uint64 & ~OFFSET_BITMASK_4K);

	if ((L4PageTable[Index4].Bits32.Ra == 0) &&
		(L4PageTable[Index4].Bits32.Wa == 0) &&
		(L4PageTable[Index4].Bits32.Xa == 0)) 
	{
		// no L3 Entry - need to create one...

		L3PageTable = AllocatePages(1);

		if (NULL == L3PageTable)     // Make sure that there were no problems
		{
			DEBUG((EFI_D_ERROR, "%ld PeMapRegionEptHelper - Alloc L3 PageTable Entry failed\n", CpuIndex));
			return(ALLOC_FAILURE);
		}

		L4PageTable[Index4].Uint64 = (UINT64)L3PageTable;
		L4PageTable[Index4].Bits32.Ra = 1;
		L4PageTable[Index4].Bits32.Wa = 1;
		L4PageTable[Index4].Bits32.Xa = 1;
#ifdef EPTCHECK
		DEBUG((EFI_D_ERROR, "%ld PeMapRegionEptHelper - Added L3PageTable[%d]: %llx\n", CpuIndex, 
			Index4, L4PageTable[Index4]));
#endif
	}
#ifdef EPTCHECK
	else
	{
		DEBUG((EFI_D_ERROR, "%ld PeMapRegionEptHelper - Found L3PageTable[%d]: %11x L4TableAddess: %llx\n", CpuIndex,
			Index4, L4PageTable[Index4], L4PageTable));
	}
#endif
	// check for page table directory and add one if necessary

	L3PageTable = (EPT_ENTRY *) ((UINTN) L4PageTable[Index4].Uint64 & ~OFFSET_BITMASK_4K);

	if((L3PageTable[Index3].Bits32.Ra == 1) ||
		(L3PageTable[Index3].Bits32.Wa == 1) ||
		(L3PageTable[Index3].Bits32.Xa == 1)) 
	{
		L2PageTable = (EPT_ENTRY *)((UINTN)L3PageTable[Index3].Uint64 & ~OFFSET_BITMASK_4K);
#ifdef EPTCHECK
		DEBUG((EFI_D_ERROR, "%ld PeMapRegionEptHelper - Found L2PageTable[%d]: %11x L3TableAddress: %llx\n", CpuIndex,
			Index3, L3PageTable[Index3], L3PageTable));
#endif
	}
	else
	{
		// none found, so allocate and add a L2 Page Table

		L2PageTable = AllocatePages(1);

		if (NULL == L2PageTable)     // Make sure that there were no problems
		{
			DEBUG((EFI_D_ERROR, "%ld PeMapRegionEptHelper - Alloc EPT Page table directory failed\n", CpuIndex));
			return(ALLOC_FAILURE);
		} 

		L3PageTable[Index3].Uint64 = (UINT64)L2PageTable;
		L3PageTable[Index3].Bits32.Ra = 1;
		L3PageTable[Index3].Bits32.Wa = 1;
		L3PageTable[Index3].Bits32.Xa = 1;
#ifdef  EPTCHECK
		DEBUG((EFI_D_ERROR, "%ld PeMapRegionEptHelper - Added L2PageTable[%d]: %llx\n", CpuIndex, 
			Index3, L3PageTable[Index3]));
#endif
	}

	// if we have a 2MB page, all we need to do is to construct
	// a 2MB page pointer and we are done...

	if(memsize == Pages_2MB)
	{
		//int permissions = ((isRead ? EPT_READ : 0 ) |(isWrite ? EPT_WRITE : 0 )|(isExec ? EPT_EXECUTE : 0 ) | (isPerm ? EPT_PERMANENT : 0));

		UINT64 StartAddress = 0;
		if(physbase != 0)
		{
			// caller wants us to map guest physical to a different hardware address
			StartAddress = physbase;
		}
		else
		{
			// Identity Mapping
			StartAddress = membase;
		}

		// check to make sure that the address is aligned on a 2MB boundry

		if(StartAddress & BITMASK_PAGE_SIZE_2MB)
		{
			DEBUG((EFI_D_ERROR, "%ld PeMapRegionEptHelper ***ERROR*** PeMapRegionEptHelper - 2MB page start not 2MB aligned (%llx)\n",
				CpuIndex, StartAddress));

			return(INVALID_RESOURCE);
		}
		// create and place the 2MB page pointer

		if(L2PageTable[Index2].Uint64 != 0)
		{
			if(L2PageTable[Index2].Bits32.Sp == 1)
			{
				// already have a super page pointer
				DEBUG((EFI_D_ERROR, "%ld PeMapRegionEptHelper - ***Warning*** 2MB Page already mapped - ignoring\n", CpuIndex));
				return STM_SUCCESS;
			}
			else
			{
				DEBUG((EFI_D_ERROR, "%ld PeMapRegionEptHelper ***Warning*** PeMapRegionEptHelper - Replacing Existing L2 pointer (%llx) with superpage Index3: %d Index2: %d Address: 0x%016llx\n", 
					CpuIndex, L2PageTable[Index2].Uint64, Index3, Index2, StartAddress));

				FreePages((UINT64 *)((UINTN)L2PageTable[Index2].Uint64), 1); // free up the L2
			}
		}

		L2PageTable[Index2].Uint64 = StartAddress; 
		L2PageTable[Index2].Bits32.Ra = isRead;
		L2PageTable[Index2].Bits32.Wa = isWrite;
		L2PageTable[Index2].Bits32.Xa = isExec;
		L2PageTable[Index2].Bits32.Emt = GetMemoryTypeTest(StartAddress);
		L2PageTable[Index2].Bits32.Sp  = 1;
#ifdef EPTCHECK      
		DEBUG((EFI_D_ERROR, "%ld PeMapRegionEptHelper - Added 2MB page pointer: %llx L2PageTable[%d]: 0x%016llx\n",
			CpuIndex, StartAddress, Index2, L2PageTable[Index2]));
#endif

		return STM_SUCCESS;
	}

	// if we are at this point we do not have 2MB pages, so
	// now find the relevent PT, and put then page pointer there

	if ((L2PageTable[Index2].Bits32.Ra == 1) ||
		(L2PageTable[Index2].Bits32.Wa == 1) ||
		(L2PageTable[Index2].Bits32.Xa == 1))    // is PT assigned?
	{
		L1PageTable = (EPT_ENTRY *)((UINTN)(L2PageTable[Index2].Uint64 & ~OFFSET_BITMASK_4K)); // get address of PTE
#ifdef EPTCHECK
		DEBUG((EFI_D_ERROR, "%ld PeMapRegionEptHelper - Found L1PageTable[%d]: %llx L2TableAddress: %llx\n", CpuIndex,
			Index2, L2PageTable[Index2], L2PageTable));
#endif
		if(L2PageTable[Index2].Bits32.Sp == 1) // is the existing page a 2MB page
		{
			DEBUG((EFI_D_ERROR, "%ld PeMapRegionEptHelper ***Warning*** attempt to reuse a 2MB pointer as a page table - request ignored\n", CpuIndex));
			return STM_SUCCESS;                // then return, because the permissions will be the same in the any can
			// also this closes a potential hole where SMM memory could be mappe
		}
	}
	else        // no PT assigned - go build one.
	{
		// allocate and initialize a PT (Level 1)

		L1PageTable = AllocatePages(1);
		if (NULL == L1PageTable)     // Make sure that there were no problems
		{
			DEBUG((EFI_D_ERROR, " %ld PeMapRegionEptHelper - Alloc EPT Page tables failed, no PTE or pLink\n", CpuIndex));
			return(ALLOC_FAILURE);
		}          
		// set the pt into the page table directory

		PhysAddr = membase & ~OFFSET_BITMASK_4K; 
		L2PageTable[Index2].Uint64 = (UINT64)L1PageTable; 
		L2PageTable[Index2].Bits32.Ra = 1;
		L2PageTable[Index2].Bits32.Wa = 1;
		L2PageTable[Index2].Bits32.Xa = 1;
		// set new address
#ifdef EPTCHECK
		DEBUG((EFI_D_ERROR, "%ld PeMapRegionEptHelper - Added L1PageTable[%d]: %llx L2TableAddress %llx\n",
			CpuIndex,
			Index2, L2PageTable[Index2], L2PageTable));
#endif
	}

	// at this point, we have a page table. It has been linked to the PD, but
	// the contents of the PT itself are scrubbed 
#ifdef OLD_PHYSBASE
	if(physbase != 0)
	{
		// caller wants us to map guest physical to a different hardware address

		insertPhysAdd(L1PageTable, physbase, Index1, memsize);
	}
#endif

	////////////////////////////Fill L1 Page Table//////////////////////////////////
	PhysAddr = membase;  // was zero
	do
	{
		int TableMod = 0;
		EPT_ENTRY  L1PageTableSave;
		// mark page as present, writable and permanent as required

		L1PageTableSave = L1PageTable[Index1];   // Just in case
		if(L1PageTable[Index1].Uint64 !=0)
		{
			TableMod = 1;         
		}

		L1PageTable[Index1].Bits32.Ra = isRead;
		L1PageTable[Index1].Bits32.Wa = isWrite;
		L1PageTable[Index1].Bits32.Xa = isExec;

		memsize -= PAGE_SIZE_4K;                    // decrement memory size
		if(physbase != 0)
		{
			L1PageTable[Index1].Bits.Addr = (physbase>>12);
			L1PageTable[Index1].Bits32.Emt = GetMemoryTypeTest(physbase);            
#ifdef EPTCHECK
			DEBUG((EFI_D_ERROR, "%ld PeMapRegionEptHelper -- physbase (Virt->Physical) %llx\n", CpuIndex, physbase));
#endif
			physbase += PAGE_SIZE_4K;     // work our way down the physical map
		}
		else
		{
			L1PageTable[Index1].Bits.Addr = (PhysAddr>>12);
			L1PageTable[Index1].Bits32.Emt = GetMemoryTypeTest(PhysAddr);

#ifdef EPTCHECK
			DEBUG((EFI_D_ERROR, "%ld PeMapRegionEptHelper -- PhysAddr (1-1 virt physical) %llx\n", CpuIndex, PhysAddr));
#endif
			PhysAddr += PAGE_SIZE_4K;
		}
		if(TableMod == 1)
		{
			DEBUG((EFI_D_ERROR, "%ld PeMapRegionEptHelper ***Warning*** Modifying existing L1 pagetable entry(%llx) Index3: %d, Index2: %d, Index1: %d Entry: %llx memsize: %llx\n",
				CpuIndex,
				L1PageTable[Index1].Uint64, Index3, Index2, Index1, L1PageTable[Index1], memsize));
			L1PageTable[Index1] = L1PageTableSave;
		}
#ifdef EPTCHECK
		else
		{
			DEBUG((EFI_D_ERROR, "%ld PeMapRegionEptHelper - L1PageTable[%d]: %llx\n", CpuIndex, Index1, L1PageTable[Index1]));
		}
#endif
		Index1++;
	} while ((Index1 < PTE_COUNT) && (memsize > 0));
	///////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////
	// when we get here, we've either exhausted our PT entries, or we've
	// exhausted the caller's memory requirement. If we ran out of PT
	// entries, recurse into us to do the next PT. If we've run out of
	// memory, we're done!

	if ((Index1 >= PTE_COUNT) &&
		(memsize > 0))
	{
		membase += (PAGE_SIZE_4K * PTE_COUNT);      // add a pagetable's worth of memory
		membase &= ~(PAGE_SIZE_4K * PTE_COUNT -1);  // then round down to PT boudary
		// recursion control. In order to limit stack use, we're going to limit
		// recursion to two levels. To do this, we'll only recurse with 1 PT worth of
		// data, then pop back to this level, and go back in for more.
		{
			UINTN   newmemsize;

			do
			{
				if (memsize > (PAGE_SIZE_4K * PTE_COUNT))
				{
					newmemsize = (PAGE_SIZE_4K * PTE_COUNT);
				}
				else
				{
					newmemsize = memsize;   // we can do it all in one call
				}
				rc = PeMapRegionEptHelper(VmType,
					membase,
					newmemsize,
					physbase,
					isRead,
					isWrite,
					isExec,
					TRUE,
					CpuIndex);

				if (rc != STM_SUCCESS)
				{
					DEBUG((EFI_D_ERROR, "%ld PeMapRegionEptHelper ***ERROR*** Could not map %llx\n", CpuIndex, membase));
					return(rc);
				}
				memsize -= newmemsize;
				membase += (PAGE_SIZE_4K * PTE_COUNT);
			} while (memsize > 0);
		}
	}

	return(rc);
}
#ifdef OLDPHYSBASE
void insertPhysAdd(EPT_ENTRY * L1PageTable, UINTN StartAddress, UINTN StartIndex, UINTN size)
{
	UINTN i;
	UINTN pte_count = (size >> 12) + 1; //zero size means one page
	UINTN k;

	DEBUG((EFI_D_ERROR, "   insertPhysAdd PTE: %llx StartAddress: %llx StartIndex: %llx, size: %x\n",
		L1PageTable, StartAddress, StartIndex, size));

	pte_count = pte_count + StartIndex;    // shift over to where the module should start

	if(pte_count > PTE_COUNT)
		pte_count = PTE_COUNT;        // make sure we stay on this page

	StartAddress = StartAddress & ~OFFSET_BITMASK_4K;  // make sure we take out unnecessary bits

	for (i=0, k=StartIndex; k < pte_count; i++, k++)
	{
		L1PageTable[k].Uint64 = (StartAddress + (i << 12));   // mark all PTEs as not present
	}

}
#endif
#endif

void PeEPTViolationHandler( IN UINT32 CpuIndex)
{
	VM_EXIT_QUALIFICATION VmexitQualification;
	char AccessAllowed[4];
	char AccessRequested[4];
	char * LinearAddressValid;
	char * AddressViolation;
	char Line[100];

	EndTimeStamp = AsmReadTsc();

	VmexitQualification.UintN = VmReadN(VMCS_N_RO_EXIT_QUALIFICATION_INDEX);

	DEBUG((EFI_D_ERROR, "%ld PeEPTViolationHandler - PE EPT Violation VMEXIT - Exit Qual: 0x%llx\n", 
		CpuIndex, VmexitQualification.UintN));

	//rc = vmexitEPT_Violation(VmexitQualification, pStmVmm);

	// at this moment the PE VM memory is already set, so
	// we expect no new addiions

	DEBUG((EFI_D_ERROR, "%ld PeEPTViolationHandler - Protected Execution VM attempted to access protected MEMORY at 0x%016llx\n",
		CpuIndex,
		VmRead64(VMCS_64_RO_GUEST_PHYSICAL_ADDR_INDEX)));

	// have to generate the error messages this way - DEBUG function does not like conditionals

	if(VmexitQualification.EptViolation.EptR == 1)
		AccessAllowed[0] = 'R';
	else
		AccessAllowed[0] = '-';

	if(VmexitQualification.EptViolation.EptW == 1)
		AccessAllowed[1] = 'W';
	else
		AccessAllowed[1] = '-';

	if(VmexitQualification.EptViolation.EptX == 1)
		AccessAllowed[2] = 'X';
	else
		AccessAllowed[2] = '-';
	AccessAllowed[3] = '\0';

	//sprintf(Line, "%ld PeEPTViolationHandler - Access Allowed: %s\n", CpuIndex, AccessAllowed);

	Line[0] = '\0';
	strcat(Line, "PeEPTViolationHandler - Access Allowed: ");
	strcat(Line, AccessAllowed);
	strcat(Line, "\n");
	DEBUG((EFI_D_ERROR, Line));

	//DEBUG((EFI_D_ERROR, "%ld PeEPTViolationHandler - Access Allowed: %s\n", CpuIndex, AccessAllowed));

	if(VmexitQualification.EptViolation.Ra == 1)
		AccessRequested[0] = 'R';
	else
		AccessRequested[0] = '-';

	if(VmexitQualification.EptViolation.Wa == 1)
		AccessRequested[1] = 'W';
	else
		AccessRequested[1] = '-';

	if(VmexitQualification.EptViolation.Xa == 1)
		AccessRequested[2] = 'X';
	else
		AccessRequested[2] = '-';

	AccessRequested[3] = '\0';

	Line[0] = '\0';
    strcat(Line, "PeEPTViolationHandler - Access Attempted causing Violation: ");
	//sprintf(Line, "%ld PeEPTViolationHandler - Access Attempted: %s\n", CpuIndex, AccessRequested);
	strcat(Line, AccessRequested);
	strcat(Line, "\n");

    DEBUG((EFI_D_ERROR, Line));

	if(VmexitQualification.EptViolation.GlaValid == 1)
		LinearAddressValid = "Valid";
	else
		LinearAddressValid = "Invalid";
	Line[0] = '\0';
	strcat(Line, "PeEPTViolationHandler - Linear address is ");
	strcat(Line, LinearAddressValid);
	strcat(Line, "\n");

	//sprintf(Line, "%ld PeEPTViolationHandler - Linear address is %s\n", CpuIndex, LinearAddressValid);

    DEBUG((EFI_D_ERROR, Line));
	
	if(VmexitQualification.EptViolation.GlaValid == 1)
	{
		if(VmexitQualification.EptViolation.Gpa == 1)
			AddressViolation = "Guest Physical EPT violation ";
		else
			AddressViolation = "Paging structure error";

    Line[0] = '\0';
	strcat(Line, "PeEPTViolationHandler - Linear address is ");
	strcat(Line, AddressViolation);
	strcat(Line, "\n");
	//sprintf(Line, "%ld PeEPTViolationHandler - Linear address is %s\n", CpuIndex, AddressViolation);

	DEBUG((EFI_D_ERROR, Line));
	}
	// take down the VM
	//breakdownNonSmmVM(PE_VM_BAD_ACCESS, pStmVmm);
	PostPeVmProc(PE_VM_BAD_ACCESS , CpuIndex, RELEASE_VM);
	// rc = STM_SUCCESS;  // get past the fatal error stuff
	return;
}

void PeEPTMisconfigurationHandler( IN UINT32 CpuIndex)
{		
	EndTimeStamp = AsmReadTsc();
	DEBUG((EFI_D_ERROR, "%ld PeEPTMisconfigurationHandler - PE EPT Misconfiguration VMEXIT\n", CpuIndex));
	DumpVmcsAllField ();
	DEBUG((EFI_D_ERROR, "ld PeEPTMisconfigurationHandler - CpuDeadLoop\n", CpuIndex));
	CpuDeadLoop();
	return;
}

void PeInvEPTHandler( IN UINT32 CpuIndex)
{
	EndTimeStamp = AsmReadTsc();
	DEBUG((EFI_D_ERROR, "%ld PeInvEPTHandler - PE - Invalid EPT Handler not implemented \n", CpuIndex));
	DEBUG((EFI_D_ERROR, "%ld PeInvEPTHandler - CpuDeadLoop\n", CpuIndex));
	CpuDeadLoop();
	return;
}


// function to free EPT tables - assume EPT pointer to tables created by PeEPTinit

static void PeEptFreeL3(UINT64 Level3);

void PeEptFree(IN UINT64 EptPointer)
{

	// calculate the size of the L4 table - the rest of the tables can be freed on
	// the basis of the pointers

	UINTN                    NumberOfPml4EntriesNeeded;
	UINTN				L3Entry;

	EPT_ENTRY * L4Table = (EPT_ENTRY *)((UINTN)(EptPointer & ~0xFFF));

	DEBUG ((EFI_D_ERROR, "PeEptFree Entered: %llx\n", L4Table));

	if (mHostContextCommon.PhysicalAddressBits <= 39) {
		NumberOfPml4EntriesNeeded = 1;
		//NumberOfPdpEntriesNeeded = (UINTN)LShiftU64 (1, mHostContextCommon.PhysicalAddressBits - 30);
	} else {
		NumberOfPml4EntriesNeeded = (UINTN)LShiftU64 (1, mHostContextCommon.PhysicalAddressBits - 39);
		//NumberOfPdpEntriesNeeded = 512;
	}

	for(L3Entry = 0; L3Entry < NumberOfPml4EntriesNeeded; L3Entry++)
	{
		if(L4Table[L3Entry].Uint64 != 0)
		{
			PeEptFreeL3((UINT64) L4Table[L3Entry].Uint64);
		}
	}

	// Finally - free the L4 table

	FreePages(L4Table, 1);
}

static void PeEptFreeL3(UINT64 Level3)
{
	UINTN L2Entry;

	EPT_ENTRY * L3Table = (EPT_ENTRY *) ((UINTN)(Level3 & ~0xFFF));

	DEBUG ((EFI_D_ERROR, "PeEptFreeL3 Entered: %llx\n", L3Table));
	for(L2Entry = 0; L2Entry < 512; L2Entry++)
	{
		if(L3Table[L2Entry].Uint64 != 0)
		{
			if(L3Table[L2Entry].Bits32.Sp != 1)
			{
				PeEptFreeL2(L3Table[L2Entry].Uint64);
			}
		}
	}
	FreePages(L3Table, 1);
}

static void PeEptFreeL2(IN UINT64 Level2)
{
	UINTN L1Entry;

	EPT_ENTRY * L2Table = (EPT_ENTRY *) ((UINTN)(Level2 & ~0xFFF));
	DEBUG ((EFI_D_ERROR, "PeEptFreeL2 Entered: %llx\n", L2Table));
	for(L1Entry = 0; L1Entry < 512; L1Entry ++)
	{
		if(L2Table[L1Entry].Uint64 != 0)
		{
			if(L2Table[L1Entry].Bits32.Sp != 1)
			{
				EPT_ENTRY * L1Table = (EPT_ENTRY *) ((UINTN)(L2Table[L1Entry].Uint64 & ~0xFFF));
				DEBUG((EFI_D_ERROR, "PeEptFreeL2 - L1Table: 0x%016llx freed L1Entry: %d\n", L1Table, L1Entry));
				FreePages(L1Table, 1);
			}
		}
	}
	FreePages(L2Table, 1);
}
