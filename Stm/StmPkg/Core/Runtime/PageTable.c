/** @file
  Page table management

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmRuntime.h"

#define PAGING_4K_MASK  0xFFF
#define PAGING_4M_MASK  0x3FFFFF
#define PAGING_2M_MASK  0x1FFFFF

#define PAGING_INDEX_MASK      0x3FF
#define PAGING_PAE_INDEX_MASK  0x1FF

#define PAGING_4K_ADDRESS_MASK_64_X64 0x000FFFFFFFFFF000ull
#define PAGING_2M_ADDRESS_MASK_64_X64 0x000FFFFFFFE00000ull

#define PAGING_4K_ADDRESS_MASK_64_IA32 0x00000000FFFFF000u
#define PAGING_2M_ADDRESS_MASK_64_IA32 0x00000000FFE00000u

#define PAGING_4K_ADDRESS_MASK_32 0xFFFFF000u
#define PAGING_4M_ADDRESS_MASK_32 0xFFC00000u

/**

  This function translate guest linear address to guest physical address.

  @param Cr3                Guest Cr3
  @param Cr0                Guest Cr0
  @param Cr4                Guest Cr4
  @param Efer               Guest Efer
  @param GuestLinearAddress Guest linear address
  @param Ia32e              If translated address is in Ia32e mode
  @param Pg                 If translated address is in page table
  @param Pae                If translated address is in pae mode
  @param Pse                If translated address is in pse mode
  @param Sp                 If translated address is in super page
  @param Entry              The page table entry that matched virtual address
                            When Pae enabled, it is UINT64 pointer.
                            When Pae disabled, it is UINT32 pointer.

  @return Guest physical address
**/
UINT64
TranslateGuestLinearToPhysical (
  IN UINTN    Cr3,
  IN UINTN    Cr0,
  IN UINTN    Cr4,
  IN UINT64   Efer,
  IN UINTN    GuestLinearAddress,
  OUT BOOLEAN *Ia32e,
  OUT BOOLEAN *Pg,
  OUT BOOLEAN *Pae,
  OUT BOOLEAN *Pse,
  OUT BOOLEAN *Sp,
  OUT UINT64  **Entry
  )
{
  UINT64                *L1PageTable;
  UINT64                *L2PageTable;
  UINT64                *L3PageTable;
  UINT64                *L4PageTable;
  UINT32                *L1PageTable32;
  UINT32                *L2PageTable32;
  UINTN                 Index1;
  UINTN                 Index2;
  UINTN                 Index3;
  UINTN                 Index4;
  UINTN                 Offset;
  UINT64                PhysicalAddress;
  UINT64                Paging4kAddressMask64;
  UINT64                Paging2mAddressMask64;

  if (sizeof(UINTN) == sizeof(UINT64)) {
    Paging4kAddressMask64 = PAGING_4K_ADDRESS_MASK_64_X64;
    Paging2mAddressMask64 = PAGING_2M_ADDRESS_MASK_64_X64;
  } else {
    Paging4kAddressMask64 = PAGING_4K_ADDRESS_MASK_64_IA32;
    Paging2mAddressMask64 = PAGING_2M_ADDRESS_MASK_64_IA32;
  }

  if ((Efer & IA32_EFER_MSR_MLA) != 0) {
    if (Ia32e != NULL) {
      *Ia32e = TRUE;
    }
    if (Pg != NULL) {
      *Pg = TRUE;
    }
    if (Pae != NULL) {
      *Pae = TRUE;
    }
    if (sizeof(UINTN) == sizeof(UINT64)) {
      // x64 paging
      Index4 = ((UINTN)RShiftU64 (GuestLinearAddress, 39)) & PAGING_PAE_INDEX_MASK;
      Index3 = ((UINTN)GuestLinearAddress >> 30) & PAGING_PAE_INDEX_MASK;
      Index2 = ((UINTN)GuestLinearAddress >> 21) & PAGING_PAE_INDEX_MASK;
      Index1 = ((UINTN)GuestLinearAddress >> 12) & PAGING_PAE_INDEX_MASK;
      Offset = ((UINTN)GuestLinearAddress & PAGING_4K_MASK);

      L4PageTable = (UINT64 *)Cr3;
      if ((L4PageTable[Index4] & IA32_PG_P) == 0) {
        return 0;
      }

      L3PageTable = (UINT64 *)(UINTN)(L4PageTable[Index4] & Paging4kAddressMask64);
      if ((L3PageTable[Index3] & IA32_PG_P) == 0) {
        return 0;
      }

      L2PageTable = (UINT64 *)(UINTN)(L3PageTable[Index3] & Paging4kAddressMask64);
      if ((L2PageTable[Index2] & IA32_PG_P) == 0) {
        return 0;
      }

      // PS
      if ((L2PageTable[Index2] & IA32_PG_PS) != 0) {
        if (Sp != NULL) {
          *Sp = TRUE;
        }
        // 2M 
        Offset = (UINTN)GuestLinearAddress & PAGING_2M_MASK;
        PhysicalAddress = (UINTN)(L2PageTable[Index2] & Paging2mAddressMask64);
        if (Entry != NULL) {
          *Entry = &L2PageTable[Index2];
        }
        return PhysicalAddress + Offset;
      } else {
        if (Sp != NULL) {
          *Sp = FALSE;
        }
        // 4k
        L1PageTable = (UINT64 *)(UINTN)(L2PageTable[Index2] & Paging4kAddressMask64);
        if ((L1PageTable[Index1] & IA32_PG_P) == 0) {
          return 0;
        }
        PhysicalAddress = (UINTN)(L1PageTable[Index1] & Paging4kAddressMask64);
        if (Entry != NULL) {
          *Entry = &L1PageTable[Index1];
        }
        return PhysicalAddress + Offset;
      }
    } else {
      Index4 = 0;
      L4PageTable = NULL;
      return 0;
    }
  } else {
    if (Ia32e != NULL) {
      *Ia32e = FALSE;
    }
    // ia32 paging
    if ((Cr0 & CR0_PG) != 0) {
      if (Pg != NULL) {
        *Pg = TRUE;
      }
      if ((Cr4 & CR4_PAE) != 0) {
        if (Pae != NULL) {
          *Pae = TRUE;
        }
        // PAE
        Index3 = ((UINT32)GuestLinearAddress >> 30) & 0x3;
        Index2 = ((UINT32)GuestLinearAddress >> 21) & PAGING_PAE_INDEX_MASK;
        Index1 = ((UINT32)GuestLinearAddress >> 12) & PAGING_PAE_INDEX_MASK;
        Offset = ((UINT32)GuestLinearAddress & PAGING_4K_MASK);

        L3PageTable = (UINT64 *)Cr3;
        if ((L3PageTable[Index3] & IA32_PG_P) == 0) {
          return 0;
        }

        L2PageTable = (UINT64 *)(UINTN)(L3PageTable[Index3] & Paging4kAddressMask64);
        if ((L2PageTable[Index2] & IA32_PG_P) == 0) {
          return 0;
        }

        // PS
        if ((L2PageTable[Index2] & IA32_PG_PS) != 0) {
          if (Sp != NULL) {
            *Sp = TRUE;
          }
          // 2M 
          Offset = (UINTN)GuestLinearAddress & PAGING_2M_MASK;
          PhysicalAddress = (UINTN)(L2PageTable[Index2] & Paging2mAddressMask64);
          if (Entry != NULL) {
            *Entry = &L2PageTable[Index2];
          }
          return PhysicalAddress + Offset;
        } else {
          if (Sp != NULL) {
            *Sp = FALSE;
          }
          // 4k
          L1PageTable = (UINT64 *)(UINTN)(L2PageTable[Index2] & Paging4kAddressMask64);
          if ((L1PageTable[Index1] & IA32_PG_P) == 0) {
            return 0;
          }
          PhysicalAddress = (UINTN)(L1PageTable[Index1] & Paging4kAddressMask64);
          if (Entry != NULL) {
            *Entry = &L1PageTable[Index1];
          }
          return PhysicalAddress + Offset;
        }
      } else {
        if (Pae != NULL) {
          *Pae = FALSE;
        }
        // Non-PAE

        if ((Cr4 & CR4_PSE) != 0) {
          // PSE
          if (Pse != NULL) {
            *Pse = TRUE;
          }
        } else {
          // Non-PSE
          if (Pse != NULL) {
            *Pse = FALSE;
          }
        }

        Index2 = ((UINT32)GuestLinearAddress >> 22) & PAGING_INDEX_MASK;
        Index1 = ((UINT32)GuestLinearAddress >> 12) & PAGING_INDEX_MASK;
        Offset = ((UINT32)GuestLinearAddress & PAGING_4K_MASK);

        L2PageTable32 = (UINT32 *)Cr3;
        if ((L2PageTable32[Index2] & IA32_PG_P) == 0) {
          return 0;
        }

        // PS
        if ((L2PageTable32[Index2] & IA32_PG_PS) != 0) {
          if (Sp != NULL) {
            *Sp = TRUE;
          }
          // 4M
          Offset = (UINT32)GuestLinearAddress & PAGING_4M_MASK;
          PhysicalAddress = (UINTN)L2PageTable32[Index2] & PAGING_4M_ADDRESS_MASK_32;
          if (Entry != NULL) {
            *Entry = (UINT64 *)&L2PageTable32[Index2];
          }
          if ((Cr4 & CR4_PSE) != 0) {
            // 4M PSE
            PhysicalAddress += RShiftU64 ((L2PageTable32[Index2] >> 13) & 0xFF, 32);
          }
          return PhysicalAddress + Offset;
        } else {
          if (Sp != NULL) {
            *Sp = FALSE;
          }
          // 4k
          L1PageTable32 = (UINT32 *)(UINTN)(L2PageTable32[Index2] & PAGING_4K_ADDRESS_MASK_32);
          if ((L1PageTable32[Index1] & IA32_PG_P) == 0) {
            return 0;
          }
          PhysicalAddress = (UINTN)L1PageTable32[Index1] & PAGING_4K_ADDRESS_MASK_32;
          if (Entry != NULL) {
            *Entry = (UINT64 *)&L1PageTable32[Index1];
          }
          return PhysicalAddress + Offset;
        }
      } // end of Non-PAE
    } // end of ia32 page
    else {
      if (Pg != NULL) {
        *Pg = FALSE;
      }
    }
  }

  // one-one mapping
  return GuestLinearAddress;
}

