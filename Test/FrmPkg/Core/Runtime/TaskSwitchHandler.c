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

CHAR8 *mTaskTypeStr[] = {
  "TASK_SWITCH_SOURCE_CALL",
  "TASK_SWITCH_SOURCE_IRET",
  "TASK_SWITCH_SOURCE_JMP",
  "TASK_SWITCH_SOURCE_TASK_GATE",
};

/**

  This function is task switch handler.

  @param Index CPU index

**/
VOID
TaskSwitchHandler (
  IN UINT32 Index
  )
{
  VM_EXIT_QUALIFICATION   Qualification;
  GDT_ENTRY               *GdtEntry;
  UINTN                   Gdtr;
  TASK_STATE              *Tss;
  UINT16                  CurrentTr;
  TASK_STATE              *CurrentTss;

//  DEBUG ((EFI_D_INFO, "(FRM) !!!TaskSwitchHandler - %08x\n", (UINTN)Index));
  DEBUG ((EFI_D_INFO, "T"));

  Qualification.UintN = VmReadN (VMCS_N_RO_EXIT_QUALIFICATION_INDEX);

//  DEBUG ((EFI_D_INFO, "(FRM) !!!TaskType - %x, %a\n", (UINTN)Qualification.TaskSwitch.TaskType, mTaskTypeStr[Qualification.TaskSwitch.TaskType]));
//  DEBUG ((EFI_D_INFO, "(FRM) !!!TSS - %04x\n", (UINTN)Qualification.TaskSwitch.Tss));

  Gdtr = VmReadN (VMCS_N_GUEST_GDTR_BASE_INDEX);
  Gdtr = GuestVirtualToGuestPhysical (Index, Gdtr);

  //
  // Current task
  //
  CurrentTr = VmRead16 (VMCS_16_GUEST_TR_INDEX);
  GdtEntry = (GDT_ENTRY *)(Gdtr + (CurrentTr & ~0x7));
  CurrentTss = (TASK_STATE *)(UINTN)BaseFromGdtEntry(GdtEntry);
  CurrentTss = (TASK_STATE *)GuestVirtualToGuestPhysical (Index, (UINTN)CurrentTss);

  if ((Qualification.TaskSwitch.TaskType == TASK_SWITCH_SOURCE_IRET) ||
      (Qualification.TaskSwitch.TaskType == TASK_SWITCH_SOURCE_JMP)) {
    GdtEntry->Attribute &= ~DESCRIPTOR_TSS_BUSY; // Clear busy flag
  }

  //
  // New task
  //
  GdtEntry = (GDT_ENTRY *)(Gdtr + (Qualification.TaskSwitch.Tss & ~0x7));
//  DEBUG ((EFI_D_INFO, "(FRM) !!!Gdt - %016lx\n", *(UINT64 *)GdtEntry));
  Tss = (TASK_STATE *)(UINTN)BaseFromGdtEntry(GdtEntry);
  Tss = (TASK_STATE *)GuestVirtualToGuestPhysical (Index, (UINTN)Tss);

  if (Qualification.TaskSwitch.TaskType == TASK_SWITCH_SOURCE_IRET) {
    ASSERT (GdtEntry->Attribute & DESCRIPTOR_TSS_BUSY);
  } else {
    ASSERT ((GdtEntry->Attribute & DESCRIPTOR_TSS_BUSY) == 0);
    GdtEntry->Attribute |= DESCRIPTOR_TSS_BUSY; // Set busy flag
  }

  VmWrite16 (VMCS_16_GUEST_TR_INDEX, (UINT16)Qualification.TaskSwitch.Tss);
  VmWriteN (VMCS_N_GUEST_TR_BASE_INDEX, BaseFromGdtEntry(GdtEntry));
  VmWrite32 (VMCS_32_GUEST_TR_LIMIT_INDEX, LimitFromGdtEntry(GdtEntry));

  //
  // Save old data
  //
  if ((Qualification.TaskSwitch.TaskType == TASK_SWITCH_SOURCE_CALL) ||
      (Qualification.TaskSwitch.TaskType == TASK_SWITCH_SOURCE_TASK_GATE)) {
    CurrentTss->EAX = (UINT32)mGuestContextCommon.GuestContextPerCpu[Index].Register.Rax;
    CurrentTss->EBX = (UINT32)mGuestContextCommon.GuestContextPerCpu[Index].Register.Rbx;
    CurrentTss->ECX = (UINT32)mGuestContextCommon.GuestContextPerCpu[Index].Register.Rcx;
    CurrentTss->EDX = (UINT32)mGuestContextCommon.GuestContextPerCpu[Index].Register.Rdx;
    CurrentTss->ESI = (UINT32)mGuestContextCommon.GuestContextPerCpu[Index].Register.Rsi;
    CurrentTss->EDI = (UINT32)mGuestContextCommon.GuestContextPerCpu[Index].Register.Rdi;
    CurrentTss->EBP = (UINT32)mGuestContextCommon.GuestContextPerCpu[Index].Register.Rbp;
    CurrentTss->ESP = (UINT32)mGuestContextCommon.GuestContextPerCpu[Index].Register.Rsp;

    CurrentTss->EIP = (UINT32)VmReadN (VMCS_N_GUEST_RIP_INDEX);
    if (Qualification.TaskSwitch.TaskType == TASK_SWITCH_SOURCE_CALL) {
      CurrentTss->EIP += VmRead32(VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX); // Next instruction
    }

    CurrentTss->ES = VmRead16 (VMCS_16_GUEST_ES_INDEX);
    CurrentTss->CS = VmRead16 (VMCS_16_GUEST_CS_INDEX);
    CurrentTss->SS = VmRead16 (VMCS_16_GUEST_SS_INDEX);
    CurrentTss->DS = VmRead16 (VMCS_16_GUEST_DS_INDEX);
    CurrentTss->FS = VmRead16 (VMCS_16_GUEST_FS_INDEX);
    CurrentTss->GS = VmRead16 (VMCS_16_GUEST_GS_INDEX);

    CurrentTss->CR3 = (UINT32)VmReadN (VMCS_N_GUEST_CR3_INDEX);
    CurrentTss->LDTSegSelector = VmRead16 (VMCS_16_GUEST_LDTR_INDEX);

  }

  CurrentTss->EFlag = (UINT32)VmReadN (VMCS_N_GUEST_RFLAGS_INDEX);
  if (Qualification.TaskSwitch.TaskType == TASK_SWITCH_SOURCE_IRET) {
    CurrentTss->EFlag &= ~RFLAGS_NT;
  }

  //
  // Prepare new data
  //
  if ((Qualification.TaskSwitch.TaskType == TASK_SWITCH_SOURCE_CALL) ||
      (Qualification.TaskSwitch.TaskType == TASK_SWITCH_SOURCE_TASK_GATE)) {
    Tss->Link = CurrentTr;
  }
  mGuestContextCommon.GuestContextPerCpu[Index].Register.Rax = Tss->EAX;
  mGuestContextCommon.GuestContextPerCpu[Index].Register.Rbx = Tss->EBX;
  mGuestContextCommon.GuestContextPerCpu[Index].Register.Rcx = Tss->ECX;
  mGuestContextCommon.GuestContextPerCpu[Index].Register.Rdx = Tss->EDX;
  mGuestContextCommon.GuestContextPerCpu[Index].Register.Rsi = Tss->ESI;
  mGuestContextCommon.GuestContextPerCpu[Index].Register.Rdi = Tss->EDI;
  mGuestContextCommon.GuestContextPerCpu[Index].Register.Rbp = Tss->EBP;
  mGuestContextCommon.GuestContextPerCpu[Index].Register.Rsp = Tss->ESP;

  VmWriteN (VMCS_N_GUEST_RIP_INDEX, Tss->EIP);

  VmWrite16 (VMCS_16_GUEST_ES_INDEX, Tss->ES);
  VmWrite16 (VMCS_16_GUEST_CS_INDEX, Tss->CS);
  VmWrite16 (VMCS_16_GUEST_SS_INDEX, Tss->SS);
  VmWrite16 (VMCS_16_GUEST_DS_INDEX, Tss->DS);
  VmWrite16 (VMCS_16_GUEST_FS_INDEX, Tss->FS);
  VmWrite16 (VMCS_16_GUEST_GS_INDEX, Tss->GS);

  VmWriteN (VMCS_N_GUEST_CR3_INDEX, Tss->CR3);
  VmWrite16 (VMCS_16_GUEST_LDTR_INDEX, Tss->LDTSegSelector);

  GdtEntry = (GDT_ENTRY *)(Gdtr + (Tss->LDTSegSelector & ~0x7));
  VmWriteN (VMCS_N_GUEST_LDTR_BASE_INDEX,             BaseFromGdtEntry(GdtEntry));
  if (Tss->LDTSegSelector == 0) {
    VmWrite32 (VMCS_32_GUEST_LDTR_ACCESS_RIGHT_INDEX, 0x1082);
  } else {
    VmWrite32 (VMCS_32_GUEST_LDTR_ACCESS_RIGHT_INDEX, ArFromGdtEntry(GdtEntry));
  }
  VmWrite32 (VMCS_32_GUEST_LDTR_LIMIT_INDEX,          LimitFromGdtEntry(GdtEntry));

  if ((Qualification.TaskSwitch.TaskType == TASK_SWITCH_SOURCE_CALL) ||
      (Qualification.TaskSwitch.TaskType == TASK_SWITCH_SOURCE_TASK_GATE)) {
    Tss->EFlag |= RFLAGS_NT;
  }
  VmWriteN (VMCS_N_GUEST_RFLAGS_INDEX, Tss->EFlag | 0x2); // set reserved bit.

  GdtEntry = (GDT_ENTRY *)(Gdtr + (Qualification.TaskSwitch.Tss & ~0x7));
  if (Tss->EFlag & RFLAGS_VM) {
    // VM86 mode
    VmWriteN (VMCS_N_GUEST_ES_BASE_INDEX, Tss->ES << 4);
    VmWriteN (VMCS_N_GUEST_CS_BASE_INDEX, Tss->CS << 4);
    VmWriteN (VMCS_N_GUEST_SS_BASE_INDEX, Tss->SS << 4);
    VmWriteN (VMCS_N_GUEST_DS_BASE_INDEX, Tss->DS << 4);
    VmWriteN (VMCS_N_GUEST_FS_BASE_INDEX, Tss->FS << 4);
    VmWriteN (VMCS_N_GUEST_GS_BASE_INDEX, Tss->GS << 4);

    VmWrite32 (VMCS_32_GUEST_ES_ACCESS_RIGHT_INDEX, 0x00f3);
    VmWrite32 (VMCS_32_GUEST_CS_ACCESS_RIGHT_INDEX, 0x00f3);
    VmWrite32 (VMCS_32_GUEST_SS_ACCESS_RIGHT_INDEX, 0x00f3);
    VmWrite32 (VMCS_32_GUEST_DS_ACCESS_RIGHT_INDEX, 0x00f3);
    VmWrite32 (VMCS_32_GUEST_FS_ACCESS_RIGHT_INDEX, 0x00f3);
    VmWrite32 (VMCS_32_GUEST_GS_ACCESS_RIGHT_INDEX, 0x00f3);

    VmWrite32 (VMCS_32_GUEST_ES_LIMIT_INDEX, 0x0000ffff);
    VmWrite32 (VMCS_32_GUEST_CS_LIMIT_INDEX, 0x0000ffff);
    VmWrite32 (VMCS_32_GUEST_SS_LIMIT_INDEX, 0x0000ffff);
    VmWrite32 (VMCS_32_GUEST_DS_LIMIT_INDEX, 0x0000ffff);
    VmWrite32 (VMCS_32_GUEST_FS_LIMIT_INDEX, 0x0000ffff);
    VmWrite32 (VMCS_32_GUEST_GS_LIMIT_INDEX, 0x0000ffff);

    VmWrite32 (VMCS_32_GUEST_TR_ACCESS_RIGHT_INDEX, DESCRIPTOR_PRESENT | DESCRIPTOR_TSS_N_BUSY);

  } else if (((GdtEntry->Attribute & DESCRIPTOR_TYPE_MASK) | DESCRIPTOR_TSS_BUSY) == DESCRIPTOR_TSS_N_BUSY) {
    // 32 bit
    GdtEntry = (GDT_ENTRY *)(Gdtr + (Tss->ES & ~0x7));
    VmWriteN (VMCS_N_GUEST_ES_BASE_INDEX,           BaseFromGdtEntry(GdtEntry));
    VmWrite32 (VMCS_32_GUEST_ES_ACCESS_RIGHT_INDEX, ArFromGdtEntry(GdtEntry) | DESCRIPTOR_DATA_ACCESS);
    VmWrite32 (VMCS_32_GUEST_ES_LIMIT_INDEX,        LimitFromGdtEntry(GdtEntry));

    GdtEntry = (GDT_ENTRY *)(Gdtr + (Tss->CS & ~0x7));
    VmWriteN (VMCS_N_GUEST_CS_BASE_INDEX,           BaseFromGdtEntry(GdtEntry));
    VmWrite32 (VMCS_32_GUEST_CS_ACCESS_RIGHT_INDEX, ArFromGdtEntry(GdtEntry) | DESCRIPTOR_CODE_ACCESS);
    VmWrite32 (VMCS_32_GUEST_CS_LIMIT_INDEX,        LimitFromGdtEntry(GdtEntry));

    GdtEntry = (GDT_ENTRY *)(Gdtr + (Tss->SS & ~0x7));
    VmWriteN (VMCS_N_GUEST_SS_BASE_INDEX,           BaseFromGdtEntry(GdtEntry));
    VmWrite32 (VMCS_32_GUEST_SS_ACCESS_RIGHT_INDEX, ArFromGdtEntry(GdtEntry) | DESCRIPTOR_DATA_ACCESS);
    VmWrite32 (VMCS_32_GUEST_SS_LIMIT_INDEX,        LimitFromGdtEntry(GdtEntry));

    GdtEntry = (GDT_ENTRY *)(Gdtr + (Tss->DS & ~0x7));
    VmWriteN (VMCS_N_GUEST_DS_BASE_INDEX,           BaseFromGdtEntry(GdtEntry));
    VmWrite32 (VMCS_32_GUEST_DS_ACCESS_RIGHT_INDEX, ArFromGdtEntry(GdtEntry) | DESCRIPTOR_DATA_ACCESS);
    VmWrite32 (VMCS_32_GUEST_DS_LIMIT_INDEX,        LimitFromGdtEntry(GdtEntry));

    GdtEntry = (GDT_ENTRY *)(Gdtr + (Tss->FS & ~0x7));
    VmWriteN (VMCS_N_GUEST_FS_BASE_INDEX,           BaseFromGdtEntry(GdtEntry));
    AsmWriteMsr64 (IA32_FS_BASE_MSR_INDEX,               BaseFromGdtEntry(GdtEntry));
    if (Tss->FS == 0) {
      VmWrite32 (VMCS_32_GUEST_FS_ACCESS_RIGHT_INDEX, 0x10000);
    } else {
      VmWrite32 (VMCS_32_GUEST_FS_ACCESS_RIGHT_INDEX, ArFromGdtEntry(GdtEntry) | DESCRIPTOR_DATA_ACCESS);
    }
    VmWrite32 (VMCS_32_GUEST_FS_LIMIT_INDEX,        LimitFromGdtEntry(GdtEntry));

    GdtEntry = (GDT_ENTRY *)(Gdtr + (Tss->GS & ~0x7));
    VmWriteN (VMCS_N_GUEST_GS_BASE_INDEX,           BaseFromGdtEntry(GdtEntry));
    AsmWriteMsr64 (IA32_GS_BASE_MSR_INDEX,               BaseFromGdtEntry(GdtEntry));
    if (Tss->GS == 0) {
      VmWrite32 (VMCS_32_GUEST_GS_ACCESS_RIGHT_INDEX, 0x10000);
    } else {
      VmWrite32 (VMCS_32_GUEST_GS_ACCESS_RIGHT_INDEX, ArFromGdtEntry(GdtEntry) | DESCRIPTOR_DATA_ACCESS);
    }
    VmWrite32 (VMCS_32_GUEST_GS_LIMIT_INDEX,        LimitFromGdtEntry(GdtEntry));

    VmWrite32 (VMCS_32_GUEST_TR_ACCESS_RIGHT_INDEX, DESCRIPTOR_PRESENT | DESCRIPTOR_TSS_N_BUSY);

  } else {
    DEBUG ((EFI_D_ERROR, "(FRM) !!!Other TSS\n"));
    DumpVmcsAllField ();
    CpuDeadLoop ();
  }

  VmWriteN (VMCS_N_GUEST_CR0_INDEX, VmReadN (VMCS_N_GUEST_CR0_INDEX) | CR0_TS);

  return ;
}
