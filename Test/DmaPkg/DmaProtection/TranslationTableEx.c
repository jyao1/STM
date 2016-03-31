/** @file

  Copyright (c) 2014, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "DmaProtection.h"

typedef union {
  struct {
    UINT32  Ignored_9:1;
    UINT32  Ignored_11:1;
    UINT32  Ignored_52:11;
    UINT32  Rsvd:19;
  } Bits;
  UINT32  Uint32;
} FIRST_LEVEL_PAGING_COUNT;

/**

  This function allocates zero pages.

  @param Pages page count

  @return zero pages
**/
VOID *
EFIAPI
AllocateZeroPages (
  IN UINTN  Pages
  );

/**

  This function get 1st level paging entry attribute.

  @param PtEntry     paging entry

  @return paging count check

**/
UINT32
GetFirstLevelPagingCountCheck (
  IN VTD_FIRST_LEVEL_PAGING_ENTRY *PtEntry
  )
{
  FIRST_LEVEL_PAGING_COUNT  Count;

  Count.Uint32 = 0;
  Count.Bits.Ignored_9  = (UINT32)PtEntry->Bits.Ignored_9;
  Count.Bits.Ignored_11 = (UINT32)PtEntry->Bits.Ignored_11;
  Count.Bits.Ignored_52 = (UINT32)PtEntry->Bits.Ignored_52;
  return Count.Uint32;
}

/**

  This function set 1st level paging entry attribute.

  @param PtEntry     paging entry
  @param CountCheck  paging count check

**/
VOID
SetFirstLevelPagingCountCheck (
  IN VTD_FIRST_LEVEL_PAGING_ENTRY *PtEntry,
  IN UINT32                        CountCheck
  )
{
  FIRST_LEVEL_PAGING_COUNT  Count;

  Count.Uint32 = CountCheck;
  ASSERT (Count.Bits.Rsvd == 0);
  PtEntry->Bits.Ignored_9  = Count.Bits.Ignored_9;
  PtEntry->Bits.Ignored_11 = Count.Bits.Ignored_11;
  PtEntry->Bits.Ignored_52 = Count.Bits.Ignored_52;
}

/**

  This function get 1st level paging entry attribute.

  @param PtEntry  paging entry

  @return  If allow access

**/
BOOLEAN
GetFirstLevelPagingEntryAttribute (
  IN VTD_FIRST_LEVEL_PAGING_ENTRY  *PtEntry
  )
{
  return (BOOLEAN)PtEntry->Bits.Present;
}

/**

  This function set 1st level paging entry attribute.

  @param PtEntry  paging entry
  @param Allow    If allow access

**/
VOID
SetFirstLevelPagingEntryAttribute (
  IN VTD_FIRST_LEVEL_PAGING_ENTRY  *PtEntry,
  IN BOOLEAN                        Allow
  )
{
  PtEntry->Bits.Present = Allow;
  PtEntry->Bits.ReadWrite = Allow;
}

