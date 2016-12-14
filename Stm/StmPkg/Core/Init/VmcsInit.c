/** @file
  VMCS initialization

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmInit.h"

/**

  This function initialize VMCS for SMI.

  NOTE: We should not trust VMCS setting by MLE,so we need reinit them to make
  sure the data is valid.

  @param Index CPU index
  @param Vmcs  VMCS pointer

**/
VOID
InitializeSmiVmcs (
  IN UINT32   Index,
  IN UINT64   *Vmcs
  )
{
  UINT64                                       Data64;
  VM_EXIT_CONTROLS                             VmExitCtrls;
  VM_ENTRY_CONTROLS                            VmEntryCtrls;
  GUEST_INTERRUPTIBILITY_STATE                 GuestInterruptibilityState;
  VM_EXIT_MSR_ENTRY                            *VmExitMsrEntry;

  Data64 = AsmReadMsr64 (IA32_VMX_ENTRY_CTLS_MSR_INDEX);
  VmEntryCtrls.Uint32 = (UINT32)Data64 & (UINT32)RShiftU64 (Data64, 32);
  VmEntryCtrls.Bits.Ia32eGuest = (sizeof(UINT64) == sizeof(UINTN));
  VmEntryCtrls.Bits.DeactivateDualMonitor = 0;
  // Upon receiving control due to an SMI, the STM shall save the contents of the IA32_PERF_GLOBAL_CTRL MSR, disable any
  // enabled bits in the IA32_PERF_GLOBAL_CTRL MSR
  VmEntryCtrls.Bits.LoadIA32_PERF_GLOBAL_CTRL = 0;
  VmEntryCtrls.Bits.LoadIA32_EFER = 1;

  Data64 = AsmReadMsr64 (IA32_VMX_EXIT_CTLS_MSR_INDEX);
  VmExitCtrls.Uint32 = (UINT32)Data64 & (UINT32)RShiftU64 (Data64, 32);
  VmExitCtrls.Bits.Ia32eHost = (sizeof(UINT64) == sizeof(UINTN));
  VmExitCtrls.Bits.SaveVmxPreemptionTimerValue = 1; // Save VmxPreemptionTimer
  // Upon receiving control due to an SMI, the STM shall save the contents of the IA32_PERF_GLOBAL_CTRL MSR, disable any
  // enabled bits in the IA32_PERF_GLOBAL_CTRL MSR
  VmExitCtrls.Bits.LoadIA32_PERF_GLOBAL_CTRL = 0;
  VmExitCtrls.Bits.SaveIA32_EFER = 1;

  GuestInterruptibilityState.Uint32 = VmRead32 (VMCS_32_GUEST_INTERRUPTIBILITY_STATE_INDEX);
  GuestInterruptibilityState.Bits.BlockingBySmi = 0;

  //
  // Control field
  //
  VmWrite32 (VMCS_32_CONTROL_VMENTRY_CONTROLS_INDEX,                 VmEntryCtrls.Uint32);
  VmWrite32 (VMCS_32_CONTROL_VMEXIT_CONTROLS_INDEX,                  VmExitCtrls.Uint32);

  //
  // Make sure the value is valid
  //
  VmWrite32 (VMCS_32_CONTROL_VMEXIT_MSR_STORE_COUNT_INDEX, mGuestContextCommonSmi.GuestContextPerCpu[Index].GuestMsrEntryCount);
  VmWrite32 (VMCS_32_CONTROL_VMEXIT_MSR_LOAD_COUNT_INDEX,  mHostContextCommon.HostContextPerCpu[Index].HostMsrEntryCount);
  VmWrite32 (VMCS_32_CONTROL_VMENTRY_MSR_LOAD_COUNT_INDEX, mGuestContextCommonSmi.GuestContextPerCpu[Index].GuestMsrEntryCount);
  //
  // Upon receiving control due to an SMI, the STM shall save the contents of the IA32_PERF_GLOBAL_CTRL MSR, disable any
  // enabled bits in the IA32_PERF_GLOBAL_CTRL MSR.
  // Do we need handle IA32_PEBS_ENABLE MSR ???
  //
  VmExitMsrEntry = (VM_EXIT_MSR_ENTRY *)(UINTN)mHostContextCommon.HostContextPerCpu[Index].HostMsrEntryAddress;
  VmExitMsrEntry[0].MsrIndex = IA32_PERF_GLOBAL_CTRL_MSR_INDEX;
  VmExitMsrEntry[0].MsrData = 0;
  VmExitMsrEntry = (VM_EXIT_MSR_ENTRY *)(UINTN)mGuestContextCommonSmi.GuestContextPerCpu[Index].GuestMsrEntryAddress;
  VmExitMsrEntry[0].MsrIndex = IA32_PERF_GLOBAL_CTRL_MSR_INDEX;
  VmExitMsrEntry[0].MsrData = AsmReadMsr64(IA32_PERF_GLOBAL_CTRL_MSR_INDEX);
  VmWrite64 (VMCS_64_CONTROL_VMEXIT_MSR_STORE_INDEX, mGuestContextCommonSmi.GuestContextPerCpu[Index].GuestMsrEntryAddress);
  VmWrite64 (VMCS_64_CONTROL_VMEXIT_MSR_LOAD_INDEX,  mHostContextCommon.HostContextPerCpu[Index].HostMsrEntryAddress);
  VmWrite64 (VMCS_64_CONTROL_VMENTRY_MSR_LOAD_INDEX, mGuestContextCommonSmi.GuestContextPerCpu[Index].GuestMsrEntryAddress);

  //
  // Host field
  //
  VmWriteN  (VMCS_N_HOST_CR0_INDEX,         AsmReadCr0 ());
  VmWriteN  (VMCS_N_HOST_CR3_INDEX,         mHostContextCommon.PageTable);
  VmWriteN  (VMCS_N_HOST_CR4_INDEX,         AsmReadCr4 ());
  VmWrite16 (VMCS_16_HOST_ES_INDEX,         AsmReadDs ());
  VmWrite16 (VMCS_16_HOST_CS_INDEX,         AsmReadCs ());
  VmWrite16 (VMCS_16_HOST_SS_INDEX,         AsmReadDs ());
  VmWrite16 (VMCS_16_HOST_DS_INDEX,         AsmReadDs ());
  VmWrite16 (VMCS_16_HOST_FS_INDEX,         AsmReadDs ());
  VmWrite16 (VMCS_16_HOST_GS_INDEX,         AsmReadDs ());
  VmWrite16 (VMCS_16_HOST_TR_INDEX,         AsmReadDs ());
  VmWriteN  (VMCS_N_HOST_TR_BASE_INDEX,     0);
  VmWriteN  (VMCS_N_HOST_GDTR_BASE_INDEX,   mHostContextCommon.Gdtr.Base);
  VmWriteN  (VMCS_N_HOST_IDTR_BASE_INDEX,   mHostContextCommon.Idtr.Base);
  VmWriteN  (VMCS_N_HOST_RSP_INDEX,         mHostContextCommon.HostContextPerCpu[Index].Stack);
  VmWriteN  (VMCS_N_HOST_RIP_INDEX,         (UINTN)AsmHostEntrypointSmi);

  VmWrite64 (VMCS_64_HOST_IA32_PERF_GLOBAL_CTRL_INDEX,   0);

  //
  // Guest field
  //
  VmWriteN  (VMCS_N_GUEST_RIP_INDEX,                     VmReadN (VMCS_N_GUEST_RIP_INDEX) + VmRead32(VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX));
  VmWriteN  (VMCS_N_GUEST_RFLAGS_INDEX,                  0x00000002); // VMCALL success
  VmWrite32 (VMCS_32_GUEST_INTERRUPTIBILITY_STATE_INDEX, GuestInterruptibilityState.Uint32);

  VmWrite64 (VMCS_64_GUEST_IA32_PERF_GLOBAL_CTRL_INDEX,  AsmReadMsr64(IA32_PERF_GLOBAL_CTRL_MSR_INDEX));

  VmWrite64 (VMCS_64_GUEST_IA32_EFER_INDEX,              mGuestContextCommonSmi.GuestContextPerCpu[Index].Efer);

  return ;
}

