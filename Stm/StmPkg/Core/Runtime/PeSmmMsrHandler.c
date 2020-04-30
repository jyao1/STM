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

// most of this code was borrowed from the Intel driver
// howver, the only MSR of interest is the EFER MSR since that is needed to configure
// the guest VM for 64 bit

// may merge this code back into to Intel reference with VM/PE mods

#include "StmRuntime.h"
#include "PeStm.h"

/**

  This function is RDMSR handler for VM/PE.

  @param Index CPU index

**/
VOID
PeReadMsrHandler (
  IN UINT32 CpuIndex
  )
{
  UINT64            Data64;
  UINT32            MsrIndex;
  X86_REGISTER      *Reg;
  UINT32	    VmType = mHostContextCommon.HostContextPerCpu[CpuIndex].GuestVmType;

  UINT32 Index = 0;   // PE/VM only has index as 0

  Reg = &mGuestContextCommonSmm[VmType].GuestContextPerCpu[Index].Register;
  MsrIndex = ReadUnaligned32 ((UINT32 *)&Reg->Rcx);

  DEBUG ((EFI_D_INFO, "%ld PeReadMsrHandler - 0x%llx\n", CpuIndex, MsrIndex));

  switch (MsrIndex) {
  case IA32_EFER_MSR_INDEX:
    Data64 = mGuestContextCommonSmm[VmType].GuestContextPerCpu[Index].Efer;
    break;
#if 0
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
#endif
  default:
    // since we do not allow the VM/PE to generally read MSRs
	// we return 0 for a read.

    Data64 = 0;
    
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
PeWriteMsrHandler (
  IN UINT32 CpuIndex
  )
{
  UINT64            Data64;
  UINT32            MsrIndex;
  VM_ENTRY_CONTROLS VmEntryControls;
  X86_REGISTER      *Reg;
  STM_SMM_CPU_STATE *SmmCpuState;
  UINT32            VmType = mHostContextCommon.HostContextPerCpu[CpuIndex].GuestVmType;
  
  UINT32 Index = 0;        // PE VM only Index = 0

  SmmCpuState = mGuestContextCommonSmi.GuestContextPerCpu[Index].SmmCpuState;

  Reg = &mGuestContextCommonSmm[VmType].GuestContextPerCpu[Index].Register;
  MsrIndex = ReadUnaligned32 ((UINT32 *)&Reg->Rcx);

  Data64 = LShiftU64 ((UINT64)ReadUnaligned32 ((UINT32 *)&Reg->Rdx), 32) | (UINT64)ReadUnaligned32 ((UINT32 *)&Reg->Rax);
  DEBUG ((EFI_D_INFO, "%ld PeWriteMsrHandler - 0x%llx 0x%llx\n", CpuIndex, MsrIndex, Data64));

  switch (MsrIndex) {
  case IA32_EFER_MSR_INDEX:
#if 0
  AcquireSpinLock (&mHostContextCommon.DebugLock);
    if ((Data64 & IA32_EFER_MSR_SCE) != 0) {
      DEBUG ((EFI_D_INFO, "%ld WriteMsrHandler - SCE\n", CpuIndex,));
    }
    if ((Data64 & IA32_EFER_MSR_XDE) != 0) {
      DEBUG ((EFI_D_INFO, "%ld WriteMsrHandler - XDE\n", CpuIndex,));
    }
  ReleaseSpinLock (&mHostContextCommon.DebugLock);
#endif
    mGuestContextCommonSmm[VmType].GuestContextPerCpu[Index].Efer = Data64;
    //
    // Check IA32e mode switch
    //
    VmEntryControls.Uint32 = VmRead32 (VMCS_32_CONTROL_VMENTRY_CONTROLS_INDEX);
    if ((Data64 & IA32_EFER_MSR_MLE) != 0) {
      mGuestContextCommonSmm[VmType].GuestContextPerCpu[Index].Efer |= IA32_EFER_MSR_MLE;
    } else {
      mGuestContextCommonSmm[VmType].GuestContextPerCpu[Index].Efer &= ~IA32_EFER_MSR_MLE;
    }
    if (((mGuestContextCommonSmm[VmType].GuestContextPerCpu[Index].Efer & IA32_EFER_MSR_MLE) != 0) && 
        ((mGuestContextCommonSmm[VmType].GuestContextPerCpu[Index].Cr0 & CR0_PG) != 0)) {
      mGuestContextCommonSmm[VmType].GuestContextPerCpu[Index].Efer |= IA32_EFER_MSR_MLA;
      VmEntryControls.Bits.Ia32eGuest = 1;
    } else {
      mGuestContextCommonSmm[VmType].GuestContextPerCpu[Index].Efer &= ~IA32_EFER_MSR_MLA;
      VmEntryControls.Bits.Ia32eGuest = 0;
    }
    VmWrite32 (VMCS_32_CONTROL_VMENTRY_CONTROLS_INDEX, VmEntryControls.Uint32);
    VmWrite64 (VMCS_64_GUEST_IA32_EFER_INDEX,          mGuestContextCommonSmm[VmType].GuestContextPerCpu[Index].Efer);

    break;
#if 0
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
   // AsmWriteMsr64 (MsrIndex, Data64); // VMM does not use FS
    break;
  case IA32_GS_BASE_MSR_INDEX:
    VmWriteN (VMCS_N_GUEST_GS_BASE_INDEX, (UINTN)Data64);
   // AsmWriteMsr64 (MsrIndex, Data64); // VMM does not use GS
    break;
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
 
  default:
    DEBUG ((EFI_D_INFO, "%ldWriteMsrHandler - VM/PE has no access to this MSR - ignoring\n", CpuIndex));
    break;
  }

  VmWriteN (VMCS_N_GUEST_RIP_INDEX, VmReadN(VMCS_N_GUEST_RIP_INDEX) + VmRead32(VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX));
  return ;
}