/**

  This function create ext context entry.

  @param VtdIndex            VTd engine index

**/
EFI_STATUS
CreateExtContextEntry (
  IN UINTN  VtdIndex
  )
{
  UINT32                 Index;
  VOID                   *Buffer;
  UINT32                 RootPages;
  UINT32                 ContextPages;
  VTD_EXT_ROOT_ENTRY     *ExtRootEntry;
  VTD_EXT_CONTEXT_ENTRY  *ExtContextEntryTable;
  VTD_EXT_CONTEXT_ENTRY  *ExtContextEntry;
  VTD_PASID_ENTRY        *PASIDEntry;
  PCI_DESCRIPTOR         *PciDescriptor;
  UINT8                  Bus;
  UINT8                  Device;
  UINT8                  Function;
  UINT64                 Pt;
  UINTN                  MaxBusNumber;
  UINTN                  EntryTablePages;
  UINTN                  PASIDIndex;
  UINTN                  PASIDEntryNumber;

  Pt = (UINT64)RShiftU64 ((UINT64)(UINTN)mVtdUnitInformation[VtdIndex].SecondLevelPagingEntry, 12);

  MaxBusNumber = 0;
  for (Index = 1; Index <= mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDescriptorNumber; Index++) {
    PciDescriptor = &mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDescriptors[Index];
    if (PciDescriptor->Bus > MaxBusNumber) {
      MaxBusNumber = PciDescriptor->Bus;
    }
  }
  DEBUG ((EFI_D_INFO,"  MaxBusNumber - 0x%x\n", MaxBusNumber));

  RootPages = EFI_SIZE_TO_PAGES (sizeof (VTD_EXT_ROOT_ENTRY) * VTD_ROOT_ENTRY_NUMBER);
  ContextPages = EFI_SIZE_TO_PAGES (sizeof (VTD_EXT_CONTEXT_ENTRY) * VTD_CONTEXT_ENTRY_NUMBER);
  EntryTablePages = RootPages + ContextPages * (MaxBusNumber + 1);
  Buffer = AllocateZeroPages (EntryTablePages);
  if (Buffer == NULL) {
    DEBUG ((EFI_D_INFO,"Could not Alloc Root Entry Table.. \n"));
    return EFI_OUT_OF_RESOURCES;
  }
  mVtdUnitInformation[VtdIndex].ExtRootEntryTable = (VTD_EXT_ROOT_ENTRY *)Buffer;
  Buffer = (UINT8 *)Buffer + EFI_PAGES_TO_SIZE (RootPages);

  for (Index = 1; Index <= mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDescriptorNumber; Index++) {
    PciDescriptor = &mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDescriptors[Index];

    Bus = (UINT8)PciDescriptor->Bus;
    Device = (UINT8)PciDescriptor->Device;
    Function = (UINT8)PciDescriptor->Function;

    ExtRootEntry = &mVtdUnitInformation[VtdIndex].ExtRootEntryTable[Bus];
    if (ExtRootEntry->Bits.LowerPresent == 0) {
      ExtRootEntry->Bits.LowerContextTablePointer  = RShiftU64 ((UINT64)(UINTN)Buffer, 12);
      ExtRootEntry->Bits.LowerPresent = 1;
      ExtRootEntry->Bits.UpperContextTablePointer  = RShiftU64 ((UINT64)(UINTN)Buffer, 12) + 1;
      ExtRootEntry->Bits.UpperPresent = 1;
      Buffer = (UINT8 *)Buffer + EFI_PAGES_TO_SIZE (ContextPages);
    }

    ExtContextEntryTable = (VTD_EXT_CONTEXT_ENTRY *)(UINTN)LShiftU64(ExtRootEntry->Bits.LowerContextTablePointer, 12) ;
    ExtContextEntry = &ExtContextEntryTable[(Device << 3) | Function];
    ExtContextEntry->Bits.DomainIdentifier = UEFI_DOMAIN_ID;
    ExtContextEntry->Bits.SecondLevelPageTranslationPointer = Pt;

    if (mVtdUnitInformation[VtdIndex].FirstLevelPagingEntry != 0) {
      Pt = (UINT64)RShiftU64 ((UINT64)(UINTN)mVtdUnitInformation[VtdIndex].FirstLevelPagingEntry, 12);
      if (mVtdUnitInformation[VtdIndex].PASIDTable == NULL) {
        // TBD
        mVtdUnitInformation[VtdIndex].PASIDTableSize = 0;
        PASIDEntryNumber = (UINTN)LShiftU64 (1, (mVtdUnitInformation[VtdIndex].PASIDTableSize + 5));
        mVtdUnitInformation[VtdIndex].PASIDTable = AllocateZeroPages (EFI_SIZE_TO_PAGES(PASIDEntryNumber * sizeof(VTD_PASID_ENTRY)));
        PASIDEntry = mVtdUnitInformation[VtdIndex].PASIDTable;
        for (PASIDIndex = 0; PASIDIndex < PASIDEntryNumber; PASIDIndex++) {
          PASIDEntry[PASIDIndex].Bits.FirstLevelPageTranslationPointer = Pt;
          if (mVtdUnitInformation[Index].ECapReg.Bits.SRS) {
            PASIDEntry[PASIDIndex].Bits.SupervisorRequestsEnable = 0;
          }
          if (mVtdUnitInformation[Index].ECapReg.Bits.MTS) {
            PASIDEntry[PASIDIndex].Bits.PageLevelWriteThrough = 0;
            PASIDEntry[PASIDIndex].Bits.PageLevelCacheDisable = 0;
          }
          PASIDEntry[PASIDIndex].Bits.Present = 1;
        }
      } else {
        PASIDEntry = mVtdUnitInformation[VtdIndex].PASIDTable;
      }

      ExtContextEntry->Bits.PASIDEnable = 1;
      ExtContextEntry->Bits.PASIDTableSize = mVtdUnitInformation[VtdIndex].PASIDTableSize;
      ExtContextEntry->Bits.PASIDTablePointer = (UINT64)RShiftU64 ((UINT64)(UINTN)mVtdUnitInformation[VtdIndex].PASIDTable, 12);
	  
      if (mVtdUnitInformation[Index].ECapReg.Bits.DIS) {
        ExtContextEntry->Bits.DeferredInvalidateEnable = 0;
      }
      if (ExtContextEntry->Bits.DeferredInvalidateEnable) {
        ExtContextEntry->Bits.PASIDStateTablePointer = 0;
      }
      if (mVtdUnitInformation[Index].ECapReg.Bits.NEST) {
        ExtContextEntry->Bits.NestedTranslationEnable = 0;
      }
      if (mVtdUnitInformation[Index].ECapReg.Bits.MTS) {
        ExtContextEntry->Bits.PageAttributeTable0 = 0;
        ExtContextEntry->Bits.PageAttributeTable1 = 0;
        ExtContextEntry->Bits.PageAttributeTable2 = 0;
        ExtContextEntry->Bits.PageAttributeTable3 = 0;
        ExtContextEntry->Bits.PageAttributeTable4 = 0;
        ExtContextEntry->Bits.PageAttributeTable5 = 0;
        ExtContextEntry->Bits.PageAttributeTable6 = 0;
        ExtContextEntry->Bits.PageAttributeTable7 = 0;
      }
      if (mVtdUnitInformation[Index].ECapReg.Bits.MTS) {
        ExtContextEntry->Bits.ExecuteRequestsEnable = 0;
      }
      if (ExtContextEntry->Bits.NestedTranslationEnable &&
          ExtContextEntry->Bits.ExecuteRequestsEnable) {
        ExtContextEntry->Bits.SecondLevelExecuteEnable = 0;
      }
      if (mVtdUnitInformation[Index].ECapReg.Bits.EAFS) {
        ExtContextEntry->Bits.ExtendedAccessedFlagEnable = 1;
      }
      if (ExtContextEntry->Bits.ExecuteRequestsEnable && PASIDEntry->Bits.SupervisorRequestsEnable) {
        ExtContextEntry->Bits.SupervisorModeExecuteProtection = 0;
      }
      if (mVtdUnitInformation[Index].ECapReg.Bits.MTS &&
          ExtContextEntry->Bits.NestedTranslationEnable) {
        ExtContextEntry->Bits.ExtendedMemoryTypeEnable = 0;
      }
      if (mVtdUnitInformation[Index].ECapReg.Bits.MTS) {
        ExtContextEntry->Bits.CacheDisable = 0;
      }
      if (PASIDEntry->Bits.SupervisorRequestsEnable) {
        ExtContextEntry->Bits.WriteProtectEnable = 0;
      }
      if (ExtContextEntry->Bits.ExecuteRequestsEnable) {
        ExtContextEntry->Bits.NoExecuteEnable = 0;
      }
      ExtContextEntry->Bits.PageGlobalEnable = 0;
    }
    if (mVtdUnitInformation[Index].ECapReg.Bits.PRS) {
      ExtContextEntry->Bits.PageRequestEnable = 0;
    }
    if (ExtContextEntry->Bits.ExtendedMemoryTypeEnable) {
      ExtContextEntry->Bits.ExtendedMemoryType = 0;
    }
    ExtContextEntry->Bits.TranslationType = 0;
    ExtContextEntry->Bits.FaultProcessingDisable = 0;
    ExtContextEntry->Bits.Present = 1;

    DEBUG ((EFI_D_INFO,"DOMAIN: B%02x D%02x F%02x\n", Bus, Device, Function));

    switch (mVtdUnitInformation[VtdIndex].CapReg.Bits.SAGAW) {
    case BIT1:
      ExtContextEntry->Bits.AddressWidth = 0x1;
      break;
    case BIT2:
      ExtContextEntry->Bits.AddressWidth = 0x2;
      break;
    }
  }

  return EFI_SUCCESS;
}

