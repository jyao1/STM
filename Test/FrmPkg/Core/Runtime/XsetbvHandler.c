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
#include <Library\FrmLib.h>

/**

  This function is XSETBV handler.

  @param Index CPU index

**/
VOID
XsetbvHandler (
  IN UINT32 Index
  )
{
  UINT64 Data64;
  UINT32 XsetbvIndex;

  XsetbvIndex = (UINT32)mGuestContextCommon.GuestContextPerCpu[Index].Register.Rcx;
  Data64 = (UINT64)(mGuestContextCommon.GuestContextPerCpu[Index].Register.Rdx & 0xFFFFFFFF);
  Data64 = LShiftU64 (Data64, 32);
  Data64 |= (UINT64)(mGuestContextCommon.GuestContextPerCpu[Index].Register.Rax & 0xFFFFFFFF);

  if (IsXStateSupoprted ()) {
    AsmWriteCr4 (AsmReadCr4() | CR4_OSXSAVE);
  }
  AsmXSetBv (XsetbvIndex, Data64);
  VmWriteN (VMCS_N_GUEST_RIP_INDEX, VmReadN(VMCS_N_GUEST_RIP_INDEX) + VmRead32(VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX));
  return ;
}
