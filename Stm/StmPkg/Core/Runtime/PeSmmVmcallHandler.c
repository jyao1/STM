/** @file

PE/SMM VMCALL Handler

This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmRuntime.h"
#include "PeStm.h"
#include "PeStmEpt.h"
#include "VmcsOffsets.h"

/**

This function translate guest physical address to host address.
found in SmmEptHandler.c

@param EptPointer     EPT pointer
@param Addr           Guest physical address
@param EntryPtr       EPT entry pointer
NULL on output means Entry not found.

@return Host physical address
**/
UINTN
	TranslateEPTGuestToHost (
	IN UINT64      EptPointer,
	IN UINTN       Addr,
	OUT EPT_ENTRY  **EntryPtr  OPTIONAL
	);


extern VMCSFIELDOFFSET VmcsFieldOffsetTable[];
extern void MapVmcs ();
extern PE_VM_DATA PeVmData[4];

/**

This function is the STM_API_MAP_ADDRESS_RANGEVMCALL handler for SMM VM/PE.

@param Index             CPU index
@param AddressParameter  Addresss parameter

@return VMCALL status

**/
STM_STATUS
	PeSmmVmcallMapAddressRangeHandler (
	IN UINT32  Index,
	IN UINT64  AddressParameter
	)
{
	STM_MAP_ADDRESS_RANGE_DESCRIPTOR   *MapAddressRangeDescriptor;
	STM_MAP_ADDRESS_RANGE_DESCRIPTOR   LocalBuffer;
	UINT32 VmType = mHostContextCommon.HostContextPerCpu[Index].GuestVmType;
	UINTN PhysAddressParameter;
	UINTN PhysAddressParameterEnd;
	UINT64 GuestSmmEnd = PeVmData[VmType].UserModule.AddressSpaceStart + PeVmData[VmType].UserModule.AddressSpaceSize - 1;

	// Make sure the parameter address is with the part of the guest that is within SMRAM

	if((AddressParameter < PeVmData[VmType].UserModule.AddressSpaceStart)||
	    (AddressParameter > GuestSmmEnd) ||
		((AddressParameter + sizeof(STM_MAP_ADDRESS_RANGE_DESCRIPTOR)) > GuestSmmEnd))
	{
		DEBUG ((EFI_D_ERROR, "%ld PeSmmVmcallMapAddressRangeHandler - Security Violation! - parameter block not in guest physical within SMRAM\n", Index));
		DEBUG ((EFI_D_ERROR, "%ld PeSmmVmcallMapAddressRangeHandler - AddressParameter = 0x%016llx",
			Index,
			AddressParameter));
		return ERROR_STM_SECURITY_VIOLATION;
	}

	PhysAddressParameter = TranslateEPTGuestToHost(mGuestContextCommonSmm[VmType].EptPointer.Uint64, (UINTN)AddressParameter, 0L);
	PhysAddressParameterEnd = TranslateEPTGuestToHost(mGuestContextCommonSmm[VmType].EptPointer.Uint64, (UINTN)AddressParameter + sizeof(STM_MAP_ADDRESS_RANGE_DESCRIPTOR), 0L);

	DEBUG((EFI_D_INFO, "%ld PeSmmVmcallMapAddressRangeHandler - STM_API_MAP_ADDRESS_RANGE started\n", Index));

	if(((PhysAddressParameter == 0)||(PhysAddressParameterEnd == 0))||
		((PhysAddressParameter & ~0xFFF) != (PhysAddressParameterEnd & ~0XFFF))) 
	{
		// TODO - need to address the potential of having a parameter block split across two pages
		// currently the VM/PE is created as a single block...

		DEBUG ((EFI_D_ERROR, "%ld PeSmmVmcallMapAddressRangeHandler - Security Violation! - parameter block not in guest physical address space or split across two pages\n", Index));
		DEBUG ((EFI_D_ERROR, "%ld PeSmmVmcallMapAddressRangeHandler - PhysAddressParameter = 0x%016llx, PhysAddressParameterEnd = 0x%016llx\n",
			Index,
			PhysAddressParameter, PhysAddressParameterEnd));
		return ERROR_STM_SECURITY_VIOLATION;
	}

	//
	// Copy data to local, to prevent time of check VS time of use attack
	//

	CopyMem (&LocalBuffer, (VOID *)(UINTN)PhysAddressParameter, sizeof(LocalBuffer));
	MapAddressRangeDescriptor = (STM_MAP_ADDRESS_RANGE_DESCRIPTOR *)&LocalBuffer;

	DEBUG((EFI_D_ERROR, "%ld PeSmmVmcallMapAddressRangeHandler - MapAddressRange base: 0x%016llx Pages:0x%016llx\n", Index, MapAddressRangeDescriptor->PhysicalAddress, MapAddressRangeDescriptor->PageCount));

	if (!IsGuestAddressValid ((UINTN)MapAddressRangeDescriptor->PhysicalAddress, STM_PAGES_TO_SIZE(MapAddressRangeDescriptor->PageCount), TRUE)) 
	{
		DEBUG ((EFI_D_ERROR, "%ld PeSmmVmcallMapAddressRangeHandler [ Security Violation!\n", Index));
		return ERROR_STM_SECURITY_VIOLATION;
	}

	if (MapAddressRangeDescriptor->PageCount == 0) 
	{
		DEBUG((EFI_D_ERROR, "%ld PeSmmVmcallMapAddressRangeHandler - Error - zero address range requested\n", Index));
		return ERROR_STM_PAGE_NOT_FOUND;
	}

	if (((MapAddressRangeDescriptor->PatCacheType > UC) && (MapAddressRangeDescriptor->PatCacheType != FOLLOW_MTRR)) ||
		(MapAddressRangeDescriptor->PatCacheType == 2) ||
		(MapAddressRangeDescriptor->PatCacheType == 3) ) 
	{
		DEBUG((EFI_D_ERROR, "%ld PeSmmVmcallMapAddressRangeHandler - Error - STM cache type not supported\n", Index));
		return ERROR_STM_CACHE_TYPE_NOT_SUPPORTED;
	}

	// for VM/PE we map guest physcal to host physical - BUG, this should be consolodated...

	EPTSetPageAttributeRange(
		mGuestContextCommonSmm[VmType].EptPointer.Uint64, 
		(UINTN) MapAddressRangeDescriptor->PhysicalAddress,
		(UINTN) STM_PAGES_TO_SIZE(MapAddressRangeDescriptor->PageCount),
		(UINTN) MapAddressRangeDescriptor->PhysicalAddress,
		TRUE,   /* Read    */
		FALSE,  /* Write   */
		FALSE,  /* Execute */
		EptPageAttributeSet,
		-1);	

	return STM_SUCCESS;
}

