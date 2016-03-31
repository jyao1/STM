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

/**

  This function return UEFI memory map information.

  @param Below4GMemoryLimit  Below4G memory limit.
  @param Above4GMemoryLimit  Above4G memory limit.

  @retval EFI_SUCCESS the memory information is returned.
**/
EFI_STATUS
ReturnUefiMemoryMap (
  OUT UINT64   *Below4GMemoryLimit,
  OUT UINT64   *Above4GMemoryLimit
  )
{
  EFI_STATUS                  Status;
  EFI_MEMORY_DESCRIPTOR       *EfiMemoryMap;
  EFI_MEMORY_DESCRIPTOR       *EfiMemoryMapEnd;
  EFI_MEMORY_DESCRIPTOR       *EfiEntry;
  EFI_MEMORY_DESCRIPTOR       *NextEfiEntry;
  EFI_MEMORY_DESCRIPTOR       TempEfiEntry;
  UINTN                       EfiMemoryMapSize;
  UINTN                       EfiMapKey;
  UINTN                       EfiDescriptorSize;
  UINT32                      EfiDescriptorVersion;
  UINT64                      MemoryBlockLength;

  *Below4GMemoryLimit = 0;
  *Above4GMemoryLimit = 0;

  //
  // Get the EFI memory map.
  //
  EfiMemoryMapSize  = 0;
  EfiMemoryMap      = NULL;
  Status = gBS->GetMemoryMap (
                  &EfiMemoryMapSize,
                  EfiMemoryMap,
                  &EfiMapKey,
                  &EfiDescriptorSize,
                  &EfiDescriptorVersion
                  );
  ASSERT (Status == EFI_BUFFER_TOO_SMALL);

  do {
    //
    // Use size returned back plus 1 descriptor for the AllocatePool.
    // We don't just multiply by 2 since the "for" loop below terminates on
    // EfiMemoryMapEnd which is dependent upon EfiMemoryMapSize. Otherwize
    // we process bogus entries and create bogus E820 entries.
    //
    EfiMemoryMap = (EFI_MEMORY_DESCRIPTOR *) AllocatePool (EfiMemoryMapSize);
    ASSERT (EfiMemoryMap != NULL);
    Status = gBS->GetMemoryMap (
                    &EfiMemoryMapSize,
                    EfiMemoryMap,
                    &EfiMapKey,
                    &EfiDescriptorSize,
                    &EfiDescriptorVersion
                    );
    if (EFI_ERROR (Status)) {
      FreePool (EfiMemoryMap);
    }
  } while (Status == EFI_BUFFER_TOO_SMALL);

  ASSERT_EFI_ERROR (Status);

  
  //
  // Sort memory map from low to high
  //
  EfiEntry        = EfiMemoryMap;
  NextEfiEntry    = NEXT_MEMORY_DESCRIPTOR (EfiEntry, EfiDescriptorSize);
  EfiMemoryMapEnd = (EFI_MEMORY_DESCRIPTOR *) ((UINT8 *) EfiMemoryMap + EfiMemoryMapSize);
  while (EfiEntry < EfiMemoryMapEnd) {
    while (NextEfiEntry < EfiMemoryMapEnd) {
      if (EfiEntry->PhysicalStart > NextEfiEntry->PhysicalStart) {
        CopyMem (&TempEfiEntry, EfiEntry, sizeof (EFI_MEMORY_DESCRIPTOR));
        CopyMem (EfiEntry, NextEfiEntry, sizeof (EFI_MEMORY_DESCRIPTOR));
        CopyMem (NextEfiEntry, &TempEfiEntry, sizeof (EFI_MEMORY_DESCRIPTOR));
      }

      NextEfiEntry = NEXT_MEMORY_DESCRIPTOR (NextEfiEntry, EfiDescriptorSize);
    }

    EfiEntry      = NEXT_MEMORY_DESCRIPTOR (EfiEntry, EfiDescriptorSize);
    NextEfiEntry  = NEXT_MEMORY_DESCRIPTOR (EfiEntry, EfiDescriptorSize);
  }

  //
  //
  //
  DEBUG ((EFI_D_INFO, "MemoryMap:\n"));
  EfiEntry        = EfiMemoryMap;
  EfiMemoryMapEnd = (EFI_MEMORY_DESCRIPTOR *) ((UINT8 *) EfiMemoryMap + EfiMemoryMapSize);
  while (EfiEntry < EfiMemoryMapEnd) {
    MemoryBlockLength = (UINT64) (LShiftU64 (EfiEntry->NumberOfPages, 12));
    DEBUG ((EFI_D_INFO, "Entry(0x%02x) 0x%016lx - 0x%016lx\n", EfiEntry->Type, EfiEntry->PhysicalStart, EfiEntry->PhysicalStart + MemoryBlockLength));
    if ((EfiEntry->PhysicalStart + MemoryBlockLength) <= BASE_1MB) {
      //
      // Skip the memory block is under 1MB
      //
    } else if (EfiEntry->PhysicalStart >= BASE_4GB) {
      switch (EfiEntry->Type) {
      case EfiLoaderCode:
      case EfiLoaderData:
      case EfiBootServicesCode:
      case EfiBootServicesData:
      case EfiConventionalMemory:
      case EfiRuntimeServicesCode:
      case EfiRuntimeServicesData:
      case EfiACPIReclaimMemory:
      case EfiACPIMemoryNVS:
      case EfiReservedMemoryType:
        if (*Above4GMemoryLimit < EfiEntry->PhysicalStart + MemoryBlockLength) {
          *Above4GMemoryLimit = EfiEntry->PhysicalStart + MemoryBlockLength;
        }
        break;
      }
    } else {
      switch (EfiEntry->Type) {
      case EfiLoaderCode:
      case EfiLoaderData:
      case EfiBootServicesCode:
      case EfiBootServicesData:
      case EfiConventionalMemory:
      case EfiRuntimeServicesCode:
      case EfiRuntimeServicesData:
      case EfiACPIReclaimMemory:
      case EfiACPIMemoryNVS:
      case EfiReservedMemoryType:
        if (*Below4GMemoryLimit < EfiEntry->PhysicalStart + MemoryBlockLength) {
          *Below4GMemoryLimit = EfiEntry->PhysicalStart + MemoryBlockLength;
        }
        break;

      //
      // All other types map to reserved.
      // Adding the code just waists FLASH space.
      //
      //  case  EfiUnusableMemory:
      //  case  EfiMemoryMappedIO:
      //  case  EfiMemoryMappedIOPortSpace:
      //  case  EfiPalCode:
      //
      default:
        break;
      }
    }
    EfiEntry = NEXT_MEMORY_DESCRIPTOR (EfiEntry, EfiDescriptorSize);
  }

  FreePool (EfiMemoryMap);

  DEBUG ((EFI_D_INFO, "Result:\n"));
  DEBUG ((EFI_D_INFO, "Below4GMemoryLimit:  0x%016lx\n", *Below4GMemoryLimit));
  DEBUG ((EFI_D_INFO, "Above4GMemoryLimit:  0x%016lx\n", *Above4GMemoryLimit));

  return EFI_SUCCESS;
}

