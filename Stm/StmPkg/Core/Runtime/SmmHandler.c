/** @file
  SMM handler

  Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmRuntime.h"

STM_HANDLER  mStmHandlerSmm[VmExitReasonMax];

/**

  This function initialize STM handle for SMM.

**/
VOID
InitStmHandlerSmm (
  VOID
  )
{
  UINT32  Index;

  for (Index = 0; Index < VmExitReasonMax; Index++) {
    mStmHandlerSmm[Index] = UnknownHandlerSmm;
  }

  mStmHandlerSmm[VmExitReasonRsm] = RsmHandler;
  mStmHandlerSmm[VmExitReasonVmCall] = SmmVmcallHandler;
  mStmHandlerSmm[VmExitReasonExceptionNmi] = SmmExceptionHandler;

  mStmHandlerSmm[VmExitReasonCrAccess] = SmmCrHandler;
  mStmHandlerSmm[VmExitReasonEptViolation] = SmmEPTViolationHandler;
  mStmHandlerSmm[VmExitReasonEptMisConfiguration] = SmmEPTMisconfigurationHandler;
  mStmHandlerSmm[VmExitReasonInvEpt] = SmmInvEPTHandler;
  mStmHandlerSmm[VmExitReasonIoInstruction] = SmmIoHandler;
  mStmHandlerSmm[VmExitReasonCpuid] = SmmCpuidHandler;
  mStmHandlerSmm[VmExitReasonRdmsr] = SmmReadMsrHandler;
  mStmHandlerSmm[VmExitReasonWrmsr] = SmmWriteMsrHandler;
  mStmHandlerSmm[VmExitReasonInvd] = SmmInvdHandler;
  mStmHandlerSmm[VmExitReasonWbinvd] = SmmWbinvdHandler;
  mStmHandlerSmm[VmExitReasonTaskSwitch] = SmmTaskSwitchHandler;
}

/**

  This function is unknown handler for SMM.

  @param Index CPU index

**/
VOID
UnknownHandlerSmm (
  IN UINT32 Index
  )
{
  AcquireSpinLock (&mHostContextCommon.DebugLock);

  DEBUG ((EFI_D_ERROR, "!!!UnknownHandlerSmm - %d\n", (UINTN)Index));
  DumpVmcsAllField ();
  DumpRegContext(&mGuestContextCommonSmm.GuestContextPerCpu[Index].Register);

  {
    UINT8  *Buffer;
    Buffer = (VOID *)VmReadN(VMCS_N_GUEST_RIP_INDEX);
    DEBUG((EFI_D_INFO, "Guest Instr: %02x %02x %02x %02x %02x %02x %02x %02x\n",
      Buffer[0],
      Buffer[1],
      Buffer[2],
      Buffer[3],
      Buffer[4],
      Buffer[5],
      Buffer[6],
      Buffer[7]
      ));
  }

  ReleaseSpinLock (&mHostContextCommon.DebugLock);

  CpuDeadLoop ();
}

/**

  This function is STM handler for SMM.

  @param Register X86 register context

**/
VOID
StmHandlerSmm (
  IN X86_REGISTER *Register
  )
{
  UINT32              Index;
  UINTN               Rflags;
  VM_EXIT_INFO_BASIC  InfoBasic;
  X86_REGISTER        *Reg;

  Index = ApicToIndex (ReadLocalApicId ());
  
  STM_PERF_END (Index, "BiosSmmHandler", "StmHandlerSmm");

  Reg = &mGuestContextCommonSmm.GuestContextPerCpu[Index].Register;
  Register->Rsp = VmReadN (VMCS_N_GUEST_RSP_INDEX);
  CopyMem (Reg, Register, sizeof(X86_REGISTER));
#if 0
  DEBUG ((EFI_D_INFO, "!!!StmHandlerSmm - %d\n", (UINTN)Index));
#endif
  //
  // Dispatch
  //
  InfoBasic.Uint32 = VmRead32 (VMCS_32_RO_EXIT_REASON_INDEX);
  if (InfoBasic.Bits.Reason >= VmExitReasonMax) {
    DEBUG ((EFI_D_ERROR, "!!!UnknownReason!!!\n"));
    DumpVmcsAllField ();

    CpuDeadLoop ();
  }
  //
  // Call dispatch handler
  //
  mStmHandlerSmm[InfoBasic.Bits.Reason] (Index);

  VmWriteN (VMCS_N_GUEST_RSP_INDEX, Reg->Rsp); // sync RSP

  STM_PERF_START (Index, InfoBasic.Bits.Reason, "BiosSmmHandler", "StmHandlerSmm");

  //
  // Resume
  //
  Rflags = AsmVmResume (&mGuestContextCommonSmm.GuestContextPerCpu[Index].Register);
  // BUGBUG: - AsmVmLaunch if AsmVmResume fail
  if (VmRead32 (VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX) == VmxFailErrorVmResumeWithNonLaunchedVmcs) {
//    DEBUG ((EFI_D_ERROR, "(STM):-(\n", (UINTN)Index));
    Rflags = AsmVmLaunch (&mGuestContextCommonSmm.GuestContextPerCpu[Index].Register);
  }

  AcquireSpinLock (&mHostContextCommon.DebugLock);

  DEBUG ((EFI_D_ERROR, "!!!ResumeGuestSmm FAIL!!! - %d\n", (UINTN)Index));
  DEBUG ((EFI_D_ERROR, "Rflags: %08x\n", Rflags));
  DEBUG ((EFI_D_ERROR, "VMCS_32_RO_VM_INSTRUCTION_ERROR: %08x\n", (UINTN)VmRead32 (VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX)));
  DumpVmcsAllField ();
  DumpRegContext (&mGuestContextCommonSmm.GuestContextPerCpu[Index].Register);

  ReleaseSpinLock (&mHostContextCommon.DebugLock);

  CpuDeadLoop ();

  return ;
}