/* STM/PE get VMCS Map */


STM_STATUS
	PeSmmVmcallGetVmcsMap (
	IN UINT32  Index,
	IN UINT64  AddressParameter
	)
{
	UINT32 VmType = mHostContextCommon.HostContextPerCpu[Index].GuestVmType;
	UINTN PhysAddressParameter;
	UINTN PhysAddressParameterEnd;
	UINT32 count;
	void * VTable;

	// figure out how big the VmcsFieldOffsetTable id

	for( count = 0; 
		VmcsFieldOffsetTable[count].FieldEncoding != 0xFFFF;
		count++){}
	count++;   // count the last element

	PhysAddressParameter = TranslateEPTGuestToHost(mGuestContextCommonSmm[VmType].EptPointer.Uint64, (UINTN)AddressParameter, 0L);
	PhysAddressParameterEnd = TranslateEPTGuestToHost(mGuestContextCommonSmm[VmType].EptPointer.Uint64, (UINTN)AddressParameter + (sizeof(VMCSFIELDOFFSET) * count), 0L);

	// EBX:ECX - STM_MAP_ADDRESS_RANGE_DESCRIPTOR
	DEBUG ((EFI_D_INFO, "%ld PE-STM_API_GET_VMCS_MAP: AddressParameter: 0x%016llx PhysAddressParameter: 0x%016llx\n", 
		Index, AddressParameter, PhysAddressParameter));

	// bug bug - need to make sure address is in part of the app that is in SMRAM..

#ifdef FIXME
	if(((PhysAddressParameter == 0)||(PhysAddressParameterEnd == 0))||
		((PhysAddressParameter & ~0xFFF) != (PhysAddressParameterEnd & ~0XFFF)))
	{
		// TODO - need to address the potential of having a parameter block split across tow oages
		DEBUG ((EFI_D_ERROR, "%ld Security Violation! - parameter block not in guest physical address space or split across two pages\n",
			Index));
		DEBUG ((EFI_D_ERROR, "%ld PhysAddressParameter = 0x%016llx, PhysAddressParameterEnd = 0x%016llx\n",
			Index,
			PhysAddressParameter, PhysAddressParameterEnd));
		return ERROR_STM_SECURITY_VIOLATION;
	}
#endif

	
	MapVmcs ();

	//
	// Copy data to local, to prevent time of check VS time of use attack
	//

	DEBUG((EFI_D_ERROR, "%ld Size of VMCSFIELDOFFSET buffer is 0x%lx\n", Index, (sizeof(VMCSFIELDOFFSET) * count)));

	VTable = (void *) VmcsFieldOffsetTable;
	CopyMem((VOID *)(UINTN)PhysAddressParameter, VTable , (sizeof(VMCSFIELDOFFSET) * count));

	return STM_SUCCESS;
}

