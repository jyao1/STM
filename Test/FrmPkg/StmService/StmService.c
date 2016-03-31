/** @file

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmService.h"

#define MAX_RESOURCE_PAGES 4
VOID *mTempResourceBuffer;

/**

  Get resource list.
  EndResource is excluded.

  @param ResourceList  A pointer to resource list to be added
  @param NumEntries    Optional number of entries.
                       If 0, list must be terminated by END_OF_RESOURCES.

  @retval TRUE  resource valid
  @retval FALSE resource invalid

**/
UINTN
GetResourceSize (
  IN  STM_RSC    *ResourceList,
  IN  UINT32      NumEntries OPTIONAL
  )
{
  UINT32      Count;
  UINTN       Index;
  STM_RSC    *Resource;

  if (ResourceList == NULL) {
    return 0;
  }

  Resource = ResourceList;

  //
  // If NumEntries == 0 make it very big. Scan will be terminated by
  // END_OF_RESOURCES. 
  // 
  if (NumEntries == 0) {
    Count = 0xFFFFFFFF;
  } else {
    Count = NumEntries;
  }

  //
  // Start from beginning of resource list.
  // 
  Resource = ResourceList;
  
  for (Index = 0; Index < Count; Index++) {
    if (Resource->Header.RscType == END_OF_RESOURCES) {
      break;
    }
    Resource = (STM_RSC *)((UINTN)Resource + Resource->Header.Length);
  }

  return (UINTN)Resource - (UINTN)ResourceList;
}

/**

  Start STM.

  @retval EFI_SUCCESS            Start successfully.
**/
EFI_STATUS
EFIAPI
Start (
  VOID
  )
{
  UINT32  Eax;
  DEBUG ((EFI_D_INFO, "(SRM) STM_API_START\n"));
  Eax = (UINT32)AsmLaunchStm (STM_API_START, 0, 0, 0);
  if (Eax == STM_SUCCESS) {
    return EFI_SUCCESS;
  } else {
    return EFI_DEVICE_ERROR;
  }
}

/**

  Stop STM.
  
  @retval EFI_SUCCESS            Stop successfully.
**/
EFI_STATUS
EFIAPI
Stop (
  VOID
  )
{
  UINT32  Eax;
  DEBUG ((EFI_D_INFO, "(SRM) STM_API_STOP\n"));
  Eax = (UINT32)AsmTeardownStm (STM_API_STOP);
  if (Eax == STM_SUCCESS) {
    return EFI_SUCCESS;
  } else {
    return EFI_DEVICE_ERROR;
  }
}

/**

  Add resources in list to database.

  @param ResourceList  A pointer to resource list to be added
  @param NumEntries    Optional number of entries.
                       If 0, list must be terminated by END_OF_RESOURCES.

  @retval EFI_SUCCESS            If resources are added
  @retval EFI_INVALID_PARAMETER  If nested procedure detected resource failer
  @retval EFI_OUT_OF_RESOURCES   If nested procedure returned it and we cannot allocate more areas.

**/
EFI_STATUS
EFIAPI
ProtectOsResource (
  IN STM_RSC *ResourceList,
  IN UINT32   NumEntries OPTIONAL
  )
{
  VOID    *Resource;
  UINTN   ResourceSize;
  UINT32  Eax;
  
  DEBUG ((EFI_D_INFO, "(SRM) STM_API_PROTECT_RESOURCE\n"));
  ResourceSize = GetResourceSize (ResourceList, NumEntries);
  if (EFI_SIZE_TO_PAGES(ResourceSize) > MAX_RESOURCE_PAGES) {
    return EFI_OUT_OF_RESOURCES;
  }
  Resource = mTempResourceBuffer;
  ZeroMem (Resource, EFI_PAGES_TO_SIZE(MAX_RESOURCE_PAGES));
  CopyMem (Resource, ResourceList, ResourceSize);
  Eax = (UINT32)AsmLaunchStm (
                  STM_API_PROTECT_RESOURCE,
                  (UINT32)((UINTN)Resource),
                  (UINT32)RShiftU64 ((UINT64)(UINTN)Resource, 32),
                  0
                  );
  if (Eax == STM_SUCCESS) {
    return EFI_SUCCESS;
  } else {
    return EFI_DEVICE_ERROR;
  }
}