/**

  This function create 1st level paging entry for below 4G memory.

  @param VtdIndex            VTd engine index
  @param Below4GMemoryLimit  Below4G memory limit
  @param Allow               If allow access

  @retval EFI_SUCCESS create 1st level paging entry successfully.

**/
EFI_STATUS
CreateFirstLevelPagingEntryBelow4G (
  IN UINTN   VtdIndex,
  IN UINT64  Below4GMemoryLimit,
  IN BOOLEAN Allow
  )
{
  UINTN                          Index4;
  UINTN                          Index3;
  UINTN                          Index2;
  UINTN                          Lvl4Num;
  UINTN                          Lvl3Num;
  VTD_FIRST_LEVEL_PAGING_ENTRY   *Lvl4PtEntry;
  VTD_FIRST_LEVEL_PAGING_ENTRY   *Lvl3PtEntry;
  VTD_FIRST_LEVEL_PAGING_ENTRY   *Lvl2PtEntry;
  VOID                           *Buffer;
  UINT64                         BaseAddress;
  UINT8                          PhysicalAddressBits;
  UINTN                          TranslationTablePages;

  BaseAddress = 0;
  PhysicalAddressBits = (UINT8)HighBitSet64 (Below4GMemoryLimit);
  DEBUG ((EFI_D_INFO,"CreateFirstLevelPagingEntryBelow4G - PhysicalAddressBits - 0x%x\n", PhysicalAddressBits));
  if (PhysicalAddressBits <= 39) {
    Lvl4Num = 1;
    Lvl3Num = (UINT32)LShiftU64 (1, (PhysicalAddressBits - 30));
  } else {
    Lvl4Num = (UINT32)LShiftU64 (1, (PhysicalAddressBits - 39));
    Lvl3Num = 512;
  }

  DEBUG ((EFI_D_INFO,"  Lvl4Num - 0x%x\n", Lvl4Num));
  DEBUG ((EFI_D_INFO,"  Lvl3Num - 0x%x\n", Lvl3Num));

  TranslationTablePages = 1 + Lvl4Num * (1 + Lvl3Num);
  Buffer = AllocateZeroPages (TranslationTablePages);
  if (Buffer == NULL) {
    DEBUG ((EFI_D_INFO,"Could not Alloc LVL4 PT. \n"));
    return EFI_OUT_OF_RESOURCES;
  }
  mVtdUnitInformation[VtdIndex].FirstLevelPagingEntry = Buffer;
  Buffer = (UINT8 *)Buffer + EFI_PAGE_SIZE;

  Lvl4PtEntry = mVtdUnitInformation[VtdIndex].FirstLevelPagingEntry;
  for (Index4 = 0; Index4 < Lvl4Num; Index4++) {
    Lvl4PtEntry[Index4].Uint64 = (UINT64)(UINTN)Buffer;
    Buffer = (UINT8 *)Buffer + EFI_PAGE_SIZE;
    SetFirstLevelPagingEntryAttribute (&Lvl4PtEntry[Index4], TRUE);

    Lvl3PtEntry = (VTD_FIRST_LEVEL_PAGING_ENTRY *)(UINTN)LShiftU64 (Lvl4PtEntry[Index4].Bits.Address, 12);
    for (Index3 = 0; Index3 < Lvl3Num; Index3++) {
      Lvl3PtEntry[Index3].Uint64 = (UINT64)(UINTN)Buffer;
      Buffer = (UINT8 *)Buffer + EFI_PAGE_SIZE;
      SetFirstLevelPagingEntryAttribute (&Lvl3PtEntry[Index3], TRUE);

      Lvl2PtEntry = (VTD_FIRST_LEVEL_PAGING_ENTRY *)(UINTN)LShiftU64 (Lvl3PtEntry[Index3].Bits.Address, 12);
      for (Index2 = 0; Index2 < 512; Index2++) {
        Lvl2PtEntry[Index2].Uint64 = BaseAddress;
        SetFirstLevelPagingEntryAttribute (&Lvl2PtEntry[Index2], Allow);
        Lvl2PtEntry[Index2].Bits.PageSize = 1;
        if (Allow) {
          SetFirstLevelPagingCountCheck (&Lvl2PtEntry[Index2], TRUE);
        }

        BaseAddress += SIZE_2MB;
        if (BaseAddress >= Below4GMemoryLimit) {
          goto Done;
        }
      }
    }
  }

Done:
  return EFI_SUCCESS;
}