/*    VM/PE allowed vmcalls  */

STM_VMCALL_HANDLER_STRUCT  mPeSmmVmcallHandler[] = {
	{STM_API_MAP_ADDRESS_RANGE,                  PeSmmVmcallMapAddressRangeHandler},
	{STM_API_GET_VMCS_MAP,						 PeSmmVmcallGetVmcsMap},
};

/* not defined yet for STM/PE VM/PE  */
/*
{STM_API_UNMAP_ADDRESS_RANGE,                },
{STM_API_ADDRESS_LOOKUP,                     },
{STM_API_RETURN_FROM_PROTECTION_EXCEPTION,   },
*/
/**

This function returns SMM VMCALL handler by FuncIndex.

@param FuncIndex         VmCall function index

@return VMCALL Handler

**/
STM_VMCALL_HANDLER
	GetPeSmmVmcallHandlerByIndex (
	IN UINT32  FuncIndex
	)
{
	UINTN  Index;
	for (Index = 0; Index < sizeof(mPeSmmVmcallHandler)/sizeof(mPeSmmVmcallHandler[0]); Index++) 
	{
		if (mPeSmmVmcallHandler[Index].FuncIndex == FuncIndex) 
		{
			return mPeSmmVmcallHandler[Index].StmVmcallHandler;
		}
	}
	return NULL;
}

/**

This function is VMCALL handler for SMM.

@param Index CPU index

**/
VOID
	PeSmmVmcallHandler (
	IN UINT32  Index
	)
{
	X86_REGISTER                       *Reg;
	STM_STATUS                         Status;
	STM_VMCALL_HANDLER                 StmVmcallHandler;
	UINT64                             AddressParameter;
	UINT32							   VmType;

	//DEBUG((EFI_D_INFO, "%ld PeSmmVmcallHandler - start\n", Index));
	VmType = mHostContextCommon.HostContextPerCpu[Index].GuestVmType;

	Reg = &mGuestContextCommonSmm[VmType].GuestContextPerCpu[0].Register;

	StmVmcallHandler = GetPeSmmVmcallHandlerByIndex (ReadUnaligned32 ((UINT32 *)&Reg->Rax));
	if (StmVmcallHandler == NULL) {
		DEBUG ((EFI_D_INFO, "%ld PeSmmVmcallHandler - GetPeSmmVmcallHandlerByIndex - 0x%llx!\n", 
			Index, (UINTN)ReadUnaligned32 ((UINT32 *)&Reg->Rax)));
		// Should not happen
		//CpuDeadLoop ();
		Status = ERROR_INVALID_API;
	} else {
		AddressParameter = ReadUnaligned32 ((UINT32 *)&Reg->Rbx) + LShiftU64 (ReadUnaligned32 ((UINT32 *)&Reg->Rcx), 32);
		Status = StmVmcallHandler (Index, AddressParameter);
	}

	WriteUnaligned32 ((UINT32 *)&Reg->Rax, Status);
	if (Status == STM_SUCCESS) {
		VmWriteN (VMCS_N_GUEST_RFLAGS_INDEX, VmReadN(VMCS_N_GUEST_RFLAGS_INDEX) & ~RFLAGS_CF);
	} else {
		VmWriteN (VMCS_N_GUEST_RFLAGS_INDEX, VmReadN(VMCS_N_GUEST_RFLAGS_INDEX) | RFLAGS_CF);
	}
	VmWriteN (VMCS_N_GUEST_RIP_INDEX, VmReadN (VMCS_N_GUEST_RIP_INDEX) + VmRead32 (VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX));

	return ;
}
