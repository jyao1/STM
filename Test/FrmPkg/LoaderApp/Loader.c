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
#include <Uefi.h>
#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeCoffLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>

#include <Protocol/LoadedImage.h>
#include <Protocol/MpService.h>
#include <Protocol/Cpu.h>
#include <Protocol/FirmwareVolume2.h>
#include <Guid/FileInfo.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/GlobalVariable.h>
#include <Guid/Acpi.h>
#include <IndustryStandard/Acpi.h>
#include <Protocol/SmMonitorService.h>

#include "Loader.h"
#include "FrmCommon.h"

FRM_COMMUNICATION_DATA    mCommunicationData = {
  FRM_COMMUNICATION_DATA_SIGNATURE,
};

EFI_GUID gFrmFileName = FRM_FILE_GUID_NAME;
EFI_GUID gStmServiceFileName = STM_SERVICE_FILE_GUID_NAME;

UINTN Argc = 0;
CHAR16 *Argv[10];
CHAR16 *ArgBuffer;

/**

  This function parse application ARG.

  @param Data     Arg string.
  @param DataSize ARG string size.

**/
VOID
GetArg (
  IN UINT8 *Data,
  IN UINTN DataSize
  )
{
  CHAR16 *Index;
  CHAR16 *End;
  CHAR16 *DIndex;
  EFI_STATUS Status;

  End = (CHAR16*)(UINTN)(Data + DataSize);
  Status = gBS->AllocatePool (EfiBootServicesData, DataSize, (VOID **)&ArgBuffer);
  ASSERT_EFI_ERROR (Status);
  DIndex = ArgBuffer;
  Argv[Argc++] = ArgBuffer;
  for (Index = (CHAR16*)Data; Index < End; ) {
    if (*Index == L' ') {
      *DIndex = L'\0';
      Argv[Argc++] = (++ DIndex);
      while(*Index == L' ') {
        Index ++;
      }
    }
    (*(DIndex ++)) = (*(Index ++));
  }
}

/**

  This function open root handler of FS.

  @param DeviceHandle device handle of FS.

  @return FS root file handle.

**/
EFI_FILE_HANDLE
LibOpenRoot (
  IN EFI_HANDLE                   DeviceHandle
  )
{
  EFI_STATUS                      Status;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Volume;
  EFI_FILE_HANDLE                 File;

  File = NULL;

  //
  // Handle the file system interface to the device
  //
  Status = gBS->HandleProtocol (
                DeviceHandle,
                &gEfiSimpleFileSystemProtocolGuid,
                (VOID *) &Volume
                );

  //
  // Open the root directory of the volume
  //
  if (!EFI_ERROR (Status)) {
    Status = Volume->OpenVolume (
                      Volume,
                      &File
                      );
  }
  //
  // Done
  //
  return EFI_ERROR (Status) ? NULL : File;
}