/**

  This function get guest linear address to guest physical address.

  @param CpuIndex           CPU index
  @param GuestLinearAddress Guest linear address

  @return Guest physical address
**/
UINT64
GuestLinearToGuestPhysical (
  IN UINT32   CpuIndex,
  IN UINTN    GuestLinearAddress
  )
{
  return TranslateGuestLinearToPhysical (
           VmReadN (VMCS_N_GUEST_CR3_INDEX),
           mGuestContextCommonSmm.GuestContextPerCpu[CpuIndex].Cr0,
           mGuestContextCommonSmm.GuestContextPerCpu[CpuIndex].Cr4,
           mGuestContextCommonSmm.GuestContextPerCpu[CpuIndex].Efer,
           GuestLinearAddress,
           NULL,
           NULL,
           NULL,
           NULL,
           NULL,
           NULL
           );
}

/**

  This function lookup SMI guest virtual address to guest physical address.

  @param SmiGuestCr3         SmiGuest Cr3
  @param SmiGuestCr4Pae      SmiGuest Cr4Pae
  @param SmiGuestCr4Pse      SmiGuest Cr4Pse
  @param SmiGuestIa32e       SmiGuest Ia32e
  @param GuestVirtualAddress Guest virtual address

  @return Guest physical address
**/
UINT64
LookupSmiGuestVirtualToGuestPhysical (
  IN UINTN   SmiGuestCr3,
  IN BOOLEAN SmiGuestCr4Pae,
  IN BOOLEAN SmiGuestCr4Pse,
  IN BOOLEAN SmiGuestIa32e,
  IN UINTN   GuestVirtualAddress
  )
{
  UINTN                 GuestLinearAddress;
  UINTN                 Cr0;
  UINTN                 Cr4;
  UINT64                Efer;

  GuestLinearAddress = GuestVirtualAddress;

  Cr0 = CR0_PG;
  Cr4 = 0;
  if (SmiGuestCr4Pae) {
    Cr4 |= CR4_PAE;
  }
  if (SmiGuestCr4Pse) {
    Cr4 |= CR4_PSE;
  }
  Efer = 0;
  if (SmiGuestIa32e) {
    Efer |= IA32_EFER_MSR_MLA;
  }

  return TranslateGuestLinearToPhysical (
           SmiGuestCr3,
           Cr0,
           Cr4,
           Efer,
           GuestLinearAddress,
           NULL,
           NULL,
           NULL,
           NULL,
           NULL,
           NULL
           );
}

