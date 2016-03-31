/** @file
  SMM BIOS state sync

  Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmRuntime.h"

//#define DOMAIN_TYPE_CHECK

/**
  =================================================================================================================================
                                           Populated                                         Propagated
  =================================================================================================================================
  UNPROTECTED                  All                                               All

  INTEGRITY_PROT_OUT_IN        All                                               No change propagation for IN from port not on BIOS
                                                                                   trap list 
                                                                                 Changes to EAX/AX/AL propagated for IN from port
                                                                                   on BIOS IO trap list.
                                                                                 No change propagation for OUT

//INTEGRITY_PROT_OUT           All                                               No changes propagated

  FULLY_PROT_OUT_IN            Only SM_REVID populated for any IO not on         No changes propagated for OUT
                                 BIOS trap list
                               Only SM_REVID, IO_MISC, IO_MEM_ADDR, and DX       Changes to EAX/AX/AL propagated for IN from port
                                 register populated for IN on BIOS trap list       on BIOS IO trap list.
                               Only SM_REVID, IO_MISC, IO_MEM_ADDR, DX, and      No changes propagated for OUT
                                 EAX/AX/AL populated for OUT on BIOS trap list 

//FULLY_PROT_IN                Only SM_REVID populated for IN not on BIOS trap   Changes to EAX/AX/AL propagated for IN from port
                                 list or for any trapped OUT.                      on BIOS IO trap list.
                               Only SM_REVID, IO_MISC, IO_MEM_ADDR and DX        No change propagation for IN from port not on BIOS
                                 register populated for IN on BIOS trap list       trap list or for any trapped OUT

//FULLY_PROT_OUT               Only SM_REVID populated for OUT not on BIOS trap  No changes propagated
                                 list or for any trapped IN
                               Only SM_REVID, IO_MISC, IO_MEM_ADDR, DX and
                                 EAX/AX/AL populated for OUT on BIOS trap list 

  FULLY_PROT                   Only SM_REVID populated                           No changes propagated
  =================================================================================================================================

**/

VMCS_RECORD_STRUCTURE              mDefaultVmcsRecord = {
  0x0,
  DOMAIN_UNPROTECTED,
  XSTATE_READWRITE,
  DOMAIN_UNPROTECTED,
};

/**

  This function write SMM state save IA32e GPR accroding to VMCS.

  @param Index    CPU index
  @param CpuState SMM CPU state
  @param Scrub    Nee Scrub

**/
VOID
WriteSyncSmmStateSaveAreaIa32eGpr (
  IN UINT32                             Index,
  IN STM_SMM_CPU_STATE                  *CpuState,
  IN BOOLEAN                            Scrub
  );

/**

  This function read SMM state save IA32e GPR sync to VMCS.

  @param Index CPU index
  @param CpuState SMM CPU state

**/
VOID
ReadSyncSmmStateSaveAreaIa32eGpr (
  IN UINT32                             Index,
  IN STM_SMM_CPU_STATE                  *CpuState
  );

/**

  This function save current XState.

  @param Index CPU index

**/
VOID
XStateSave (
  IN UINT32 Index
  )
{
#if 0 // TBD - workaround
  if (IsXStateEnabled()) {
    AsmXSave (
      0xFFFFFFFFFFFFFFFFull,
      (IA32_X_BUFFER *)mGuestContextCommonSmi.GuestContextPerCpu[Index].XStateBuffer
      );
  } else {
#if defined (MDE_CPU_IA32)
    AsmFxSave ((IA32_FX_BUFFER *)mGuestContextCommonSmi.GuestContextPerCpu[Index].XStateBuffer);
#endif
#if defined (MDE_CPU_X64)
    CopyMem ((IA32_FX_BUFFER *)mGuestContextCommonSmi.GuestContextPerCpu[Index].XStateBuffer, &mGuestContextCommonSmi.GuestContextPerCpu[Index].Register.FxBuffer, sizeof(IA32_FX_BUFFER))
#endif
  }
#endif
}