/**

  This function create 1st level paging entry for above 4G memory.

  @param VtdIndex            VTd engine index
  @param Above4GMemoryLimit  Above4G memory limit
  @param Allow               If allow access

  @retval EFI_SUCCESS create 1st level paging entry successfully.

**/
EFI_STATUS
CreateFirstLevelPagingEntryAbove4G (
  IN UINTN   VtdIndex,
  IN UINT64  Above4GMemoryLimit,
  IN BOOLEAN Allow
  )
{
  UINTN                          Index4;
  UINTN                          Index3;
  UINTN                          Index2;
  UINTN                          Lvl4Start;
  UINTN                          Lvl4End;
  UINTN                          Lvl3Start;
  UINTN                          Lvl3End;
  VTD_FIRST_LEVEL_PAGING_ENTRY   *Lvl4PtEntry;
  VTD_FIRST_LEVEL_PAGING_ENTRY   *Lvl3PtEntry;
  VTD_FIRST_LEVEL_PAGING_ENTRY   *Lvl2PtEntry;
  UINT64                         BaseAddress;
  UINT64                         EndAddress;

  if (Above4GMemoryLimit == 0) {
    return EFI_SUCCESS;
  }

  BaseAddress = SIZE_4GB;
  EndAddress = ALIGN_VALUE_UP(Above4GMemoryLimit, SIZE_2MB);
  DEBUG ((EFI_D_INFO,"CreateFirstLevelPagingEntryAbove4G - EndAddress - 0x%016lx\n", EndAddress));

  Lvl4Start = RShiftU64 (BaseAddress, 39) & 0x1FF;
  Lvl4End = RShiftU64 (EndAddress - 1, 39) & 0x1FF;

  DEBUG ((EFI_D_INFO,"  Lvl4Start - 0x%x, Lvl4End - 0x%x\n", Lvl4Start, Lvl4End));

  Lvl4PtEntry = (VTD_FIRST_LEVEL_PAGING_ENTRY *)mVtdUnitInformation[VtdIndex].FirstLevelPagingEntry;
  for (Index4 = Lvl4Start; Index4 <= Lvl4End; Index4++) {
    if (Lvl4PtEntry[Index4].Uint64 == 0) {
      Lvl4PtEntry[Index4].Uint64 = (UINT64)(UINTN)AllocateZeroPages (1);
      if (Lvl4PtEntry[Index4].Uint64 == 0) {
        DEBUG ((EFI_D_INFO,"!!!!!! ALLOCATE LVL4 PAGE FAIL (0x%x)!!!!!!\n", Index4));
        ASSERT(FALSE);
        return EFI_OUT_OF_RESOURCES;
      }
      SetFirstLevelPagingEntryAttribute (&Lvl4PtEntry[Index4], TRUE);
    }

    Lvl3Start = RShiftU64 (BaseAddress, 30) & 0x1FF;
    if (ALIGN_VALUE_LOW(BaseAddress + SIZE_1GB, SIZE_1GB) <= EndAddress) {
      Lvl3End = SIZE_4KB/sizeof(VTD_SECOND_LEVEL_PAGING_ENTRY);
    } else {
      Lvl3End = RShiftU64 (EndAddress - 1, 30) & 0x1FF;
    }
    DEBUG ((EFI_D_INFO,"  Lvl4(0x%x): Lvl3Start - 0x%x, Lvl3End - 0x%x\n", Index4, Lvl3Start, Lvl3End));

    Lvl3PtEntry = (VTD_FIRST_LEVEL_PAGING_ENTRY *)(UINTN)LShiftU64 (Lvl4PtEntry[Index4].Bits.Address, 12);
    for (Index3 = Lvl3Start; Index3 <= Lvl3End; Index3++) {
      if (Lvl3PtEntry[Index3].Uint64 == 0) {
        Lvl3PtEntry[Index3].Uint64 = (UINT64)(UINTN)AllocateZeroPages (1);
        if (Lvl3PtEntry[Index3].Uint64 == 0) {
          DEBUG ((EFI_D_INFO,"!!!!!! ALLOCATE LVL3 PAGE FAIL (0x%x, 0x%x)!!!!!!\n", Index4, Index3));
          ASSERT(FALSE);
          return EFI_OUT_OF_RESOURCES;
        }
        SetFirstLevelPagingEntryAttribute (&Lvl3PtEntry[Index3], TRUE);
      }

      Lvl2PtEntry = (VTD_FIRST_LEVEL_PAGING_ENTRY *)(UINTN)LShiftU64 (Lvl3PtEntry[Index3].Bits.Address, 12);
      for (Index2 = 0; Index2 < SIZE_4KB/sizeof(VTD_FIRST_LEVEL_PAGING_ENTRY); Index2++) {
        Lvl2PtEntry[Index2].Uint64 = BaseAddress;
        SetFirstLevelPagingEntryAttribute (&Lvl2PtEntry[Index2], Allow);
        Lvl2PtEntry[Index2].Bits.PageSize = 1;
        if (Allow) {
          SetFirstLevelPagingCountCheck (&Lvl2PtEntry[Index2], TRUE);
        }

        BaseAddress += SIZE_2MB;
        if (BaseAddress >= Above4GMemoryLimit) {
          goto Done;
        }
      }
    }
  }

Done:
  return EFI_SUCCESS;
}