/**

  This function map linear address to physical address.

  @param CpuIndex           CPU index
  @param PhysicalAddress    Guest physical address
  @param LinearAddress      Guest linear address
  @param SuperPageSize      Super page size (4KB means not sp, 2MB/4MB means sp)

**/
VOID
MapLinearAddressOneEntry (
  IN UINT32   CpuIndex,
  IN UINT64   PhysicalAddress,
  IN UINTN    LinearAddress,
  IN UINTN    SuperPageSize
  )
{
  UINT64   CurrentPhysicalAddress;
  BOOLEAN  Ia32e;
  BOOLEAN  Pg;
  BOOLEAN  Pae;
  BOOLEAN  Pse;
  BOOLEAN  Sp;
  UINT64   *Entry;
  UINT32   *Entry32;
  UINTN    BaseAddress;
  UINT64   *L1PageTable;
  UINT32   *L1PageTable32;
  UINTN    Index1;

  Ia32e = FALSE;
  Pg    = FALSE;
  Pae   = FALSE;
  Pse   = FALSE;
  Sp    = FALSE;
  Entry = NULL;
  CurrentPhysicalAddress = TranslateGuestLinearToPhysical (
                             VmReadN (VMCS_N_GUEST_CR3_INDEX),
                             mGuestContextCommonSmm.GuestContextPerCpu[CpuIndex].Cr0,
                             mGuestContextCommonSmm.GuestContextPerCpu[CpuIndex].Cr4,
                             mGuestContextCommonSmm.GuestContextPerCpu[CpuIndex].Efer,
                             LinearAddress,
                             &Ia32e,
                             &Pg,
                             &Pae,
                             &Pse,
                             &Sp,
                             &Entry
                             );
  if ((CurrentPhysicalAddress == 0) || (Entry == NULL)) {
    DEBUG ((EFI_D_ERROR, "!!!MapVirtualAddressToPhysicalAddress not found!!!\n"));
    // TBD - add CreateIfNotExist
    CpuDeadLoop ();
    return ;
  }
  Entry32 = (UINT32 *)Entry;

  if (!Pg) {
    return ;
  }

  if (((SuperPageSize != SIZE_4KB) && Sp) || (SuperPageSize == SIZE_4KB && (!Sp))) {
    if (Pae) {
      *Entry = PhysicalAddress | (*Entry & 0xFFF);
    } else {
      *Entry32 = (UINT32)PhysicalAddress | (*Entry32 & 0xFFF);
    }
    return ;
  }

  if ((SuperPageSize != SIZE_4KB) && (!Sp)) {
    // Go through all entry
    for (BaseAddress = LinearAddress; BaseAddress < LinearAddress + SuperPageSize; BaseAddress += SIZE_4KB) {
      MapLinearAddressOneEntry (CpuIndex, PhysicalAddress, BaseAddress, SIZE_4KB);
    }
  } else {
    // Need split - worst case need split
    L1PageTable = (UINT64 *)AllocatePages (1);
    L1PageTable32 = (UINT32 *)L1PageTable;
    if (Pae) {
      BaseAddress = (UINTN)((*Entry) & PAGING_2M_ADDRESS_MASK_64_X64);
      for (Index1 = 0; Index1 < SIZE_4KB/sizeof(*L1PageTable); Index1 ++) {
        L1PageTable[Index1] = BaseAddress | (*Entry & 0xFFF & ~IA32_PG_PAT_4K) | RShiftU64 (*Entry & IA32_PG_PAT_2M, 5);
        if (BaseAddress == LinearAddress) {
          L1PageTable[Index1] = PhysicalAddress | (*Entry & 0xFFF) | IA32_PG_P;
        }
        BaseAddress += SIZE_4KB;
      }
      *Entry = (UINT64)(UINTN)L1PageTable | (*Entry & 0xFFF & ~IA32_PG_PAT_4K) | RShiftU64 (*Entry & IA32_PG_PAT_2M, 5);
    } else {
      BaseAddress = *Entry32 & PAGING_4M_ADDRESS_MASK_32;
      for (Index1 = 0; Index1 < SIZE_4KB/sizeof(*L1PageTable32); Index1 ++) {
        L1PageTable32[Index1] = (UINT32)BaseAddress | (*Entry32 & 0xFFF & ~IA32_PG_PAT_4K) | ((*Entry32 & IA32_PG_PAT_2M) >> 5);
        if (BaseAddress == LinearAddress) {
          L1PageTable32[Index1] = (UINT32)PhysicalAddress | (*Entry32 & 0xFFF) | IA32_PG_P;
        }
        BaseAddress += SIZE_4KB;
      }
      *Entry32 = (UINT32)(UINTN)L1PageTable32 | (*Entry32 & 0xFFF & ~IA32_PG_PAT_4K) | ((*Entry32 & IA32_PG_PAT_2M) >> 5);
    }
  }

  return ;
}