/**

  This function restore current XState.

  @param Index CPU index

**/
VOID
XStateRestore (
  IN UINT32 Index
  )
{
#if 0 // TBD - workaround
  if (IsXStateEnabled()) {
    AsmXRestore (
      0xFFFFFFFFFFFFFFFFull,
      (IA32_X_BUFFER *)mGuestContextCommonSmi.GuestContextPerCpu[Index].XStateBuffer
      );
  } else {
#if defined (MDE_CPU_IA32)
    AsmFxRestore ((IA32_FX_BUFFER *)mGuestContextCommonSmi.GuestContextPerCpu[Index].XStateBuffer);
#endif
#if defined (MDE_CPU_X64)
    CopyMem (&mGuestContextCommonSmi.GuestContextPerCpu[Index].Register.FxBuffer, (IA32_FX_BUFFER *)mGuestContextCommonSmi.GuestContextPerCpu[Index].XStateBuffer, sizeof(IA32_FX_BUFFER))
#endif
  }
#endif
}

/**

  This function scrub current XState.

  @param Index CPU index

**/
VOID
XStateScrub (
  IN UINT32 Index
  )
{
#if 0 // TBD - workaround
  if (IsXStateEnabled()) {
    AsmXRestore (
      0xFFFFFFFFFFFFFFFFull,
      (IA32_X_BUFFER *)mGuestContextCommonSmi.ZeroXStateBuffer
      );
  } else {
#if defined (MDE_CPU_IA32)
    AsmFxRestore ((IA32_FX_BUFFER *)mGuestContextCommonSmi.ZeroXStateBuffer);
#endif
#if defined (MDE_CPU_X64)
    ZeroMem (&mGuestContextCommonSmm.GuestContextPerCpu[Index].Register.FxBuffer, sizeof(IA32_FX_BUFFER))
#endif
  }
#endif
}