/**

  This function create 1st level paging entry.

  @param VtdIndex            VTd engine index
  @param Below4GMemoryLimit  Below4G memory limit
  @param Above4GMemoryLimit  Above4G memory limit
  @param Allow               If allow access

  @retval EFI_SUCCESS create 1st level paging entry successfully.

**/
EFI_STATUS
CreateFirstLevelPagingEntry (
  IN UINTN   VtdIndex,
  IN UINT64  Below4GMemoryLimit,
  IN UINT64  Above4GMemoryLimit,
  IN BOOLEAN Allow
  )
{
  EFI_STATUS  Status;

  Status = CreateFirstLevelPagingEntryBelow4G (VtdIndex, Below4GMemoryLimit, Allow);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = CreateFirstLevelPagingEntryAbove4G (VtdIndex, Above4GMemoryLimit, Allow);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}

/**

  This function dump ext context entry table.

  @param ExtRootEntry  ext root entry

**/
VOID
DumpDmarExtContextEntryTable (
  IN VTD_EXT_ROOT_ENTRY *ExtRootEntry
  )
{
  UINTN                 Index;
  UINTN                 Index2;
  VTD_EXT_CONTEXT_ENTRY *ExtContextEntry;

  DEBUG ((EFI_D_INFO,"DMAR Context Entry Table:\n"));
  DEBUG ((EFI_D_INFO,"=========================\n"));

  DEBUG ((EFI_D_INFO,"ExtRootEntry Address - 0x%x\n", ExtRootEntry));

  for (Index = 0; Index < VTD_ROOT_ENTRY_NUMBER; Index++) {
    if ((ExtRootEntry[Index].Uint128.Uint64Lo != 0) || (ExtRootEntry[Index].Uint128.Uint64Hi != 0)) {
      DEBUG ((EFI_D_INFO,"  ExtRootEntry(0x%02x) B%02x - 0x%016lx %016lx\n",
        Index, Index, ExtRootEntry[Index].Uint128.Uint64Hi, ExtRootEntry[Index].Uint128.Uint64Lo));
    }
    if (ExtRootEntry[Index].Bits.LowerPresent == 0) {
      continue;
    }
    ExtContextEntry = (VTD_EXT_CONTEXT_ENTRY *)(UINTN)LShiftU64 (ExtRootEntry[Index].Bits.LowerContextTablePointer, 12);
    for (Index2 = 0; Index2 < VTD_CONTEXT_ENTRY_NUMBER/2; Index2++) {
      if ((ExtContextEntry[Index2].Uint256.Uint64_1 != 0) || (ExtContextEntry[Index2].Uint256.Uint64_2 != 0) ||
          (ExtContextEntry[Index2].Uint256.Uint64_3 != 0) || (ExtContextEntry[Index2].Uint256.Uint64_4 != 0)) {
        DEBUG ((EFI_D_INFO,"    ExtContextEntryLower(0x%02x) D%02xF%02x - 0x%016lx %016lx %016lx %016lx\n",
          Index2, Index2 >> 3, Index2 & 0x7, ExtContextEntry[Index2].Uint256.Uint64_4, ExtContextEntry[Index2].Uint256.Uint64_3, ExtContextEntry[Index2].Uint256.Uint64_2, ExtContextEntry[Index2].Uint256.Uint64_1));
      }
      if (ExtContextEntry[Index].Bits.Present == 0) {
        continue;
      }
    }
    
    if (ExtRootEntry[Index].Bits.UpperPresent == 0) {
      continue;
    }
    ExtContextEntry = (VTD_EXT_CONTEXT_ENTRY *)(UINTN)LShiftU64 (ExtRootEntry[Index].Bits.UpperContextTablePointer, 12);
    for (Index2 = 0; Index2 < VTD_CONTEXT_ENTRY_NUMBER/2; Index2++) {
      if ((ExtContextEntry[Index2].Uint256.Uint64_1 != 0) || (ExtContextEntry[Index2].Uint256.Uint64_2 != 0) ||
          (ExtContextEntry[Index2].Uint256.Uint64_3 != 0) || (ExtContextEntry[Index2].Uint256.Uint64_4 != 0)) {
        DEBUG ((EFI_D_INFO,"    ExtContextEntryUpper(0x%02x) D%02xF%02x - 0x%016lx %016lx %016lx %016lx\n",
          Index2, (Index2 + 128) >> 3, (Index2 + 128) & 0x7, ExtContextEntry[Index2].Uint256.Uint64_4, ExtContextEntry[Index2].Uint256.Uint64_3, ExtContextEntry[Index2].Uint256.Uint64_2, ExtContextEntry[Index2].Uint256.Uint64_1));
      }
      if (ExtContextEntry[Index].Bits.Present == 0) {
        continue;
      }
    }
  }
}