/**

  This function map virtual address to physical address.

  @param CpuIndex           CPU index
  @param PhysicalAddress    Guest physical address
  @param VirtualAddress     Guest virtual address
  @param PageCount          Guest page count

**/
VOID
MapVirtualAddressToPhysicalAddress (
  IN UINT32   CpuIndex,
  IN UINT64   PhysicalAddress,
  IN UINTN    VirtualAddress,
  IN UINTN    PageCount
  )
{
  UINTN    LinearAddress;
  UINTN    SuperPageSize;
  UINTN    Address;
  UINTN    Base;
  UINTN    Length;

  if ((mGuestContextCommonSmm.GuestContextPerCpu[CpuIndex].Cr4 & CR4_PAE) == 0) {
    SuperPageSize = SIZE_4MB;
  } else {
    SuperPageSize = SIZE_2MB;
  }

  // BUGBUG: Do not know the segmentation information
  LinearAddress = VirtualAddress;

  Base = LinearAddress & ~(SIZE_4KB - 1);
  Length = STM_PAGES_TO_SIZE(PageCount);

  for (Address = Base; Address < Base + Length; ) {
    if (((Address & (SuperPageSize - 1)) == 0) &&
        ((PhysicalAddress & (SuperPageSize - 1)) == 0) &&
        ((Base <= Address + SuperPageSize) && (Address + SuperPageSize < Base + Length))) {
      MapLinearAddressOneEntry (CpuIndex, PhysicalAddress, Address, SuperPageSize);
      Address += SuperPageSize;
      PhysicalAddress += SuperPageSize;
    } else {
      MapLinearAddressOneEntry (CpuIndex, PhysicalAddress, Address, SIZE_4KB);
      Address += SIZE_4KB;
      PhysicalAddress += SIZE_4KB;
    }
  }

  return ;
}

