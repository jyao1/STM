/** @file

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/PeCoffLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/UefiLib.h>
#include <Protocol/LoadedImage.h>

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
  )
{
  EFI_LOADED_IMAGE_PROTOCOL                     *LoadedImage;
  EFI_STATUS                                    Status;
  UINT8                                         *Buffer;
  UINTN                                         BufferSize;
  EFI_HANDLE                                    NewImageHandle;
  UINTN                                         Pages;
  EFI_PHYSICAL_ADDRESS                          FfsBuffer;
  PE_COFF_LOADER_IMAGE_CONTEXT                  ImageContext;
  VOID                                          *Interface;

  //
  // If locate gEfiCallerIdGuid success, it means 2nd entry.
  //
  Status = gBS->LocateProtocol (&gEfiCallerIdGuid, NULL, &Interface);
  if (!EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "StmService - 2nd entry\n"));
    return EFI_SUCCESS;
  }

  DEBUG ((EFI_D_INFO, "StmService - 1st entry\n"));

  //
  // Reload image itself to <4G mem
  //
  Status = GetSectionFromAnyFv  (
             &gEfiCallerIdGuid,
             EFI_SECTION_PE32,
             0,
             (VOID **) &Buffer,
             &BufferSize
             );
  ASSERT_EFI_ERROR (Status);
  ImageContext.Handle    = Buffer;
  ImageContext.ImageRead = PeCoffLoaderImageReadFromMemory;
  //
  // Get information about the image being loaded
  //
  Status = PeCoffLoaderGetImageInfo (&ImageContext);
  ASSERT_EFI_ERROR (Status);
  if (ImageContext.SectionAlignment > EFI_PAGE_SIZE) {
    Pages = EFI_SIZE_TO_PAGES ((UINTN) (ImageContext.ImageSize + ImageContext.SectionAlignment));
  } else {
    Pages = EFI_SIZE_TO_PAGES ((UINTN) ImageContext.ImageSize);
  }
  FfsBuffer = 0xFFFFFFFF;
  Status = gBS->AllocatePages (
                  AllocateMaxAddress,
                  EfiReservedMemoryType,
                  Pages,
                  &FfsBuffer
                  );
  ASSERT_EFI_ERROR (Status);
  ImageContext.ImageAddress = (PHYSICAL_ADDRESS)(UINTN)FfsBuffer;
  //
  // Align buffer on section boundry
  //
  ImageContext.ImageAddress += ImageContext.SectionAlignment - 1;
  ImageContext.ImageAddress &= ~((EFI_PHYSICAL_ADDRESS)(ImageContext.SectionAlignment - 1));
  //
  // Load the image to our new buffer
  //
  Status = PeCoffLoaderLoadImage (&ImageContext);
  ASSERT_EFI_ERROR (Status);

  //
  // Relocate the image in our new buffer
  //
  Status = PeCoffLoaderRelocateImage (&ImageContext);
  ASSERT_EFI_ERROR (Status);

  //
  // Free the buffer allocated by ReadSection since the image has been relocated in the new buffer
  //
  gBS->FreePool (Buffer);

  //
  // Flush the instruction cache so the image data is written before we execute it
  //
  InvalidateInstructionCacheRange ((VOID *)(UINTN)ImageContext.ImageAddress, (UINTN)ImageContext.ImageSize);
    
  //
  // install loaded image
  //
  LoadedImage = AllocateZeroPool (sizeof(EFI_LOADED_IMAGE_PROTOCOL));
  ASSERT (LoadedImage != NULL);

  LoadedImage->Revision = EFI_LOADED_IMAGE_PROTOCOL_REVISION;
  LoadedImage->ParentHandle = NULL;
  LoadedImage->SystemTable = gST;
  LoadedImage->DeviceHandle = NULL;
  LoadedImage->LoadOptionsSize = 0;
  LoadedImage->LoadOptions = NULL;
  LoadedImage->ImageBase = (VOID *)(UINTN)ImageContext.ImageAddress;
  LoadedImage->ImageSize = ImageContext.ImageSize;
  LoadedImage->ImageCodeType = EfiReservedMemoryType;
  LoadedImage->ImageDataType = EfiReservedMemoryType;
  LoadedImage->Unload = NULL;

  NewImageHandle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &NewImageHandle,
                  &gEfiCallerIdGuid, LoadedImage,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);
  DEBUG ((EFI_D_INFO, "NewImageHandle - 0x%x\n", NewImageHandle));

  //
  // Call entrypoint
  //
  DEBUG ((EFI_D_INFO, "Loading driver at 0x%08x EntryPoint=0x%08x\n", (UINTN)ImageContext.ImageAddress, (UINTN)ImageContext.EntryPoint));
  Status = ((EFI_IMAGE_ENTRY_POINT)(UINTN)(ImageContext.EntryPoint)) (NewImageHandle, gST);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Error: Image at 0x%08x start failed: %r\n", ImageContext.ImageAddress, Status));
    gBS->FreePages (FfsBuffer, Pages);
  }

  //
  // return error to unload DXE copy, if we already relocate itself to reserved memory.
  //
  return EFI_ACCESS_DENIED;
}