/**

  This function write SMM state save area accroding to VMCS.

  @param Index CPU index

**/
VOID
WriteSyncSmmStateSaveArea (
  IN UINT32 Index
  )
{
  STM_SMM_CPU_STATE                  *CpuState;
  SMM_SAVE_STATE_IO_MISC             IOMisc;
  VM_EXIT_QUALIFICATION              Qualification;
  UINT64                             ExecutiveVmcsPtr;
  VMCS_RECORD_STRUCTURE              *VmcsRecord;
  TXT_PROCESSOR_SMM_DESCRIPTOR       *TxtProcessorSmmDescriptor;
  X86_REGISTER                       *Reg;
#ifdef DOMAIN_TYPE_CHECK
  STM_RSC_TRAPPED_IO_DESC            *TrappedIoDesc;
#endif

  Reg = &mGuestContextCommonSmi.GuestContextPerCpu[Index].Register;

  ExecutiveVmcsPtr = VmRead64 (VMCS_64_CONTROL_EXECUTIVE_VMCS_PTR_INDEX);
  VmcsRecord = GetVmcsRecord (mHostContextCommon.VmcsDatabase, ExecutiveVmcsPtr);
  if (VmcsRecord == NULL) {
    VmcsRecord = &mDefaultVmcsRecord;
  }
  VmcsRecord->DegradedDomainType = VmcsRecord->DomainType;
#if 0
  DEBUG ((EFI_D_INFO, "GetVmcsRecord - %08x\n", VmcsRecord));
#endif
  TxtProcessorSmmDescriptor = (TXT_PROCESSOR_SMM_DESCRIPTOR *)(UINTN)mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor;
  TxtProcessorSmmDescriptor->SmmResumeState.SmramToVmcsRestoreRequired = 0;

  CpuState = (STM_SMM_CPU_STATE *)(UINTN)(mHostContextCommon.HostContextPerCpu[Index].Smbase + SMM_CPU_STATE_OFFSET);

  //
  // Basic info
  //
  if (CpuState->Smbase != mHostContextCommon.HostContextPerCpu[Index].Smbase) {
    CpuDeadLoop ();
  }
  CpuState->SMMRevId = STM_SMM_REV_ID;

  //
  // Start sync
  //
  if ((VmcsRecord->DomainType & DOMAIN_CONFIDENTIALITY) == 0) {
    // Only the IO_MISC (and IO_MEM_ADDR if IO_MISC[0] is set) and SM_REVID fields of the SMRAM state save area are valid.
    // Additionally, the register state has been scrubbed.
    CpuState->GdtBaseHiDword = (UINT32)RShiftU64 (VmReadN (VMCS_N_GUEST_GDTR_BASE_INDEX), 32);
    CpuState->GdtBaseLoDword = (UINT32)VmReadN (VMCS_N_GUEST_GDTR_BASE_INDEX);
//    CpuState->GdtLimit = VmRead32 (VMCS_32_GUEST_GDTR_LIMIT_INDEX);

    CpuState->LdtBaseHiDword = (UINT32)RShiftU64 (VmReadN (VMCS_N_GUEST_LDTR_BASE_INDEX), 32);
    CpuState->LdtBaseLoDword = (UINT32)VmReadN (VMCS_N_GUEST_LDTR_BASE_INDEX);
//    CpuState->LdtLimit = VmRead32 (VMCS_32_GUEST_LDTR_LIMIT_INDEX);

    CpuState->IdtBaseHiDword = (UINT32)RShiftU64 (VmReadN (VMCS_N_GUEST_IDTR_BASE_INDEX), 32);
    CpuState->IdtBaseLoDword = (UINT32)VmReadN (VMCS_N_GUEST_IDTR_BASE_INDEX);
//    CpuState->IdtLimit = VmRead32 (VMCS_32_GUEST_IDTR_LIMIT_INDEX);

    CpuState->Rax = Reg->Rax;
    CpuState->Rcx = Reg->Rcx;
    CpuState->Rdx = Reg->Rdx;
    CpuState->Rbx = Reg->Rbx;
    CpuState->Rbp = Reg->Rbp;
    CpuState->Rsi = Reg->Rsi;
    CpuState->Rdi = Reg->Rdi;
    WriteSyncSmmStateSaveAreaIa32eGpr (Index, CpuState, FALSE);
    CpuState->Rsp = VmReadN (VMCS_N_GUEST_RSP_INDEX);

    if (VmcsRecord->XStatePolicy != XSTATE_SCRUB) {
      WriteSyncSmmStateSaveAreaSse2 (Index, FALSE);
    }

    CpuState->Es = VmRead16 (VMCS_16_GUEST_ES_INDEX);
    CpuState->Cs = VmRead16 (VMCS_16_GUEST_CS_INDEX);
    CpuState->Ss = VmRead16 (VMCS_16_GUEST_SS_INDEX);
    CpuState->Ds = VmRead16 (VMCS_16_GUEST_DS_INDEX);
    CpuState->Fs = VmRead16 (VMCS_16_GUEST_FS_INDEX);
    CpuState->Gs = VmRead16 (VMCS_16_GUEST_GS_INDEX);

    CpuState->Ldtr = VmRead16 (VMCS_16_GUEST_LDTR_INDEX);
    CpuState->Tr   = VmRead16 (VMCS_16_GUEST_TR_INDEX);
    CpuState->Dr7  = VmReadN (VMCS_N_GUEST_DR7_INDEX);
    CpuState->Dr6  = AsmReadDr6 ();

    CpuState->Rip = VmReadN (VMCS_N_GUEST_RIP_INDEX);

    CpuState->Rflags = VmReadN (VMCS_N_GUEST_RFLAGS_INDEX);

    CpuState->Cr4 = (UINT32)VmReadN (VMCS_N_GUEST_CR4_INDEX);
    CpuState->Cr3 = VmReadN (VMCS_N_GUEST_CR3_INDEX);
    CpuState->Cr0 = VmReadN (VMCS_N_GUEST_CR0_INDEX);
  } else {
    CpuState->GdtBaseHiDword = 0;
    CpuState->GdtBaseLoDword = 0;
//    CpuState->GdtLimit = 0;

    CpuState->LdtBaseHiDword = 0;
    CpuState->LdtBaseLoDword = 0;
//    CpuState->LdtLimit = 0;

    CpuState->IdtBaseHiDword = 0;
    CpuState->IdtBaseLoDword = 0;
//    CpuState->IdtLimit = 0;

    CpuState->Rax = 0;
    CpuState->Rcx = 0;
    CpuState->Rdx = 0;
    CpuState->Rbx = 0;
    CpuState->Rbp = 0;
    CpuState->Rsi = 0;
    CpuState->Rdi = 0;
    WriteSyncSmmStateSaveAreaIa32eGpr (Index, CpuState, TRUE);
    CpuState->Rsp = 0;

    CpuState->Es = 0;
    CpuState->Cs = 0;
    CpuState->Ss = 0;
    CpuState->Ds = 0;
    CpuState->Fs = 0;
    CpuState->Gs = 0;

    CpuState->Ldtr = 0;
    CpuState->Tr   = 0;
    CpuState->Dr7  = 0;
    CpuState->Dr6  = 0;

    CpuState->Rip = 0;

    CpuState->Rflags = 0;

    CpuState->Cr4 = 0;
    CpuState->Cr3 = 0;
    CpuState->Cr0 = 0;
  }
  mGuestContextCommonSmi.GuestContextPerCpu[Index].InfoBasic.Uint32 = VmRead32 (VMCS_32_RO_EXIT_REASON_INDEX);
  mGuestContextCommonSmi.GuestContextPerCpu[Index].Qualification.UintN = VmReadN (VMCS_N_RO_EXIT_QUALIFICATION_INDEX);

  CpuState->IOMisc = 0;
  //
  // Init IO restart as 0x0
  //
  CpuState->IORestart = 0x0;

  if (mGuestContextCommonSmi.GuestContextPerCpu[Index].InfoBasic.Bits.Reason == VmExitReasonIoSmi) {
    if ((VmcsRecord->DomainType & DOMAIN_CONFIDENTIALITY) == 0) {
      CpuState->IoEip = VmReadN (VMCS_N_GUEST_RIP_INDEX) - VmRead32 (VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX);
      //CpuState->IoEip = VmReadN (VMCS_N_RO_IO_RIP_INDEX);
      CpuState->IoRcx = VmReadN (VMCS_N_RO_IO_RCX_INDEX);
      CpuState->IoRsi = VmReadN (VMCS_N_RO_IO_RSI_INDEX);
      CpuState->IoRdi = VmReadN (VMCS_N_RO_IO_RDI_INDEX);
    } else {
      CpuState->IoEip = 0;
      CpuState->IoRcx = 0;
      CpuState->IoRsi = 0;
      CpuState->IoRdi = 0;
    }

    Qualification.UintN = mGuestContextCommonSmi.GuestContextPerCpu[Index].Qualification.UintN;

    IOMisc.Uint32 = 0;
    IOMisc.Bits.IoSmi = 1;
    IOMisc.Bits.Length = Qualification.IoInstruction.Size + 1;
    IOMisc.Bits.Direction = Qualification.IoInstruction.Direction;
    IOMisc.Bits.String = Qualification.IoInstruction.String;
    IOMisc.Bits.Rep = Qualification.IoInstruction.Rep;
    IOMisc.Bits.OpEncoding = Qualification.IoInstruction.Operand;
    IOMisc.Bits.Port = Qualification.IoInstruction.PortNum;
    if ((VmcsRecord->DomainType & DOMAIN_CONFIDENTIALITY) == 0) {
      CpuState->IOMisc = IOMisc.Uint32;
    }

#ifdef DOMAIN_TYPE_CHECK
    //
    // Consult DomainType to decide if populate IO_MISC
    //
    TrappedIoDesc = GetStmResourceTrappedIo (mHostContextCommon.MleProtectedTrappedIoResource.Base, (UINT16)Qualification.IoInstruction.PortNum);
    if ((((VmcsRecord->DomainType & DOMAIN_DISALLOWED_IO_IN) == 0) && (TrappedIoDesc != NULL) && (TrappedIoDesc->In != 0) && (Qualification.IoInstruction.Direction != 0)) ||
        (((VmcsRecord->DomainType & DOMAIN_DISALLOWED_IO_OUT) == 0) && (TrappedIoDesc != NULL) && (TrappedIoDesc->Out != 0) && (Qualification.IoInstruction.Direction == 0))) {
      CpuState->IOMisc = IOMisc.Uint32;
    }
    //
    // Handle 2 special cases on populating DX EAX/AX/AL
    //
    if (((VmcsRecord->DomainType & DOMAIN_DISALLOWED_IO_IN) == 0) && (TrappedIoDesc != NULL) && (TrappedIoDesc->In != 0) && (Qualification.IoInstruction.Direction != 0)) {
      CopyMem (&CpuState->Rdx, &Reg->Rdx, sizeof(UINT16));
    }
    if (((VmcsRecord->DomainType & DOMAIN_DISALLOWED_IO_OUT) == 0) && (TrappedIoDesc != NULL) && (TrappedIoDesc->Out != 0) && (Qualification.IoInstruction.Direction == 0)) {
      CopyMem (&CpuState->Rdx, &Reg->Rdx, sizeof(UINT16));
      CopyMem (&CpuState->Rax, &Reg->Rax, Qualification.IoInstruction.Size + 1);
    }

    //
    // Apply Domain type degradation rules:
    // 1. A FULLY_PROT domain should avoid IO operations that are declared by BIOS via an STM_RSC_TRAPPED_IO_DESC.
    //    If such access occurs, the STM will degrade the protection properties from FULLY_PROT to FULLY_PROT_OUT or FULLY_PROT_IN.
    // 2. An INTEGRITY_PROT_OUT domain should avoid IN operations that are declared by BIOS via an STM_RSC_TRAPPED_IO_DESC.
    //    If such access occurs, the STM will degrade the protection properties from INTEGRITY_PROT_OUT to INTEGRITY_PROT_OUT_IN.
    // 3. If any domain performs an IO operation that is declared by BIOS to be a synchronous SMI API via an STM_RSC_TRAPPED_IO_DESC, the domain type is degraded to UNPROTECTED.
    //    Therefore, access to these ports should be avoided by protected domains since the effect of such an operation is to open the domain's TCB to the SMI handler.
    //
    // 4. If logging is enabled, the STM must record all domain type degradations using an ENTRY_EVT_MLE_DOMAIN_TYPE_DEGRADED log entry.
    // 5. If in any case domain type degradation is needed, but precluded by the MLE's DegradationPolicy in the VMCS database,
    //    the STM must write STM_CRASH_DOMAIN_DEGRADATION_FAILURE to the TXT.ERRORCODE register, followed by a TXT.CMD.SYS_RESET to force a system reset.
    //
    if ((VmcsRecord->DomainType & DOMAIN_CONFIDENTIALITY) != 0) {
      if (((VmcsRecord->DomainType & DOMAIN_DISALLOWED_IO_IN) != 0) && (TrappedIoDesc != NULL) && (TrappedIoDesc->In != 0) && (Qualification.IoInstruction.Direction != 0)) {
        // Degrade IN
        DEBUG ((EFI_D_INFO, "Degrade IN for %02x\n", (UINTN)VmcsRecord->DomainType));
        if (VmcsRecord->DegradationPolicy <= (VmcsRecord->DomainType & ~DOMAIN_DISALLOWED_IO_IN)) {
          // Allow
          CopyMem (&CpuState->Rdx, &Reg->Rdx, sizeof(UINT16));
          AddEventLogDomainDegration (ExecutiveVmcsPtr, (UINT8)(VmcsRecord->DomainType & ~DOMAIN_DISALLOWED_IO_IN), (UINT8)VmcsRecord->DegradationPolicy);
          VmcsRecord->DegradedDomainType = VmcsRecord->DomainType & ~DOMAIN_DISALLOWED_IO_IN;
        } else {
          // Denied
          StmTxtReset (STM_CRASH_DOMAIN_DEGRADATION_FAILURE);
        }
      }
      if (((VmcsRecord->DomainType & DOMAIN_DISALLOWED_IO_OUT) != 0) && (TrappedIoDesc != NULL) && (TrappedIoDesc->Out != 0) && (Qualification.IoInstruction.Direction == 0)) {
        // Degrade OUT
        DEBUG ((EFI_D_INFO, "Degrade OUT for %02x\n", (UINTN)VmcsRecord->DomainType));
        if (VmcsRecord->DegradationPolicy <= (VmcsRecord->DomainType & ~DOMAIN_DISALLOWED_IO_OUT)) {
          // Allow
          CopyMem (&CpuState->Rdx, &Reg->Rdx, sizeof(UINT16));
          CopyMem (&CpuState->Rax, &Reg->Rax, Qualification.IoInstruction.Size + 1);
          AddEventLogDomainDegration (ExecutiveVmcsPtr, (UINT8)(VmcsRecord->DomainType & ~DOMAIN_DISALLOWED_IO_OUT), (UINT8)VmcsRecord->DegradationPolicy);
          VmcsRecord->DegradedDomainType = VmcsRecord->DomainType & ~DOMAIN_DISALLOWED_IO_OUT;
        } else {
          // Denied
          StmTxtReset (STM_CRASH_DOMAIN_DEGRADATION_FAILURE);
        }
      }
    }
    if ((TrappedIoDesc != NULL) && (TrappedIoDesc->Api != 0)) {
      if (VmcsRecord->DomainType != DOMAIN_UNPROTECTED) {
        DEBUG ((EFI_D_INFO, "Degrade API for %02x\n", (UINTN)VmcsRecord->DomainType));
        if (VmcsRecord->DegradationPolicy <= DOMAIN_UNPROTECTED) {
          // Allow
          CpuState->Rax = Reg->Rax;
          CpuState->Rcx = Reg->Rcx;
          CpuState->Rdx = Reg->Rdx;
          CpuState->Rbx = Reg->Rbx;
          CpuState->Rbp = Reg->Rbp;
          CpuState->Rsi = Reg->Rsi;
          CpuState->Rdi = Reg->Rdi;
          WriteSyncSmmStateSaveAreaIa32eGpr (Index, CpuState, FALSE);
          CpuState->Rsp = VmReadN (VMCS_N_GUEST_RSP_INDEX);
          AddEventLogDomainDegration (ExecutiveVmcsPtr, DOMAIN_UNPROTECTED, (UINT8)VmcsRecord->DegradationPolicy);
          VmcsRecord->DegradedDomainType = DOMAIN_UNPROTECTED;
        } else {
          // Denied
          StmTxtReset (STM_CRASH_DOMAIN_DEGRADATION_FAILURE);
        }
      }
    }
#endif
  }

  //
  // Init AutoHalt restart
  //
  if (VmRead32 (VMCS_32_GUEST_ACTIVITY_STATE_INDEX) == GUEST_ACTIVITY_STATE_HLT) {
    CpuState->AutoHALTRestart = 0x1;
  } else {
    CpuState->AutoHALTRestart = 0x0;
  }

  if (sizeof(UINTN) == sizeof(UINT64)) {
    CpuState->Ia32Efer = VmRead64 (VMCS_64_GUEST_IA32_EFER_INDEX);
    mGuestContextCommonSmi.GuestContextPerCpu[Index].Efer = CpuState->Ia32Efer;
  }

  TxtProcessorSmmDescriptor->StmSmmState.EptEnabled = 1;
  TxtProcessorSmmDescriptor->StmSmmState.DomainType = (UINT8)VmcsRecord->DomainType;
  TxtProcessorSmmDescriptor->StmSmmState.XStatePolicy = (UINT8)VmcsRecord->XStatePolicy;
  if (VmcsRecord->XStatePolicy != XSTATE_READWRITE) {
    XStateSave (Index);
  }
  if (VmcsRecord->XStatePolicy == XSTATE_SCRUB) {
    XStateScrub (Index);
  }

  return ;
}

