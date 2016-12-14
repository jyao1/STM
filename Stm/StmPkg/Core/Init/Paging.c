/** @file
  STM paging

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmInit.h"

/**
  Check if 1-GByte pages is supported by processor or not.

  @retval TRUE   1-GByte pages is supported.
  @retval FALSE  1-GByte pages is not supported.

**/
BOOLEAN
Is1GPageSupport (
  VOID
  )
{
  UINT32         RegEax;
  UINT32         RegEdx;

  AsmCpuid (0x80000000, &RegEax, NULL, NULL, NULL);
  if (RegEax >= 0x80000001) {
    AsmCpuid (0x80000001, NULL, NULL, NULL, &RegEdx);
    if ((RegEdx & BIT26) != 0) {
      return TRUE;
    }
  }
  return FALSE;
}

/**

  This function create Ia32e page table for SMM guest.

  @return pages table address

**/
UINTN
CreateIa32ePageTable (
  VOID
  )
{
  UINTN                             PageTable;
  UINTN                             Index;
  UINTN                             SubIndex;
  UINT64                            *Pde;
  UINT64                            *Pte;
  UINT64                            *Pml4;

  PageTable = (UINTN)AllocatePages (6);

  Pml4 = (UINT64*)(UINTN)PageTable;
  PageTable += SIZE_4KB;
  *Pml4 = PageTable | IA32_PG_P;

  Pde = (UINT64*)(UINTN)PageTable;
  Pte = Pde + SIZE_4KB / sizeof (*Pde);

  for (Index = 0; Index < 4; Index++) {
    *Pde = (UINTN)Pte | IA32_PG_P;
    Pde++;

    for (SubIndex = 0; SubIndex < SIZE_4KB / sizeof (*Pte); SubIndex++) {
      *Pte = (((Index << 9) + SubIndex) << 21) |
        IA32_PG_PS | IA32_PG_RW | IA32_PG_P;
      Pte++;
    }
  }

  PageTable = (UINTN)Pml4;

  return PageTable;
}

/**

  This function create compatible page table for SMM guest.

  @return pages table address

**/
UINTN
CreateCompatiblePageTable (
  VOID
  )
{
  UINTN                             PageTable;
  UINTN                             Index;
  UINT32                            *Pte;
  UINT32                            Address;

  PageTable = (UINTN)AllocatePages (1);

  Pte = (UINT32*)(UINTN)PageTable;

  Address = 0;
  for (Index = 0; Index < SIZE_4KB / sizeof (*Pte); Index++) {
    *Pte = Address | IA32_PG_PS | IA32_PG_RW | IA32_PG_P;
    Pte++;
    Address += SIZE_4MB;
  }

  return PageTable;
}

/**

  This function create compatible PAE page table for SMM guest.

  @return pages table address

**/
UINTN
CreateCompatiblePaePageTable (
  VOID
  )
{
  UINTN                             PageTable;
  UINTN                             Index;
  UINTN                             SubIndex;
  UINT64                            *Pde;
  UINT64                            *Pte;

  PageTable = (UINTN)AllocatePages (5);

  Pde = (UINT64*)(UINTN)PageTable;
  Pte = Pde + SIZE_4KB / sizeof (*Pde);

  for (Index = 0; Index < 4; Index++) {
    *Pde = (UINTN)Pte | IA32_PG_P;
    Pde++;

    for (SubIndex = 0; SubIndex < SIZE_4KB / sizeof (*Pte); SubIndex++) {
      *Pte = (((Index << 9) + SubIndex) << 21) |
        IA32_PG_PS | IA32_PG_RW | IA32_PG_P;
      Pte++;
    }
  }

  return PageTable;
}

/**

  This function create page table for STM host.
  The SINIT/StmLoader should already configured 4G paging, so here
  we just create >4G paging for X64 mode.

**/
VOID
CreateHostPaging (
  VOID
  )
{
  UINTN                             PageTable;
  UINTN                             Index;
  UINTN                             SubIndex;
  UINTN                             Pml4Index;
  UINT64                            *Pde;
  UINT64                            *Pte;
  UINT64                            *Pml4;
  UINT64                            BaseAddress;
  UINTN                             NumberOfPml4EntriesNeeded;
  UINTN                             NumberOfPdpEntriesNeeded;

  if (sizeof(UINTN) == sizeof(UINT64)) {
    PageTable = AsmReadCr3 ();
    Pml4 = (UINT64 *)PageTable;

    if (mHostContextCommon.PhysicalAddressBits <= 39) {
      NumberOfPml4EntriesNeeded = 1;
      NumberOfPdpEntriesNeeded = (UINTN)LShiftU64 (1, mHostContextCommon.PhysicalAddressBits - 30);
    } else {
      NumberOfPml4EntriesNeeded = (UINTN)LShiftU64 (1, mHostContextCommon.PhysicalAddressBits - 39);
      NumberOfPdpEntriesNeeded = 512;
    }

    BaseAddress = BASE_4GB;
    for (Pml4Index = 0; Pml4Index < NumberOfPml4EntriesNeeded; Pml4Index++) {
      if (Pml4Index > 0) {
        Pde = (UINT64 *)(UINTN)AllocatePages (1);
        Pml4[Pml4Index] = (UINT64)(UINTN)Pde | IA32_PG_P;
        Index = 0;
      } else {
        // Start from 4G - Pml4[0] already allocated.
        Pde = (UINT64 *)(UINTN)(Pml4[0] & 0xFFFFF000);
        Index = 4;
      }

      if (Is1GPageSupport()) {
        for (; Index < NumberOfPdpEntriesNeeded; Index++) {
          Pde[Index] = (UINT64)(UINTN)BaseAddress | IA32_PG_PS | IA32_PG_RW | IA32_PG_P;
          BaseAddress += SIZE_1GB;
        }
      } else {
        for (; Index < NumberOfPdpEntriesNeeded; Index++) {
          Pte = (UINT64 *)AllocatePages (1);
          Pde[Index] = (UINT64)(UINTN)Pte | IA32_PG_P;

          for (SubIndex = 0; SubIndex < SIZE_4KB / sizeof(*Pte); SubIndex++) {
            Pte[SubIndex] = BaseAddress | IA32_PG_PS | IA32_PG_RW | IA32_PG_P;
            BaseAddress += SIZE_2MB;
          }
        }
      }
    }
  }
}
