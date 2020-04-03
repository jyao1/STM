/** @file
  VM/PE VMCS initialization

  Copyright (c) 2010, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

/// Intel Copyright left in as this is a modification of their code

#include "StmInit.h"
#include "PeStm.h"

/**

  This function initialize VMCS for Protected Execution VMs.

  @param Index CPU index
  @param Vmcs  VMCS pointer

**/

VOID
InitPeGuestVmcs (
  IN UINT32 CpuIndex,
  IN UINT32 VmType,
  IN PE_GUEST_CONTEXT_PER_CPU   *Vmcs
  )
{
  UINT64                                       Data64;
  //VM_EXIT_CONTROLS                             VmExitCtrls;
  //VM_ENTRY_CONTROLS                            VmEntryCtrls;
  VM_EXEC_PIN_BASES_VMEXIT_CONTROLS            PinBasedCtls;
  VM_EXEC_PROCESSOR_BASES_VMEXIT_CONTROLS      ProcessorBasedCtrls;
  VM_EXEC_2ND_PROCESSOR_BASES_VMEXIT_CONTROLS  ProcessorBasedCtrls2nd;
  GUEST_INTERRUPTIBILITY_STATE                 GuestInterruptibilityState;
  VM_EXIT_MSR_ENTRY                            *VmExitMsrEntry;

 // UINT32 ExceptionBitmap;
 // UINT32 PageFaultErrorCodeMask;
 // UINT32 PageFaultErrorCodeMatch;

  Data64 = AsmReadMsr64 (IA32_VMX_PINBASED_CTLS_MSR_INDEX);
  PinBasedCtls.Uint32 = (UINT32)(Data64 & 0xFFFFFFFF);
  PinBasedCtls.Bits.ExternalInterrupt = 0; // external interrupt
  PinBasedCtls.Bits.Nmi = 1;                // NMI is used to allow for when an SMI occurs when one of the processors
                                            // is running a VM/PE to allow the other processors to interrupt the VM/PE
                                            // so that he SMI handler can process the SMI
  PinBasedCtls.Bits.VmxPreemptionTimer = 1; // Timer  (was zero)

  //  Processor based controls

  Data64 = AsmReadMsr64 (IA32_VMX_PROCBASED_CTLS_MSR_INDEX);
  ProcessorBasedCtrls.Uint32 = (UINT32)Data64;
  ProcessorBasedCtrls.Bits.Hlt = 0;
  ProcessorBasedCtrls.Bits.InterruptWindow = 0; // interrupt window
  ProcessorBasedCtrls.Bits.NmiWindow = 0;
  ProcessorBasedCtrls.Bits.UnconditionalIo = 1;   // unconditional I/O exiting
  ProcessorBasedCtrls.Bits.IoBitmap = 0;     // was 1
  ProcessorBasedCtrls.Bits.MsrBitmap = 0;    // was 1
  ProcessorBasedCtrls.Bits.SecondaryControl = 1;
  ProcessorBasedCtrls.Bits.Cr3Load = 0;
  ProcessorBasedCtrls.Bits.Cr3Store = 0;

  ProcessorBasedCtrls.Uint32 &= (UINT32)RShiftU64 (Data64, 32);

  // Secondary Processor Based Controls

  Data64 = AsmReadMsr64 (IA32_VMX_PROCBASED_CTLS2_MSR_INDEX);

  // Force this for now

  ProcessorBasedCtrls2nd.Uint32 = (UINT32) Data64;
  mGuestContextCommonSmm[VmType].GuestContextPerCpu[0].UnrestrictedGuest = TRUE;
  
  ProcessorBasedCtrls2nd.Bits.Ept = 1;
  ProcessorBasedCtrls2nd.Bits.UnrestrictedGuest = 1;
  DEBUG((EFI_D_ERROR, "%ld InitPeGuestVmcs - PE Guest set as unrestricted\n", CpuIndex));
 
  ProcessorBasedCtrls2nd.Uint32 &= (UINT32)RShiftU64 (Data64, 32);
  
  /*EDM - forced what we did before */

  //VmExitCtrls.Uint32 = 0x13EFFB;

  GuestInterruptibilityState.Uint32 = 0;
  GuestInterruptibilityState.Bits.BlockingBySmi = 1;
  //#define VMCS_32_CONTROL_EXCEPTION_BITMAP_INDEX                 0x4004
//#define VMCS_32_CONTROL_PAGE_FAULT_ERROR_CODE_MASK_INDEX       0x4006
//#define VMCS_32_CONTROL_PAGE_FAULT_ERROR_CODE_MATCH_INDEX      0x4008

  //
  // Control field
  //

  VmWrite32 (VMCS_32_GUEST_VMX_PREEMPTION_TIMER_VALUE_INDEX, 4000000000); // peemption timer...

  VmWrite32 (VMCS_32_CONTROL_PIN_BASED_VM_EXECUTION_INDEX,           PinBasedCtls.Uint32);
  VmWrite32 (VMCS_32_CONTROL_PROCESSOR_BASED_VM_EXECUTION_INDEX,     ProcessorBasedCtrls.Uint32);
  VmWrite32 (VMCS_32_CONTROL_2ND_PROCESSOR_BASED_VM_EXECUTION_INDEX, ProcessorBasedCtrls2nd.Uint32);
  VmWrite32 (VMCS_32_CONTROL_EXCEPTION_BITMAP_INDEX,                 0);

  DEBUG((EFI_D_ERROR, "%ld InitPeGuestVmcs - Exception Bitmap set to: 0x%08lx\n", CpuIndex, VmRead32(VMCS_32_CONTROL_EXCEPTION_BITMAP_INDEX)));
  VmWrite32 (VMCS_32_CONTROL_VMENTRY_CONTROLS_INDEX,                 Vmcs->VmEntryCtrls.Uint32);
  VmWrite32 (VMCS_32_CONTROL_VMEXIT_CONTROLS_INDEX,                  Vmcs->VmExitCtrls.Uint32);

  VmWrite64 (VMCS_64_CONTROL_EPT_PTR_INDEX,                          mGuestContextCommonSmm[VmType].EptPointer.Uint64);

  // turn the below on and the VM/PE CR handling code will be invoked
  // can enter 64-bit w/o these setting turned on with Sandybridge

  VmWriteN (VMCS_N_CONTROL_CR0_GUEST_HOST_MASK_INDEX,                0);//((UINTN)AsmReadMsr64 (IA32_VMX_CR0_FIXED0_MSR_INDEX) & (UINTN)AsmReadMsr64 (IA32_VMX_CR0_FIXED1_MSR_INDEX)) | CR0_CD);
  VmWriteN (VMCS_N_CONTROL_CR4_GUEST_HOST_MASK_INDEX,                0);//((UINTN)AsmReadMsr64 (IA32_VMX_CR4_FIXED0_MSR_INDEX) & (UINTN)AsmReadMsr64 (IA32_VMX_CR4_FIXED1_MSR_INDEX)) );

  VmWriteN (VMCS_N_CONTROL_CR0_READ_SHADOW_INDEX,                    0);//mGuestContextCommonSmm[VmType].GuestContextPerCpu[0].Cr0 | ((UINTN)AsmReadMsr64 (IA32_VMX_CR0_FIXED0_MSR_INDEX) & (UINTN)AsmReadMsr64 (IA32_VMX_CR0_FIXED1_MSR_INDEX)));
  VmWriteN (VMCS_N_CONTROL_CR4_READ_SHADOW_INDEX,                    0);//mGuestContextCommonSmm[VmType].GuestContextPerCpu[0].Cr4 | ((UINTN)AsmReadMsr64 (IA32_VMX_CR4_FIXED0_MSR_INDEX) & (UINTN)AsmReadMsr64 (IA32_VMX_CR4_FIXED1_MSR_INDEX)) | CR4_PAE);

  if (ProcessorBasedCtrls.Bits.IoBitmap != 0) 
  {
	 // Since we want to use IO Bitmaps, then point to the Bitmaps
    VmWrite64 (VMCS_64_CONTROL_IO_BITMAP_A_INDEX, mGuestContextCommonSmm[VmType].IoBitmapA);
    VmWrite64 (VMCS_64_CONTROL_IO_BITMAP_B_INDEX, mGuestContextCommonSmm[VmType].IoBitmapB);
  }
  else
  {
    VmWrite64 (VMCS_64_CONTROL_IO_BITMAP_A_INDEX, 0);
    VmWrite64 (VMCS_64_CONTROL_IO_BITMAP_B_INDEX, 0);
  }

  if (ProcessorBasedCtrls.Bits.MsrBitmap != 0) {
    VmWrite64 (VMCS_64_CONTROL_MSR_BITMAP_INDEX, mGuestContextCommonSmm[VmType].MsrBitmap);
  }
  else
  {
    VmWrite64 (VMCS_64_CONTROL_MSR_BITMAP_INDEX, 0);
  }

  //
  // Make sure the value is valid
  //
  VmWrite32 (VMCS_32_CONTROL_VMEXIT_MSR_STORE_COUNT_INDEX, mGuestContextCommonSmi.GuestContextPerCpu[CpuIndex].GuestMsrEntryCount);
  VmWrite32 (VMCS_32_CONTROL_VMEXIT_MSR_LOAD_COUNT_INDEX,  mHostContextCommon.HostContextPerCpu[CpuIndex].HostMsrEntryCount);
  VmWrite32 (VMCS_32_CONTROL_VMENTRY_MSR_LOAD_COUNT_INDEX, mGuestContextCommonSmi.GuestContextPerCpu[CpuIndex].GuestMsrEntryCount);
  //
  // Upon receiving control due to an SMI, the STM shall save the contents of the IA32_PERF_GLOBAL_CTRL MSR, disable any
  // enabled bits in the IA32_PERF_GLOBAL_CTRL MSR.
  // Do we need handle IA32_PEBS_ENABLE MSR ???
  //
  VmExitMsrEntry = (VM_EXIT_MSR_ENTRY *)(UINTN)mHostContextCommon.HostContextPerCpu[CpuIndex].HostMsrEntryAddress;
  VmExitMsrEntry[CpuIndex].MsrIndex = IA32_PERF_GLOBAL_CTRL_MSR_INDEX;
  VmExitMsrEntry[CpuIndex].MsrData = 0;
  VmExitMsrEntry = (VM_EXIT_MSR_ENTRY *)(UINTN)mGuestContextCommonSmi.GuestContextPerCpu[CpuIndex].GuestMsrEntryAddress;
  VmExitMsrEntry[CpuIndex].MsrIndex = IA32_PERF_GLOBAL_CTRL_MSR_INDEX;
  VmExitMsrEntry[CpuIndex].MsrData = AsmReadMsr64(IA32_PERF_GLOBAL_CTRL_MSR_INDEX);
  VmWrite64 (VMCS_64_CONTROL_VMEXIT_MSR_STORE_INDEX, mGuestContextCommonSmi.GuestContextPerCpu[CpuIndex].GuestMsrEntryAddress);
  VmWrite64 (VMCS_64_CONTROL_VMEXIT_MSR_LOAD_INDEX,  mHostContextCommon.HostContextPerCpu[CpuIndex].HostMsrEntryAddress);
  VmWrite64 (VMCS_64_CONTROL_VMENTRY_MSR_LOAD_INDEX, mGuestContextCommonSmi.GuestContextPerCpu[CpuIndex].GuestMsrEntryAddress);

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
  VmWriteN  (VMCS_N_HOST_RSP_INDEX,                      mHostContextCommon.HostContextPerCpu[CpuIndex].Stack);
  VmWriteN  (VMCS_N_HOST_RIP_INDEX,                      (UINTN)AsmHostEntrypointSmmPe);

  VmWrite64 (VMCS_64_HOST_IA32_PERF_GLOBAL_CTRL_INDEX,   0);

  //
  // Guest field
  //
    VmWriteN(VMCS_N_GUEST_CR0_INDEX, mGuestContextCommonSmm[VmType].GuestContextPerCpu[0].Cr0);
    VmWriteN(VMCS_N_GUEST_CR3_INDEX, mGuestContextCommonSmm[VmType].GuestContextPerCpu[0].Cr3);
    VmWriteN(VMCS_N_GUEST_CR4_INDEX, mGuestContextCommonSmm[VmType].GuestContextPerCpu[0].Cr4);
  
    VmWriteN  (VMCS_N_GUEST_LDTR_BASE_INDEX,               Vmcs->LdtrBase);
    VmWriteN  (VMCS_N_GUEST_GDTR_BASE_INDEX,               Vmcs->GdtrBase);
    VmWriteN  (VMCS_N_GUEST_IDTR_BASE_INDEX,               Vmcs->IdtrBase);
    VmWriteN  (VMCS_N_GUEST_RSP_INDEX,                     Vmcs->Rsp);
    VmWriteN  (VMCS_N_GUEST_RIP_INDEX,                     Vmcs->Rip);
    VmWriteN  (VMCS_N_GUEST_RFLAGS_INDEX,                  0x2);

    VmWriteN  (VMCS_N_GUEST_PENDING_DEBUG_EXCEPTIONS_INDEX,0);
    VmWriteN  (VMCS_N_GUEST_IA32_SYSENTER_ESP_INDEX,       0);
    VmWriteN  (VMCS_N_GUEST_IA32_SYSENTER_EIP_INDEX,       0);

    //VmWriteN (VMCS_N_GUEST_DR7_INDEX, 0);

    VmWrite16 (VMCS_16_GUEST_ES_INDEX,                     Vmcs->EsSelector);
    VmWrite16 (VMCS_16_GUEST_CS_INDEX,                     Vmcs->CsSelector);
    VmWrite16 (VMCS_16_GUEST_SS_INDEX,                     Vmcs->SsSelector);
    VmWrite16 (VMCS_16_GUEST_DS_INDEX,                     Vmcs->DsSelector);
    VmWrite16 (VMCS_16_GUEST_FS_INDEX,                     Vmcs->FsSelector);
    VmWrite16 (VMCS_16_GUEST_GS_INDEX,                     Vmcs->GsSelector);
    VmWrite16 (VMCS_16_GUEST_LDTR_INDEX,                   Vmcs->LdtrSelector);
    VmWrite16 (VMCS_16_GUEST_TR_INDEX,                     Vmcs->TrSelector);
    VmWrite32 (VMCS_32_GUEST_LDTR_LIMIT_INDEX,             Vmcs->LdtrLimit);
    VmWrite32 (VMCS_32_GUEST_GDTR_LIMIT_INDEX,             Vmcs->GdtrLimit);
    VmWrite32 (VMCS_32_GUEST_IDTR_LIMIT_INDEX,             Vmcs->IdtrLimit);
    VmWrite32 (VMCS_32_GUEST_IA32_SYSENTER_CS_INDEX,       0);
    VmWriteN  (VMCS_N_GUEST_IA32_SYSENTER_ESP_INDEX,       0);
    VmWriteN  (VMCS_N_GUEST_IA32_SYSENTER_EIP_INDEX,       0);

    VmWrite32 (VMCS_32_GUEST_LDTR_ACCESS_RIGHT_INDEX,      Vmcs->LdtrAccessRights);

    VmWriteN  (VMCS_N_GUEST_CS_BASE_INDEX,                 Vmcs->CsBase);
    VmWrite32 (VMCS_32_GUEST_CS_ACCESS_RIGHT_INDEX,        Vmcs->CsAccessRights);
    VmWrite32 (VMCS_32_GUEST_CS_LIMIT_INDEX,               Vmcs->CsLimit);
    
	VmWriteN  (VMCS_N_GUEST_SS_BASE_INDEX,                 Vmcs->SsBase);
    VmWrite32 (VMCS_32_GUEST_SS_ACCESS_RIGHT_INDEX,        Vmcs->SsAccessRights);
    VmWrite32 (VMCS_32_GUEST_SS_LIMIT_INDEX,               Vmcs->SsLimit);
    
	VmWriteN  (VMCS_N_GUEST_DS_BASE_INDEX,                 Vmcs->DsBase);
    VmWrite32 (VMCS_32_GUEST_DS_ACCESS_RIGHT_INDEX,        Vmcs->DsAccessRights);
    VmWrite32 (VMCS_32_GUEST_DS_LIMIT_INDEX,               Vmcs->DsLimit);
    
	VmWriteN  (VMCS_N_GUEST_ES_BASE_INDEX,                 Vmcs->EsBase);
    VmWrite32 (VMCS_32_GUEST_ES_ACCESS_RIGHT_INDEX,        Vmcs->EsAccessRights);
    VmWrite32 (VMCS_32_GUEST_ES_LIMIT_INDEX,               Vmcs->EsLimit);
    
	VmWriteN  (VMCS_N_GUEST_FS_BASE_INDEX,                 Vmcs->FsBase);
    VmWrite32 (VMCS_32_GUEST_FS_ACCESS_RIGHT_INDEX,        Vmcs->FsAccessRights);
    VmWrite32 (VMCS_32_GUEST_FS_LIMIT_INDEX,               Vmcs->FsLimit);
    
	VmWriteN  (VMCS_N_GUEST_GS_BASE_INDEX,                 Vmcs->GsBase);
    VmWrite32 (VMCS_32_GUEST_GS_ACCESS_RIGHT_INDEX,        Vmcs->GsAccessRights);
    VmWrite32 (VMCS_32_GUEST_GS_LIMIT_INDEX,               Vmcs->GsLimit);

    VmWriteN  (VMCS_N_GUEST_TR_BASE_INDEX,                 Vmcs->TrBase);
    VmWrite32 (VMCS_32_GUEST_TR_ACCESS_RIGHT_INDEX,        Vmcs->TrAccessRights);
    VmWrite32 (VMCS_32_GUEST_TR_LIMIT_INDEX,               Vmcs->TrLimit);

    VmWrite32 (VMCS_32_GUEST_INTERRUPTIBILITY_STATE_INDEX, Vmcs->InterruptibilityState.Uint32);

    VmWrite64 (VMCS_64_GUEST_IA32_PERF_GLOBAL_CTRL_INDEX,  AsmReadMsr64(IA32_PERF_GLOBAL_CTRL_MSR_INDEX));

    VmWrite32(VMCS_32_GUEST_ACTIVITY_STATE_INDEX, Vmcs->ActivityState);
    
    VmWrite64(VMCS_64_GUEST_VMCS_LINK_PTR_INDEX, Vmcs->VmcsLinkPointerFull);
  
    VmWrite64(VMCS_64_GUEST_IA32_EFER_INDEX, mGuestContextCommonSmm[VmType].GuestContextPerCpu[0].Efer);
     DEBUG((EFI_D_ERROR, "%ld InitPeGuestVmcs - done\n", CpuIndex));
  return ;
}