/**

  Delete resources in list to database.

  @param ResourceList  A pointer to resource list to be deleted
                       NULL means delete all resources.
  @param NumEntries    Optional number of entries.
                       If 0, list must be terminated by END_OF_RESOURCES.

  @retval EFI_SUCCESS            If resources are deleted
  @retval EFI_INVALID_PARAMETER  If nested procedure detected resource failer

**/
EFI_STATUS
EFIAPI
UnprotectOsResource (
  IN STM_RSC *ResourceList OPTIONAL,
  IN UINT32   NumEntries OPTIONAL
  )
{
  VOID    *Resource;
  UINTN   ResourceSize;
  UINT32  Eax;

  DEBUG ((EFI_D_INFO, "(SRM) STM_API_UNPROTECT_RESOURCE\n"));
  ResourceSize = GetResourceSize (ResourceList, NumEntries);
  if (EFI_SIZE_TO_PAGES(ResourceSize) > MAX_RESOURCE_PAGES) {
    return EFI_OUT_OF_RESOURCES;
  }
  Resource = mTempResourceBuffer;
  ZeroMem (Resource, EFI_PAGES_TO_SIZE(MAX_RESOURCE_PAGES));
  CopyMem (Resource, ResourceList, ResourceSize);
  Eax = (UINT32)AsmLaunchStm (
                  STM_API_UNPROTECT_RESOURCE,
                  (UINT32)((UINTN)Resource),
                  (UINT32)RShiftU64 ((UINT64)(UINTN)Resource, 32),
                  0
                  );
  if (Eax == STM_SUCCESS) {
    return EFI_SUCCESS;
  } else {
    return EFI_DEVICE_ERROR;
  }
}

/**

  Get OS resources.

  @param ResourceList  A pointer to resource list to be filled
  @param ResourceSize  On input it means size of resource list input.
                       On output it means size of resource list filled,
                       or the size of resource list to be filled if size of too small.

  @retval EFI_SUCCESS            If resources are returned.
  @retval EFI_BUFFER_TOO_SMALL   If resource list buffer is too small to hold the whole resources.

**/
EFI_STATUS
EFIAPI
GetOsResource (
  OUT    STM_RSC *ResourceList,
  IN OUT UINT32  *ResourceSize
  )
{
  return EFI_UNSUPPORTED;
}

/**

  Get BIOS resources.

  @param ResourceList  A pointer to resource list to be filled
  @param ResourceSize  On input it means size of resource list input.
                       On output it means size of resource list filled,
                       or the size of resource list to be filled if size of too small.

  @retval EFI_SUCCESS            If resources are returned.
  @retval EFI_BUFFER_TOO_SMALL   If resource list buffer is too small to hold the whole resources.

**/
EFI_STATUS
EFIAPI
ReturnPiResource (
  OUT    STM_RSC *ResourceList,
  IN OUT UINT32  *ResourceSize
  )
{
  VOID    *Resource;
  UINT32  PageIndex;
  UINT64  Ret;
  UINT32  Eax;
  
  DEBUG ((EFI_D_INFO, "(SRM) STM_API_GET_BIOS_RESOURCES\n"));
  Resource = mTempResourceBuffer;
  ZeroMem (Resource, EFI_PAGES_TO_SIZE(MAX_RESOURCE_PAGES));
  for (PageIndex = 0; PageIndex < MAX_RESOURCE_PAGES; PageIndex++) {
    Ret = AsmLaunchStm (
            STM_API_GET_BIOS_RESOURCES,
            (UINT32)((UINTN)Resource + EFI_PAGES_TO_SIZE(PageIndex)),
            (UINT32)RShiftU64 ((UINT64)(UINTN)Resource + EFI_PAGES_TO_SIZE(PageIndex), 32),
            PageIndex
            );
    Eax = (UINT32)Ret;
    if (Eax != STM_SUCCESS) {
      break;
    }
    //
    // Page index of next page to read.
    // A return of EDX=0 signifies that the entire list has been read.
    //
  }
  if (PageIndex == 0) {
    return EFI_DEVICE_ERROR;
  }
  if (*ResourceSize < EFI_PAGES_TO_SIZE(PageIndex)) {
    *ResourceSize = EFI_PAGES_TO_SIZE(PageIndex);
    return EFI_BUFFER_TOO_SMALL;
  }
  CopyMem (ResourceList, Resource, EFI_PAGES_TO_SIZE(PageIndex));
  *ResourceSize = EFI_PAGES_TO_SIZE(PageIndex);
  return EFI_SUCCESS;
}

