/** @file
  STM teardown

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmRuntime.h"

extern volatile BOOLEAN         mIsBspInitialized;
extern volatile BOOLEAN         *mCpuInitStatus;
extern MRTT_INFO    mMtrrInfo;
extern STM_HANDLER  mStmHandlerSmm[VmExitReasonMax];
extern STM_HANDLER  mStmHandlerSmi[VmExitReasonMax];

/**

  This function restore STM data to original value.
  Currently, we just zero updated data here - <common> in .DATA section and .BBS section.
  It can be enhanced later.

**/
VOID
RestoreStmData (
  VOID
  )
{
  ZeroMem (&mMtrrInfo, sizeof(mMtrrInfo));
  ZeroMem (&mStmHandlerSmm, sizeof(mStmHandlerSmm));
  ZeroMem (&mStmHandlerSmi, sizeof(mStmHandlerSmi));
  ZeroMem (&mGuestContextCommonSmm, sizeof(mGuestContextCommonSmm));
  ZeroMem (&mHostContextCommon, sizeof(mHostContextCommon));
  ZeroMem (&mGuestContextCommonSmi, sizeof(mGuestContextCommonSmi));
  mIsBspInitialized = FALSE;
  mCpuInitStatus = NULL;
}

/**

  This function teardown STM.

  @param Index CPU index

**/
VOID
StmTeardown (
  IN UINT32  Index
  )
{
  VM_ENTRY_CONTROLS VmEntryCtrls;
  UINTN             Rflags;
  X86_REGISTER      *Reg;
  UINT32            CurrentJoinedCpuNum;

  DEBUG ((EFI_D_INFO, "StmTeardown - %d\n", (UINTN)Index));

  Reg = &mGuestContextCommonSmi.GuestContextPerCpu[Index].Register;

  VmEntryCtrls.Uint32 = VmRead32 (VMCS_32_CONTROL_VMENTRY_CONTROLS_INDEX);
  VmEntryCtrls.Bits.DeactivateDualMonitor = 1;
  VmWrite32 (VMCS_32_CONTROL_VMENTRY_CONTROLS_INDEX, VmEntryCtrls.Uint32);

  VmWriteN (VMCS_N_GUEST_RIP_INDEX, VmReadN (VMCS_N_GUEST_RIP_INDEX) + VmRead32 (VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX));

  CurrentJoinedCpuNum = InterlockedDecrement (&mHostContextCommon.JoinedCpuNum);

  STM_PERF_END (Index, "OsSmiHandler", "StmTeardown");

  // We should not WaitAllProcessorRendezVous() because we can not assume VMM will invoke this at one time.

  if (CurrentJoinedCpuNum == 0) {
    //
    // If CurrentJoinedCpuNum is zero that means every CPUs has alraedy run code above, it is safe to clean up the environment.
    //

    STM_PERF_DUMP;

    //
    // Need reset [RSP] as 0, just in case that SINIT does not do it.
    //
    *(UINT32 *)((UINTN)mHostContextCommon.StmHeader + mHostContextCommon.StmHeader->HwStmHdr.EspOffset) = 0;

    //
    // Restore data
    //
    RestoreStmData ();

    //
    // Relocate image back at last CPU
    //
    RelocateStmImage (TRUE);
  }

  //
  // After that we should NOT use global variable.
  //

  //
  // Launch back
  //
  Rflags = AsmVmResume (Reg);
  // BUGBUG: - AsmVmLaunch if AsmVmResume fail
  if (VmRead32 (VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX) == VmxFailErrorVmResumeWithNonLaunchedVmcs) {
//    DEBUG ((EFI_D_ERROR, "(STM):-(\n", (UINTN)Index));
    Rflags = AsmVmLaunch (Reg);
  }

#if 1
  //
  // Should not print debug message, because image has been relocated back, and NO global variable should be used.
  //
  AcquireSpinLock (&mHostContextCommon.DebugLock);
  DEBUG ((EFI_D_ERROR, "!!!StmTeardown FAIL!!!\n"));
  DEBUG ((EFI_D_ERROR, "Rflags: %08x\n", Rflags));
  DEBUG ((EFI_D_ERROR, "VMCS_32_RO_VM_INSTRUCTION_ERROR: %08x\n", (UINTN)VmRead32 (VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX)));
  DumpVmcsAllField ();
  DumpRegContext (Reg);
  ReleaseSpinLock (&mHostContextCommon.DebugLock);
#endif

  CpuDeadLoop ();

  return ;
}
