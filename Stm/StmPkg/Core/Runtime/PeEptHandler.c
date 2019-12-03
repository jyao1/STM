/** @file

EPT Handler functions specific to a VM/PE

This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/
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

#define memcpy CopyMem

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

/**

This function create EPT l4/L3 tables for VM/PE guest.
L2 and L1 are added during the VM/PE build

@param EptPointer EPT pointer
@param Xa         Execute access

**/

//EptCreatePageTable (&mGuestContextCommonSmm[VmType].EptPointer);
INT32 PeEptInit (OUT EPT_POINTER  *EptPointer)
{
	EptCreatePageTable (EptPointer, 1);

	return 0;
}

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

	//Line[0] = '\0';
#define Message1 "PeEPTViolationHandler - Access Allowed: "
        int count = sizeof(Message1);
        memcpy(Line, Message1, count - 1);
        memcpy(&Line[count], AccessAllowed, 3);
        count = count + 3;
        Line[count] = '\n';
	Line[count+ 1] = '\0';
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

	///Line[0] = '\0';
#define Message2 "PeEPTViolationHandler - Access Attempted causing Violation: "
        count = sizeof(Message2);
	//sprintf(Line, "%ld PeEPTViolationHandler - Access Attempted: %s\n", CpuIndex, AccessRequested);
	memcpy(Line, Message2, count - 1 ); // account for null
	memcpy(&Line[count], AccessRequested, 3);
        count = count + 3;
        Line[count] = '\n';
	Line[count + 1] = '\0';

    DEBUG((EFI_D_ERROR, Line));

	if(VmexitQualification.EptViolation.GlaValid == 1)
		LinearAddressValid = "Valid  ";
	else
		LinearAddressValid = "Invalid";
	//Line[0] = '\0';
#define Message3 "PeEPTViolationHandler - Linear address is "
	count = sizeof(Message3);
        memcpy(Line, Message3, count);
        memcpy(&Line[count], LinearAddressValid, 7);
        count = count + 7;
        Line[count] = '\n';
        Line[count + 1] = '\0';

	//sprintf(Line, "%ld PeEPTViolationHandler - Linear address is %s\n", CpuIndex, LinearAddressValid);

    DEBUG((EFI_D_ERROR, Line));
	
	if(VmexitQualification.EptViolation.GlaValid == 1)
	{
		if(VmexitQualification.EptViolation.Gpa == 1)
			AddressViolation = "Guest Physical EPT violation ";
		else
			AddressViolation = "Paging structure error       ";

    //Line[0] = '\0';
	//sprintf(Line, "%ld PeEPTViolationHandler - Linear address is %s\n", CpuIndex, AddressViolation);
	count = sizeof(Message3);
        memcpy(Line, Message3, count);
        memcpy(&Line[count], AddressViolation, 28);
        count = count + 28;
        Line[count] = '\n';
        Line[count + 1] = '\0';
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
	DumpVmcsAllField (CpuIndex);
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