/**

  This function unmap linear address.

  @param CpuIndex           CPU index
  @param LinearAddress      Guest linear address
  @param SuperPageSize      Super page size (4KB means not sp, 2MB/4MB means sp)

**/
VOID
UnmapLinearAddressOneEntry (
  IN UINT32   CpuIndex,
  IN UINTN    LinearAddress,
  IN UINTN    SuperPageSize
  )
{
  UINT64   CurrentPhysicalAddress;
  BOOLEAN  Ia32e;
  BOOLEAN  Pg;
  BOOLEAN  Pae;
  BOOLEAN  Pse;
  BOOLEAN  Sp;
  UINT64   *Entry;
  UINT32   *Entry32;
  UINTN    BaseAddress;
  UINT64   *L1PageTable;
  UINT32   *L1PageTable32;
  UINTN    Index1;

  ASSERT ((SuperPageSize == SIZE_4KB) || (SuperPageSize == SIZE_4MB) || (SuperPageSize == SIZE_2MB));

  Pae   = FALSE;
  Sp    = FALSE;
  Entry = NULL;
  CurrentPhysicalAddress = TranslateGuestLinearToPhysical (
                             VmReadN (VMCS_N_GUEST_CR3_INDEX),
                             mGuestContextCommonSmm.GuestContextPerCpu[CpuIndex].Cr0,
                             mGuestContextCommonSmm.GuestContextPerCpu[CpuIndex].Cr4,
                             mGuestContextCommonSmm.GuestContextPerCpu[CpuIndex].Efer,
                             LinearAddress,
                             &Ia32e,
                             &Pg,
                             &Pae,
                             &Pse,
                             &Sp,
                             &Entry
                             );
  if ((CurrentPhysicalAddress == 0) || (Entry == NULL)) {
    // Done
    return ;
  }
  Entry32 = (UINT32 *)Entry;

  if (((SuperPageSize != SIZE_4KB) && Sp) || (SuperPageSize == SIZE_4KB && (!Sp))) {
    if (Pae) {
      *Entry = *Entry & (~IA32_PG_P);
    } else {
      *Entry32 = *Entry32 & (~IA32_PG_P);
    }
    return ;
  }

  if ((SuperPageSize != SIZE_4KB) && (!Sp)) {
    // Go through all entry
    for (BaseAddress = LinearAddress; BaseAddress < LinearAddress + SuperPageSize; BaseAddress += SIZE_4KB) {
      UnmapLinearAddressOneEntry (CpuIndex, BaseAddress, SIZE_4KB);
    }
  } else {
    // Need split - worst case need split
    L1PageTable = (UINT64 *)AllocatePages (1);
    L1PageTable32 = (UINT32 *)L1PageTable;
    if (Pae) {
      BaseAddress = (UINTN)((*Entry) & PAGING_2M_ADDRESS_MASK_64_X64);
      for (Index1 = 0; Index1 < SIZE_4KB/sizeof(*L1PageTable); Index1 ++) {
        L1PageTable[Index1] = BaseAddress | (*Entry & 0xFFF & ~IA32_PG_PAT_4K) | RShiftU64 (*Entry & IA32_PG_PAT_2M, 5);
        if (BaseAddress == LinearAddress) {
          L1PageTable[Index1] &= ~IA32_PG_P;
        }
        BaseAddress += SIZE_4KB;
      }
      *Entry = (UINT64)(UINTN)L1PageTable | (*Entry & 0xFFF & ~IA32_PG_PAT_4K) | RShiftU64 (*Entry & IA32_PG_PAT_2M, 5);
    } else {
      BaseAddress = *Entry32 & PAGING_4M_ADDRESS_MASK_32;
      for (Index1 = 0; Index1 < SIZE_4KB/sizeof(*L1PageTable32); Index1 ++) {
        L1PageTable32[Index1] = (UINT32)BaseAddress | (*Entry32 & 0xFFF & ~IA32_PG_PAT_4K) | ((*Entry32 & IA32_PG_PAT_2M) >> 5);
        if (BaseAddress == LinearAddress) {
          L1PageTable32[Index1] &= ~IA32_PG_P;
        }
        BaseAddress += SIZE_4KB;
      }
      *Entry32 = (UINT32)(UINTN)L1PageTable32 | (*Entry32 & 0xFFF & ~IA32_PG_PAT_4K) | ((*Entry32 & IA32_PG_PAT_2M) >> 5);
    }
  }

  return ;
}

