/** @file
  SMM BIOS exception

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

  This function resume to BIOS exception handler.

  @param Index CPU index

**/
VOID
ResumeToBiosExceptionHandler (
  UINT32 Index
  )
{
  STM_PROTECTION_EXCEPTION_HANDLER          *StmProtectionExceptionHandler;
  STM_PROTECTION_EXCEPTION_STACK_FRAME_X64  *StackFrame;
  UINTN                                     Rflags;
  X86_REGISTER                              *Reg;

  Reg = &mGuestContextCommonSmm.GuestContextPerCpu[Index].Register;

  StmProtectionExceptionHandler = &mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->StmProtectionExceptionHandler;

  //
  // Check valid
  //
  if (StmProtectionExceptionHandler->SpeRip == 0) {
    DEBUG ((EFI_D_INFO, "SpeRip unsupported!\n"));
    // Unsupported;
    return ;
  }

  //
  // Fill exception stack
  //
  StackFrame = (STM_PROTECTION_EXCEPTION_STACK_FRAME_X64 *)(UINTN)StmProtectionExceptionHandler->SpeRsp;
  StackFrame -= 1;

  //
  // make it 16bytes align
  //
  StackFrame = (STM_PROTECTION_EXCEPTION_STACK_FRAME_X64 *)(((UINTN)StackFrame - 0x10) & ~0xF);
  StackFrame = (STM_PROTECTION_EXCEPTION_STACK_FRAME_X64 *)((UINTN)StackFrame - 0x8);

  mGuestContextCommonSmm.GuestContextPerCpu[Index].InfoBasic.Uint32 = VmRead32 (VMCS_32_RO_EXIT_REASON_INDEX);

  mGuestContextCommonSmm.GuestContextPerCpu[Index].VmExitInstructionLength = VmRead32 (VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX);

  StackFrame->VmcsExitQualification = VmReadN (VMCS_N_RO_EXIT_QUALIFICATION_INDEX);
  StackFrame->VmcsExitInstructionLength = VmRead32 (VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX);
  StackFrame->VmcsExitInstructionInfo = VmRead32 (VMCS_32_RO_VMEXIT_INSTRUCTION_INFO_INDEX);
  switch (mGuestContextCommonSmm.GuestContextPerCpu[Index].InfoBasic.Bits.Reason) {
  case VmExitReasonExceptionNmi:
  case VmExitReasonEptViolation:
    if (StmProtectionExceptionHandler->PageViolationException) {
      StackFrame->ErrorCode = TxtSmmPageViolation;
    } else {
      return ;
    }
    break;
  case VmExitReasonRdmsr:
  case VmExitReasonWrmsr:
    if (StmProtectionExceptionHandler->MsrViolationException) {
      StackFrame->ErrorCode = TxtSmmMsrViolation;
    } else {
      return ;
    }
    break;
  case VmExitReasonIoInstruction:
    if (StmProtectionExceptionHandler->IoViolationException) {
      StackFrame->ErrorCode = TxtSmmIoViolation;
    } else {
      VM_EXIT_QUALIFICATION   Qualification;
      UINT16                  Port;

      Qualification.UintN = VmReadN (VMCS_N_RO_EXIT_QUALIFICATION_INDEX);
      Port = (UINT16)Qualification.IoInstruction.PortNum;
      if ((Port >= 0xCFC) && (Port <= 0xCFF)) {
        if (StmProtectionExceptionHandler->PciViolationException) {
          StackFrame->ErrorCode = TxtSmmPciViolation;
        } else {
          return ;
        }
      } else {
        return ;
      }
    }
    break;
  case VmExitReasonCrAccess:
    if (StmProtectionExceptionHandler->RegisterViolationException) {
      StackFrame->ErrorCode = TxtSmmRegisterViolation;
    } else {
      return ;
    }
    break;
  default:
    return;
  }

  StackFrame->R15 = Reg->R15;
  StackFrame->R14 = Reg->R14;
  StackFrame->R13 = Reg->R13;
  StackFrame->R12 = Reg->R12;
  StackFrame->R11 = Reg->R11;
  StackFrame->R10 = Reg->R10;
  StackFrame->R9  = Reg->R9;
  StackFrame->R8  = Reg->R8;
  StackFrame->Rdi = Reg->Rdi;
  StackFrame->Rsi = Reg->Rsi;
  StackFrame->Rbp = Reg->Rbp;
  StackFrame->Rdx = Reg->Rdx;
  StackFrame->Rcx = Reg->Rcx;
  StackFrame->Rbx = Reg->Rbx;
  StackFrame->Rax = Reg->Rax;
  StackFrame->Cr8 = 0; // AsmReadCr8();
  StackFrame->Cr3 = VmReadN (VMCS_N_GUEST_CR3_INDEX);
  if (mGuestContextCommonSmm.GuestContextPerCpu[Index].InfoBasic.Bits.Reason == VmExitReasonEptViolation) {
    // For SMM handle, linear addr == physical addr
    StackFrame->Cr2 = (UINTN)VmRead64(VMCS_64_RO_GUEST_PHYSICAL_ADDR_INDEX);
  } else {
    StackFrame->Cr2 = AsmReadCr2();
  }
  StackFrame->Cr0 = VmReadN (VMCS_N_GUEST_CR0_INDEX);
  StackFrame->Rip = VmReadN (VMCS_N_GUEST_RIP_INDEX);
  StackFrame->Cs = VmRead16 (VMCS_16_GUEST_CS_INDEX);
  StackFrame->Rflags = VmReadN (VMCS_N_GUEST_RFLAGS_INDEX);
  StackFrame->Rsp = VmReadN (VMCS_N_GUEST_RSP_INDEX);
  StackFrame->Ss = VmRead16 (VMCS_16_GUEST_SS_INDEX);

  DEBUG ((EFI_D_INFO, "ResumeToBiosExceptionHandler - %d\n", (UINTN)Index));

  //
  // Start resume to BiosExceptionHandler
  //
  VmWriteN (VMCS_N_GUEST_RIP_INDEX, (UINTN)StmProtectionExceptionHandler->SpeRip);
  VmWriteN (VMCS_N_GUEST_RSP_INDEX, (UINTN)StackFrame);
  VmWrite16 (VMCS_16_GUEST_SS_INDEX, (UINT16)StmProtectionExceptionHandler->SpeSs);

  STM_PERF_START (Index, 0, "BiosSmmHandler", "ResumeToBiosExceptionHandler");

  Rflags = AsmVmResume (Reg);
  // BUGBUG: - AsmVmLaunch if AsmVmResume fail
  if (VmRead32 (VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX) == VmxFailErrorVmResumeWithNonLaunchedVmcs) {
//    DEBUG ((EFI_D_ERROR, "(STM):-(\n", (UINTN)Index));
    Rflags = AsmVmLaunch (Reg);
  }
  AcquireSpinLock (&mHostContextCommon.DebugLock);
  DEBUG ((EFI_D_ERROR, "!!!ResumeToBiosExceptionHandler FAIL!!!\n"));
  DEBUG ((EFI_D_ERROR, "Rflags: %08x\n", Rflags));
  DEBUG ((EFI_D_ERROR, "VMCS_32_RO_VM_INSTRUCTION_ERROR: %08x\n", (UINTN)VmRead32 (VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX)));
  DumpVmcsAllField ();
  DumpRegContext (Reg);
  ReleaseSpinLock (&mHostContextCommon.DebugLock);
  CpuDeadLoop ();
  return ;
}

