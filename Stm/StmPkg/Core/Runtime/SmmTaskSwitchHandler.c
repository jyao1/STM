/** @file
  SMM TaskSwitch handler

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmRuntime.h"

GLOBAL_REMOVE_IF_UNREFERENCED
CHAR8 *mTaskTypeStr[] = {
  "TASK_SWITCH_SOURCE_CALL",
  "TASK_SWITCH_SOURCE_IRET",
  "TASK_SWITCH_SOURCE_JMP",
  "TASK_SWITCH_SOURCE_TASK_GATE",
};

/**

  This function is TaskSwitch handler for SMM.

  @param Index CPU index

**/
VOID
SmmTaskSwitchHandler (
  IN UINT32 Index
  )
{
  VM_EXIT_QUALIFICATION   Qualification;
  GDT_ENTRY               *GdtEntry;
  UINTN                   Gdtr;
  TASK_STATE              *Tss;
  UINT16                  CurrentTr;
  TASK_STATE              *CurrentTss;

//  DEBUG ((EFI_D_INFO, "!!!TaskSwitchHandler - %08x\n", (UINTN)Index));
  DEBUG ((EFI_D_INFO, "T"));

  Qualification.UintN = VmReadN (VMCS_N_RO_EXIT_QUALIFICATION_INDEX);

//  DEBUG ((EFI_D_INFO, "!!!TaskType - %x, %a\n", (UINTN)Qualification.TaskSwitch.TaskType, mTaskTypeStr[Qualification.TaskSwitch.TaskType]));
//  DEBUG ((EFI_D_INFO, "!!!TSS - %04x\n", (UINTN)Qualification.TaskSwitch.TSS));

  Gdtr = VmReadN (VMCS_N_GUEST_GDTR_BASE_INDEX);
  Gdtr = (UINTN)GuestLinearToGuestPhysical (Index, Gdtr);

  //
  // Current task
  //
  CurrentTr = VmRead16 (VMCS_16_GUEST_TR_INDEX);
  GdtEntry = (GDT_ENTRY *)(Gdtr + (CurrentTr & ~0x7));
  CurrentTss = (TASK_STATE *)(UINTN)BaseFromGdtEntry(GdtEntry);
  CurrentTss = (TASK_STATE *)(UINTN)GuestLinearToGuestPhysical (Index, (UINTN)CurrentTss);

  if ((Qualification.TaskSwitch.TaskType == TASK_SWITCH_SOURCE_IRET) ||
      (Qualification.TaskSwitch.TaskType == TASK_SWITCH_SOURCE_JMP)) {
    GdtEntry->Attribute &= ~DESCRIPTOR_TSS_BUSY; // Clear busy flag
  }

  //
  // New task
  //
  GdtEntry = (GDT_ENTRY *)(Gdtr + (Qualification.TaskSwitch.Tss & ~0x7));
//  DEBUG ((EFI_D_INFO, "!!!Gdt - %016lx\n", *(UINT64 *)GdtEntry));
  Tss = (TASK_STATE *)(UINTN)BaseFromGdtEntry(GdtEntry);
  Tss = (TASK_STATE *)(UINTN)GuestLinearToGuestPhysical (Index, (UINTN)Tss);

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
    CurrentTss->Eax = (UINT32)mGuestContextCommonSmm.GuestContextPerCpu[Index].Register.Rax;
    CurrentTss->Ebx = (UINT32)mGuestContextCommonSmm.GuestContextPerCpu[Index].Register.Rbx;
    CurrentTss->Ecx = (UINT32)mGuestContextCommonSmm.GuestContextPerCpu[Index].Register.Rcx;
    CurrentTss->Edx = (UINT32)mGuestContextCommonSmm.GuestContextPerCpu[Index].Register.Rdx;
    CurrentTss->Esi = (UINT32)mGuestContextCommonSmm.GuestContextPerCpu[Index].Register.Rsi;
    CurrentTss->Edi = (UINT32)mGuestContextCommonSmm.GuestContextPerCpu[Index].Register.Rdi;
    CurrentTss->Ebp = (UINT32)mGuestContextCommonSmm.GuestContextPerCpu[Index].Register.Rbp;
    CurrentTss->Esp = (UINT32)mGuestContextCommonSmm.GuestContextPerCpu[Index].Register.Rsp;

    CurrentTss->Eip = (UINT32)VmReadN (VMCS_N_GUEST_RIP_INDEX);
    if (Qualification.TaskSwitch.TaskType == TASK_SWITCH_SOURCE_CALL) {
      CurrentTss->Eip += VmRead32(VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX); // Next instruction
    }

    CurrentTss->Es = VmRead16 (VMCS_16_GUEST_ES_INDEX);
    CurrentTss->Cs = VmRead16 (VMCS_16_GUEST_CS_INDEX);
    CurrentTss->Ss = VmRead16 (VMCS_16_GUEST_SS_INDEX);
    CurrentTss->Ds = VmRead16 (VMCS_16_GUEST_DS_INDEX);
    CurrentTss->Fs = VmRead16 (VMCS_16_GUEST_FS_INDEX);
    CurrentTss->Gs = VmRead16 (VMCS_16_GUEST_GS_INDEX);

    CurrentTss->Cr3 = (UINT32)VmReadN (VMCS_N_GUEST_CR3_INDEX);
    CurrentTss->LdtSegSelector = VmRead16 (VMCS_16_GUEST_LDTR_INDEX);

  }

  CurrentTss->Eflags = (UINT32)VmReadN (VMCS_N_GUEST_RFLAGS_INDEX);
  if (Qualification.TaskSwitch.TaskType == TASK_SWITCH_SOURCE_IRET) {
    CurrentTss->Eflags &= ~RFLAGS_NT;
  }

  //
  // Prepare new data
  //
  if ((Qualification.TaskSwitch.TaskType == TASK_SWITCH_SOURCE_CALL) ||
      (Qualification.TaskSwitch.TaskType == TASK_SWITCH_SOURCE_TASK_GATE)) {
    Tss->Link = CurrentTr;
  }
  mGuestContextCommonSmm.GuestContextPerCpu[Index].Register.Rax = Tss->Eax;
  mGuestContextCommonSmm.GuestContextPerCpu[Index].Register.Rbx = Tss->Ebx;
  mGuestContextCommonSmm.GuestContextPerCpu[Index].Register.Rcx = Tss->Ecx;
  mGuestContextCommonSmm.GuestContextPerCpu[Index].Register.Rdx = Tss->Edx;
  mGuestContextCommonSmm.GuestContextPerCpu[Index].Register.Rsi = Tss->Esi;
  mGuestContextCommonSmm.GuestContextPerCpu[Index].Register.Rdi = Tss->Edi;
  mGuestContextCommonSmm.GuestContextPerCpu[Index].Register.Rbp = Tss->Ebp;
  mGuestContextCommonSmm.GuestContextPerCpu[Index].Register.Rsp = Tss->Esp;

  VmWriteN (VMCS_N_GUEST_RIP_INDEX, Tss->Eip);

  VmWrite16 (VMCS_16_GUEST_ES_INDEX, Tss->Es);
  VmWrite16 (VMCS_16_GUEST_CS_INDEX, Tss->Cs);
  VmWrite16 (VMCS_16_GUEST_SS_INDEX, Tss->Ss);
  VmWrite16 (VMCS_16_GUEST_DS_INDEX, Tss->Ds);
  VmWrite16 (VMCS_16_GUEST_FS_INDEX, Tss->Fs);
  VmWrite16 (VMCS_16_GUEST_GS_INDEX, Tss->Gs);

  VmWriteN (VMCS_N_GUEST_CR3_INDEX, Tss->Cr3);
  VmWrite16 (VMCS_16_GUEST_LDTR_INDEX, Tss->LdtSegSelector);

  GdtEntry = (GDT_ENTRY *)(Gdtr + (Tss->LdtSegSelector & ~0x7));
  VmWriteN (VMCS_N_GUEST_LDTR_BASE_INDEX,             BaseFromGdtEntry(GdtEntry));
  if (Tss->LdtSegSelector == 0) {
    VmWrite32 (VMCS_32_GUEST_LDTR_ACCESS_RIGHT_INDEX, 0x1082);
  } else {
    VmWrite32 (VMCS_32_GUEST_LDTR_ACCESS_RIGHT_INDEX, ArFromGdtEntry(GdtEntry));
  }
  VmWrite32 (VMCS_32_GUEST_LDTR_LIMIT_INDEX,          LimitFromGdtEntry(GdtEntry));

  if ((Qualification.TaskSwitch.TaskType == TASK_SWITCH_SOURCE_CALL) ||
      (Qualification.TaskSwitch.TaskType == TASK_SWITCH_SOURCE_TASK_GATE)) {
    Tss->Eflags |= RFLAGS_NT;
  }
  VmWriteN (VMCS_N_GUEST_RFLAGS_INDEX, Tss->Eflags | 0x2); // set reserved bit.

  GdtEntry = (GDT_ENTRY *)(Gdtr + (Qualification.TaskSwitch.Tss & ~0x7));
  if ((Tss->Eflags & RFLAGS_VM) != 0) {
    // VM86 mode
    VmWriteN (VMCS_N_GUEST_ES_BASE_INDEX, Tss->Es << 4);
    VmWriteN (VMCS_N_GUEST_CS_BASE_INDEX, Tss->Cs << 4);
    VmWriteN (VMCS_N_GUEST_SS_BASE_INDEX, Tss->Ss << 4);
    VmWriteN (VMCS_N_GUEST_DS_BASE_INDEX, Tss->Ds << 4);
    VmWriteN (VMCS_N_GUEST_FS_BASE_INDEX, Tss->Fs << 4);
    VmWriteN (VMCS_N_GUEST_GS_BASE_INDEX, Tss->Gs << 4);

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
    GdtEntry = (GDT_ENTRY *)(Gdtr + (Tss->Es & ~0x7));
    VmWriteN (VMCS_N_GUEST_ES_BASE_INDEX,           BaseFromGdtEntry(GdtEntry));
    VmWrite32 (VMCS_32_GUEST_ES_ACCESS_RIGHT_INDEX, ArFromGdtEntry(GdtEntry) | DESCRIPTOR_DATA_ACCESS);
    VmWrite32 (VMCS_32_GUEST_ES_LIMIT_INDEX,        LimitFromGdtEntry(GdtEntry));

    GdtEntry = (GDT_ENTRY *)(Gdtr + (Tss->Cs & ~0x7));
    VmWriteN (VMCS_N_GUEST_CS_BASE_INDEX,           BaseFromGdtEntry(GdtEntry));
    VmWrite32 (VMCS_32_GUEST_CS_ACCESS_RIGHT_INDEX, ArFromGdtEntry(GdtEntry) | DESCRIPTOR_CODE_ACCESS);
    VmWrite32 (VMCS_32_GUEST_CS_LIMIT_INDEX,        LimitFromGdtEntry(GdtEntry));

    GdtEntry = (GDT_ENTRY *)(Gdtr + (Tss->Ss & ~0x7));
    VmWriteN (VMCS_N_GUEST_SS_BASE_INDEX,           BaseFromGdtEntry(GdtEntry));
    VmWrite32 (VMCS_32_GUEST_SS_ACCESS_RIGHT_INDEX, ArFromGdtEntry(GdtEntry) | DESCRIPTOR_DATA_ACCESS);
    VmWrite32 (VMCS_32_GUEST_SS_LIMIT_INDEX,        LimitFromGdtEntry(GdtEntry));

    GdtEntry = (GDT_ENTRY *)(Gdtr + (Tss->Ds & ~0x7));
    VmWriteN (VMCS_N_GUEST_DS_BASE_INDEX,           BaseFromGdtEntry(GdtEntry));
    VmWrite32 (VMCS_32_GUEST_DS_ACCESS_RIGHT_INDEX, ArFromGdtEntry(GdtEntry) | DESCRIPTOR_DATA_ACCESS);
    VmWrite32 (VMCS_32_GUEST_DS_LIMIT_INDEX,        LimitFromGdtEntry(GdtEntry));

    GdtEntry = (GDT_ENTRY *)(Gdtr + (Tss->Fs & ~0x7));
    VmWriteN (VMCS_N_GUEST_FS_BASE_INDEX,           BaseFromGdtEntry(GdtEntry));
    AsmWriteMsr64 (IA32_FS_BASE_MSR_INDEX,          BaseFromGdtEntry(GdtEntry));
    if (Tss->Fs == 0) {
      VmWrite32 (VMCS_32_GUEST_FS_ACCESS_RIGHT_INDEX, 0x10000);
    } else {
      VmWrite32 (VMCS_32_GUEST_FS_ACCESS_RIGHT_INDEX, ArFromGdtEntry(GdtEntry) | DESCRIPTOR_DATA_ACCESS);
    }
    VmWrite32 (VMCS_32_GUEST_FS_LIMIT_INDEX,        LimitFromGdtEntry(GdtEntry));

    GdtEntry = (GDT_ENTRY *)(Gdtr + (Tss->Gs & ~0x7));
    VmWriteN (VMCS_N_GUEST_GS_BASE_INDEX,           BaseFromGdtEntry(GdtEntry));
    AsmWriteMsr64 (IA32_GS_BASE_MSR_INDEX,          BaseFromGdtEntry(GdtEntry));
    if (Tss->Gs == 0) {
      VmWrite32 (VMCS_32_GUEST_GS_ACCESS_RIGHT_INDEX, 0x10000);
    } else {
      VmWrite32 (VMCS_32_GUEST_GS_ACCESS_RIGHT_INDEX, ArFromGdtEntry(GdtEntry) | DESCRIPTOR_DATA_ACCESS);
    }
    VmWrite32 (VMCS_32_GUEST_GS_LIMIT_INDEX,        LimitFromGdtEntry(GdtEntry));

    VmWrite32 (VMCS_32_GUEST_TR_ACCESS_RIGHT_INDEX, DESCRIPTOR_PRESENT | DESCRIPTOR_TSS_N_BUSY);

  } else {
    DEBUG ((EFI_D_INFO, "!!!Other TSS\n"));
    DumpVmcsAllField ();
    CpuDeadLoop ();
  }

  VmWriteN (VMCS_N_GUEST_CR0_INDEX, VmReadN (VMCS_N_GUEST_CR0_INDEX) | CR0_TS);

  return ;
}
