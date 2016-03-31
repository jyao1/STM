/** @file
  VMX related function

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Base.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include "Library/FrmLib.h"

#define RFLAGS_CF   1u
#define RFLAGS_ZF   (1u << 6)

/**

  This function read UINT64 data from VMCS region.

  @param Index VMCS region index

  @return VMCS region value

**/
UINT64
VmRead64 (
  IN UINT32  Index
  )
{
  UINT64 Data64;
  UINT32 Data32High;
  UINT32 Data32Low;
  UINTN  Rflags;

  Rflags = AsmVmRead (Index, &Data32Low);
  if ((Rflags & (RFLAGS_CF | RFLAGS_ZF)) != 0) {
    DEBUG ((EFI_D_ERROR, "ERROR: AsmVmRead(0x%x) : %08x\n", (UINTN)Index, Rflags));
  }

  Rflags = AsmVmRead (Index + 1, &Data32High);
  if ((Rflags & (RFLAGS_CF | RFLAGS_ZF)) != 0) {
    DEBUG ((EFI_D_ERROR, "ERROR: AsmVmRead(0x%x) : %08x\n", (UINTN)Index + 1, Rflags));
  }

  Data64 = Data32Low | LShiftU64 (Data32High, 32);
  return Data64;
}

/**

  This function write UINN64 data to VMCS region.

  @param Index VMCS region index
  @param Data  VMCS region value

**/
VOID
VmWrite64 (
  IN UINT32  Index,
  IN UINT64  Data
  )
{
  UINT32 Data32High;
  UINT32 Data32Low;
  UINTN  Rflags;

  Data32Low = (UINT32)Data;
  Data32High = (UINT32)RShiftU64 (Data, 32);
  Rflags = AsmVmWrite (Index, Data32Low);
  if ((Rflags & (RFLAGS_CF | RFLAGS_ZF)) != 0) {
    DEBUG ((EFI_D_ERROR, "ERROR: AsmVmWrite(0x%x - 0x%08x) : %08x\n", (UINTN)Index, (UINT32)Data32Low, Rflags));
  }
  Rflags = AsmVmWrite (Index + 1, Data32High);
  if ((Rflags & (RFLAGS_CF | RFLAGS_ZF)) != 0) {
    DEBUG ((EFI_D_ERROR, "ERROR: AsmVmWrite(0x%x - 0x%08x) : %08x\n", (UINTN)Index + 1, (UINT32)Data32High, Rflags));
  }
}