/**

  This function returns image buffer from file.

  @param FileName     file name.
  @param DeviceHandle device handle of FS.
  @param ImageBuffer  image buffer.
  @param FileSize     image file size.

  @retval EFI_SUCCESS image buffer is read successfully.
  @retval EFI_ERROR   image buffer is not read.

**/
EFI_STATUS
GetImageFromFile (
  IN    CHAR16     *FileName,
  IN    EFI_HANDLE DeviceHandle,
  OUT   UINT8      **ImageBuffer,
  OUT   UINTN      *FileSize
  )
{
  EFI_FILE_HANDLE       Handle;
  UINT8                 *Buffer;
  UINTN                 ImageSize;
  EFI_STATUS            Status;
  EFI_FILE_INFO         *Info;
  UINTN                 BufferSize;

  EFI_FILE_HANDLE       RootHandle;

  RootHandle = LibOpenRoot (DeviceHandle);

  Status = RootHandle->Open (RootHandle, &Handle, FileName, EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  RootHandle->Close (RootHandle);

  BufferSize = SIZE_OF_EFI_FILE_INFO + 1024;
  Status = gBS->AllocatePool (EfiBootServicesData,BufferSize, (VOID **)&Info);
  ASSERT_EFI_ERROR (Status);
  Status = Handle->GetInfo (
                     Handle, 
                     &gEfiFileInfoGuid, 
                     &BufferSize, 
                     Info
                     );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  *FileSize = (UINTN)Info->FileSize;
  ImageSize = *FileSize;
  Status = gBS->AllocatePool (EfiBootServicesData,ImageSize, (VOID **)&Buffer);
  ASSERT_EFI_ERROR (Status);
  Status = Handle->Read (
                     Handle,
                     &ImageSize,
                     Buffer
                     );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Handle->Close (Handle);
  *ImageBuffer = Buffer;

  return EFI_SUCCESS;
}

/**

  This function load image.

  @param SrcBuffer  image buffer
  @param SrcSize    image buffer size
  @param AllocPages AllocatePages function pointer
  @param FreePages  FreePages function pointer
  @param MemType    memory type for AllocatePages function
  @param MaxAddress MaxAddress in AllocatePages
  @param ImageBase  Loaded image base
  @param ImageSize  Loaded image size
  @param Entrypoint Loaded image entrypoint

  @retval EFI_SUCCESS load image success
**/
EFI_STATUS
LoadImage (
  IN      VOID                      *SrcBuffer,
  IN      UINTN                     SrcSize,
  IN      EFI_ALLOCATE_PAGES        AllocPages,
  IN      EFI_FREE_PAGES            FreePages,
  IN      EFI_MEMORY_TYPE           MemType,
  IN      EFI_PHYSICAL_ADDRESS      MaxAddress,
  OUT     EFI_PHYSICAL_ADDRESS      *ImageBase,
  OUT     UINT64                    *ImageSize,
  OUT     EFI_PHYSICAL_ADDRESS      *Entrypoint
  )
{
  PE_COFF_LOADER_IMAGE_CONTEXT  ImageContext;
  EFI_PHYSICAL_ADDRESS          DestinationBuffer;
  EFI_STATUS                    Status;
  UINTN                         PageCount;

  ImageContext.Handle    = SrcBuffer;
  ImageContext.ImageRead = PeCoffLoaderImageReadFromMemory;

  //
  // Get information about the image being loaded
  //
  Status = PeCoffLoaderGetImageInfo (&ImageContext);
  ASSERT_EFI_ERROR (Status);

  //
  // Allocate memory for the image being loaded from the DPR
  //
  PageCount = (UINTN)EFI_SIZE_TO_PAGES(ImageContext.ImageSize + ImageContext.SectionAlignment);
  DestinationBuffer = MaxAddress;
  Status = AllocPages (
                  AllocateMaxAddress,
                  MemType,
                  PageCount,
                  &DestinationBuffer
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Align buffer on section boundry
  //
  ImageContext.ImageAddress = DestinationBuffer;
  ImageContext.ImageAddress += ImageContext.SectionAlignment - 1;
  ImageContext.ImageAddress &= ~(ImageContext.SectionAlignment - 1);

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
  // Fill information
  //
  *Entrypoint = (UINT64)(UINTN)ImageContext.EntryPoint;
  *ImageBase = ImageContext.ImageAddress;
  *ImageSize = (UINT64)ImageContext.ImageSize;

  return EFI_SUCCESS;
}

/**

  Read a typed section from an FFS file indicated by NameGuid.

  @param NameGuid     File name
  @param SectionType  Section type
  @param Buffer       Data read
  @param Size         Size of the buffer
  @param ImageHandle  Image handle

**/
VOID
GetFvImage (
  IN      EFI_GUID                  *NameGuid,
  IN      EFI_SECTION_TYPE          SectionType,
  IN OUT  VOID                      **Buffer,
  IN OUT  UINTN                     *Size,
  IN      EFI_HANDLE                ImageHandle
  )
{
  EFI_STATUS                    Status;
  EFI_FIRMWARE_VOLUME2_PROTOCOL *Fv;
  UINT32                        AuthenticationStatus;
  EFI_LOADED_IMAGE_PROTOCOL     *LoadedImageInfo;

  *Buffer = NULL;
  *Size = 0;
    
  Status = gBS->HandleProtocol (
                  ImageHandle,
                  &gEfiLoadedImageProtocolGuid,
                  (VOID **)&LoadedImageInfo
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Find desired image in Fv where SmmBase is 
  //
  Status = gBS->HandleProtocol (
                  LoadedImageInfo->DeviceHandle,
                  &gEfiFirmwareVolume2ProtocolGuid,
                  (VOID **)&Fv
                  );
  ASSERT_EFI_ERROR (Status);

  Status = Fv->ReadSection (
                 Fv,
                 NameGuid,
                 SectionType,
                 0,
                 Buffer,
                 Size,
                 &AuthenticationStatus
                 );
  ASSERT_EFI_ERROR (Status);
}

/**

  This function load image from file.

  @param FileName     File name
  @param DeviceHandle image device handle
  @param ParentHandle image parent handle
  @param AllocPages   AllocatePages function pointer
  @param FreePages    FreePages function pointer
  @param MemType      memory type for AllocatePages function
  @param MaxAddress   MaxAddress in AllocatePages
  @param ImageBase    Loaded image base
  @param ImageSize    Loaded image size
  @param Entrypoint   Loaded image entrypoint

  @retval EFI_SUCCESS load image success
**/
EFI_STATUS
LoadImageFromFile (
  IN      CHAR16                    *FileName,
  IN      EFI_HANDLE                DeviceHandle,
  IN      EFI_HANDLE                ParentHandle,
  IN      EFI_ALLOCATE_PAGES        AllocPages,
  IN      EFI_FREE_PAGES            FreePages,
  IN      EFI_MEMORY_TYPE           MemType,
  IN      EFI_PHYSICAL_ADDRESS      MaxAddress,
  OUT     EFI_PHYSICAL_ADDRESS      *ImageBase,
  OUT     UINT64                    *ImageSize,
  OUT     EFI_PHYSICAL_ADDRESS      *Entrypoint
  )
{
  EFI_STATUS                        Status;
  VOID                              *SrcBuffer;
  UINTN                             SrcSize;

//  Status = GetImageEx (ParentHandle, FileName, EFI_SECTION_PE32, &SrcBuffer, &SrcSize, TRUE);
//  ASSERT_EFI_ERROR (Status);
  
  GetImageFromFile (FileName, DeviceHandle, (UINT8 **)&SrcBuffer, &SrcSize);

  Status = LoadImage (
             SrcBuffer,
             SrcSize,
             AllocPages,
             FreePages,
             MemType,
             MaxAddress,
             ImageBase,
             ImageSize,
             Entrypoint
             );
  ASSERT_EFI_ERROR (Status);

  gBS->FreePool (SrcBuffer);
  return Status;
}

/**
  Get RSDP ACPI table by Guid.

  @param AcpiTableGuid  ACPI table GUID.

  @return RSDP pointer.
**/
VOID *
GetRsdpByGuid (
  IN EFI_GUID  *AcpiTableGuid
  )
{
  UINTN   Index;

  for (Index = 0; Index < gST->NumberOfTableEntries; Index++) {
    if (CompareGuid (&(gST->ConfigurationTable[Index].VendorGuid), AcpiTableGuid)) {
      return gST->ConfigurationTable[Index].VendorTable;
    }
  }

  return NULL;
}

/**
  Get RSDP ACPI table.

  @return RSDP pointer.
**/
VOID *
GetRsdp (
  VOID
  )
{
  VOID    *Rsdp;

  Rsdp = GetRsdpByGuid (&gEfiAcpi20TableGuid);
  if (Rsdp != NULL) {
    return Rsdp;
  }
  return GetRsdpByGuid (&gEfiAcpi10TableGuid);
}

/**

  This function return image base and size which include a specific address.

  @param Address   the address inside the image.
  @param ImageBase image base
  @param ImageSize image size

  @retval EFI_SUCCESS the image base and size is found.

**/
EFI_STATUS
GetLoadedImageBaseAndSize (
  IN EFI_PHYSICAL_ADDRESS  Address,
  OUT EFI_PHYSICAL_ADDRESS *ImageBase,
  OUT UINT64               *ImageSize
  )
{
  EFI_LOADED_IMAGE_PROTOCOL  *LoadedImage;
  EFI_STATUS                 Status;

  DEBUG ((EFI_D_INFO, "Stm Service Entrypoint - 0x%x\n", Address));
  
  Status = gBS->LocateProtocol (
                  &gStmServiceFileName,
                  NULL,
                  (VOID **)&LoadedImage
                  );
  ASSERT_EFI_ERROR (Status);
    
  DEBUG ((EFI_D_INFO, "LoadedImage - [0x%x - 0x%x]\n", (UINTN)LoadedImage->ImageBase, (UINTN)LoadedImage->ImageBase + LoadedImage->ImageSize));

  ASSERT ((EFI_PHYSICAL_ADDRESS)(UINTN)LoadedImage->ImageBase <= Address);
  ASSERT (Address < ((EFI_PHYSICAL_ADDRESS)(UINTN)LoadedImage->ImageBase + LoadedImage->ImageSize));
  *ImageBase = (EFI_PHYSICAL_ADDRESS)(UINTN)LoadedImage->ImageBase;
  *ImageSize = LoadedImage->ImageSize;
  return EFI_SUCCESS;
}

/**

  FRM loader entry point function.

  @param ImageHandle   image handle for this driver image
  @param SystemTable   pointer to the EFI System Table

  @retval EFI_SUCCESS

**/
EFI_STATUS
EFIAPI
LoaderEntrypoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                Status;
  EFI_PHYSICAL_ADDRESS      Address;
  EFI_MP_SERVICES_PROTOCOL  *MpService;
  EFI_CPU_ARCH_PROTOCOL     *CpuArch;
  UINTN                     NumberOfCPUs;
  UINTN                     NumberOfEnabledCPUs;
  UINT64                    TimerValue;
  UINT64                    TimerPeriod;
  VOID                      *SmMonitorServiceProtocol;
  EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;
  FRM_ENTRYPOINT            Entry;

  Status = gBS->HandleProtocol (
                  ImageHandle,
                  &gEfiLoadedImageProtocolGuid,
                  (VOID**)&LoadedImage
                  );
  if (EFI_ERROR(Status)) {
    return Status;                 
  }
  
  Print(L"LoaderEntrypoint: LoadOptions - %s\n", LoadedImage->LoadOptions);
  GetArg(LoadedImage->LoadOptions, LoadedImage->LoadOptionsSize);
  if (Argc <= 2) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Get ACPI RSDP
  //
  mCommunicationData.AcpiRsdp = (UINT64)(UINTN)GetRsdp ();

  //
  // Allocate EBDA for ACPI pointer
  //
  Address = 0x9FFFF;
  Status = gBS->AllocatePages (
                  AllocateMaxAddress,
                  EfiReservedMemoryType,
                  EFI_SIZE_TO_PAGES ((UINTN)EBDA_MEMORY_SIZE),
                  &Address
                  );
  ASSERT_EFI_ERROR (Status);
  ZeroMem ((VOID *)(UINTN)Address, EFI_PAGES_TO_SIZE (EFI_SIZE_TO_PAGES ((UINTN)EBDA_MEMORY_SIZE)));
  CopyMem ((VOID *)(UINTN)Address, (VOID *)(UINTN)mCommunicationData.AcpiRsdp, sizeof(EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER));
  (*(UINT16 *)(UINTN)(EBDA_BASE_ADDRESS)) = (UINT16)(Address >> 4);

  //
  // Get CPU Number
  //
  Status = gBS->LocateProtocol (&gEfiMpServiceProtocolGuid, NULL, (VOID **)&MpService);
  ASSERT_EFI_ERROR (Status);
  Status = MpService->GetNumberOfProcessors (
                        MpService,
                        &NumberOfCPUs,
                        &NumberOfEnabledCPUs
                        );
  ASSERT_EFI_ERROR (Status);

  //
  // Get CPU Timer
  //
  Status = gBS->LocateProtocol (&gEfiCpuArchProtocolGuid, NULL, (VOID **)&CpuArch);
  ASSERT_EFI_ERROR (Status);
  Status = CpuArch->GetTimerValue (CpuArch, 0, &TimerValue, &TimerPeriod);
  ASSERT_EFI_ERROR (Status);
  mCommunicationData.TimerPeriod = TimerPeriod;

  //
  // Allocate High Memory
  //
  mCommunicationData.HighMemorySize = HIGH_MEMORY_SIZE_COMMON + HIGH_MEMORY_SIZE_PER_CPU * NumberOfCPUs;
  Address = 0xFFFFFFFF;
  Status = gBS->AllocatePages (
                  AllocateMaxAddress,
                  EfiReservedMemoryType,
                  EFI_SIZE_TO_PAGES ((UINTN)mCommunicationData.HighMemorySize),
                  &Address
                  );
  ASSERT_EFI_ERROR (Status);
  mCommunicationData.HighMemoryBase = Address;
  ZeroMem ((VOID *)(UINTN)Address, EFI_PAGES_TO_SIZE (EFI_SIZE_TO_PAGES ((UINTN)mCommunicationData.HighMemorySize)));
  //
  // Allocate Low Memory
  //
  mCommunicationData.LowMemorySize = LOW_MEMORY_SIZE;
  Address = 0xFFFFF;
  Status = gBS->AllocatePages (
                  AllocateMaxAddress,
                  EfiReservedMemoryType,
                  EFI_SIZE_TO_PAGES ((UINTN)mCommunicationData.LowMemorySize),
                  &Address
                  );
  ASSERT_EFI_ERROR (Status);
  mCommunicationData.LowMemoryBase = Address;
  ZeroMem ((VOID *)(UINTN)Address, EFI_PAGES_TO_SIZE (EFI_SIZE_TO_PAGES ((UINTN)mCommunicationData.LowMemorySize)));

  //
  // Get binary
  //
  Status = LoadImageFromFile (
             Argv[1],
             LoadedImage->DeviceHandle,
             ImageHandle,
             gBS->AllocatePages,
             gBS->FreePages,
             EfiReservedMemoryType,
             0xFFFFFFFF,
             &mCommunicationData.ImageBase,
             &mCommunicationData.ImageSize,
             &mCommunicationData.ImageEntrypoint
             );
  ASSERT_EFI_ERROR (Status);
  Print(L"CommunicationData: Entrypoint - %x, ImageBase - %x, ImageSize - %x\n", mCommunicationData.ImageEntrypoint, mCommunicationData.ImageBase, mCommunicationData.ImageSize);

  Status = gBS->LocateProtocol (
                  &gEfiSmMonitorServiceProtocolGuid,
                  NULL,
                  &SmMonitorServiceProtocol
                  );
  ASSERT_EFI_ERROR (Status);

  mCommunicationData.SmMonitorServiceProtocol = (EFI_PHYSICAL_ADDRESS)(UINTN)SmMonitorServiceProtocol;

  Status = GetLoadedImageBaseAndSize (
             mCommunicationData.SmMonitorServiceProtocol,
             &mCommunicationData.SmMonitorServiceImageBase,
             &mCommunicationData.SmMonitorServiceImageSize
             );
  ASSERT_EFI_ERROR (Status);

  Entry = (FRM_ENTRYPOINT)(UINTN)mCommunicationData.ImageEntrypoint;
  Entry(&mCommunicationData);
  return Status;
}