/**

  This function is EndOfDxe notificaiton.

  @param Event    event.
  @param Context  event context.

**/
VOID
EFIAPI
OnEndOfDxe (
  EFI_EVENT                               Event,
  VOID                                    *Context
  )
{
  EFI_STATUS      Status;
  VOID            *PciEnumerationComplete;
  UINTN           Index;
  UINT64          Below4GMemoryLimit;
  UINT64          Above4GMemoryLimit;
  
  if (Event != NULL) {
    gBS->CloseEvent (Event);
  }

  DEBUG((EFI_D_INFO, "Vtd OnEndOfDxe\n"));

  //
  // PCI Enumeration must be done
  //
  Status = gBS->LocateProtocol (
                  &gEfiPciEnumerationCompleteProtocolGuid,
                  NULL,
                  &PciEnumerationComplete
                  );
  ASSERT_EFI_ERROR (Status);

  ReturnUefiMemoryMap (
    &Below4GMemoryLimit,
    &Above4GMemoryLimit
    );
  Below4GMemoryLimit = ALIGN_VALUE_UP(Below4GMemoryLimit, SIZE_256MB);
  DEBUG ((EFI_D_INFO, " Adjusted Below4GMemoryLimit: 0x%016lx\n", Below4GMemoryLimit));

  //
  // 1. setup
  //
  DEBUG ((EFI_D_INFO, "GetDmarAcpiTable\n"));
  Status = GetDmarAcpiTable ();
  if (EFI_ERROR (Status)) {
    return;
  }
  DEBUG ((EFI_D_INFO, "ParseDmarAcpiTable\n"));
  Status = ParseDmarAcpiTable ();
  if (EFI_ERROR (Status)) {
    return;
  }
  DEBUG ((EFI_D_INFO, "PrepareVtdConfig\n"));
  PrepareVtdConfig ();

  //
  // 2. initialization
  //
  DEBUG ((EFI_D_INFO, "SetupTranslationTable\n"));
  Status = SetupTranslationTable (Below4GMemoryLimit, Above4GMemoryLimit);
  if (EFI_ERROR (Status)) {
    return;
  }

  ActivateHookPciProtocol ();

  for (Index = 0; Index < mVtdUnitNumber; Index++) {
    if (mVtdUnitInformation[Index].ExtRootEntryTable != NULL) {
      DumpDmarExtContextEntryTable (mVtdUnitInformation[Index].ExtRootEntryTable);
    } else {
      DumpDmarContextEntryTable (mVtdUnitInformation[Index].RootEntryTable);
    }
    if (mVtdUnitInformation[Index].FirstLevelPagingEntry != NULL) {
//      DumpFirstLevelPagingEntry (mVtdUnitInformation[Index].FirstLevelPagingEntry);
    }
    if (mVtdUnitInformation[Index].SecondLevelPagingEntry != NULL) {
//      DumpSecondLevelPagingEntry (mVtdUnitInformation[Index].SecondLevelPagingEntry);
    }
  }

  //
  // 3. enable
  //
  DEBUG ((EFI_D_INFO, "EnableDmar\n"));
  Status = EnableDmar ();
  if (EFI_ERROR (Status)) {
    return;
  }
  DEBUG ((EFI_D_INFO, "DumpVtdRegs\n"));
  DumpVtdRegs();
}

