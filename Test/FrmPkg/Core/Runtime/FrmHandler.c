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

FRM_HANDLER  mFrmHandler[VmExitReasonMax];

UINTN mExitCount[VmExitReasonMax];

/**

  This function initialize FRM handler.

**/
VOID
InitFrmHandler (
  VOID
  )
{
  UINT32  Index;

  for (Index = 0; Index < VmExitReasonMax; Index++) {
    mFrmHandler[Index] = UnknownHandler;
  }

  mFrmHandler[VmExitReasonExceptionNmi] = ExceptionNMIHandler;
  mFrmHandler[VmExitReasonCrAccess] = CrHandler;
  mFrmHandler[VmExitReasonEptViolation] = EPTViolationHandler;
  mFrmHandler[VmExitReasonEptMisConfiguration] = EPTMisconfigurationHandler;
  mFrmHandler[VmExitReasonInvEpt] = InvEPTHandler;
  mFrmHandler[VmExitReasonIoInstruction] = IoHandler;
  mFrmHandler[VmExitReasonCpuid] = CpuidHandler;
  mFrmHandler[VmExitReasonRdmsr] = ReadMsrHandler;
  mFrmHandler[VmExitReasonWrmsr] = WriteMsrHandler;
  mFrmHandler[VmExitReasonInit] = InitHandler;
  mFrmHandler[VmExitReasonSipi] = SipiHandler;
  mFrmHandler[VmExitReasonInvd] = InvdHandler;
  mFrmHandler[VmExitReasonWbinvd] = WbinvdHandler;
  mFrmHandler[VmExitReasonVmxPreEmptionTimerExpired] = VmxTimerHandler;
  mFrmHandler[VmExitReasonExternalInterrupt] = ExternalInterruptHandler;
  mFrmHandler[VmExitReasonInterruptWindow] = InterruptWindowHandler;
  mFrmHandler[VmExitReasonTaskSwitch] = TaskSwitchHandler;
  mFrmHandler[VmExitReasonXsetbv] = XsetbvHandler;
  mFrmHandler[VmExitReasonVmCall] = VmcallHandler;
}

/**

  This function return CPU index according to APICID.

  @param ApicId APIC ID

  @return CPU index

**/
UINT32
ApicToIndex (
  IN UINT32  ApicId
  )
{
  UINT32 Index;

  for (Index = 0; Index < mHostContextCommon.CpuNum; Index++) {
    if (mHostContextCommon.HostContextPerCpu[Index].ApicId == ApicId) {
      return Index;
    }
  }
  if (ApicId == 0xFF) {
    // standard PC
    return 0;
  }
  DEBUG ((EFI_D_ERROR, "(FRM) ApicToIndex fail\n"));
  CpuDeadLoop ();
  return 0;
}

/**

  This function is unknown handler.

  @param Index CPU index

**/
VOID
UnknownHandler (
  IN UINT32 Index
  )
{
  VM_EXIT_INFO_BASIC  InfoBasic;

  InfoBasic.Uint32 = VmRead32 (VMCS_32_RO_EXIT_REASON_INDEX);
  AcquireSpinLock (&mHostContextCommon.DebugLock);

  DEBUG ((EFI_D_ERROR, "(FRM) !!!UnknownHandler - CPU %d, reason %d\n", (UINTN)Index, InfoBasic.Bits.Reason));
  DumpVmcsAllField ();

  ReleaseSpinLock (&mHostContextCommon.DebugLock);

  CpuDeadLoop ();
}

/**

  This function is FRM C handler.

  @param Register  General purpose register set

**/
VOID
EFIAPI
FrmHandlerC (
  IN X86_REGISTER *Register
  )
{
  UINT32              Index;
  UINTN               Rflags;
  VM_EXIT_INFO_BASIC  InfoBasic;
//  DEBUG ((EFI_D_INFO, "(FRM) !!!Enter FrmHandlerC!!!\n"));
  //
  // save register
  //
  Index = ApicToIndex (ReadLocalApicId ());
  Register->Rsp = VmReadN (VMCS_N_GUEST_RSP_INDEX);
  CopyMem (&mGuestContextCommon.GuestContextPerCpu[Index].Register, Register, sizeof(X86_REGISTER));

  //
  // Dispatch
  //
  InfoBasic.Uint32 = VmRead32 (VMCS_32_RO_EXIT_REASON_INDEX);
  if (InfoBasic.Bits.Reason >= VmExitReasonMax) {
    DEBUG ((EFI_D_ERROR, "(FRM) !!!UnknownReason!!! (0x%x)\n", InfoBasic.Bits.Reason));
    DumpVmcsAllField ();

    CpuDeadLoop ();
  }
  mExitCount[InfoBasic.Bits.Reason] ++;
  VmWrite32 (VMCS_32_CONTROL_VMENTRY_INTERRUPTION_INFO_INDEX, 0);

  //
  // Call dispatch handler
  //
  mFrmHandler[InfoBasic.Bits.Reason] (Index);

  VmWriteN (VMCS_N_GUEST_RSP_INDEX, mGuestContextCommon.GuestContextPerCpu[Index].Register.Rsp); // sync RSP

  // check CD
  if (VmReadN (VMCS_N_GUEST_CR0_INDEX) & CR0_CD) {
    AsmWbinvd ();
  }

  //
  // Resume
  //
  Rflags = AsmVmResume (&mGuestContextCommon.GuestContextPerCpu[Index].Register);
  AcquireSpinLock (&mHostContextCommon.DebugLock);

  DEBUG ((EFI_D_ERROR, "(FRM) !!!ResumeGuest FAIL!!! - %d\n", (UINTN)Index));
  DEBUG ((EFI_D_ERROR, "(FRM) Rflags: %08x\n", Rflags));
  DEBUG ((EFI_D_ERROR, "(FRM) VMCS_32_RO_VM_INSTRUCTION_ERROR: %08x\n", (UINTN)VmRead32 (VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX)));

  DumpVmcsAllField ();

  ReleaseSpinLock (&mHostContextCommon.DebugLock);

  CpuDeadLoop ();

  return ;
}