/**

  This function dump 1st level paging entry.

  @param FirstLevelPagingEntry  1st level paging entry

**/
VOID
DumpFirstLevelPagingEntry (
  IN VOID *FirstLevelPagingEntry
  )
{
  UINTN                          Index4;
  UINTN                          Index3;
  UINTN                          Index2;
  UINTN                          Index1;
  VTD_FIRST_LEVEL_PAGING_ENTRY  *Lvl4PtEntry;
  VTD_FIRST_LEVEL_PAGING_ENTRY  *Lvl3PtEntry;
  VTD_FIRST_LEVEL_PAGING_ENTRY  *Lvl2PtEntry;
  VTD_FIRST_LEVEL_PAGING_ENTRY  *Lvl1PtEntry;

  DEBUG ((EFI_D_INFO,"DMAR First Level Page Table:\n"));
  DEBUG ((EFI_D_INFO,"================\n"));

  DEBUG ((EFI_D_INFO,"FirstLevelPagingEntry Base - 0x%x\n", FirstLevelPagingEntry));
  Lvl4PtEntry = (VTD_FIRST_LEVEL_PAGING_ENTRY *)FirstLevelPagingEntry;
  for (Index4 = 0; Index4 < SIZE_4KB/sizeof(VTD_FIRST_LEVEL_PAGING_ENTRY); Index4++) {
    if (Lvl4PtEntry[Index4].Uint64 != 0) {
      DEBUG ((EFI_D_INFO,"  Lvl4Pt Entry(0x%03x) - 0x%016lx\n", Index4, Lvl4PtEntry[Index4].Uint64));
    }
    if (Lvl4PtEntry[Index4].Uint64 == 0) {
      continue;
    }
    Lvl3PtEntry = (VTD_FIRST_LEVEL_PAGING_ENTRY *)(UINTN)LShiftU64 (Lvl4PtEntry[Index4].Bits.Address, 12);
    for (Index3 = 0; Index3 < SIZE_4KB/sizeof(VTD_FIRST_LEVEL_PAGING_ENTRY); Index3++) {
      if (Lvl3PtEntry[Index3].Uint64 != 0) {
        DEBUG ((EFI_D_INFO,"    Lvl3Pt Entry(0x%03x) - 0x%016lx\n", Index3, Lvl3PtEntry[Index3].Uint64));
      }
      if (Lvl3PtEntry[Index3].Uint64 == 0) {
        continue;
      }

      Lvl2PtEntry = (VTD_FIRST_LEVEL_PAGING_ENTRY *)(UINTN)LShiftU64 (Lvl3PtEntry[Index3].Bits.Address, 12);
      for (Index2 = 0; Index2 < SIZE_4KB/sizeof(VTD_FIRST_LEVEL_PAGING_ENTRY); Index2++) {
        if (Lvl2PtEntry[Index2].Uint64 != 0) {
          DEBUG ((EFI_D_INFO,"      Lvl2Pt Entry(0x%03x) - 0x%016lx\n", Index2, Lvl2PtEntry[Index2].Uint64));
        }
        if (Lvl2PtEntry[Index2].Uint64 == 0) {
          continue;
        }
        if (Lvl2PtEntry[Index2].Bits.PageSize == 0) {
          Lvl1PtEntry = (VTD_FIRST_LEVEL_PAGING_ENTRY *)(UINTN)LShiftU64 (Lvl2PtEntry[Index2].Bits.Address, 12);
          for (Index1 = 0; Index1 < SIZE_4KB/sizeof(VTD_FIRST_LEVEL_PAGING_ENTRY); Index1++) {
            if (Lvl1PtEntry[Index1].Uint64 != 0) {
              DEBUG ((EFI_D_INFO,"        Lvl1Pt Entry(0x%03x) - 0x%016lx\n", Index1, Lvl1PtEntry[Index1].Uint64));
            }
          }
        }
      }
    }
  }
  DEBUG ((EFI_D_INFO,"================\n"));
}

