/** @file
  SMM CPUID handler

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

  This function is CPUID handler for SMM.

  @param Index CPU index

**/
VOID
SmmCpuidHandler (
  IN UINT32 Index
  )
{
  X86_REGISTER      *Reg;

  Reg = &mGuestContextCommonSmm.GuestContextPerCpu[Index].Register;

  AsmCpuidEx (
    ReadUnaligned32 ((UINT32 *)&Reg->Rax),
    ReadUnaligned32 ((UINT32 *)&Reg->Rcx),
    (UINT32 *)&Reg->Rax,
    (UINT32 *)&Reg->Rbx,
    (UINT32 *)&Reg->Rcx,
    (UINT32 *)&Reg->Rdx
    );
  VmWriteN (VMCS_N_GUEST_RIP_INDEX, VmReadN(VMCS_N_GUEST_RIP_INDEX) + VmRead32(VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX));
  return ;
}