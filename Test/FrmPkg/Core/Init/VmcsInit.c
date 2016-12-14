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
#include "FrmInit.h"

//#define NOT_USE_UNRESTRICTEDGUEST

/**

This function set VMCS host field.

@param Index   CPU index

**/
VOID
SetVmcsHostField (
  IN UINT32 Index
  )
{
  VmWrite16 (VMCS_16_HOST_ES_INDEX, AsmReadDs ());
  VmWrite16 (VMCS_16_HOST_CS_INDEX, AsmReadCs ());
  VmWrite16 (VMCS_16_HOST_SS_INDEX, AsmReadSs ());
  VmWrite16 (VMCS_16_HOST_DS_INDEX, AsmReadDs ());
  VmWrite16 (VMCS_16_HOST_FS_INDEX, AsmReadDs ());
  VmWrite16 (VMCS_16_HOST_GS_INDEX, AsmReadDs ());
  VmWrite16 (VMCS_16_HOST_TR_INDEX, AsmReadDs ());

  VmWriteN (VMCS_N_HOST_CR0_INDEX,         AsmReadCr0 () | ((UINTN)AsmReadMsr64 (IA32_VMX_CR0_FIXED0_MSR_INDEX) & (UINTN)AsmReadMsr64 (IA32_VMX_CR0_FIXED1_MSR_INDEX)));
  VmWriteN (VMCS_N_HOST_CR3_INDEX,         mHostContextCommon.PageTable);
  VmWriteN (VMCS_N_HOST_CR4_INDEX,         AsmReadCr4 () | CR4_PAE | CR4_OSFXSR | CR4_OSXMMEXCPT | ((UINTN)AsmReadMsr64 (IA32_VMX_CR4_FIXED0_MSR_INDEX) & (UINTN)AsmReadMsr64 (IA32_VMX_CR4_FIXED1_MSR_INDEX)));
  VmWriteN (VMCS_N_HOST_FS_BASE_INDEX,     0);
  VmWriteN (VMCS_N_HOST_GS_BASE_INDEX,     0);
  VmWriteN (VMCS_N_HOST_TR_BASE_INDEX,     0);
  VmWriteN (VMCS_N_HOST_GDTR_BASE_INDEX,   mHostContextCommon.Gdtr.Base);
  VmWriteN (VMCS_N_HOST_IDTR_BASE_INDEX,   mHostContextCommon.Idtr.Base);
  VmWriteN (VMCS_N_HOST_RSP_INDEX,         mHostContextCommon.HostContextPerCpu[Index].Stack);
  VmWriteN (VMCS_N_HOST_RIP_INDEX,         (UINTN)AsmHostEntrypoint);
}

