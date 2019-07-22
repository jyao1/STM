/** @file
  SMM VMCALL handler

  Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmRuntime.h"
#include "PeStm.h"

/**

  This function issue TXT reset.

  @param ErrorCode            TXT reset error code

**/
VOID
StmTxtReset (
  IN UINT32  ErrorCode
  )
{
	DEBUG((EFI_D_ERROR, "StmTxTReset issued - Error Code: %d\n", ErrorCode));
  if (IsSentryEnabled()) {
    // TXT reset
    TxtPriWrite32 (TXT_ERRORCODE, ErrorCode);
    TxtPriWrite32 (TXT_CMD_SYS_RESET, 1);
  }
  // Power Cycle reset
  IoWrite8 (0xCF9, 0xE);
  // Should not run here
  CpuDeadLoop ();
}

/**

  This function validate input address region.
  Address region is not allowed to overlap with MSEG.
  Address region is not allowed to exceed STM accessable region.

  @param Address              Address to be validated
  @param Length               Address length to be validated
  @param FromProtectedDomain  If this request is from protected domain
                              TRUE means this address region is allowed to overlap with MLE protected region.
                              FALSE means this address region is not allowed to overlap with MLE protected region.

  @retval TRUE  Validation pass
  @retval FALSE Validation fail

**/
BOOLEAN
IsGuestAddressValid (
  IN UINTN    Address,
  IN UINTN    Length,
  IN BOOLEAN  FromProtectedDomain
  )
{
  STM_RSC  ResourceNode;

  //
  // Check max address
  //
  if ((Length > mHostContextCommon.MaximumSupportAddress) ||
      (Address > mHostContextCommon.MaximumSupportAddress) ||
      ((Length != 0) && (Address > (mHostContextCommon.MaximumSupportAddress - (Length - 1)))) ) {
    //
    // Overflow happen
    // NOTE: (B:0->L:4G) is invalid for IA32, but (B:1->L:4G-1)/(B:4G-1->L:1) is valid.
    //
    DEBUG ((EFI_D_ERROR, "IsGuestAddressValid - Max - %16lx\n", mHostContextCommon.MaximumSupportAddress + 1));
    DEBUG ((EFI_D_ERROR, "IsGuestAddressValid - Address - %x\n", Address));
    DEBUG ((EFI_D_ERROR, "IsGuestAddressValid - Length - %x\n", Length));
    DEBUG ((EFI_D_ERROR, "IsGuestAddressValid - Address + Length - %lx\n", (UINT64)Address+(UINT64)Length));
    DEBUG ((EFI_D_ERROR, "IsGuestAddressValid - Check max address\n"));
    return FALSE;
  }
  if (IsOverlap (Address, Length, (UINTN)mHostContextCommon.StmHeader, mHostContextCommon.StmSize)) {
    // Overlap MSEG
    DEBUG ((EFI_D_ERROR, "IsGuestAddressValid - Overlap MSEG\n"));
    return FALSE;
  }

  if (FromProtectedDomain) {
    // Overlap TSEG, check for MLE only
    if (IsOverlap (Address, Length, mHostContextCommon.TsegBase, mHostContextCommon.TsegLength)) {
      // Overlap TSEG
      DEBUG ((EFI_D_ERROR, "IsGuestAddressValid - Overlap TSEG\n"));
      return FALSE;
    }
  }

  if (FromProtectedDomain) {
    return TRUE;
  }

  //
  // Check MLE protected resource
  //
  ZeroMem (&ResourceNode, sizeof(ResourceNode.Mem));
  ResourceNode.Mem.Hdr.RscType = MEM_RANGE;
  ResourceNode.Mem.Hdr.Length = (UINT16)sizeof(ResourceNode.Mem);
  ResourceNode.Mem.Base = Address;
  ResourceNode.Mem.Length = Length;
  ResourceNode.Mem.RWXAttributes = STM_RSC_MEM_R | STM_RSC_MEM_W | STM_RSC_MEM_X;
  if (IsResourceListOverlapWithNode (&ResourceNode, mHostContextCommon.MleProtectedResource.Base)) {
    DEBUG ((EFI_D_ERROR, "IsGuestAddressValid - MLE protected resource\n"));
    return FALSE;
  }

  //
  // Besides MEM_RANGE, we also need check MMIO_RANGE because MEM and MMIO use same way for protection.
  //
  ResourceNode.Mmio.Hdr.RscType = MMIO_RANGE;
  ResourceNode.Mmio.Hdr.Length = (UINT16)sizeof(ResourceNode.Mem);
  ResourceNode.Mmio.Base = Address;
  ResourceNode.Mmio.Length = Length;
  ResourceNode.Mmio.RWXAttributes = STM_RSC_MMIO_R | STM_RSC_MMIO_W | STM_RSC_MMIO_X;
  if (IsResourceListOverlapWithNode (&ResourceNode, mHostContextCommon.MleProtectedResource.Base)) {
    DEBUG((EFI_D_ERROR, "IsGuestAddressValid - MLE protected resource\n"));
    return FALSE;
  }

  return TRUE;
}

