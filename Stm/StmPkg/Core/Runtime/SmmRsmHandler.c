/** @file
  SMM RSM handler

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

  This function is RSM handler for SMM.

  @param Index CPU index

**/
VOID
RsmHandler (
  IN UINT32  Index
  )
{
  UINTN                          Rflags;
  UINT64                         ExecutiveVmcsPtr;
  UINT64                         VmcsLinkPtr;
  UINT32                         VmcsSize;
  
  VmcsSize = GetVmcsSize();
  ExecutiveVmcsPtr = VmRead64 (VMCS_64_CONTROL_EXECUTIVE_VMCS_PTR_INDEX);
  if (IsOverlap (ExecutiveVmcsPtr, VmcsSize, mHostContextCommon.TsegBase, mHostContextCommon.TsegLength)) {
    // Overlap TSEG
    DEBUG ((EFI_D_ERROR, "ExecutiveVmcsPtr violation (RsmHandler) - %016lx\n", ExecutiveVmcsPtr));
    CpuDeadLoop() ;
  }

  VmcsLinkPtr = VmRead64 (VMCS_64_GUEST_VMCS_LINK_PTR_INDEX);
  if (IsOverlap (VmcsLinkPtr, VmcsSize, mHostContextCommon.TsegBase, mHostContextCommon.TsegLength)) {
    // Overlap TSEG
    DEBUG ((EFI_D_ERROR, "VmcsLinkPtr violation (RsmHandler) - %016lx\n", VmcsLinkPtr));
    CpuDeadLoop() ;
  }
  
  if (mHostContextCommon.HostContextPerCpu[Index].JumpBufferValid) {
    //
    // return from Setup/TearDown
    //
    mHostContextCommon.HostContextPerCpu[Index].JumpBufferValid = FALSE;
    LongJump (&mHostContextCommon.HostContextPerCpu[Index].JumpBuffer, (UINTN)-1);
    // Should not get here
    CpuDeadLoop ();
  }

  AsmVmPtrStore (&mGuestContextCommonSmm.GuestContextPerCpu[Index].Vmcs);
  Rflags = AsmVmPtrLoad (&mGuestContextCommonSmi.GuestContextPerCpu[Index].Vmcs);
  if ((Rflags & (RFLAGS_CF | RFLAGS_ZF)) != 0) {
    DEBUG ((EFI_D_ERROR, "ERROR: AsmVmPtrLoad(%d) - %016lx : %08x\n", (UINTN)Index, mGuestContextCommonSmi.GuestContextPerCpu[Index].Vmcs, Rflags));
    CpuDeadLoop ();
  }

  STM_PERF_START (Index, 0, "ReadSyncSmmStateSaveArea", "RsmHandler");
  ReadSyncSmmStateSaveArea (Index);
  STM_PERF_END (Index, "ReadSyncSmmStateSaveArea", "RsmHandler");

#if 0
  DEBUG ((EFI_D_INFO, "Exit SmmHandler - %d\n", (UINTN)Index));
#endif

  // We should not WaitAllProcessorRendezVous() because we can not assume SMM will bring all CPU into BIOS SMM handler.
//  WaitAllProcessorRendezVous (Index);

  STM_PERF_END (Index, "OsSmiHandler", "RsmHandler");

  CheckPendingMtf (Index);

  //
  // Launch back
  //
  Rflags = AsmVmResume (&mGuestContextCommonSmi.GuestContextPerCpu[Index].Register);
  // BUGBUG: - AsmVmLaunch if AsmVmResume fail
  if (VmRead32 (VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX) == VmxFailErrorVmResumeWithNonLaunchedVmcs) {
//    DEBUG ((EFI_D_ERROR, "(STM):o(\n", (UINTN)Index));
    Rflags = AsmVmLaunch (&mGuestContextCommonSmi.GuestContextPerCpu[Index].Register);
  }

  AcquireSpinLock (&mHostContextCommon.DebugLock);
  DEBUG ((EFI_D_ERROR, "!!!RsmHandler FAIL!!!\n"));
  DEBUG ((EFI_D_ERROR, "Rflags: %08x\n", Rflags));
  DEBUG ((EFI_D_ERROR, "VMCS_32_RO_VM_INSTRUCTION_ERROR: %08x\n", (UINTN)VmRead32 (VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX)));
  DumpVmcsAllField ();
  DumpRegContext (&mGuestContextCommonSmi.GuestContextPerCpu[Index].Register);
  ReleaseSpinLock (&mHostContextCommon.DebugLock);

  CpuDeadLoop ();

  return ;
}