/**

  This function set VMCS control field.

  @param Index   CPU index

**/
VOID
SetVmcsControlField (
  IN UINT32 Index
  )
{
  UINT64                                       Data64;
  VM_EXEC_PIN_BASES_VMEXIT_CONTROLS            PinBasedCtls;
  VM_EXEC_PROCESSOR_BASES_VMEXIT_CONTROLS      ProcessorBasedCtrls;
  VM_EXEC_2ND_PROCESSOR_BASES_VMEXIT_CONTROLS  ProcessorBasedCtrls2nd;
  VM_EXIT_CONTROLS                             VmExitCtrls;
  VM_ENTRY_CONTROLS                            VmEntryCtrls;

  //
  // Init VM Exit control fields
  // Init VM Entry control fields
  // Init VM Execution control fields
  //
  Data64 = AsmReadMsr64 (IA32_VMX_PINBASED_CTLS_MSR_INDEX);
  PinBasedCtls.Uint32 = (UINT32)Data64 & (UINT32)RShiftU64 (Data64, 32);
  PinBasedCtls.Bits.ExternalInterrupt = 0; // external interrupt (To be configured)
  PinBasedCtls.Bits.Nmi = 0;
  PinBasedCtls.Bits.VmxPreemptionTimer = 1; // Enable Periodic Timer to tear down AP.

  Data64 = AsmReadMsr64 (IA32_VMX_PROCBASED_CTLS_MSR_INDEX);
  ProcessorBasedCtrls.Uint32 = (UINT32)Data64 & (UINT32)RShiftU64 (Data64, 32);
  ProcessorBasedCtrls.Bits.Hlt = 0;
  ProcessorBasedCtrls.Bits.InterruptWindow = 0; // interrupt window
  ProcessorBasedCtrls.Bits.NmiWindow = 0;
  ProcessorBasedCtrls.Bits.IoBitmap = 1;
  ProcessorBasedCtrls.Bits.MsrBitmap = 0;
  ProcessorBasedCtrls.Bits.SecondaryControl = 1;

  Data64 = AsmReadMsr64 (IA32_VMX_PROCBASED_CTLS2_MSR_INDEX);

  ProcessorBasedCtrls2nd.Uint32 = (UINT32)RShiftU64 (Data64, 32);
  if (ProcessorBasedCtrls2nd.Bits.UnrestrictedGuest == 1) {
    mGuestContextCommon.GuestContextPerCpu[Index].UnrestrictedGuest = TRUE;
  } else {
    mGuestContextCommon.GuestContextPerCpu[Index].UnrestrictedGuest = FALSE;
  }
#ifdef NOT_USE_UNRESTRICTEDGUEST
  mGuestContextCommon.GuestContextPerCpu[Index].UnrestrictedGuest = FALSE;
#endif
  ProcessorBasedCtrls2nd.Uint32 = (UINT32)Data64 & (UINT32)RShiftU64 (Data64, 32);
  ProcessorBasedCtrls2nd.Bits.Ept = 1;
  ProcessorBasedCtrls2nd.Bits.VirtualizeX2Apic = 0;
  if (mGuestContextCommon.GuestContextPerCpu[Index].UnrestrictedGuest) {
    ProcessorBasedCtrls2nd.Bits.UnrestrictedGuest = 1;
  }

  Data64 = AsmReadMsr64 (IA32_VMX_EXIT_CTLS_MSR_INDEX);
  VmExitCtrls.Uint32 = (UINT32)Data64 & (UINT32)RShiftU64 (Data64, 32);
  VmExitCtrls.Bits.AcknowledgeInterrupt = 0; // Ack
  VmExitCtrls.Bits.Ia32eHost = (sizeof(UINT64) == sizeof(UINTN));
  if (PinBasedCtls.Bits.VmxPreemptionTimer) {
    VmExitCtrls.Bits.SaveVmxPreemptionTimerValue = 1;
  }
  VmExitCtrls.Bits.SaveIA32_EFER = 1;

  Data64 = AsmReadMsr64 (IA32_VMX_ENTRY_CTLS_MSR_INDEX);
  VmEntryCtrls.Uint32 = (UINT32)Data64 & (UINT32)RShiftU64 (Data64, 32);
  VmEntryCtrls.Bits.Ia32eGuest = (sizeof(UINT64) == sizeof(UINTN));
  VmEntryCtrls.Bits.LoadIA32_EFER = 1;

  VmWrite32 (VMCS_32_CONTROL_PIN_BASED_VM_EXECUTION_INDEX,           PinBasedCtls.Uint32);
  VmWrite32 (VMCS_32_CONTROL_PROCESSOR_BASED_VM_EXECUTION_INDEX,     ProcessorBasedCtrls.Uint32);
  VmWrite32 (VMCS_32_CONTROL_2ND_PROCESSOR_BASED_VM_EXECUTION_INDEX, ProcessorBasedCtrls2nd.Uint32);
  VmWrite32 (VMCS_32_CONTROL_EXCEPTION_BITMAP_INDEX,                 0);
  VmWrite32 (VMCS_32_CONTROL_VMEXIT_CONTROLS_INDEX,                  VmExitCtrls.Uint32);
  VmWrite32 (VMCS_32_CONTROL_VMENTRY_CONTROLS_INDEX,                 VmEntryCtrls.Uint32);

  VmWrite64 (VMCS_64_GUEST_IA32_EFER_INDEX,                          mGuestContextCommon.GuestContextPerCpu[Index].EFER);

  VmWriteN (VMCS_N_CONTROL_CR0_GUEST_HOST_MASK_INDEX,                ((UINTN)AsmReadMsr64 (IA32_VMX_CR0_FIXED0_MSR_INDEX) & (UINTN)AsmReadMsr64 (IA32_VMX_CR0_FIXED1_MSR_INDEX)));
  VmWriteN (VMCS_N_CONTROL_CR4_GUEST_HOST_MASK_INDEX,                (UINTN)-1);

  VmWriteN (VMCS_N_CONTROL_CR0_READ_SHADOW_INDEX,                    mGuestContextCommon.GuestContextPerCpu[Index].Cr0);
  VmWriteN (VMCS_N_CONTROL_CR4_READ_SHADOW_INDEX,                    mGuestContextCommon.GuestContextPerCpu[Index].Cr4);

  if (ProcessorBasedCtrls.Bits.IoBitmap) {
    VmWrite64 (VMCS_64_CONTROL_IO_BITMAP_A_INDEX,                    mGuestContextCommon.IoBitmapA);
    VmWrite64 (VMCS_64_CONTROL_IO_BITMAP_B_INDEX,                    mGuestContextCommon.IoBitmapB);
  }

  if (ProcessorBasedCtrls.Bits.MsrBitmap) {
    VmWrite64 (VMCS_64_CONTROL_MSR_BITMAP_INDEX,                     mGuestContextCommon.MsrBitmap);
  }

  VmWrite32 (VMCS_32_CONTROL_VMEXIT_MSR_STORE_COUNT_INDEX,           0);
  VmWrite64 (VMCS_64_CONTROL_VMEXIT_MSR_STORE_INDEX,                 mGuestContextCommon.GuestContextPerCpu[Index].VmExitMsrStore);

  VmWrite32 (VMCS_32_CONTROL_VMEXIT_MSR_LOAD_COUNT_INDEX,            0);
  VmWrite64 (VMCS_64_CONTROL_VMEXIT_MSR_LOAD_INDEX,                  mGuestContextCommon.GuestContextPerCpu[Index].VmExitMsrLoad);

  VmWrite32 (VMCS_32_CONTROL_VMENTRY_MSR_LOAD_COUNT_INDEX,           0);
  VmWrite64 (VMCS_64_CONTROL_VMENTRY_MSR_LOAD_INDEX,                 mGuestContextCommon.GuestContextPerCpu[Index].VmEnterMsrLoad);

  VmWrite32 (VMCS_32_CONTROL_CR3_TARGET_COUNT_INDEX, 0);
  for (Index = 0; Index < 4; Index++) {
    VmWriteN ((UINT32)(VMCS_N_CONTROL_CR3_TARGET_VALUE0_INDEX + (Index << 1)), (UINTN)-1);
  }

  VmWrite32 (VMCS_32_CONTROL_PAGE_FAULT_ERROR_CODE_MASK_INDEX, 0);
  VmWrite32 (VMCS_32_CONTROL_PAGE_FAULT_ERROR_CODE_MATCH_INDEX, 0);
  VmWrite32 (VMCS_32_CONTROL_VMENTRY_INTERRUPTION_INFO_INDEX, 0);
  VmWrite32 (VMCS_32_CONTROL_VMENTRY_EXCEPTION_ERROR_CODE_INDEX, 0);

  VmWrite64 (VMCS_64_CONTROL_EPT_PTR_INDEX, mGuestContextCommon.EptPointer.Uint64);

}