/**

  This function is VMCALL handler for SMM.

  @param Index             CPU index
  @param AddressParameter  Addresss parameter

  @return VMCALL status

**/
STM_STATUS
SmmVmcallMapAddressRangeHandler (
  IN UINT32  Index,
  IN UINT64  AddressParameter
  )
{
  STM_MAP_ADDRESS_RANGE_DESCRIPTOR   *MapAddressRangeDescriptor;
  STM_MAP_ADDRESS_RANGE_DESCRIPTOR   LocalBuffer;

  // EBX:ECX - STM_MAP_ADDRESS_RANGE_DESCRIPTOR
  DEBUG ((EFI_D_INFO, "STM_API_MAP_ADDRESS_RANGE:\n"));
  if (!IsGuestAddressValid ((UINTN)AddressParameter, sizeof(STM_MAP_ADDRESS_RANGE_DESCRIPTOR), FALSE)) {
    DEBUG ((EFI_D_ERROR, "Security Violation!\n"));
    return ERROR_STM_SECURITY_VIOLATION;
  }

  //
  // Copy data to local, to prevent time of check VS time of use attack
  //
  CopyMem (&LocalBuffer, (VOID *)(UINTN)AddressParameter, sizeof(LocalBuffer));
  MapAddressRangeDescriptor = (STM_MAP_ADDRESS_RANGE_DESCRIPTOR *)&LocalBuffer;

  if (!IsGuestAddressValid ((UINTN)MapAddressRangeDescriptor->PhysicalAddress, STM_PAGES_TO_SIZE(MapAddressRangeDescriptor->PageCount), FALSE)) {
    DEBUG ((EFI_D_ERROR, "Security Violation!\n"));
    return ERROR_STM_SECURITY_VIOLATION;
  }

  if (MapAddressRangeDescriptor->PageCount == 0) {
    return ERROR_STM_PAGE_NOT_FOUND;
  }

  if (((MapAddressRangeDescriptor->PatCacheType > UC) && (MapAddressRangeDescriptor->PatCacheType != FOLLOW_MTRR)) ||
      (MapAddressRangeDescriptor->PatCacheType == 2) ||
      (MapAddressRangeDescriptor->PatCacheType == 3) ) {
    return ERROR_STM_CACHE_TYPE_NOT_SUPPORTED;
  }

  MapVirtualAddressToPhysicalAddress (
      Index,
      MapAddressRangeDescriptor->PhysicalAddress,
      (UINTN)MapAddressRangeDescriptor->VirtualAddress,
      MapAddressRangeDescriptor->PageCount
      );

  return STM_SUCCESS;
}