/**

  This function return from BIOS exception handler.

  @param Index CPU index

**/
VOID
ReturnFromBiosExceptionHandler (
  UINT32 Index
  )
{
  STM_PROTECTION_EXCEPTION_HANDLER          *StmProtectionExceptionHandler;
  STM_PROTECTION_EXCEPTION_STACK_FRAME_X64  *StackFrame;
  UINTN                                     Rflags;
  X86_REGISTER                              *Reg;

  Reg = &mGuestContextCommonSmm.GuestContextPerCpu[Index].Register;

  StmProtectionExceptionHandler = &mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->StmProtectionExceptionHandler;

  //
  // Check valid
  //
  if (StmProtectionExceptionHandler->SpeRip == 0) {
    // Unsupported;
    return ;
  }

  //
  // Get exception stack
  //
  StackFrame = (STM_PROTECTION_EXCEPTION_STACK_FRAME_X64 *)(UINTN)StmProtectionExceptionHandler->SpeRsp;
  StackFrame -= 1;

  //
  // make it 16bytes align
  //
  StackFrame = (STM_PROTECTION_EXCEPTION_STACK_FRAME_X64 *)(((UINTN)StackFrame - 0x10) & ~0xF);
  StackFrame = (STM_PROTECTION_EXCEPTION_STACK_FRAME_X64 *)((UINTN)StackFrame - 0x8);

  Reg->R15 = StackFrame->R15;
  Reg->R14 = StackFrame->R14;
  Reg->R13 = StackFrame->R13;
  Reg->R12 = StackFrame->R12;
  Reg->R11 = StackFrame->R11;
  Reg->R10 = StackFrame->R10;
  Reg->R9  = StackFrame->R9;
  Reg->R8  = StackFrame->R8;
  Reg->Rdi = StackFrame->Rdi;
  Reg->Rsi = StackFrame->Rsi;
  Reg->Rbp = StackFrame->Rbp;
  Reg->Rdx = StackFrame->Rdx;
  Reg->Rcx = StackFrame->Rcx;
  Reg->Rbx = StackFrame->Rbx;
  Reg->Rax = StackFrame->Rax;
//  AsmWriteCr8 (StackFrame->Cr8);
  VmWriteN (VMCS_N_GUEST_CR3_INDEX, StackFrame->Cr3);
  AsmWriteCr2 (StackFrame->Cr2);
  VmWriteN (VMCS_N_GUEST_CR0_INDEX, StackFrame->Cr0);
  VmWriteN (VMCS_N_GUEST_RIP_INDEX, StackFrame->Rip);
  VmWrite16 (VMCS_16_GUEST_CS_INDEX, (UINT16)StackFrame->Cs);
  VmWriteN (VMCS_N_GUEST_RFLAGS_INDEX, StackFrame->Rflags);
  VmWriteN (VMCS_N_GUEST_RSP_INDEX, StackFrame->Rsp);
  VmWrite16 (VMCS_16_GUEST_SS_INDEX, (UINT16)StackFrame->Ss);

  DEBUG ((EFI_D_INFO, "ReturnFromBiosExceptionHandler - %d\n", (UINTN)Index));

  STM_PERF_START (Index, 0, "BiosSmmHandler", "ReturnFromBiosExceptionHandler");

  //
  // Start resume back to BiosExceptionHandler
  //
  Rflags = AsmVmResume (Reg);
  // BUGBUG: - AsmVmLaunch if AsmVmResume fail
  if (VmRead32 (VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX) == VmxFailErrorVmResumeWithNonLaunchedVmcs) {
//    DEBUG ((EFI_D_ERROR, "(STM):-(\n", (UINTN)Index));
    Rflags = AsmVmLaunch (Reg);
  }
  AcquireSpinLock (&mHostContextCommon.DebugLock);
  DEBUG ((EFI_D_ERROR, "!!!ReturnFromBiosExceptionHandler FAIL!!!\n"));
  DEBUG ((EFI_D_ERROR, "Rflags: %08x\n", Rflags));
  DEBUG ((EFI_D_ERROR, "VMCS_32_RO_VM_INSTRUCTION_ERROR: %08x\n", (UINTN)VmRead32 (VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX)));
  DumpVmcsAllField ();
  DumpRegContext (Reg);
  ReleaseSpinLock (&mHostContextCommon.DebugLock);
  CpuDeadLoop ();
  return ;
}
