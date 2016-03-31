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
  CpuState->R15 = 0;
  CpuState->R14 = 0;
  CpuState->R13 = 0;
  CpuState->R12 = 0;
  CpuState->R11 = 0;
  CpuState->R10 = 0;
  CpuState->R9 = 0;
  CpuState->R8 = 0;

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
  return ;
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
  return ;
}