/**

  This function update 1st level paging entry.

  @param VtdIndex      VTd engine index
  @param PtEntry       1st level paging entry
  @param Allow         If allow access
  @param AddressMode   Address mode

**/
VOID
UpdateFirstLevelPagingEntry (
  IN UINTN                          VtdIndex,
  IN VTD_FIRST_LEVEL_PAGING_ENTRY   *PtEntry,
  IN BOOLEAN                        Allow,
  IN UINT8                          AddressMode
  )
{
  BOOLEAN                        Org;
  BOOLEAN                        Curr;
  UINT32                         CountCheck;

//  DEBUG ((EFI_D_INFO,"====== UpdateFirstLevelPagingEntry(%d) ====== (0x%016lx, %d)\n", AddressMode, PtEntry->Uint64, Allow));

  Org = (BOOLEAN)GetFirstLevelPagingEntryAttribute(PtEntry);
  CountCheck = GetFirstLevelPagingCountCheck (PtEntry);
  if (Allow) {
    CountCheck++;
    SetFirstLevelPagingEntryAttribute (PtEntry, 1);
  } else {
    ASSERT (CountCheck != 0);
    CountCheck--;
    if (CountCheck == 0) {
      SetFirstLevelPagingEntryAttribute (PtEntry, 0);
//      DEBUG ((EFI_D_INFO,"!!!!!! DISABLE MAPPING !!!!!! (0x%016lx)\n", LShiftU64 (PtEntry->Bits.Address, 12)));
    }
  }
  SetFirstLevelPagingCountCheck (PtEntry, CountCheck);

  Curr = (BOOLEAN)GetFirstLevelPagingEntryAttribute(PtEntry);
  if (Org != Curr) {
    mVtdUnitInformation[VtdIndex].HasDirtyPages = TRUE;
  }
}

/**

  This function split 1st level paging entry.

  @param VtdIndex      VTd engine index
  @param Lvl2PtEntry   1st level paging entry

  @retval EFI_SUCCESS split 1st level paging entry successfully.

**/
EFI_STATUS
SplitFirstLevelPagingEntry (
  IN UINTN                          VtdIndex,
  IN VTD_FIRST_LEVEL_PAGING_ENTRY   *Lvl2PtEntry
  )
{
  UINTN                          Index1;
  VTD_FIRST_LEVEL_PAGING_ENTRY   *Lvl1PtEntry;
  UINT64                         BaseAddress;
  UINT32                         CountCheck;
  BOOLEAN                        Allow;

  BaseAddress = LShiftU64 (Lvl2PtEntry->Bits.Address, 12);
  CountCheck = GetFirstLevelPagingCountCheck (Lvl2PtEntry);
  Allow = GetFirstLevelPagingEntryAttribute (Lvl2PtEntry);

  Lvl1PtEntry = AllocateZeroPages (1);
  if (Lvl1PtEntry == NULL) {
    DEBUG ((EFI_D_INFO,"!!!!!! SPLIT 2MB PAGE FAIL !!!!!! (0x%016lx)\n", Lvl2PtEntry->Uint64));
    ASSERT(FALSE);
    return EFI_OUT_OF_RESOURCES;
  } else {
    DEBUG ((EFI_D_INFO,"====== SPLIT 2MB PAGE ====== (0x%016lx=>0x%08x)\n", Lvl2PtEntry->Uint64, Lvl1PtEntry));
    Lvl2PtEntry->Uint64 = (UINT64)(UINTN)Lvl1PtEntry;
    SetFirstLevelPagingEntryAttribute (Lvl2PtEntry, TRUE);
    Lvl2PtEntry->Bits.PageSize = 0;
    SetFirstLevelPagingCountCheck (Lvl2PtEntry, 0);

    for (Index1 = 0; Index1 < SIZE_4KB/sizeof(VTD_SECOND_LEVEL_PAGING_ENTRY); Index1++) {
      Lvl1PtEntry[Index1].Uint64 = BaseAddress;
      SetFirstLevelPagingEntryAttribute (&Lvl1PtEntry[Index1], Allow);
      SetFirstLevelPagingCountCheck (&Lvl1PtEntry[Index1], CountCheck);
      BaseAddress += SIZE_4KB;
    }
  }

  mVtdUnitInformation[VtdIndex].HasDirtyPages = TRUE;

  return EFI_SUCCESS;
}

/**

  This function invalidate VTd page entry.

  @param VtdIndex      VTd engine index

**/
VOID
InvalidatePageEntry (
  IN UINTN                 VtdIndex
  );

