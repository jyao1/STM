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

  This function is CPUID handler.

  @param Index CPU index

**/
VOID
CpuidHandler (
  IN UINT32 Index
  )
{
  AsmCpuidEx (
    (UINT32)mGuestContextCommon.GuestContextPerCpu[Index].Register.Rax & 0xFFFFFFFF,
    (UINT32)mGuestContextCommon.GuestContextPerCpu[Index].Register.Rcx & 0xFFFFFFFF,
    (UINT32 *)&mGuestContextCommon.GuestContextPerCpu[Index].Register.Rax,
    (UINT32 *)&mGuestContextCommon.GuestContextPerCpu[Index].Register.Rbx,
    (UINT32 *)&mGuestContextCommon.GuestContextPerCpu[Index].Register.Rcx,
    (UINT32 *)&mGuestContextCommon.GuestContextPerCpu[Index].Register.Rdx
    );
#if 0
  DEBUG ((EFI_D_INFO,
    "(FRM) !!!CpuidHandler!!! (%08x) - (%08x, %08x, %08x, %08x)",
    (UINTN)CpuidIndex,
    (UINTN)mGuestContextCommon.GuestContextPerCpu[Index].Register.Rax,
    (UINTN)mGuestContextCommon.GuestContextPerCpu[Index].Register.Rbx,
    (UINTN)mGuestContextCommon.GuestContextPerCpu[Index].Register.Rcx,
    (UINTN)mGuestContextCommon.GuestContextPerCpu[Index].Register.Rdx
    ));
#endif
  VmWriteN (VMCS_N_GUEST_RIP_INDEX, VmReadN(VMCS_N_GUEST_RIP_INDEX) + VmRead32(VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX));
  return ;
}