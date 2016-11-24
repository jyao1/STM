/** @file
  SMM MSR handler

  Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmRuntime.h"

/**

  This function is RDMSR handler for SMM.

  @param Index CPU index

**/
VOID
SmmReadMsrHandler (
  IN UINT32 Index
  )
{
  UINT64            Data64;
  UINT32            MsrIndex;
  X86_REGISTER      *Reg;
  STM_RSC_MSR_DESC  *MsrDesc;
  STM_RSC_MSR_DESC  LocalMsrDesc;
  BOOLEAN           Result;

  Reg = &mGuestContextCommonSmm.GuestContextPerCpu[Index].Register;
  MsrIndex = ReadUnaligned32 ((UINT32 *)&Reg->Rcx);

  MsrDesc = GetStmResourceMsr (mHostContextCommon.MleProtectedResource.Base, MsrIndex);
  if ((MsrDesc != NULL) && (MsrDesc->ReadMask != 0)) {
    DEBUG ((EFI_D_ERROR, "RDMSR (%x) violation!\n", MsrIndex));
    AddEventLogForResource (EvtHandledProtectionException, (STM_RSC *)MsrDesc);
    SmmExceptionHandler (Index);
    CpuDeadLoop ();
  }

  MsrDesc = GetStmResourceMsr ((STM_RSC *)(UINTN)mGuestContextCommonSmm.BiosHwResourceRequirementsPtr, MsrIndex);
  if ((MsrDesc == NULL) || (MsrDesc->ReadMask == 0) || (MsrDesc->KernelModeProcessing == 0)) {
    ZeroMem (&LocalMsrDesc, sizeof(LocalMsrDesc));
    LocalMsrDesc.Hdr.RscType = MACHINE_SPECIFIC_REG;
    LocalMsrDesc.Hdr.Length = sizeof(LocalMsrDesc);
    LocalMsrDesc.MsrIndex = MsrIndex;
    LocalMsrDesc.ReadMask = (UINT64)-1;
    LocalMsrDesc.WriteMask = 0;
    AddEventLogForResource (EvtBiosAccessToUnclaimedResource, (STM_RSC *)&LocalMsrDesc);
  }

//  DEBUG ((EFI_D_INFO, "!!!ReadMsrHandler!!!\n"));

  switch (MsrIndex) {
  case IA32_EFER_MSR_INDEX:
    Data64 = VmRead64 (VMCS_64_GUEST_IA32_EFER_INDEX);
    break;

  case IA32_SYSENTER_CS_MSR_INDEX:
    Data64 = (UINT64)VmRead32 (VMCS_32_GUEST_IA32_SYSENTER_CS_INDEX);
    break;
  case IA32_SYSENTER_ESP_MSR_INDEX:
    Data64 = (UINT64)VmReadN (VMCS_N_GUEST_IA32_SYSENTER_ESP_INDEX);
    break;
  case IA32_SYSENTER_EIP_MSR_INDEX:
    Data64 = (UINT64)VmReadN (VMCS_N_GUEST_IA32_SYSENTER_EIP_INDEX);
    break;

  case IA32_FS_BASE_MSR_INDEX:
    Data64 = (UINT64)VmReadN (VMCS_N_GUEST_FS_BASE_INDEX);
    break;
  case IA32_GS_BASE_MSR_INDEX:
    Data64 = (UINT64)VmReadN (VMCS_N_GUEST_GS_BASE_INDEX);
    break;

  default:
    Result = StmPlatformLibMsrRead (MsrIndex, &Data64);
    if (Result) {
      break;
    }
    //
    // For rest one, we need pass back to BIOS
    //

    Data64 = AsmReadMsr64 (MsrIndex);
    //
    // Need mask read item
    //
    MsrDesc = GetStmResourceMsr (mHostContextCommon.MleProtectedResource.Base, MsrIndex);
    if (MsrDesc != NULL) {
      Data64 &= MsrDesc->ReadMask;
    }

    break;
  }

  Reg->Rax = (UINTN)(UINT32)Data64; // HIGH bits are cleared
  Reg->Rdx = (UINTN)(UINT32)RShiftU64 (Data64, 32); // HIGH bits are cleared

  VmWriteN (VMCS_N_GUEST_RIP_INDEX, VmReadN(VMCS_N_GUEST_RIP_INDEX) + VmRead32(VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX));
  return ;
}