/**

  This function unmap virtual address.

  @param CpuIndex           CPU index
  @param VirtualAddress     Guest virtual address
  @param PageCount          Guest page count

**/
VOID
UnmapVirtualAddressToPhysicalAddress (
  IN UINT32   CpuIndex,
  IN UINTN    VirtualAddress,
  IN UINTN    PageCount
  )
{
  UINTN    LinearAddress;
  UINTN    SuperPageSize;
  UINTN    Address;
  UINTN    Base;
  UINTN    Length;

  if ((mGuestContextCommonSmm.GuestContextPerCpu[CpuIndex].Cr4 & CR4_PAE) == 0) {
    SuperPageSize = SIZE_4MB;
  } else {
    SuperPageSize = SIZE_2MB;
  }

  // BUGBUG: Do not know the segmentation information
  LinearAddress = VirtualAddress;

  Base = LinearAddress & ~(SIZE_4KB - 1);
  Length = STM_PAGES_TO_SIZE(PageCount);

  for (Address = Base; Address < Base + Length; ) {
    if (((Address & (SuperPageSize - 1)) == 0) &&
        ((Base <= Address + SuperPageSize) && (Address + SuperPageSize < Base + Length))) {
      UnmapLinearAddressOneEntry (CpuIndex, Address, SuperPageSize);
      Address += SuperPageSize;
    } else {
      UnmapLinearAddressOneEntry (CpuIndex, Address, SIZE_4KB);
      Address += SIZE_4KB;
    }
  }

  return ;
}