/**

  This function set VMCS guest field.

  @param Index   CPU index

**/
VOID
SetVmcsGuestField (
  IN UINT32 Index
  )
{
  VmWriteN (VMCS_N_GUEST_CR0_INDEX,                      mGuestContextCommon.GuestContextPerCpu[mBspIndex].Cr0 | ((UINTN)AsmReadMsr64 (IA32_VMX_CR0_FIXED0_MSR_INDEX) & (UINTN)AsmReadMsr64 (IA32_VMX_CR0_FIXED1_MSR_INDEX)));
  VmWriteN (VMCS_N_GUEST_CR3_INDEX,                      mGuestContextCommon.GuestContextPerCpu[mBspIndex].Cr3);
  VmWriteN (VMCS_N_GUEST_CR4_INDEX,                      mGuestContextCommon.GuestContextPerCpu[mBspIndex].Cr4 | ((UINTN)AsmReadMsr64 (IA32_VMX_CR4_FIXED0_MSR_INDEX) & (UINTN)AsmReadMsr64 (IA32_VMX_CR4_FIXED1_MSR_INDEX)));
  VmWriteN (VMCS_N_GUEST_ES_BASE_INDEX,                  0);
  VmWriteN (VMCS_N_GUEST_CS_BASE_INDEX,                  0);
  VmWriteN (VMCS_N_GUEST_SS_BASE_INDEX,                  0);
  VmWriteN (VMCS_N_GUEST_DS_BASE_INDEX,                  0);
  VmWriteN (VMCS_N_GUEST_FS_BASE_INDEX,                  0);
  VmWriteN (VMCS_N_GUEST_GS_BASE_INDEX,                  0);
  VmWriteN (VMCS_N_GUEST_LDTR_BASE_INDEX,                0);
  VmWriteN (VMCS_N_GUEST_TR_BASE_INDEX,                  0);
  VmWriteN (VMCS_N_GUEST_GDTR_BASE_INDEX,                mGuestContextCommon.GuestContextPerCpu[mBspIndex].Gdtr.Base);
  VmWriteN (VMCS_N_GUEST_IDTR_BASE_INDEX,                mGuestContextCommon.GuestContextPerCpu[mBspIndex].Idtr.Base);
  VmWriteN (VMCS_N_GUEST_DR7_INDEX,                      AsmReadDr7 ());
  VmWriteN (VMCS_N_GUEST_RSP_INDEX,                      mGuestContextCommon.GuestContextPerCpu[Index].Stack);
  VmWriteN (VMCS_N_GUEST_RIP_INDEX,                      (UINTN)GuestEntrypoint);
  VmWriteN (VMCS_N_GUEST_RFLAGS_INDEX,                   0x00000002); // not interrupt for S3
  VmWriteN (VMCS_N_GUEST_PENDING_DEBUG_EXCEPTIONS_INDEX, 0);

  VmWrite16 (VMCS_16_GUEST_ES_INDEX,                     AsmReadEs ());
  VmWrite16 (VMCS_16_GUEST_CS_INDEX,                     AsmReadCs ());
  VmWrite16 (VMCS_16_GUEST_SS_INDEX,                     AsmReadSs ());
  VmWrite16 (VMCS_16_GUEST_DS_INDEX,                     AsmReadDs ());
  VmWrite16 (VMCS_16_GUEST_FS_INDEX,                     AsmReadFs ());
  VmWrite16 (VMCS_16_GUEST_GS_INDEX,                     AsmReadGs ());
  VmWrite16 (VMCS_16_GUEST_LDTR_INDEX,                   0);
  VmWrite16 (VMCS_16_GUEST_TR_INDEX,                     AsmReadDs ());

  VmWrite32 (VMCS_32_GUEST_ES_LIMIT_INDEX,               0xffffffff);
  VmWrite32 (VMCS_32_GUEST_CS_LIMIT_INDEX,               0xffffffff);
  VmWrite32 (VMCS_32_GUEST_SS_LIMIT_INDEX,               0xffffffff);
  VmWrite32 (VMCS_32_GUEST_DS_LIMIT_INDEX,               0xffffffff);
  VmWrite32 (VMCS_32_GUEST_FS_LIMIT_INDEX,               0xffffffff);
  VmWrite32 (VMCS_32_GUEST_GS_LIMIT_INDEX,               0xffffffff);
  VmWrite32 (VMCS_32_GUEST_LDTR_LIMIT_INDEX,             0);
  VmWrite32 (VMCS_32_GUEST_TR_LIMIT_INDEX,               0x0000ffff);
  VmWrite32 (VMCS_32_GUEST_GDTR_LIMIT_INDEX,             mGuestContextCommon.GuestContextPerCpu[mBspIndex].Gdtr.Limit);
  VmWrite32 (VMCS_32_GUEST_IDTR_LIMIT_INDEX,             mGuestContextCommon.GuestContextPerCpu[mBspIndex].Idtr.Limit);
  VmWrite32 (VMCS_32_GUEST_ES_ACCESS_RIGHT_INDEX,        0xc093);
  if (sizeof(UINTN) == sizeof(UINT32)) {
    VmWrite32 (VMCS_32_GUEST_CS_ACCESS_RIGHT_INDEX,        0xc09b);
  } else {
    VmWrite32 (VMCS_32_GUEST_CS_ACCESS_RIGHT_INDEX,        0xa09b);
  }
  VmWrite32 (VMCS_32_GUEST_SS_ACCESS_RIGHT_INDEX,        0xc093);
  VmWrite32 (VMCS_32_GUEST_DS_ACCESS_RIGHT_INDEX,        0xc093);
  VmWrite32 (VMCS_32_GUEST_FS_ACCESS_RIGHT_INDEX,        0xc093);
  VmWrite32 (VMCS_32_GUEST_GS_ACCESS_RIGHT_INDEX,        0xc093);
  VmWrite32 (VMCS_32_GUEST_LDTR_ACCESS_RIGHT_INDEX,      0x10082);
  VmWrite32 (VMCS_32_GUEST_TR_ACCESS_RIGHT_INDEX,        0x008b);
  VmWrite32 (VMCS_32_GUEST_INTERRUPTIBILITY_STATE_INDEX, 0);
  VmWrite32 (VMCS_32_GUEST_ACTIVITY_STATE_INDEX,         GUEST_ACTIVITY_STATE_ACTIVE);

  VmWrite32 (VMCS_32_GUEST_VMX_PREEMPTION_TIMER_VALUE_INDEX, mGuestContextCommon.VmxTimerValue);
  VmWrite64 (VMCS_64_GUEST_IA32_DEBUGCTL_INDEX,          AsmReadMsr64 (IA32_DBG_CTL_MSR_INDEX));
  VmWrite64 (VMCS_64_GUEST_VMCS_LINK_PTR_INDEX,          0xffffffffffffffff);

  if ((mGuestContextCommon.GuestContextPerCpu[mBspIndex].Cr0 & CR0_PG) == 0) {
    // do not assert here, because it will be assigned later.
    //ASSERT(mGuestContextCommon.GuestContextPerCpu[Index].UnrestrictedGuest);
    VmWriteN(VMCS_N_GUEST_CR0_INDEX, VmReadN(VMCS_N_GUEST_CR0_INDEX) & (~CR0_PG));
  }

  VmWrite64 (VMCS_64_GUEST_IA32_EFER_INDEX, mGuestContextCommon.GuestContextPerCpu[Index].EFER);
  return ;
}