/**

  This function initialize VMCS for SMM.

  NOTE: We should not trust VMCS setting by MLE,so we need reinit them to make
  sure the data is valid.

  @param Index CPU index
  @param Vmcs  VMCS pointer

**/
VOID
InitializeSmmVmcs (
  IN UINT32   Index,
  IN UINT64   *Vmcs
  )
{
  UINT64                                       Data64;
  VM_EXIT_CONTROLS                             VmExitCtrls;
  VM_ENTRY_CONTROLS                            VmEntryCtrls;
  VM_EXEC_PIN_BASES_VMEXIT_CONTROLS            PinBasedCtls;
  VM_EXEC_PROCESSOR_BASES_VMEXIT_CONTROLS      ProcessorBasedCtrls;
  VM_EXEC_2ND_PROCESSOR_BASES_VMEXIT_CONTROLS  ProcessorBasedCtrls2nd;
  GUEST_INTERRUPTIBILITY_STATE                 GuestInterruptibilityState;
  GDT_ENTRY                                    *GdtEntry;
  VM_EXIT_MSR_ENTRY                            *VmExitMsrEntry;

  Data64 = AsmReadMsr64 (IA32_VMX_PINBASED_CTLS_MSR_INDEX);
  PinBasedCtls.Uint32 = (UINT32)Data64 & (UINT32)RShiftU64 (Data64, 32);
  PinBasedCtls.Bits.ExternalInterrupt = 0; // external interrupt
  PinBasedCtls.Bits.Nmi = 0;
  PinBasedCtls.Bits.VmxPreemptionTimer = 0; // Timer

  Data64 = AsmReadMsr64 (IA32_VMX_PROCBASED_CTLS_MSR_INDEX);
  ProcessorBasedCtrls.Uint32 = (UINT32)Data64 & (UINT32)RShiftU64 (Data64, 32);
  ProcessorBasedCtrls.Bits.Hlt = 0;
  ProcessorBasedCtrls.Bits.InterruptWindow = 0; // interrupt window
  ProcessorBasedCtrls.Bits.NmiWindow = 0;
  ProcessorBasedCtrls.Bits.IoBitmap = 1;
  ProcessorBasedCtrls.Bits.MsrBitmap = 1;
  ProcessorBasedCtrls.Bits.SecondaryControl = 1;

  Data64 = AsmReadMsr64 (IA32_VMX_PROCBASED_CTLS2_MSR_INDEX);
  ProcessorBasedCtrls2nd.Uint32 = (UINT32)RShiftU64 (Data64, 32);
  if (ProcessorBasedCtrls2nd.Bits.UnrestrictedGuest != 0) {
    mGuestContextCommonSmm.GuestContextPerCpu[Index].UnrestrictedGuest = TRUE;
  } else {
    mGuestContextCommonSmm.GuestContextPerCpu[Index].UnrestrictedGuest = FALSE;
  }

  ProcessorBasedCtrls2nd.Uint32 = (UINT32)Data64 & (UINT32)RShiftU64 (Data64, 32);
  ProcessorBasedCtrls2nd.Bits.Ept = 1;
  if (mGuestContextCommonSmm.GuestContextPerCpu[Index].UnrestrictedGuest) {
    ProcessorBasedCtrls2nd.Bits.UnrestrictedGuest = 1;
  }

  Data64 = AsmReadMsr64 (IA32_VMX_ENTRY_CTLS_MSR_INDEX);
  VmEntryCtrls.Uint32 = (UINT32)Data64 & (UINT32)RShiftU64 (Data64, 32);
  VmEntryCtrls.Bits.Ia32eGuest = mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmEntryState.Intel64Mode;
  VmEntryCtrls.Bits.EntryToSmm = 1;
  // Upon receiving control due to an SMI, the STM shall save the contents of the IA32_PERF_GLOBAL_CTRL MSR, disable any
  // enabled bits in the IA32_PERF_GLOBAL_CTRL MSR
  VmEntryCtrls.Bits.LoadIA32_PERF_GLOBAL_CTRL = 0;
  // Need load EFER to support guest enable XD
  VmEntryCtrls.Bits.LoadIA32_EFER = 1;

  Data64 = AsmReadMsr64 (IA32_VMX_EXIT_CTLS_MSR_INDEX);
  VmExitCtrls.Uint32 = (UINT32)Data64 & (UINT32)RShiftU64 (Data64, 32);
  VmExitCtrls.Bits.Ia32eHost = (sizeof(UINT64) == sizeof(UINTN));
  // Upon receiving control due to an SMI, the STM shall save the contents of the IA32_PERF_GLOBAL_CTRL MSR, disable any
  // enabled bits in the IA32_PERF_GLOBAL_CTRL MSR
  VmExitCtrls.Bits.LoadIA32_PERF_GLOBAL_CTRL = 0;
  // Need load EFER to support guest enable XD
  VmExitCtrls.Bits.SaveIA32_EFER = 1;

  GuestInterruptibilityState.Uint32 = 0;
  GuestInterruptibilityState.Bits.BlockingBySmi = 1;

  //
  // Control field
  //
  VmWrite32 (VMCS_32_CONTROL_PIN_BASED_VM_EXECUTION_INDEX,           PinBasedCtls.Uint32);
  VmWrite32 (VMCS_32_CONTROL_PROCESSOR_BASED_VM_EXECUTION_INDEX,     ProcessorBasedCtrls.Uint32);
  VmWrite32 (VMCS_32_CONTROL_2ND_PROCESSOR_BASED_VM_EXECUTION_INDEX, ProcessorBasedCtrls2nd.Uint32);
  VmWrite32 (VMCS_32_CONTROL_EXCEPTION_BITMAP_INDEX,                 0);
  VmWrite32 (VMCS_32_CONTROL_VMENTRY_CONTROLS_INDEX,                 VmEntryCtrls.Uint32);
  VmWrite32 (VMCS_32_CONTROL_VMEXIT_CONTROLS_INDEX,                  VmExitCtrls.Uint32);

  VmWrite64 (VMCS_64_CONTROL_EPT_PTR_INDEX,                          mGuestContextCommonSmm.EptPointer.Uint64);

  VmWriteN (VMCS_N_CONTROL_CR0_GUEST_HOST_MASK_INDEX,                ((UINTN)AsmReadMsr64 (IA32_VMX_CR0_FIXED0_MSR_INDEX) & (UINTN)AsmReadMsr64 (IA32_VMX_CR0_FIXED1_MSR_INDEX)) | CR0_CD);
  VmWriteN (VMCS_N_CONTROL_CR4_GUEST_HOST_MASK_INDEX,                (UINTN)-1);

  VmWriteN (VMCS_N_CONTROL_CR0_READ_SHADOW_INDEX,                    mGuestContextCommonSmm.GuestContextPerCpu[Index].Cr0 | ((UINTN)AsmReadMsr64 (IA32_VMX_CR0_FIXED0_MSR_INDEX) & (UINTN)AsmReadMsr64 (IA32_VMX_CR0_FIXED1_MSR_INDEX)));
  VmWriteN (VMCS_N_CONTROL_CR4_READ_SHADOW_INDEX,                    mGuestContextCommonSmm.GuestContextPerCpu[Index].Cr4 | ((UINTN)AsmReadMsr64 (IA32_VMX_CR4_FIXED0_MSR_INDEX) & (UINTN)AsmReadMsr64 (IA32_VMX_CR4_FIXED1_MSR_INDEX)) | CR4_PAE);

  if (ProcessorBasedCtrls.Bits.IoBitmap != 0) {
    VmWrite64 (VMCS_64_CONTROL_IO_BITMAP_A_INDEX,                    mGuestContextCommonSmm.IoBitmapA);
    VmWrite64 (VMCS_64_CONTROL_IO_BITMAP_B_INDEX,                    mGuestContextCommonSmm.IoBitmapB);
  }

  if (ProcessorBasedCtrls.Bits.MsrBitmap != 0) {
    VmWrite64 (VMCS_64_CONTROL_MSR_BITMAP_INDEX,                     mGuestContextCommonSmm.MsrBitmap);
  }

  //
  // Make sure the value is valid
  //
  VmWrite32 (VMCS_32_CONTROL_VMEXIT_MSR_STORE_COUNT_INDEX, mGuestContextCommonSmi.GuestContextPerCpu[Index].GuestMsrEntryCount);
  VmWrite32 (VMCS_32_CONTROL_VMEXIT_MSR_LOAD_COUNT_INDEX,  mHostContextCommon.HostContextPerCpu[Index].HostMsrEntryCount);
  VmWrite32 (VMCS_32_CONTROL_VMENTRY_MSR_LOAD_COUNT_INDEX, mGuestContextCommonSmi.GuestContextPerCpu[Index].GuestMsrEntryCount);
  //
  // Upon receiving control due to an SMI, the STM shall save the contents of the IA32_PERF_GLOBAL_CTRL MSR, disable any
  // enabled bits in the IA32_PERF_GLOBAL_CTRL MSR.
  // Do we need handle IA32_PEBS_ENABLE MSR ???
  //
  VmExitMsrEntry = (VM_EXIT_MSR_ENTRY *)(UINTN)mHostContextCommon.HostContextPerCpu[Index].HostMsrEntryAddress;
  VmExitMsrEntry[0].MsrIndex = IA32_PERF_GLOBAL_CTRL_MSR_INDEX;
  VmExitMsrEntry[0].MsrData = 0;
  VmExitMsrEntry = (VM_EXIT_MSR_ENTRY *)(UINTN)mGuestContextCommonSmi.GuestContextPerCpu[Index].GuestMsrEntryAddress;
  VmExitMsrEntry[0].MsrIndex = IA32_PERF_GLOBAL_CTRL_MSR_INDEX;
  VmExitMsrEntry[0].MsrData = AsmReadMsr64(IA32_PERF_GLOBAL_CTRL_MSR_INDEX);
  VmWrite64 (VMCS_64_CONTROL_VMEXIT_MSR_STORE_INDEX, mGuestContextCommonSmi.GuestContextPerCpu[Index].GuestMsrEntryAddress);
  VmWrite64 (VMCS_64_CONTROL_VMEXIT_MSR_LOAD_INDEX,  mHostContextCommon.HostContextPerCpu[Index].HostMsrEntryAddress);
  VmWrite64 (VMCS_64_CONTROL_VMENTRY_MSR_LOAD_INDEX, mGuestContextCommonSmi.GuestContextPerCpu[Index].GuestMsrEntryAddress);

  //
  // Host field
  //
  VmWriteN  (VMCS_N_HOST_CR0_INDEX,                      AsmReadCr0 ());
  VmWriteN  (VMCS_N_HOST_CR3_INDEX,                      mHostContextCommon.PageTable);
  VmWriteN  (VMCS_N_HOST_CR4_INDEX,                      AsmReadCr4 ());
  VmWrite16 (VMCS_16_HOST_ES_INDEX,                      AsmReadDs ());
  VmWrite16 (VMCS_16_HOST_CS_INDEX,                      AsmReadCs ());
  VmWrite16 (VMCS_16_HOST_SS_INDEX,                      AsmReadDs ());
  VmWrite16 (VMCS_16_HOST_DS_INDEX,                      AsmReadDs ());
  VmWrite16 (VMCS_16_HOST_FS_INDEX,                      AsmReadDs ());
  VmWrite16 (VMCS_16_HOST_GS_INDEX,                      AsmReadDs ());
  VmWrite16 (VMCS_16_HOST_TR_INDEX,                      AsmReadDs ());
  VmWriteN  (VMCS_N_HOST_TR_BASE_INDEX,                  0);
  VmWriteN  (VMCS_N_HOST_GDTR_BASE_INDEX,                mHostContextCommon.Gdtr.Base);
  VmWriteN  (VMCS_N_HOST_IDTR_BASE_INDEX,                mHostContextCommon.Idtr.Base);
  VmWriteN  (VMCS_N_HOST_RSP_INDEX,                      mHostContextCommon.HostContextPerCpu[Index].Stack);
  VmWriteN  (VMCS_N_HOST_RIP_INDEX,                      (UINTN)AsmHostEntrypointSmm);

  VmWrite64 (VMCS_64_HOST_IA32_PERF_GLOBAL_CTRL_INDEX,   0);

  //
  // Guest field
  //
  VmWriteN  (VMCS_N_GUEST_CR0_INDEX,                     mGuestContextCommonSmm.GuestContextPerCpu[Index].Cr0 | ((UINTN)AsmReadMsr64 (IA32_VMX_CR0_FIXED0_MSR_INDEX) & (UINTN)AsmReadMsr64 (IA32_VMX_CR0_FIXED1_MSR_INDEX)));
  VmWriteN  (VMCS_N_GUEST_CR3_INDEX,                     (UINTN)mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmCr3);
  VmWriteN  (VMCS_N_GUEST_CR4_INDEX,                     mGuestContextCommonSmm.GuestContextPerCpu[Index].Cr4 | ((UINTN)AsmReadMsr64 (IA32_VMX_CR4_FIXED0_MSR_INDEX) & (UINTN)AsmReadMsr64 (IA32_VMX_CR4_FIXED1_MSR_INDEX)));
  if (sizeof(UINTN) == sizeof(UINT64)) {
    VmWriteN  (VMCS_N_GUEST_CR4_INDEX,                   VmReadN(VMCS_N_GUEST_CR4_INDEX) | CR4_PAE);
  } else {
    if (mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmEntryState.Cr4Pae) {
      VmWriteN  (VMCS_N_GUEST_CR4_INDEX,                 VmReadN(VMCS_N_GUEST_CR4_INDEX) | CR4_PAE);
    }
    if (mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmEntryState.Cr4Pse) {
      VmWriteN  (VMCS_N_GUEST_CR4_INDEX,                 VmReadN(VMCS_N_GUEST_CR4_INDEX) | CR4_PSE);
    }
  }
  VmWriteN  (VMCS_N_GUEST_LDTR_BASE_INDEX,               0);
  VmWriteN  (VMCS_N_GUEST_GDTR_BASE_INDEX,               (UINTN)mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmGdtPtr);
  VmWriteN  (VMCS_N_GUEST_IDTR_BASE_INDEX,               0);
  VmWriteN  (VMCS_N_GUEST_RSP_INDEX,                     (UINTN)mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmSmiHandlerRsp);
  VmWriteN  (VMCS_N_GUEST_RIP_INDEX,                     (UINTN)mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmSmiHandlerRip);
  VmWriteN  (VMCS_N_GUEST_RFLAGS_INDEX,                  0x00000002);
  VmWriteN  (VMCS_N_GUEST_PENDING_DEBUG_EXCEPTIONS_INDEX,0);
  VmWriteN  (VMCS_N_GUEST_IA32_SYSENTER_ESP_INDEX,       0);
  VmWriteN  (VMCS_N_GUEST_IA32_SYSENTER_EIP_INDEX,       0);

  VmWrite16 (VMCS_16_GUEST_ES_INDEX,                     mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmOtherSegment);
  VmWrite16 (VMCS_16_GUEST_CS_INDEX,                     mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmCs);
  VmWrite16 (VMCS_16_GUEST_SS_INDEX,                     mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmSs);
  VmWrite16 (VMCS_16_GUEST_DS_INDEX,                     mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmDs);
  VmWrite16 (VMCS_16_GUEST_FS_INDEX,                     mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmOtherSegment);
  VmWrite16 (VMCS_16_GUEST_GS_INDEX,                     mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmOtherSegment);
  VmWrite16 (VMCS_16_GUEST_LDTR_INDEX,                   0);
  VmWrite16 (VMCS_16_GUEST_TR_INDEX,                     mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmTr);
  VmWrite32 (VMCS_32_GUEST_LDTR_LIMIT_INDEX,             0);
  VmWrite32 (VMCS_32_GUEST_GDTR_LIMIT_INDEX,             mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmGdtSize - 1);
  VmWrite32 (VMCS_32_GUEST_IDTR_LIMIT_INDEX,             0x0000ffff);
  VmWrite32 (VMCS_32_GUEST_IA32_SYSENTER_CS_INDEX,       0);
  VmWrite32 (VMCS_32_GUEST_LDTR_ACCESS_RIGHT_INDEX,      0x10082);

  GdtEntry = (GDT_ENTRY *)((UINTN)mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmGdtPtr + mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmCs);
  VmWriteN  (VMCS_N_GUEST_CS_BASE_INDEX,                 BaseFromGdtEntry (GdtEntry));
  VmWrite32 (VMCS_32_GUEST_CS_ACCESS_RIGHT_INDEX,        ArFromGdtEntry (GdtEntry));
  VmWrite32 (VMCS_32_GUEST_CS_LIMIT_INDEX,               LimitFromGdtEntry (GdtEntry));
  GdtEntry = (GDT_ENTRY *)((UINTN)mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmGdtPtr + mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmSs);
  VmWriteN  (VMCS_N_GUEST_SS_BASE_INDEX,                 BaseFromGdtEntry (GdtEntry));
  VmWrite32 (VMCS_32_GUEST_SS_ACCESS_RIGHT_INDEX,        ArFromGdtEntry (GdtEntry));
  VmWrite32 (VMCS_32_GUEST_SS_LIMIT_INDEX,               LimitFromGdtEntry (GdtEntry));
  GdtEntry = (GDT_ENTRY *)((UINTN)mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmGdtPtr + mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmDs);
  VmWriteN  (VMCS_N_GUEST_DS_BASE_INDEX,                 BaseFromGdtEntry (GdtEntry));
  VmWrite32 (VMCS_32_GUEST_DS_ACCESS_RIGHT_INDEX,        ArFromGdtEntry (GdtEntry));
  VmWrite32 (VMCS_32_GUEST_DS_LIMIT_INDEX,               LimitFromGdtEntry (GdtEntry));
  GdtEntry = (GDT_ENTRY *)((UINTN)mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmGdtPtr + mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmOtherSegment);
  VmWriteN  (VMCS_N_GUEST_ES_BASE_INDEX,                 BaseFromGdtEntry (GdtEntry));
  VmWrite32 (VMCS_32_GUEST_ES_ACCESS_RIGHT_INDEX,        ArFromGdtEntry (GdtEntry));
  VmWrite32 (VMCS_32_GUEST_ES_LIMIT_INDEX,               LimitFromGdtEntry (GdtEntry));
  VmWriteN  (VMCS_N_GUEST_FS_BASE_INDEX,                 BaseFromGdtEntry (GdtEntry));
  VmWrite32 (VMCS_32_GUEST_FS_ACCESS_RIGHT_INDEX,        ArFromGdtEntry (GdtEntry));
  VmWrite32 (VMCS_32_GUEST_FS_LIMIT_INDEX,               LimitFromGdtEntry (GdtEntry));
  VmWriteN  (VMCS_N_GUEST_GS_BASE_INDEX,                 BaseFromGdtEntry (GdtEntry));
  VmWrite32 (VMCS_32_GUEST_GS_ACCESS_RIGHT_INDEX,        ArFromGdtEntry (GdtEntry));
  VmWrite32 (VMCS_32_GUEST_GS_LIMIT_INDEX,               LimitFromGdtEntry (GdtEntry));

  GdtEntry = (GDT_ENTRY *)((UINTN)mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmGdtPtr + mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmTr);
  VmWriteN  (VMCS_N_GUEST_TR_BASE_INDEX,                 BaseFromGdtEntry (GdtEntry));
  VmWrite32 (VMCS_32_GUEST_TR_ACCESS_RIGHT_INDEX,        ArFromGdtEntry (GdtEntry));
  VmWrite32 (VMCS_32_GUEST_TR_LIMIT_INDEX,               LimitFromGdtEntry (GdtEntry));

  VmWrite32 (VMCS_32_GUEST_INTERRUPTIBILITY_STATE_INDEX, GuestInterruptibilityState.Uint32);

  VmWrite64 (VMCS_64_GUEST_IA32_PERF_GLOBAL_CTRL_INDEX,  AsmReadMsr64(IA32_PERF_GLOBAL_CTRL_MSR_INDEX));

  VmWrite64 (VMCS_64_GUEST_IA32_EFER_INDEX,              mGuestContextCommonSmm.GuestContextPerCpu[Index].Efer);
  return ;
}
