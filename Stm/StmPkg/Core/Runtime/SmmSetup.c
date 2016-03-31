/** @file
  SMM BIOS setup

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmInit.h"
#include "StmRuntime.h"

/**

  This function run BIOS SMM provided SetupRip.

  @param Index CPU index

**/
VOID
SmmSetup (
  IN UINT32 Index
  )
{
  UINTN  JumpFlag;
  UINTN  Rflags;

  if (mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmStmSetupRip == 0) {
    return ;
  }

  AsmVmPtrStore (&mGuestContextCommonSmi.GuestContextPerCpu[Index].Vmcs);
  Rflags = AsmVmPtrLoad (&mGuestContextCommonSmm.GuestContextPerCpu[Index].Vmcs);
  if ((Rflags & (RFLAGS_CF | RFLAGS_ZF)) != 0) {
    DEBUG ((EFI_D_ERROR, "ERROR: AsmVmPtrLoad(%d) - %016lx : %08x\n", (UINTN)Index, mGuestContextCommonSmm.GuestContextPerCpu[Index].Vmcs, Rflags));
    CpuDeadLoop ();
  }

  VmWriteN (VMCS_N_GUEST_RIP_INDEX, (UINTN)mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmStmSetupRip);
  VmWriteN (VMCS_N_GUEST_RSP_INDEX, (UINTN)mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmSmiHandlerRsp);
  VmWriteN (VMCS_N_GUEST_CR3_INDEX, mGuestContextCommonSmm.GuestContextPerCpu[Index].Cr3);

  //
  // We need update HOST_RSP to save context for SetJump.
  //
  VmWriteN  (VMCS_N_HOST_RSP_INDEX,         mHostContextCommon.HostContextPerCpu[Index].Stack - (mHostContextCommon.StmHeader->SwStmHdr.PerProcDynamicMemorySize / 2));

  JumpFlag = SetJump (&mHostContextCommon.HostContextPerCpu[Index].JumpBuffer);
  if (JumpFlag == 0) {

    WriteSyncSmmStateSaveAreaSse2 (Index, FALSE);

    STM_PERF_START (Index, 0, "BiosSmmHandler", "SmmSetup");

    DEBUG ((EFI_D_INFO, "SmmStmSetupRip start (%d) ...\n", (UINTN)Index));
    DEBUG ((EFI_D_INFO, "New HostStack (%d) - %08x\n", (UINTN)Index, VmReadN  (VMCS_N_HOST_RSP_INDEX)));
    mHostContextCommon.HostContextPerCpu[Index].JumpBufferValid = TRUE;
    mGuestContextCommonSmm.GuestContextPerCpu[Index].Launched = TRUE;
    Rflags = AsmVmLaunch (&mGuestContextCommonSmm.GuestContextPerCpu[Index].Register);
    mGuestContextCommonSmm.GuestContextPerCpu[Index].Launched = FALSE;
    AcquireSpinLock (&mHostContextCommon.DebugLock);
    DEBUG ((EFI_D_ERROR, "!!!SmmSetup FAIL!!!\n"));
    DEBUG ((EFI_D_ERROR, "Rflags: %08x\n", Rflags));
    DEBUG ((EFI_D_ERROR, "VMCS_32_RO_VM_INSTRUCTION_ERROR: %08x\n", (UINTN)VmRead32 (VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX)));
    ReleaseSpinLock (&mHostContextCommon.DebugLock);
    CpuDeadLoop ();
  }
  DEBUG ((EFI_D_INFO, "SmmStmSetupRip end (%d)\n", (UINTN)Index));

  //
  // Restore HOST_RSP
  //
  VmWriteN  (VMCS_N_HOST_RSP_INDEX,         mHostContextCommon.HostContextPerCpu[Index].Stack);

  AsmVmPtrStore (&mGuestContextCommonSmm.GuestContextPerCpu[Index].Vmcs);

  Rflags = AsmVmPtrLoad (&mGuestContextCommonSmi.GuestContextPerCpu[Index].Vmcs);
  if ((Rflags & (RFLAGS_CF | RFLAGS_ZF)) != 0) {
    DEBUG ((EFI_D_ERROR, "ERROR: AsmVmPtrLoad(%d) - %016lx : %08x\n", (UINTN)Index, mGuestContextCommonSmi.GuestContextPerCpu[Index].Vmcs, Rflags));
    CpuDeadLoop ();
  }
}