/**

  This function is VMCALL handler for SMM.

  @param Index             CPU index
  @param AddressParameter  Addresss parameter

  @return VMCALL status

**/
STM_STATUS
SmmVmcallUnmapAddressRangeHandler (
  IN UINT32  Index,
  IN UINT64  AddressParameter
  )
{
  STM_UNMAP_ADDRESS_RANGE_DESCRIPTOR *UnmapAddressRangeDescriptor;
  STM_UNMAP_ADDRESS_RANGE_DESCRIPTOR LocalBuffer;

  // EBX:ECX - STM_UNMAP_ADDRESS_RANGE_DESCRIPTOR
  DEBUG ((EFI_D_INFO, "STM_API_UNMAP_ADDRESS_RANGE:\n"));
  if (!IsGuestAddressValid ((UINTN)AddressParameter, sizeof(STM_UNMAP_ADDRESS_RANGE_DESCRIPTOR), FALSE)) {
    DEBUG ((EFI_D_ERROR, "Security Violation!\n"));
    return ERROR_STM_SECURITY_VIOLATION;
  }

  //
  // Copy data to local, to prevent time of check VS time of use attack
  //
  CopyMem (&LocalBuffer, (VOID *)(UINTN)AddressParameter, sizeof(LocalBuffer));
  UnmapAddressRangeDescriptor = (STM_UNMAP_ADDRESS_RANGE_DESCRIPTOR *)&LocalBuffer;

  UnmapVirtualAddressToPhysicalAddress (
      Index,
      (UINTN)UnmapAddressRangeDescriptor->VirtualAddress,
      STM_SIZE_TO_PAGES (UnmapAddressRangeDescriptor->Length)
      );

  return STM_SUCCESS;
}