/**

  This function set 1st level page attribute on VTd engine.

  @param VtdIndex      VTd engine index
  @param BaseAddress   BaseAddress
  @param Length        Length
  @param Allow         If allow access

  @retval EFI_SUCCESS  set page attribute successfully

**/
EFI_STATUS
SetFirstLevelPagingAttribute (
  IN UINTN                 VtdIndex,
  IN UINT64                BaseAddress,
  IN UINT64                Length,
  IN BOOLEAN               Allow
  )
{
  UINTN                          Index4;
  UINTN                          Index3;
  UINTN                          Index2;
  UINTN                          Index1;
  UINTN                          Lvl4Start;
  UINTN                          Lvl4End;
  UINTN                          Lvl3Start;
  UINTN                          Lvl3End;
  VTD_FIRST_LEVEL_PAGING_ENTRY   *Lvl4PtEntry;
  VTD_FIRST_LEVEL_PAGING_ENTRY   *Lvl3PtEntry;
  VTD_FIRST_LEVEL_PAGING_ENTRY   *Lvl2PtEntry;
  VTD_FIRST_LEVEL_PAGING_ENTRY   *Lvl1PtEntry;
  UINT64                         StartAddress;
  UINT64                         CurrentAddress;
  UINT64                         EndAddress;
  UINT64                         VirtualAddress;
  EFI_STATUS                     Status;

  StartAddress = ALIGN_VALUE_LOW(BaseAddress, SIZE_4KB);
  EndAddress = ALIGN_VALUE_UP(BaseAddress + Length, SIZE_4KB);
  CurrentAddress = StartAddress;

  Lvl4Start = RShiftU64 (CurrentAddress, 39) & 0x1FF;
  Lvl4End = RShiftU64 (EndAddress - 1, 39) & 0x1FF;

//  DEBUG ((EFI_D_INFO,"  Addjusted Addr (0x%016lx - 0x%016lx)\n", StartAddress, EndAddress - StartAddress));

//  DEBUG ((EFI_D_INFO,"FirstLevelPagingEntry Base - 0x%x\n", mVtdUnitInformation[VtdIndex].FirstLevelPagingEntry));
  Lvl4PtEntry = (VTD_FIRST_LEVEL_PAGING_ENTRY *)mVtdUnitInformation[VtdIndex].FirstLevelPagingEntry;
  for (Index4 = Lvl4Start; Index4 <= Lvl4End; Index4++) {
    if (Lvl4PtEntry[Index4].Uint64 == 0) {
      continue;
    }

    Lvl3Start = RShiftU64 (CurrentAddress, 30) & 0x1FF;
    if (ALIGN_VALUE_LOW(BaseAddress + SIZE_1GB, SIZE_1GB) <= EndAddress) {
      Lvl3End = SIZE_4KB/sizeof(VTD_FIRST_LEVEL_PAGING_ENTRY);
    } else {
      Lvl3End = RShiftU64 (EndAddress - 1, 30) & 0x1FF;
    }

    Lvl3PtEntry = (VTD_FIRST_LEVEL_PAGING_ENTRY *)(UINTN)LShiftU64 (Lvl4PtEntry[Index4].Bits.Address, 12);
    for (Index3 = Lvl3Start; Index3 <= Lvl3End; Index3++) {
      if (Lvl3PtEntry[Index3].Uint64 == 0) {
        continue;
      }

      Lvl2PtEntry = (VTD_FIRST_LEVEL_PAGING_ENTRY *)(UINTN)LShiftU64 (Lvl3PtEntry[Index3].Bits.Address, 12);
      for (Index2 = 0; Index2 < SIZE_4KB/sizeof(VTD_FIRST_LEVEL_PAGING_ENTRY); Index2++) {
        if (Lvl2PtEntry[Index2].Uint64 == 0) {
          continue;
        }
        if (Lvl2PtEntry[Index2].Bits.PageSize != 0) {
          //
          // 2M page
          //
          VirtualAddress = LShiftU64 (Lvl2PtEntry[Index2].Bits.Address, 12);
          if ((VirtualAddress == CurrentAddress) && (CurrentAddress + SIZE_2MB <= EndAddress)) {
            if ((mVtdUnitInformation[VtdIndex].CapReg.Bits.SLLPS & BIT0) == 0) {
              Status = SplitFirstLevelPagingEntry (VtdIndex, &Lvl2PtEntry[Index2]);
              if (EFI_ERROR (Status)) {
                return Status;
              }
              return SetFirstLevelPagingAttribute (VtdIndex, CurrentAddress, BaseAddress + Length - CurrentAddress, Allow);
            }
            UpdateFirstLevelPagingEntry (VtdIndex, &Lvl2PtEntry[Index2], Allow, B_IVA_REG_AM_2M);
            CurrentAddress += SIZE_2MB;
            if (CurrentAddress >= EndAddress) {
              goto Done;
            }
          } else if ((VirtualAddress <= CurrentAddress) && (CurrentAddress < VirtualAddress + SIZE_2MB)) {
            //
            // Split 2M and recersive call
            //
            Status = SplitFirstLevelPagingEntry (VtdIndex, &Lvl2PtEntry[Index2]);
            if (EFI_ERROR (Status)) {
              return Status;
            }
            return SetFirstLevelPagingAttribute (VtdIndex, CurrentAddress, BaseAddress + Length - CurrentAddress, Allow);
          }
        } else {
          //
          // 4K page
          //
          Lvl1PtEntry = (VTD_FIRST_LEVEL_PAGING_ENTRY *)(UINTN)LShiftU64 (Lvl2PtEntry[Index2].Bits.Address, 12);
          for (Index1 = 0; Index1 < SIZE_4KB/sizeof(VTD_FIRST_LEVEL_PAGING_ENTRY); Index1++) {
            VirtualAddress = LShiftU64 (Lvl1PtEntry[Index1].Bits.Address, 12);
            if (VirtualAddress == CurrentAddress) {
              UpdateFirstLevelPagingEntry (VtdIndex, &Lvl1PtEntry[Index1], Allow, B_IVA_REG_AM_4K);
              CurrentAddress += SIZE_4KB;
              if (CurrentAddress >= EndAddress) {
                goto Done;
              }
            }
          }
        } // 2M/4K page
      }
    }
  }

Done:
  InvalidatePageEntry (VtdIndex);

  return EFI_SUCCESS;
}
