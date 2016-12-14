/** @file

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Base.h>
#include "FrmHandler.h"

/**

  This function is RDMSR handler.

  @param Index CPU index

**/
VOID
ReadMsrHandler (
  IN UINT32 Index
  )
{
  UINT64 Data64;
  UINT32 MsrIndex;

  MsrIndex = (UINT32)mGuestContextCommon.GuestContextPerCpu[Index].Register.Rcx;
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
    if (MsrIndex >= 0x40000000 && MsrIndex <= 0x400000FF) {
      Data64 = 0;
      DEBUG ((EFI_D_INFO, "(FRM) !!!ReadMsrHandler - Other!!! %08x<-%016lx\n", (UINTN)MsrIndex, Data64));
      break;
    }
    Data64 = AsmReadMsr64 (MsrIndex);
#if 0
    DEBUG ((EFI_D_INFO, "(FRM) !!!ReadMsrHandler - Other!!! %08x<-%016lx\n", (UINTN)MsrIndex, Data64));
#endif
    break;
  }

  mGuestContextCommon.GuestContextPerCpu[Index].Register.Rax = (UINTN)(UINT32)Data64; // HIGH bits are cleared
  mGuestContextCommon.GuestContextPerCpu[Index].Register.Rdx = (UINTN)(UINT32)RShiftU64 (Data64, 32); // HIGH bits are cleared

  VmWriteN (VMCS_N_GUEST_RIP_INDEX, VmReadN(VMCS_N_GUEST_RIP_INDEX) + VmRead32(VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX));
  return ;
}

/**

  This function is WRMSR handler.

  @param Index CPU index

**/
VOID
WriteMsrHandler (
  IN UINT32 Index
  )
{
  UINT64 Data64;
  UINT32 MsrIndex;
  VM_ENTRY_CONTROLS       VmEntryControls;

  MsrIndex = (UINT32)mGuestContextCommon.GuestContextPerCpu[Index].Register.Rcx;
  Data64 = (UINT64)(mGuestContextCommon.GuestContextPerCpu[Index].Register.Rdx & 0xFFFFFFFF);
  Data64 = LShiftU64 (Data64, 32);
  Data64 |= (UINT64)(mGuestContextCommon.GuestContextPerCpu[Index].Register.Rax & 0xFFFFFFFF);

  switch (MsrIndex) {
  case IA32_EFER_MSR_INDEX:
#if 0
    AcquireDebugLock ();
    if ((Data64 & IA32_EFER_MSR_SCE) != 0) {
      DEBUG ((EFI_D_INFO, "(FRM) !!!WriteMsrHandler - SCE!!!\n"));
    }
    if ((Data64 & IA32_EFER_MSR_XDE) != 0) {
      DEBUG ((EFI_D_INFO, "(FRM) !!!WriteMsrHandler - XDE!!!\n"));
    }
    ReleaseDebugLock ();
#endif
#if 0
    DEBUG ((EFI_D_INFO, "(FRM) !!!WriteMsrHandler - EFER!!! %016lx->%016lx\n", mGuestContextCommon.GuestContextPerCpu[Index].EFER, Data64));
#endif
    mGuestContextCommon.GuestContextPerCpu[Index].EFER = Data64;
    //
    // Check IA32e mode switch
    //
    VmEntryControls.Uint32 = VmRead32 (VMCS_32_CONTROL_VMENTRY_CONTROLS_INDEX);
    if ((Data64 & IA32_EFER_MSR_MLE) != 0) {
      mGuestContextCommon.GuestContextPerCpu[Index].EFER |= IA32_EFER_MSR_MLE;
    } else {
      mGuestContextCommon.GuestContextPerCpu[Index].EFER &= ~IA32_EFER_MSR_MLE;
    }
    if ((mGuestContextCommon.GuestContextPerCpu[Index].EFER & IA32_EFER_MSR_MLE) && 
        (mGuestContextCommon.GuestContextPerCpu[Index].Cr0 & CR0_PG)) {
      mGuestContextCommon.GuestContextPerCpu[Index].EFER |= IA32_EFER_MSR_MLA;
      VmEntryControls.Bits.Ia32eGuest = 1;
    } else {
      mGuestContextCommon.GuestContextPerCpu[Index].EFER &= ~IA32_EFER_MSR_MLA;
      VmEntryControls.Bits.Ia32eGuest = 0;
    }
    if ((!mGuestContextCommon.GuestContextPerCpu[Index].UnrestrictedGuest) &&
        ((mGuestContextCommon.GuestContextPerCpu[Index].Cr0 & CR0_PG) == 0)) {
      //
      // Need cache current Setting
      //
      VmWrite64 (VMCS_64_GUEST_IA32_EFER_INDEX,          mGuestContextCommon.GuestContextPerCpu[Index].EFER & ~IA32_EFER_MSR_MLA & ~IA32_EFER_MSR_MLE);
      VmEntryControls.Bits.Ia32eGuest = 0;
      VmWrite32 (VMCS_32_CONTROL_VMENTRY_CONTROLS_INDEX, VmEntryControls.Uint32);
    } else {
      VmWrite32 (VMCS_32_CONTROL_VMENTRY_CONTROLS_INDEX, VmEntryControls.Uint32);
      VmWrite64 (VMCS_64_GUEST_IA32_EFER_INDEX,          mGuestContextCommon.GuestContextPerCpu[Index].EFER);
    }

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
    AsmWriteMsr (MsrIndex, Data64); // VMM does not use this
    break;
  case IA32_STAR_MSR_INDEX:
    AsmWriteMsr (MsrIndex, Data64); // VMM does not use this
    break;
  case IA32_LSTAR_MSR_INDEX:
    AsmWriteMsr (MsrIndex, Data64); // VMM does not use this
    break;
  case IA32_FMASK_MSR_INDEX:
    AsmWriteMsr (MsrIndex, Data64); // VMM does not use this
    break;
#endif
  case IA32_BIOS_UPDT_TRIG_MSR_INDEX:
    DEBUG((EFI_D_INFO, "(FRM) !!!WriteMsrHandler (Microcode) - !!! %08x<-%016lx\n", (UINTN)MsrIndex, Data64));
    Data64 = GuestVirtualToHostPhysical(Index, (UINTN)Data64);
    if (Data64 != 0) {
      DEBUG((EFI_D_INFO, "(FRM) !!! Microcode %016lx\n", Data64));
      AsmWriteMsr64(MsrIndex, Data64);
    }
    break;
  default:
#if 0
    DEBUG ((EFI_D_INFO, "(FRM) !!!WriteMsrHandler - Other!!! %08x<-%016lx\n", (UINTN)MsrIndex, Data64));
#endif
    if (MsrIndex >=0x40000000 && MsrIndex <=0x400000FF) {
      DEBUG ((EFI_D_INFO, "(FRM) !!!WriteMsrHandler - Other!!! %08x<-%016lx\n", (UINTN)MsrIndex, Data64));
      break;
    }
    AsmWriteMsr64 (MsrIndex, Data64);
    break;
  }

  VmWriteN (VMCS_N_GUEST_RIP_INDEX, VmReadN(VMCS_N_GUEST_RIP_INDEX) + VmRead32(VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX));
  return ;
}