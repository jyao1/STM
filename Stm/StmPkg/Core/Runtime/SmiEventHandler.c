/** @file
  SMI event handler

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmRuntime.h"

/**

  This function is SMI event handler for SMI.

  @param Index CPU index

**/
VOID
SmiEventHandler (
  IN UINT32  Index
  )
{
  UINTN                          Rflags;
  UINT64                         ExecutiveVmcsPtr;
  UINT64                         VmcsLinkPtr;
  UINT32                         VmcsSize;

  if (!mGuestContextCommonSmm.GuestContextPerCpu[Index].Actived) {
    return ;
  }

  VmcsSize = GetVmcsSize();
  ExecutiveVmcsPtr = VmRead64 (VMCS_64_CONTROL_EXECUTIVE_VMCS_PTR_INDEX);
  if (IsOverlap (ExecutiveVmcsPtr, VmcsSize, mHostContextCommon.TsegBase, mHostContextCommon.TsegLength)) {
    // Overlap TSEG
    DEBUG ((EFI_D_ERROR, "ExecutiveVmcsPtr violation (SmiEventHandler) - %016lx\n", ExecutiveVmcsPtr));
    return ;
  }

  VmcsLinkPtr = VmRead64 (VMCS_64_GUEST_VMCS_LINK_PTR_INDEX);
  if (IsOverlap (VmcsLinkPtr, VmcsSize, mHostContextCommon.TsegBase, mHostContextCommon.TsegLength)) {
    // Overlap TSEG
    DEBUG ((EFI_D_ERROR, "VmcsLinkPtr violation (SmiEventHandler) - %016lx\n", VmcsLinkPtr));
    return ;
  }

  STM_PERF_START (Index, 0, "WriteSyncSmmStateSaveArea", "SmiEventHandler");
  WriteSyncSmmStateSaveArea (Index);
  STM_PERF_END (Index, "WriteSyncSmmStateSaveArea", "SmiEventHandler");

  AsmVmPtrStore (&mGuestContextCommonSmi.GuestContextPerCpu[Index].Vmcs);
  Rflags = AsmVmPtrLoad (&mGuestContextCommonSmm.GuestContextPerCpu[Index].Vmcs);
  if ((Rflags & (RFLAGS_CF | RFLAGS_ZF)) != 0) {
    DEBUG ((EFI_D_ERROR, "ERROR: AsmVmPtrLoad(%d) - %016lx : %08x\n", (UINTN)Index, mGuestContextCommonSmm.GuestContextPerCpu[Index].Vmcs, Rflags));
    CpuDeadLoop ();
  }

  VmWriteN (VMCS_N_GUEST_RIP_INDEX, (UINTN)mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmSmiHandlerRip);
  VmWriteN (VMCS_N_GUEST_RSP_INDEX, (UINTN)mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmSmiHandlerRsp);
  VmWriteN (VMCS_N_GUEST_CR3_INDEX, mGuestContextCommonSmm.GuestContextPerCpu[Index].Cr3);
#if 0
  DEBUG ((EFI_D_INFO, "!!!Enter SmmHandler - %d\n", (UINTN)Index));
#endif

  STM_PERF_START (Index, 0, "BiosSmmHandler", "SmiEventHandler");

  //
  // Launch SMM
  //
  if (mGuestContextCommonSmm.GuestContextPerCpu[Index].Launched) {
    Rflags = AsmVmResume (&mGuestContextCommonSmm.GuestContextPerCpu[Index].Register);
    // BUGBUG: - AsmVmLaunch if AsmVmResume fail
    if (VmRead32 (VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX) == VmxFailErrorVmResumeWithNonLaunchedVmcs) {
//      DEBUG ((EFI_D_INFO, "(STM):-(\n", (UINTN)Index));
      Rflags = AsmVmLaunch (&mGuestContextCommonSmm.GuestContextPerCpu[Index].Register);
    }
  } else {
    mGuestContextCommonSmm.GuestContextPerCpu[Index].Launched = TRUE;
    Rflags = AsmVmLaunch (&mGuestContextCommonSmm.GuestContextPerCpu[Index].Register);
    mGuestContextCommonSmm.GuestContextPerCpu[Index].Launched = FALSE;
  }

  AcquireSpinLock (&mHostContextCommon.DebugLock);
  if (mGuestContextCommonSmm.GuestContextPerCpu[Index].Launched) {
    DEBUG ((EFI_D_ERROR, "!!!ResumeSmm FAIL!!!\n"));
  } else {
    DEBUG ((EFI_D_ERROR, "!!!LaunchSmm FAIL!!!\n"));
  }
  DEBUG ((EFI_D_ERROR, "Rflags: %08x\n", Rflags));
  DEBUG ((EFI_D_ERROR, "VMCS_32_RO_VM_INSTRUCTION_ERROR: %08x\n", (UINTN)VmRead32 (VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX)));

  DumpVmcsAllField ();
  DumpRegContext (&mGuestContextCommonSmm.GuestContextPerCpu[Index].Register);
  ReleaseSpinLock (&mHostContextCommon.DebugLock);

  CpuDeadLoop ();
  return ;
}