/**

  This function is VMCALL handler for SMM.

  @param Index             CPU index
  @param AddressParameter  Addresss parameter

  @return VMCALL status

**/
STM_STATUS
SmmVmcallAddressLookupHandler (
  IN UINT32  Index,
  IN UINT64  AddressParameter
  )
{
  STM_ADDRESS_LOOKUP_DESCRIPTOR      *AddressLookupDescriptor;
  UINTN                              PhysicalAddress;
  BOOLEAN                            AddressFound;
  STM_ADDRESS_LOOKUP_DESCRIPTOR      LocalBuffer;

  // EBX:ECX - STM_ADDRESS_LOOKUP_DESCRIPTOR
  DEBUG ((EFI_D_INFO, "STM_API_ADDRESS_LOOKUP:\n"));
  if (!IsGuestAddressValid ((UINTN)AddressParameter, sizeof(STM_ADDRESS_LOOKUP_DESCRIPTOR), FALSE)) {
    DEBUG ((EFI_D_ERROR, "Security Violation!\n"));
    return ERROR_STM_SECURITY_VIOLATION;
  }

  //
  // Copy data to local, to prevent time of check VS time of use attack
  //
  CopyMem (&LocalBuffer, (VOID *)(UINTN)AddressParameter, sizeof(LocalBuffer));
  AddressLookupDescriptor = (STM_ADDRESS_LOOKUP_DESCRIPTOR *)&LocalBuffer;

  DEBUG ((EFI_D_INFO, "  InterruptedGuestVirtualAddress: 0x%016lx\n", AddressLookupDescriptor->InterruptedGuestVirtualAddress));
  DEBUG ((EFI_D_INFO, "  Length:                         0x%08x\n",   AddressLookupDescriptor->Length));
  DEBUG ((EFI_D_INFO, "  InterruptedCr3:                 0x%016x\n",  AddressLookupDescriptor->InterruptedCr3));
  DEBUG ((EFI_D_INFO, "  InterruptedEptp:                0x%016x\n",  AddressLookupDescriptor->InterruptedEptp));
  DEBUG ((EFI_D_INFO, "  MapToSmmGuest:                  0x%08x\n",   AddressLookupDescriptor->MapToSmmGuest));
  DEBUG ((EFI_D_INFO, "  InterruptedCr4Pae:              0x%08x\n",   AddressLookupDescriptor->InterruptedCr4Pae));
  DEBUG ((EFI_D_INFO, "  InterruptedCr4Pse:              0x%08x\n",   AddressLookupDescriptor->InterruptedCr4Pse));
  DEBUG ((EFI_D_INFO, "  InterruptedIa32eMode:           0x%08x\n",   AddressLookupDescriptor->InterruptedIa32eMode));
  DEBUG ((EFI_D_INFO, "  PhysicalAddress:                0x%016lx\n", AddressLookupDescriptor->PhysicalAddress));
  DEBUG ((EFI_D_INFO, "  SmmGuestVirtualAddress:         0x%016lx\n", AddressLookupDescriptor->SmmGuestVirtualAddress));

  AddressLookupDescriptor->PhysicalAddress = LookupSmiGuestVirtualToGuestPhysical (
                                               (UINTN)AddressLookupDescriptor->InterruptedCr3,
                                               (BOOLEAN)AddressLookupDescriptor->InterruptedCr4Pae,
                                               (BOOLEAN)AddressLookupDescriptor->InterruptedCr4Pse,
                                               (BOOLEAN)AddressLookupDescriptor->InterruptedIa32eMode,
                                               (UINTN)AddressLookupDescriptor->InterruptedGuestVirtualAddress
                                               );
  if (AddressLookupDescriptor->PhysicalAddress == 0) {
    return ERROR_STM_BAD_CR3;
  }
  //
  // Guest physical to host physical
  //
  AddressFound = LookupSmiGuestPhysicalToHostPhysical (
                   AddressLookupDescriptor->InterruptedEptp,
                   (UINTN)AddressLookupDescriptor->PhysicalAddress,
                   &PhysicalAddress
                   );
  AddressLookupDescriptor->PhysicalAddress = PhysicalAddress;
  if (!AddressFound) {
    return ERROR_STM_BAD_CR3;
  }

  switch (AddressLookupDescriptor->MapToSmmGuest) {
  case DO_NOT_MAP:
    // PhysicalAddress is populated with the address determined by walking the interrupted environment's page tables
    break;
  case ONE_TO_ONE:
    // PhysicalAddress is populated with the address determined by walking the interrupted environment's page tables
    //   If PhysicalAddress is > 4G, the function fails and returns with CF=1 and EAX = ERROR_STM_PHYSICAL_OVER_4G.
    // SmmGuestVirtualAddress is modified to contain the SMM guest's virtual mapping on output, which is the same as PhysicalAddress
    AddressLookupDescriptor->SmmGuestVirtualAddress = AddressLookupDescriptor->PhysicalAddress;
    if (AddressLookupDescriptor->PhysicalAddress >= BASE_4GB) {
      return ERROR_STM_PHYSICAL_OVER_4G;
    }
    break;
  case VIRTUAL_ADDRESS_SPECIFIED:
    // PhysicalAddress is populated with the address determined by walking the interrupted environment's page tables
    // and mapped to the SMM guest virtual address specified by the SmmGuestVirtualAddress input.
    if (((mGuestContextCommonSmi.GuestContextPerCpu[Index].Efer & IA32_EFER_MSR_MLA) == 0) &&
        ((AddressLookupDescriptor->SmmGuestVirtualAddress >= BASE_4GB) ||
         (AddressLookupDescriptor->SmmGuestVirtualAddress + AddressLookupDescriptor->Length >= BASE_4GB))) {
      return ERROR_STM_VIRTUAL_SPACE_TOO_SMALL;
    }
    MapVirtualAddressToPhysicalAddress (
      Index,
      AddressLookupDescriptor->PhysicalAddress,
      (UINTN)AddressLookupDescriptor->SmmGuestVirtualAddress,
      STM_SIZE_TO_PAGES (AddressLookupDescriptor->Length)
      );
    break;
  default:
    return ERROR_INVALID_API;
  }

  //
  // CopyData back
  //
  ((STM_ADDRESS_LOOKUP_DESCRIPTOR *)(UINTN)AddressParameter)->PhysicalAddress = AddressLookupDescriptor->PhysicalAddress;
  ((STM_ADDRESS_LOOKUP_DESCRIPTOR *)(UINTN)AddressParameter)->SmmGuestVirtualAddress = AddressLookupDescriptor->SmmGuestVirtualAddress;
  return STM_SUCCESS;
}