/**

  This function read SMM state save area sync to VMCS.

  @param Index CPU index

**/
VOID
ReadSyncSmmStateSaveArea (
  IN UINT32 Index
  )
{
  STM_SMM_CPU_STATE                  *CpuState;
  UINT64                             ExecutiveVmcsPtr;
  VMCS_RECORD_STRUCTURE              *VmcsRecord;
  TXT_PROCESSOR_SMM_DESCRIPTOR       *TxtProcessorSmmDescriptor;
  X86_REGISTER                       *Reg;
#ifdef DOMAIN_TYPE_CHECK
  VM_EXIT_QUALIFICATION              Qualification;
  STM_RSC_TRAPPED_IO_DESC            *TrappedIoDesc;
#endif

  Reg = &mGuestContextCommonSmi.GuestContextPerCpu[Index].Register;

  ExecutiveVmcsPtr = VmRead64 (VMCS_64_CONTROL_EXECUTIVE_VMCS_PTR_INDEX);
  VmcsRecord = GetVmcsRecord (mHostContextCommon.VmcsDatabase, ExecutiveVmcsPtr);
  if (VmcsRecord == NULL) {
    VmcsRecord = &mDefaultVmcsRecord;
  }
#if 0
  DEBUG ((EFI_D_INFO, "GetVmcsRecord - %08x\n", VmcsRecord));
#endif
  TxtProcessorSmmDescriptor = (TXT_PROCESSOR_SMM_DESCRIPTOR *)(UINTN)mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor;

  CpuState = (STM_SMM_CPU_STATE *)(UINTN)(mHostContextCommon.HostContextPerCpu[Index].Smbase + SMM_CPU_STATE_OFFSET);

  //
  // Basic info
  //
  if (CpuState->Smbase != mHostContextCommon.HostContextPerCpu[Index].Smbase) {
    CpuDeadLoop ();
  }
  
  // If this bit is not set by the SMM guest, the STM will not attempt to propagate any
  // changes the SMI handler has made to the SMRAM state save area to the
  // interrupted context's VMCS or process register state.
  if (TxtProcessorSmmDescriptor->SmmResumeState.SmramToVmcsRestoreRequired != 0) {
    if (VmcsRecord->DegradedDomainType == DOMAIN_UNPROTECTED) {
      //
      // Start sync writable state
      //
      Reg->Rax = (UINTN)CpuState->Rax;
      Reg->Rcx = (UINTN)CpuState->Rcx;
      Reg->Rdx = (UINTN)CpuState->Rdx;
      Reg->Rbx = (UINTN)CpuState->Rbx;
      Reg->Rbp = (UINTN)CpuState->Rbp;
      Reg->Rsi = (UINTN)CpuState->Rsi;
      Reg->Rdi = (UINTN)CpuState->Rdi;
      ReadSyncSmmStateSaveAreaIa32eGpr (Index, CpuState);

      if (VmcsRecord->XStatePolicy == XSTATE_READWRITE) {
        ReadSyncSmmStateSaveAreaSse2 (Index);
      }

      VmWriteN (VMCS_N_GUEST_RSP_INDEX, (UINTN)CpuState->Rsp);

      VmWriteN (VMCS_N_GUEST_RIP_INDEX, (UINTN)CpuState->Rip);

      VmWriteN (VMCS_N_GUEST_RFLAGS_INDEX, (UINTN)CpuState->Rflags);
    }

#ifdef DOMAIN_TYPE_CHECK
    Qualification.UintN = mGuestContextCommonSmi.GuestContextPerCpu[Index].Qualification.UintN;
    //
    // Consult DomainType to decide if propagate EAX
    //
    if (mGuestContextCommonSmi.GuestContextPerCpu[Index].InfoBasic.Bits.Reason == VmExitReasonIoSmi) {
      TrappedIoDesc = GetStmResourceTrappedIo (mHostContextCommon.MleProtectedTrappedIoResource.Base, (UINT16)Qualification.IoInstruction.PortNum);
      if (((VmcsRecord->DegradedDomainType & DOMAIN_DISALLOWED_IO_IN) == 0) && (TrappedIoDesc != NULL) && (TrappedIoDesc->In != 0) && (Qualification.IoInstruction.Direction != 0)) {
        CopyMem (&Reg->Rax, &CpuState->Rax, Qualification.IoInstruction.Size + 1);
      }
    }
#endif
  }

  if (mGuestContextCommonSmi.GuestContextPerCpu[Index].InfoBasic.Bits.Reason == VmExitReasonIoSmi) {
    if (CpuState->IORestart == 0xFF) {
      //
      // SMM code required for re-execute IO instruction
      //
      VmWriteN (VMCS_N_GUEST_RIP_INDEX, VmReadN (VMCS_N_GUEST_RIP_INDEX) - VmRead32 (VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX));
    }
  }

  if (CpuState->AutoHALTRestart == 0x0) {
    //
    // exit halt state
    //
    VmWrite32 (VMCS_32_GUEST_ACTIVITY_STATE_INDEX, GUEST_ACTIVITY_STATE_ACTIVE);
  } else {
    //
    // Still halt
    //
    VmWrite32 (VMCS_32_GUEST_ACTIVITY_STATE_INDEX, GUEST_ACTIVITY_STATE_HLT);
  }

  if (VmcsRecord->XStatePolicy != XSTATE_READWRITE) {
    XStateRestore (Index);
  }

  return ;
}
