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

/**

  This function is INIT handler.

  @param Index CPU index

**/
VOID 
InitHandler (
  IN UINT32 Index
  )
{
#if 0
  DEBUG ((EFI_D_INFO, "(FRM) !!!InitHandler!!! - %d\n", (UINTN)Index));
#endif
  VmWrite32 (VMCS_32_GUEST_ACTIVITY_STATE_INDEX, GUEST_ACTIVITY_STATE_WAIT_FOR_SIPI);
  return ;
}

/**

  This function set VMCS guest AP wakeup field.

  @param Index  CPU index
  @param Vector AP wakeup vector

**/
VOID
SetVmcsGuestApWakeupField (
  IN UINT32 Index,
  IN UINT8  Vector
  )
{
  VM_ENTRY_CONTROLS       VmEntryControls;

  //
  // AP state:
  // eax: 0
  // ebx: 0
  // ecx: 0
  // edx: CPUID_EAX (1)
  // esi: 0
  // edi: 0
  // ebp: 0
  // esp: 0
  // esp: 0
  // cr0: 00000010
  // cr2: 00000000
  // cr3: 00000000
  // cr4: 00000000
  // dr0: 00000000
  // dr1: 00000000
  // dr2: 00000000
  // dr3: 00000000
  // dr6: ffff0ff0
  // dr7: 00000400
  //
  // eip: 0
  // eflags: 00010002
  //
  // idtbase: 00000000
  // gdtbase: 00000000
  // ldtbase: 00000000
  // tssbase: 00000000
  // idtlim: 0000ffff
  // gdtlim: 0000ffff
  // ldtlim: 0000ffff
  // tsslim: 0000ffff
  // ldtar: 0082
  // tssar: 008b
  // ldtr: 0000
  // tr: 0000
  //
  // csbase: vector << 12
  // dsbase: 0
  // ssbase: 0
  // esbase: 0
  // fsbase: 0
  // gsbase: 0
  // cslim: 0000ffff
  // dslim: 0000ffff
  // sslim: 0000ffff
  // eslim: 0000ffff
  // fslim: 0000ffff
  // gslim: 0000ffff
  // csar: 0093
  // dsar: 0093
  // ssar: 0093
  // esar: 0093
  // fsar: 0093
  // gsar: 0093
  // cs: vector << 8
  // ds: 0
  // ss: 0
  // es: 0
  // fs: 0
  // gs: 0
  //

  VmWrite32 (VMCS_32_GUEST_ACTIVITY_STATE_INDEX, GUEST_ACTIVITY_STATE_ACTIVE);
  VmWrite16 (VMCS_16_GUEST_CS_INDEX, (UINT16)Vector << 8);
  VmWriteN (VMCS_N_GUEST_CS_BASE_INDEX, (UINTN)Vector << 12);
  VmWriteN (VMCS_N_GUEST_RIP_INDEX, 0);
  VmWriteN (VMCS_N_GUEST_SS_BASE_INDEX, 0);
  VmWriteN (VMCS_N_GUEST_RSP_INDEX, 0);

  VmWriteN (VMCS_N_GUEST_RFLAGS_INDEX, 0x00010002);

  VmWrite16 (VMCS_16_GUEST_ES_INDEX, 0);
  VmWrite16 (VMCS_16_GUEST_SS_INDEX, 0);
  VmWrite16 (VMCS_16_GUEST_DS_INDEX, 0);
  VmWrite16 (VMCS_16_GUEST_FS_INDEX, 0);
  VmWrite16 (VMCS_16_GUEST_GS_INDEX, 0);
  VmWrite16 (VMCS_16_GUEST_TR_INDEX, 0);

  VmWriteN (VMCS_N_GUEST_ES_BASE_INDEX,                  0);
  VmWriteN (VMCS_N_GUEST_DS_BASE_INDEX,                  0);
  VmWriteN (VMCS_N_GUEST_FS_BASE_INDEX,                  0);
  VmWriteN (VMCS_N_GUEST_GS_BASE_INDEX,                  0);

  VmWrite32 (VMCS_32_GUEST_ES_ACCESS_RIGHT_INDEX,        0x0093);
  VmWrite32 (VMCS_32_GUEST_CS_ACCESS_RIGHT_INDEX,        0x0093);
  VmWrite32 (VMCS_32_GUEST_SS_ACCESS_RIGHT_INDEX,        0x0093);
  VmWrite32 (VMCS_32_GUEST_DS_ACCESS_RIGHT_INDEX,        0x0093);
  VmWrite32 (VMCS_32_GUEST_FS_ACCESS_RIGHT_INDEX,        0x0093);
  VmWrite32 (VMCS_32_GUEST_GS_ACCESS_RIGHT_INDEX,        0x0093);

  VmWrite32 (VMCS_32_GUEST_ES_LIMIT_INDEX,               0x0000ffff);
  VmWrite32 (VMCS_32_GUEST_CS_LIMIT_INDEX,               0x0000ffff);
  VmWrite32 (VMCS_32_GUEST_SS_LIMIT_INDEX,               0x0000ffff);
  VmWrite32 (VMCS_32_GUEST_DS_LIMIT_INDEX,               0x0000ffff);
  VmWrite32 (VMCS_32_GUEST_FS_LIMIT_INDEX,               0x0000ffff);
  VmWrite32 (VMCS_32_GUEST_GS_LIMIT_INDEX,               0x0000ffff);

  VmWriteN (VMCS_N_GUEST_DR7_INDEX,                      0x00000400);
  VmWriteN (VMCS_N_GUEST_LDTR_BASE_INDEX,                0);
  VmWriteN (VMCS_N_GUEST_TR_BASE_INDEX,                  0);
  VmWriteN (VMCS_N_GUEST_GDTR_BASE_INDEX,                0);
  VmWriteN (VMCS_N_GUEST_IDTR_BASE_INDEX,                0);
  VmWrite32 (VMCS_32_GUEST_LDTR_LIMIT_INDEX,             0x0000ffff);
  VmWrite32 (VMCS_32_GUEST_TR_LIMIT_INDEX,               0x0000ffff);
  VmWrite32 (VMCS_32_GUEST_GDTR_LIMIT_INDEX,             0x0000ffff);
  VmWrite32 (VMCS_32_GUEST_IDTR_LIMIT_INDEX,             0x0000ffff);
  VmWrite32 (VMCS_32_GUEST_LDTR_ACCESS_RIGHT_INDEX,      0x0082);
  VmWrite32 (VMCS_32_GUEST_TR_ACCESS_RIGHT_INDEX,        0x008b);
  VmWrite32 (VMCS_32_GUEST_INTERRUPTIBILITY_STATE_INDEX, 0);

  VmEntryControls.Uint32 = VmRead32 (VMCS_32_CONTROL_VMENTRY_CONTROLS_INDEX);
  VmEntryControls.Bits.Ia32eGuest = 0;
  VmWrite32 (VMCS_32_CONTROL_VMENTRY_CONTROLS_INDEX, VmEntryControls.Uint32);

  mGuestContextCommon.GuestContextPerCpu[Index].Cr0 = (UINTN)(AsmReadMsr64 (IA32_VMX_CR0_FIXED0_MSR_INDEX) & AsmReadMsr64 (IA32_VMX_CR0_FIXED1_MSR_INDEX) & ~CR0_PE & ~CR0_PG);
  mGuestContextCommon.GuestContextPerCpu[Index].Cr4 = (UINTN)(AsmReadMsr64 (IA32_VMX_CR4_FIXED0_MSR_INDEX) & AsmReadMsr64 (IA32_VMX_CR4_FIXED1_MSR_INDEX));
  mGuestContextCommon.GuestContextPerCpu[Index].EFER = 0;

  VmWrite64 (VMCS_64_GUEST_IA32_EFER_INDEX, 0);

  VmWriteN (VMCS_N_GUEST_CR0_INDEX, mGuestContextCommon.GuestContextPerCpu[Index].Cr0);
  VmWriteN (VMCS_N_GUEST_CR4_INDEX, mGuestContextCommon.GuestContextPerCpu[Index].Cr4);
  VmWriteN (VMCS_N_GUEST_CR3_INDEX, 0);

  VmWriteN (VMCS_N_CONTROL_CR0_READ_SHADOW_INDEX, mGuestContextCommon.GuestContextPerCpu[Index].Cr0);
  VmWriteN (VMCS_N_CONTROL_CR4_READ_SHADOW_INDEX, mGuestContextCommon.GuestContextPerCpu[Index].Cr4 & ~CR4_VMXE & ~CR4_SMXE);

  VmWrite64 (VMCS_64_GUEST_IA32_EFER_INDEX,       mGuestContextCommon.GuestContextPerCpu[Index].EFER);

  ZeroMem (&mGuestContextCommon.GuestContextPerCpu[Index].Register, sizeof(X86_REGISTER));
  AsmCpuidEx (
    CPUID_FEATURE_INFORMATION,
    0,
    (UINT32 *)&mGuestContextCommon.GuestContextPerCpu[Index].Register.Rax,
    (UINT32 *)&mGuestContextCommon.GuestContextPerCpu[Index].Register.Rbx,
    (UINT32 *)&mGuestContextCommon.GuestContextPerCpu[Index].Register.Rcx,
    (UINT32 *)&mGuestContextCommon.GuestContextPerCpu[Index].Register.Rdx
    );
  mGuestContextCommon.GuestContextPerCpu[Index].Register.Rdx = mGuestContextCommon.GuestContextPerCpu[Index].Register.Rax;
  mGuestContextCommon.GuestContextPerCpu[Index].Register.Rax = 0;
  mGuestContextCommon.GuestContextPerCpu[Index].Register.Rbx = 0;
  mGuestContextCommon.GuestContextPerCpu[Index].Register.Rcx = 0;

  if (!mGuestContextCommon.GuestContextPerCpu[Index].UnrestrictedGuest) {
    mGuestContextCommon.GuestContextPerCpu[Index].Cr0 |= CR0_PE | CR0_PG;
    VmWriteN (VMCS_N_GUEST_CR0_INDEX, mGuestContextCommon.GuestContextPerCpu[Index].Cr0);
    VmWriteN (VMCS_N_CONTROL_CR0_READ_SHADOW_INDEX, mGuestContextCommon.GuestContextPerCpu[Index].Cr0);
    // BUGBUG: Start emulator...
    CpuDeadLoop ();
  }
}

/**

  This function is SIPI handler.

  @param Index CPU index

**/
VOID
SipiHandler (
  IN UINT32 Index
  )
{
  UINT8                   Vector;

  //
  // Good! start run real mode code
  //
  Vector = (UINT8)VmReadN (VMCS_N_RO_EXIT_QUALIFICATION_INDEX);

#if 0
  DEBUG ((EFI_D_INFO, "(FRM) !!!SipiHandler!!! - %d (Vector - %02x)\n", (UINTN)Index, (UINTN)Vector));
#endif

  SetVmcsGuestApWakeupField (Index, Vector);

  return ;
}