/**

  This function is VMCALL handler for SMM.

  @param Index             CPU index
  @param AddressParameter  Addresss parameter

  @return VMCALL status

**/
STM_STATUS
SmmVmcallReturnFromProtectionExceptionHandler (
  IN UINT32  Index,
  IN UINT64  AddressParameter
  )
{
  X86_REGISTER                       *Reg;
  UINT32 VmType = SMI_HANDLER;

  Reg = &mGuestContextCommonSmm[VmType].GuestContextPerCpu[Index].Register;

  // EBX = 0: resume SMM guest using register state found on exception stack
  // EBX = 1 to 0x0F: EBX contains a BIOS error code which the STM must record in the TXT.ERRORCODE
  //                  register and subsequently reset the system via TXT.CMD.SYS_RESET. The value of the TXT.ERRORCODE
  //                  register is calculated as follows:
  //                      TXT.ERRORCODE = (EBX & 0x0F) | STM_CRASH_BIOS_PANIC
  // EBX = 0x10 to 0xFFFFFFFF - reserved, do not use. Upper three bytes of EBX are ignored by the STM.
  if (ReadUnaligned32 ((UINT32 *)&Reg->Rbx) == 0) {
    ReturnFromBiosExceptionHandler (Index);
    // Should not return
    CpuDeadLoop ();
  } else {
    // TXT reset
    StmTxtReset ((ReadUnaligned32 ((UINT32 *)&Reg->Rbx) & 0xF) | STM_CRASH_BIOS_PANIC);
  }

  return STM_SUCCESS;
}

STM_VMCALL_HANDLER_STRUCT  mSmmVmcallHandler[] = {
  {STM_API_MAP_ADDRESS_RANGE,                  SmmVmcallMapAddressRangeHandler},
  {STM_API_UNMAP_ADDRESS_RANGE,                SmmVmcallUnmapAddressRangeHandler},
  {STM_API_ADDRESS_LOOKUP,                     SmmVmcallAddressLookupHandler},
  {STM_API_RETURN_FROM_PROTECTION_EXCEPTION,   SmmVmcallReturnFromProtectionExceptionHandler},
};

/**

  This function returns SMM VMCALL handler by FuncIndex.

  @param FuncIndex         VmCall function index

  @return VMCALL Handler

**/
STM_VMCALL_HANDLER
GetSmmVmcallHandlerByIndex (
  IN UINT32  FuncIndex
  )
{
  UINTN  Index;
  for (Index = 0; Index < sizeof(mSmmVmcallHandler)/sizeof(mSmmVmcallHandler[0]); Index++) {
    if (mSmmVmcallHandler[Index].FuncIndex == FuncIndex) {
      return mSmmVmcallHandler[Index].StmVmcallHandler;
    }
  }
  return NULL;
}

/**

  This function is VMCALL handler for SMM.

  @param Index CPU index

**/
VOID
SmmVmcallHandler (
  IN UINT32  Index
  )
{
  X86_REGISTER                       *Reg;
  STM_STATUS                         Status;
  STM_VMCALL_HANDLER                 StmVmcallHandler;
  UINT64                             AddressParameter;
  UINT32							 VmType = SMI_HANDLER;

  Reg = &mGuestContextCommonSmm[VmType].GuestContextPerCpu[Index].Register;

  StmVmcallHandler = GetSmmVmcallHandlerByIndex (ReadUnaligned32 ((UINT32 *)&Reg->Rax));
  if (StmVmcallHandler == NULL) {
    DEBUG((EFI_D_INFO, "%ld SmmVmcallHandler - GetSmmVmcallHandlerByIndex - %x!\n", Index, (UINTN)ReadUnaligned32 ((UINT32 *)&Reg->Rax)));
    // Should not happen
    CpuDeadLoop ();
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