/**

  This function is WRMSR handler for SMM.

  @param Index CPU index

**/
VOID
SmmWriteMsrHandler (
  IN UINT32 Index
  )
{
  UINT64            Data64;
  UINT32            MsrIndex;
  VM_ENTRY_CONTROLS VmEntryControls;
  X86_REGISTER      *Reg;
  STM_RSC_MSR_DESC  *MsrDesc;
  STM_RSC_MSR_DESC  LocalMsrDesc;
  BOOLEAN           Result;

  Reg = &mGuestContextCommonSmm.GuestContextPerCpu[Index].Register;
  MsrIndex = ReadUnaligned32 ((UINT32 *)&Reg->Rcx);

  MsrDesc = GetStmResourceMsr (mHostContextCommon.MleProtectedResource.Base, MsrIndex);
  if ((MsrDesc != NULL) && (MsrDesc->WriteMask != 0)) {
    DEBUG ((EFI_D_ERROR, "WRMSR (%x) violation!\n", MsrIndex));
    AddEventLogForResource (EvtHandledProtectionException, (STM_RSC *)MsrDesc);
    SmmExceptionHandler (Index);
    CpuDeadLoop ();
  }

  MsrDesc = GetStmResourceMsr ((STM_RSC *)(UINTN)mGuestContextCommonSmm.BiosHwResourceRequirementsPtr, MsrIndex);
  if ((MsrDesc == NULL) || (MsrDesc->WriteMask == 0) || (MsrDesc->KernelModeProcessing == 0)) {
    ZeroMem (&LocalMsrDesc, sizeof(LocalMsrDesc));
    LocalMsrDesc.Hdr.RscType = MACHINE_SPECIFIC_REG;
    LocalMsrDesc.Hdr.Length = sizeof(LocalMsrDesc);
    LocalMsrDesc.MsrIndex = MsrIndex;
    LocalMsrDesc.ReadMask = 0;
    LocalMsrDesc.WriteMask = (UINT64)-1;
    AddEventLogForResource (EvtBiosAccessToUnclaimedResource, (STM_RSC *)&LocalMsrDesc);
  }

//  DEBUG ((EFI_D_INFO, "!!!WriteMsrHandler!!!\n"));
  Data64 = LShiftU64 ((UINT64)ReadUnaligned32 ((UINT32 *)&Reg->Rdx), 32) | (UINT64)ReadUnaligned32 ((UINT32 *)&Reg->Rax);

  switch (MsrIndex) {
  case IA32_EFER_MSR_INDEX:
#if 0
  AcquireSpinLock (&mHostContextCommon.DebugLock);
    if ((Data64 & IA32_EFER_MSR_SCE) != 0) {
      DEBUG ((EFI_D_INFO, "!!!WriteMsrHandler - SCE!!!\n"));
    }
    if ((Data64 & IA32_EFER_MSR_XDE) != 0) {
      DEBUG ((EFI_D_INFO, "!!!WriteMsrHandler - XDE!!!\n"));
    }
  ReleaseSpinLock (&mHostContextCommon.DebugLock);
#endif
    mGuestContextCommonSmm.GuestContextPerCpu[Index].Efer = Data64;
    //
    // Check IA32e mode switch
    //
    VmEntryControls.Uint32 = VmRead32 (VMCS_32_CONTROL_VMENTRY_CONTROLS_INDEX);
    if ((Data64 & IA32_EFER_MSR_MLE) != 0) {
      mGuestContextCommonSmm.GuestContextPerCpu[Index].Efer |= IA32_EFER_MSR_MLE;
    } else {
      mGuestContextCommonSmm.GuestContextPerCpu[Index].Efer &= ~IA32_EFER_MSR_MLE;
    }
    if (((mGuestContextCommonSmm.GuestContextPerCpu[Index].Efer & IA32_EFER_MSR_MLE) != 0) && 
        ((VmReadN (VMCS_N_GUEST_CR0_INDEX) & CR0_PG) != 0)) {
      mGuestContextCommonSmm.GuestContextPerCpu[Index].Efer |= IA32_EFER_MSR_MLA;
      VmEntryControls.Bits.Ia32eGuest = 1;
    } else {
      mGuestContextCommonSmm.GuestContextPerCpu[Index].Efer &= ~IA32_EFER_MSR_MLA;
      VmEntryControls.Bits.Ia32eGuest = 0;
    }
    VmWrite32 (VMCS_32_CONTROL_VMENTRY_CONTROLS_INDEX, VmEntryControls.Uint32);
    VmWrite64 (VMCS_64_GUEST_IA32_EFER_INDEX,          mGuestContextCommonSmm.GuestContextPerCpu[Index].Efer);

    break;

  case IA32_SYSENTER_CS_MSR_INDEX:
    VmWrite32 (VMCS_32_GUEST_IA32_SYSENTER_CS_INDEX, (UINT32)Data64);
    break;
  case IA32_SYSENTER_ESP_MSR_INDEX:
    VmWriteN (VMCS_N_GUEST_IA32_SYSENTER_ESP_INDEX, (UINTN)Data64);
    break;
  case IA32_SYSENTER_EIP_MSR_INDEX:
    VmWriteN (VMCS_N_GUEST_IA32_SYSENTER_EIP_INDEX, (UINTN)Data64);
    break;

  case IA32_FS_BASE_MSR_INDEX:
    VmWriteN (VMCS_N_GUEST_FS_BASE_INDEX, (UINTN)Data64);
    AsmWriteMsr64 (MsrIndex, Data64); // VMM does not use FS
    break;
  case IA32_GS_BASE_MSR_INDEX:
    VmWriteN (VMCS_N_GUEST_GS_BASE_INDEX, (UINTN)Data64);
    AsmWriteMsr64 (MsrIndex, Data64); // VMM does not use GS
    break;
#if 0
  case IA32_KERNAL_GS_BASE_MSR_INDEX:
    AsmWriteMsr64 (MsrIndex, Data64); // VMM does not use this
    break;
  case IA32_STAR_MSR_INDEX:
    AsmWriteMsr64 (MsrIndex, Data64); // VMM does not use this
    break;
  case IA32_LSTAR_MSR_INDEX:
    AsmWriteMsr64 (MsrIndex, Data64); // VMM does not use this
    break;
  case IA32_FMASK_MSR_INDEX:
    AsmWriteMsr64 (MsrIndex, Data64); // VMM does not use this
    break;
#endif

  case IA32_SMM_MONITOR_CTL_MSR_INDEX:
    break;

  case EFI_MSR_NEHALEM_SMRR_PHYS_BASE:
  case EFI_MSR_NEHALEM_SMRR_PHYS_MASK:
    // Ignore the write
    break;
    
  case IA32_BIOS_UPDT_TRIG_MSR_INDEX:
    // Only write it when BIOS request MicrocodeUpdate
    MsrDesc = GetStmResourceMsr ((STM_RSC *)(UINTN)mGuestContextCommonSmm.BiosHwResourceRequirementsPtr, IA32_BIOS_UPDT_TRIG_MSR_INDEX);
    if (MsrDesc != NULL) {
      AsmWriteMsr64 (MsrIndex, Data64);
    }
    break;

  default:
    Result = StmPlatformLibMsrWrite (MsrIndex, Data64);
    if (Result) {
      break;
    }
    //
    // For rest one, we need pass back to BIOS
    //

    //
    // Need mask write item
    //
    if (MsrDesc != NULL) {
      Data64 |= MsrDesc->WriteMask;
    }

    AsmWriteMsr64 (MsrIndex, Data64);
    break;
  }

  VmWriteN (VMCS_N_GUEST_RIP_INDEX, VmReadN(VMCS_N_GUEST_RIP_INDEX) + VmRead32(VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX));
  return ;
}
