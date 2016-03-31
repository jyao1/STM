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
#include "FrmInit.h"

/**

  This function create 4G page table for Host.

  @return Page table pointer

**/
UINTN
CreatePageTable (
  VOID
  )
{
  UINTN                             PageTable;
  UINTN                             IndexI;
  UINTN                             IndexJ;
  UINT64                            *Pde;
  UINT64                            *Pte;
  UINT64                            *Pml4;

  //
  // One more page is needed in 64-bit mode
  //
  IndexI = sizeof (UINTN) == sizeof (UINT64) ? 6 : 5;

  //
  // Allocate the page table
  //
  PageTable = (UINTN)AllocatePages (IndexI);

  //
  // This step is needed only in 64-bit mode
  //
  if (sizeof(UINTN) == sizeof(UINT64)) {
    Pml4 = (UINT64*)(UINTN)PageTable;
    PageTable += SIZE_4KB;
    *Pml4 = PageTable | IA32_PG_P;
  }

  Pde = (UINT64*)(UINTN)PageTable;
  Pte = Pde + SIZE_4KB / sizeof (*Pde);

  for (IndexI = 0; IndexI < 4; IndexI++) {
    *Pde = (UINTN)Pte | IA32_PG_P;
    Pde++;

    for (IndexJ = 0; IndexJ < SIZE_4KB / sizeof (*Pte); IndexJ++) {
      *Pte = (((IndexI << 9) + IndexJ) << 21) | IA32_PG_PS | IA32_PG_RW | IA32_PG_P;
      Pte++;
    }
  }

  //
  // This step is needed only in 64-bit mode
  //
  if (sizeof(UINTN) == sizeof(UINT64)) {
    PageTable = (UINTN)Pml4;
  }
  return PageTable;
}

/**

  This function create 4G page table for Guest.

  @return Page table pointer

**/
UINTN
CreateCompatiblePageTable (
  VOID
  )
{
  UINTN                             PageTable;
  UINTN                             IndexI;
  UINTN                             IndexJ;
  UINT32                            *Pde;
  UINT32                            *Pte;
  UINT32                            Address;

  IndexI = 1 + 1024;

  //
  // Allocate the page table
  //
  PageTable = (UINTN)AllocatePages (IndexI);

  Pde = (UINT32*)(UINTN)PageTable;
  Pte = (UINT32*)((UINTN)PageTable + SIZE_4KB);

  Address = 0;
  for (IndexI = 0; IndexI < SIZE_4KB / sizeof (*Pde); IndexI++) {
    *Pde = (UINT32)((UINTN)Pte | IA32_PG_P);
    Pde++;

    for (IndexJ = 0; IndexJ < SIZE_4KB / sizeof (*Pte); IndexJ++) {
      *Pte = (UINT32)(Address | IA32_PG_USR | IA32_PG_RW | IA32_PG_P);
      Pte++;
      Address += SIZE_4KB;
    }
  }

  return PageTable;
}

/**

  This function create 4G PAE page table for Guest.

  @return Page table pointer

**/
UINTN
CreateCompatiblePageTablePae (
  VOID
  )
{
  UINTN                             PageTable;
  UINTN                             IndexI;
  UINTN                             IndexJ;
  UINTN                             IndexK;
  UINT64                            *Pdpte;
  UINT64                            *Pde;
  UINT64                            *Pte;
  UINT64                            Address;

  IndexI = 1 + 4 + 1024 * 4;

  //
  // Allocate the page table
  //
  PageTable = (UINTN)AllocatePages (IndexI);

  Pdpte = (UINT64*)(UINTN)PageTable;
  Pde = (UINT64*)((UINTN)PageTable + SIZE_4KB);
  Pte = (UINT64*)((UINTN)PageTable + SIZE_4KB * 5);

  Address = 0;
  for (IndexK = 0; IndexK < 4; IndexK++) {
    *Pdpte = (UINT64)((UINTN)Pde | IA32_PG_P);
    Pdpte++;

    for (IndexI = 0; IndexI < SIZE_4KB / sizeof (*Pde); IndexI++) {
      *Pde = (UINT64)((UINTN)Pte | IA32_PG_USR | IA32_PG_RW | IA32_PG_P);
      Pde++;

      for (IndexJ = 0; IndexJ < SIZE_4KB / sizeof (*Pte); IndexJ++) {
        *Pte = (UINT64)(Address | IA32_PG_USR | IA32_PG_RW | IA32_PG_P);
        Pte++;
        Address += SIZE_4KB;
      }
    }
  }

  return PageTable;
}
