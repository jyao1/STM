/** @file
  SMM BIOS state sync

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
  )
{
  X86_REGISTER                       *Reg;

  Reg = &mGuestContextCommonSmi.GuestContextPerCpu[Index].Register;

  if (!Scrub) {
    CpuState->R15 = Reg->R15;
    CpuState->R14 = Reg->R14;
    CpuState->R13 = Reg->R13;
    CpuState->R12 = Reg->R12;
    CpuState->R11 = Reg->R11;
    CpuState->R10 = Reg->R10;
    CpuState->R9  = Reg->R9;
    CpuState->R8  = Reg->R8;
  } else {
    CpuState->R15 = 0;
    CpuState->R14 = 0;
    CpuState->R13 = 0;
    CpuState->R12 = 0;
    CpuState->R11 = 0;
    CpuState->R10 = 0;
    CpuState->R9  = 0;
    CpuState->R8  = 0;
  }

  return ;
}

/**

  This function read SMM state save IA32e GPR sync to VMCS.

  @param Index CPU index
  @param CpuState SMM CPU state

**/
VOID
ReadSyncSmmStateSaveAreaIa32eGpr (
  IN UINT32                             Index,
  IN STM_SMM_CPU_STATE                  *CpuState
  )
{
  X86_REGISTER                       *Reg;

  Reg = &mGuestContextCommonSmi.GuestContextPerCpu[Index].Register;

  Reg->R8  = (UINTN)CpuState->R8;
  Reg->R9  = (UINTN)CpuState->R9;
  Reg->R10 = (UINTN)CpuState->R10;
  Reg->R11 = (UINTN)CpuState->R11;
  Reg->R12 = (UINTN)CpuState->R12;
  Reg->R13 = (UINTN)CpuState->R13;
  Reg->R14 = (UINTN)CpuState->R14;
  Reg->R15 = (UINTN)CpuState->R15;

  return ;
}

/**

  This function write SMM state save SSE2 accroding to VMCS.

  @param Index    CPU index
  @param Scrub    Nee Scrub

**/
VOID
WriteSyncSmmStateSaveAreaSse2 (
  IN UINT32                             Index,
  IN BOOLEAN                            Scrub
  )
{
  if (!Scrub) {
    CopyMem (&mGuestContextCommonSmm.GuestContextPerCpu[Index].Register.FxBuffer, &mGuestContextCommonSmi.GuestContextPerCpu[Index].Register.FxBuffer, sizeof(IA32_FX_BUFFER));
  } else {
    ZeroMem (&mGuestContextCommonSmm.GuestContextPerCpu[Index].Register.FxBuffer, sizeof(IA32_FX_BUFFER));
  }
}

/**

  This function read SMM state save SSE2 sync to VMCS.

  @param Index CPU index

**/
VOID
ReadSyncSmmStateSaveAreaSse2 (
  IN UINT32                             Index
  )
{
  CopyMem (&mGuestContextCommonSmi.GuestContextPerCpu[Index].Register.FxBuffer, &mGuestContextCommonSmm.GuestContextPerCpu[Index].Register.FxBuffer, sizeof(IA32_FX_BUFFER));
}