/**

  Initialize protection.
  
  @retval EFI_SUCCESS            Initialize successfully.
**/
EFI_STATUS
EFIAPI
InitializeProtection (
  VOID
  )
{
  UINT32  Eax;
  DEBUG ((EFI_D_INFO, "(SRM) STM_API_INITIALIZE_PROTECTION\n"));
  Eax = (UINT32)AsmLaunchStm (STM_API_INITIALIZE_PROTECTION, 0, 0, 0);
  if (Eax == STM_SUCCESS) {
    return EFI_SUCCESS;
  } else {
    return EFI_DEVICE_ERROR;
  }
}

/**

  Add resources in list to database.

  @param ResourceList  A pointer to resource list to be added
  @param NumEntries    Optional number of entries.
                       If 0, list must be terminated by END_OF_RESOURCES.

  @retval EFI_SUCCESS            If resources are added
  @retval EFI_INVALID_PARAMETER  If nested procedure detected resource failer
  @retval EFI_OUT_OF_RESOURCES   If nested procedure returned it and we cannot allocate more areas.

**/
EFI_STATUS
EFIAPI
AddOsTrustedRegion (
  IN STM_RSC *ResourceList,
  IN UINT32   NumEntries OPTIONAL
  )
{
  return EFI_UNSUPPORTED;
}

/**

  Delete resources in list to database.

  @param ResourceList  A pointer to resource list to be deleted
                       NULL means delete all resources.
  @param NumEntries    Optional number of entries.
                       If 0, list must be terminated by END_OF_RESOURCES.

  @retval EFI_SUCCESS            If resources are deleted
  @retval EFI_INVALID_PARAMETER  If nested procedure detected resource failer

**/
EFI_STATUS
EFIAPI
DeleteOsTrustedRegion (
  IN STM_RSC *ResourceList OPTIONAL,
  IN UINT32   NumEntries OPTIONAL
  )
{
  return EFI_UNSUPPORTED;
}

/**

  Get OS trusted region resources.

  @param ResourceList  A pointer to resource list to be filled
  @param ResourceSize  On input it means size of resource list input.
                       On output it means size of resource list filled,
                       or the size of resource list to be filled if size of too small.

  @retval EFI_SUCCESS            If resources are returned.
  @retval EFI_BUFFER_TOO_SMALL   If resource list buffer is too small to hold the whole resources.

**/
EFI_STATUS
EFIAPI
GetOsTrustedRegion (
  OUT    STM_RSC *ResourceList,
  IN OUT UINT32  *ResourceSize
  )
{
  return EFI_UNSUPPORTED;
}

EFI_SM_MONITOR_SERVICE_PROTOCOL  mSmMonitorServiceProtocol = {
  Start,
  Stop,
  ProtectOsResource,
  UnprotectOsResource,
  GetOsResource,
  ReturnPiResource,
  InitializeProtection,
  AddOsTrustedRegion,
  DeleteOsTrustedRegion,
  GetOsTrustedRegion,
};

/**
  Relocate this image to reserved memory.

  @param  ImageHandle  Handle of driver image.
  @param  SystemTable  Pointer to system table.

  @retval EFI_SUCCESS  Image successfully relocated.
  @retval EFI_ABORTED  Failed to relocate image.

**/
EFI_STATUS
RelocateImageToReserved (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

/**

  STM service driver entry point function.

  @param ImageHandle   image handle for this driver image
  @param SystemTable   pointer to the EFI System Table

  @retval EFI_SUCCESS

**/
EFI_STATUS
EFIAPI
StmServiceEntrypoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS            Status;
  
  //
  // Load this driver's image to memory
  //
  Status = RelocateImageToReserved (ImageHandle, SystemTable);
  if (EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gEfiSmMonitorServiceProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mSmMonitorServiceProtocol
                  );
  ASSERT_EFI_ERROR (Status);

  mTempResourceBuffer = AllocateReservedPages (MAX_RESOURCE_PAGES);
  ASSERT (mTempResourceBuffer != NULL);
  return Status;
}