/**

  This function is ReadyToBoot event notification.

  @param Event    event.
  @param Context  event context.

**/
VOID
EFIAPI
OnReadyToBoot (
  EFI_EVENT                               Event,
  VOID                                    *Context
  )
{
  UINTN  Index;

  if (Event != NULL) {
    gBS->CloseEvent (Event);
  }

  DEBUG ((EFI_D_INFO, "Vtd OnReadyToBoot\n"));
  DumpVtdRegs();
  for (Index = 0; Index < mVtdUnitNumber; Index++) {
    if (mVtdUnitInformation[Index].FirstLevelPagingEntry != NULL) {
//      DumpFirstLevelPagingEntry (mVtdUnitInformation[Index].FirstLevelPagingEntry);
    }
    if (mVtdUnitInformation[Index].SecondLevelPagingEntry != NULL) {
//      DumpSecondLevelPagingEntry (mVtdUnitInformation[Index].SecondLevelPagingEntry);
    }
  }
}

/**

  This function is ExitBootServices event notification.

  @param Event    event.
  @param Context  event context.

**/
VOID
EFIAPI
OnExitBootServices (
  EFI_EVENT                               Event,
  VOID                                    *Context
  )
{
  DEBUG ((EFI_D_INFO, "Vtd OnExitBootServices\n"));
  DumpVtdRegs();
  DisableDmar ();
  DumpVtdRegs();
}

/**

  DMA protection driver entry point function.

  @param ImageHandle   image handle for this driver image
  @param SystemTable   pointer to the EFI System Table

  @retval EFI_SUCCESS

**/
EFI_STATUS
EFIAPI
DmaProtectionEntrypoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS  Status;
  EFI_EVENT   EndOfDxeEvent;
  EFI_EVENT   ReadyToBootEvent;
  EFI_EVENT   ExitBootServicesEvent;
  VOID        *ReadyToLock;

  Status = gBS->LocateProtocol (
                  &gEfiDxeSmmReadyToLockProtocolGuid,
                  NULL,
                  &ReadyToLock
                  );
  if (EFI_ERROR (Status)) {
    Status = gBS->CreateEventEx (
                    EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    OnEndOfDxe,
                    NULL,
                    &gEfiEndOfDxeEventGroupGuid,
                    &EndOfDxeEvent
                    );
    ASSERT_EFI_ERROR (Status);
  } else {
    OnEndOfDxe (NULL, NULL);
  }
  
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  OnExitBootServices,
                  NULL,
                  &gEfiEventExitBootServicesGuid,
                  &ExitBootServicesEvent
                  );
  ASSERT_EFI_ERROR (Status);

  Status = EfiCreateEventReadyToBootEx (
             TPL_NOTIFY,
             OnReadyToBoot,
             NULL,
             &ReadyToBootEvent
             );
  ASSERT_EFI_ERROR (Status);

  DEBUG((EFI_D_INFO, "HookPciProtocol\n"));
  HookPciProtocol ();

  CsmInit ();

  return EFI_SUCCESS;
}
