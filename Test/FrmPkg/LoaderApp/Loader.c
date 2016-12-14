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
#include <Library/FrmLib.h>

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
#include <Protocol/TcgService.h>
#include <Protocol/Tcg2Protocol.h>

#include "Loader.h"
#include "FrmCommon.h"

#define IA32_FEATURE_CONTROL_MSR_INDEX      0x3A
#define   IA32_FEATURE_CONTROL_LCK          1u
#define   IA32_FEATURE_CONTROL_SMX          (1u << 1)
#define   IA32_FEATURE_CONTROL_VMX          (1u << 2)


FRM_COMMUNICATION_DATA    mCommunicationData = {
  FRM_COMMUNICATION_DATA_SIGNATURE,
};

EFI_GUID gFrmFileName = FRM_FILE_GUID_NAME;
EFI_GUID gStmServiceFileName = STM_SERVICE_FILE_GUID_NAME;

EFI_MP_SERVICES_PROTOCOL  *mMpService;

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

  This function returns measured image size.

  @param ImageBase  image base address
  @param ImageSize  image base size

  @return measured image size

**/
UINTN
GetMeasuredImageSize(
  IN UINTN  ImageBase,
  IN UINTN  ImageSize
  )
{
  EFI_IMAGE_DOS_HEADER                 *DosHdr;
  UINT32                               PeCoffHeaderOffset;
  EFI_IMAGE_SECTION_HEADER             *Section;
  EFI_IMAGE_OPTIONAL_HEADER_PTR_UNION  Hdr;
  UINTN                                Index;
  UINTN                                MeasuredImageSize;

  //
  // Check PE/COFF image
  //
  DosHdr = (EFI_IMAGE_DOS_HEADER *)(UINTN)ImageBase;
  PeCoffHeaderOffset = 0;
  if (DosHdr->e_magic == EFI_IMAGE_DOS_SIGNATURE) {
    PeCoffHeaderOffset = DosHdr->e_lfanew;
  }

  Hdr.Pe32 = (EFI_IMAGE_NT_HEADERS32 *)((UINT8 *)(UINTN)ImageBase + PeCoffHeaderOffset);
  if (Hdr.Pe32->Signature != EFI_IMAGE_NT_SIGNATURE) {
    ASSERT(FALSE);
    return 0;
  }
  Section = (EFI_IMAGE_SECTION_HEADER *) (
               (UINT8 *) (UINTN) ImageBase +
               PeCoffHeaderOffset +
               sizeof(UINT32) +
               sizeof(EFI_IMAGE_FILE_HEADER) +
               Hdr.Pe32->FileHeader.SizeOfOptionalHeader
               );
  MeasuredImageSize = (UINTN)&Section[Hdr.Pe32->FileHeader.NumberOfSections] - ImageBase;
  DEBUG((EFI_D_INFO, "MeasuredImageSize - 0x%x\n", MeasuredImageSize));
  for (Index = 0; Index < Hdr.Pe32->FileHeader.NumberOfSections; Index++) {
    // Check CODE section or RODATA section
    if (((Section[Index].Characteristics & EFI_IMAGE_SCN_CNT_CODE) != 0) ||
        (((Section[Index].Characteristics & EFI_IMAGE_SCN_CNT_INITIALIZED_DATA) != 0) &&
         ((Section[Index].Characteristics & EFI_IMAGE_SCN_MEM_WRITE) == 0))) {
      MeasuredImageSize = Section[Index].Misc.VirtualSize + Section[Index].VirtualAddress;
      DEBUG((EFI_D_INFO, "MeasuredImageSize - 0x%x\n", MeasuredImageSize));
    } else {
      break;
    }
  }
  return MeasuredImageSize;
}

/**

  This function returns Bios loaded SinitAcm size.

  @return Bios loaded SinitAcm size

**/
UINT32
GetTxtBiosSinitSize (
  VOID
  )
{
  TXT_BIOS_TO_OS_DATA               *BiosToOsData;

  BiosToOsData = GetTxtBiosToOsData ();
  return BiosToOsData->BiosSinitSize;
}

/**

  This function sets SinitAcm size to BiosToOsData.

  @param  BiosSinitSize          Bios loaded SinitAcm size.

**/
VOID
SetTxtBiosSinitSize (
  IN UINT32   BiosSinitSize
  )
{
  TXT_BIOS_TO_OS_DATA               *BiosToOsData;

  BiosToOsData = GetTxtBiosToOsData ();
  BiosToOsData->BiosSinitSize = BiosSinitSize;
  return ;
}

/**

  This function load Dce related module from flash.

  @retval EFI_SUCCESS           load module successfully
  @retval EFI_NOT_FOUND         Sinit not found
  @retval EFI_UNSUPPORTED       not support load SinitAcm

**/
EFI_STATUS
LoadModule (
  VOID
  )
{
  VOID                      *SinitBase;
  UINTN                     SinitSize;

  SinitBase = (VOID *)(UINTN)TxtPubRead32 (TXT_SINIT_BASE);
  SinitSize = (UINTN)TxtPubRead32 (TXT_SINIT_SIZE);

  //ASSERT ((SinitBase != NULL) && (SinitSize != 0));
  if ((SinitBase == NULL) || (SinitSize == 0)) {
    DEBUG ((EFI_D_ERROR, "!!!TXT SINIT not set!\n"));
    return EFI_UNSUPPORTED;
  }

  //
  // Sinit ACM
  //
  if (GetTxtBiosSinitSize () != 0) {
    // Already provided
    return EFI_SUCCESS;
  }

  //
  // BIOS not provide SINIT ACM
  //
  DEBUG ((EFI_D_ERROR, "!!!TXT SINIT ACM not provided by BIOS TXT!\n"));
  
  ASSERT (mCommunicationData.SinitAcmSize <= SinitSize);
  if (mCommunicationData.SinitAcmSize > SinitSize) {
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Fill data
  //
  CopyMem (SinitBase, (VOID *)(UINTN)mCommunicationData.SinitAcmBase, (UINTN)mCommunicationData.SinitAcmSize);

  //
  // Set BiosSinitSize because it will be used later.
  //
  SetTxtBiosSinitSize ((UINT32)mCommunicationData.SinitAcmSize);

  return EFI_SUCCESS;
}

/**

  This function get DPR region according to loaded image size.

  @param LoadedImageSize  size of loaded image
  @param DprImageBase     the image base address in DPR region

  @retval EFI_SUCCESS          DPR region is found to hold image (TXT DPR or PMR region)
  @retval EFI_UNSUPPORTED      TXT DPR is not supported or locked.
  @retval EFI_OUT_OF_RESOURCES No TXT DPR or PMR region

**/
EFI_STATUS
GetDprRegion (
  IN  UINT64   LoadedImageSize,
  OUT UINT64   *DprImageBase
  )
{
  UINT32                               DprBase;
  UINT32                               DprSize;
  UINT32                               ImageBase;
  UINT32                               MleOccupiedSize;
  UINT32                               TxtOccupiedSize;
  UINT32                               SinitBase;
  UINT32                               SinitSize;
  UINT32                               HeapBase;
  UINT32                               HeapSize;
  EFI_STATUS                           Status;
  UINT64                               UnalignmentBase;
  UINT32                               LcpPoSize;
  UINT32                               EventLogSize;

  // Exclude Sinit/TxtHeap on the top
  SinitBase = TxtPubRead32 (TXT_SINIT_BASE);
  SinitSize = TxtPubRead32 (TXT_SINIT_SIZE);
  HeapBase  = (UINT32)(UINTN)GetTxtHeap ();
  HeapSize  = (UINT32)GetTxtHeapSize ();

  if ((TxtPubRead32 (TXT_DPR_REG) & TXT_DPR_REG_SIZE_MASK) == 0) {
    //
    // No DPR??? How to protect SINIT memory/Txt Heap?
    //
    DEBUG ((EFI_D_ERROR, "!!!TXT DPR not set!\n"));
    return EFI_UNSUPPORTED;
  }
  if ((TxtPubRead32 (TXT_DPR_REG) & TXT_DPR_REG_LCK) == 0) {
    DEBUG ((EFI_D_ERROR, "!!!DPR not locked!\n"));
  }

  DprSize = (UINT32)((TxtPubRead32 (TXT_DPR_REG) & TXT_DPR_REG_SIZE_MASK) << TXT_DPR_REG_SIZE_OFFSET);

  DprBase = (UINT32)((TxtPubRead32 (TXT_DPR_REG) & TXT_DPR_REG_BASE_MASK) - DprSize);

  EventLogSize = EFI_PAGES_TO_SIZE(MLE_EVENT_LOG_PAGES);
  MleOccupiedSize = EFI_PAGES_TO_SIZE (MLE_PAGE_TABLE_PAGES) + EFI_PAGES_TO_SIZE (MLE_LOADER_PAGES);
  //
  // Exclude SINIT memory/Txt Heap
  //
  ASSERT ((SinitBase >= DprBase) && (SinitBase + SinitSize <= DprBase + DprSize));
  ASSERT ((HeapBase >= DprBase) && (HeapBase + HeapSize <= DprBase + DprSize));
  if (SinitBase < HeapBase) {
    TxtOccupiedSize = DprBase + DprSize - SinitBase;
  } else {
    TxtOccupiedSize = DprBase + DprSize - HeapBase;
  }

  ASSERT (DprSize >= TxtOccupiedSize);
  if (DprSize < TxtOccupiedSize) {
    DEBUG ((EFI_D_ERROR, "!!!DPR size less than TXT heap and TXT SINIT!\n"));
    return EFI_OUT_OF_RESOURCES;
  }

  LcpPoSize = (UINT32)mCommunicationData.LcpPoSize;

  if (MleOccupiedSize + EventLogSize + LoadedImageSize + LcpPoSize <= DprSize - TxtOccupiedSize) {
    //
    // DPR is enough
    //
    DEBUG ((EFI_D_ERROR, "!!!TXT DPR region is enough, use DPR region!\n"));
    DEBUG ((EFI_D_ERROR, "   Required size  - %08x\n", MleOccupiedSize + EventLogSize + LoadedImageSize + LcpPoSize));
    DEBUG ((EFI_D_ERROR, "   Available size - %08x\n", DprSize - TxtOccupiedSize));

    ImageBase = DprBase + MleOccupiedSize;

    *DprImageBase = (UINT64)ImageBase;

    mCommunicationData.DprBase     = (UINT64)DprBase;
    mCommunicationData.DprSize     = (UINT64)DprSize;
    mCommunicationData.PmrLowBase  = 0;
    mCommunicationData.PmrLowSize  = 0;
    mCommunicationData.PmrHighBase = 0;
    mCommunicationData.PmrHighSize = 0;
    mCommunicationData.EventLogBase = ImageBase + LoadedImageSize;
    mCommunicationData.EventLogSize = EventLogSize;

    //
    // Relocate LcpPoBase
    //
    if (LcpPoSize != 0) {
      CopyMem ((VOID *)(UINTN)(ImageBase + LoadedImageSize + EventLogSize), (VOID *)(UINTN)mCommunicationData.LcpPoBase, (UINTN)LcpPoSize);
      mCommunicationData.LcpPoBase = (UINT64)(ImageBase + LoadedImageSize + EventLogSize);
    }
    return EFI_SUCCESS;
  }

  //
  // DPR is not enough
  //
  DEBUG ((EFI_D_ERROR, "!!!TXT DPR region too small, use PMR region!\n"));

  DEBUG ((EFI_D_ERROR, "   Required size  - %08x\n", MleOccupiedSize + EventLogSize + LoadedImageSize + LcpPoSize));
  DEBUG ((EFI_D_ERROR, "   Available size - %08x\n", DprSize - TxtOccupiedSize));

  //
  // Need PMR: 2M alignment
  //
  UnalignmentBase = 0xFFFFFFFF;
  DprSize = (UINT32)(MleOccupiedSize + EventLogSize + LoadedImageSize + LcpPoSize);
  DprSize = EFI_PAGES_TO_SIZE (EFI_SIZE_TO_PAGES (DprSize));
  DprSize += SIZE_2MB * 2;
  Status = gBS->AllocatePages (
                  AllocateMaxAddress,
                  EfiReservedMemoryType,
                  EFI_SIZE_TO_PAGES (DprSize),
                  &UnalignmentBase
                  );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "!!!No enough space for PMR!\n"));
    return EFI_OUT_OF_RESOURCES;
  }

  DprBase = (UINT32)((UnalignmentBase + SIZE_2MB - 1) & ~(SIZE_2MB - 1));
  DprSize = (UINT32)(MleOccupiedSize + EventLogSize + LoadedImageSize + LcpPoSize);
  DprSize = EFI_PAGES_TO_SIZE (EFI_SIZE_TO_PAGES (DprSize));
  DprSize += SIZE_2MB;
  DprSize &= ~(SIZE_2MB - 1);

  ZeroMem ((VOID *)(UINTN)DprBase, (UINTN)DprSize);
  ImageBase = DprBase + MleOccupiedSize;

  *DprImageBase = (UINT64)ImageBase;

  mCommunicationData.DprBase     = (UINT64)DprBase;
  mCommunicationData.DprSize     = (UINT64)DprSize;
  mCommunicationData.PmrLowBase  = (UINT64)DprBase;
  mCommunicationData.PmrLowSize  = (UINT64)DprSize;
  mCommunicationData.PmrHighBase = 0;
  mCommunicationData.PmrHighSize = 0;
  mCommunicationData.EventLogBase = ImageBase + LoadedImageSize;
  mCommunicationData.EventLogSize = EventLogSize;

  //
  // Relocate LcpPoBase
  //
  if (LcpPoSize != 0) {
    CopyMem ((VOID *)(UINTN)(ImageBase + LoadedImageSize + EventLogSize), (VOID *)(UINTN)mCommunicationData.LcpPoBase, (UINTN)LcpPoSize);
    mCommunicationData.LcpPoBase = (UINT64)(ImageBase + LoadedImageSize + EventLogSize);
  }

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
  if ((AsmReadMsr64(IA32_FEATURE_CONTROL_MSR_INDEX) & (IA32_FEATURE_CONTROL_SMX | IA32_FEATURE_CONTROL_LCK)) ==
      (IA32_FEATURE_CONTROL_SMX | IA32_FEATURE_CONTROL_LCK)) {
    DEBUG((EFI_D_INFO, "Allocate From DPR\n"));
    Status = GetDprRegion(EFI_PAGES_TO_SIZE(PageCount), &DestinationBuffer);
    ASSERT_EFI_ERROR(Status);
  } else {
    DEBUG((EFI_D_INFO, "Allocate From Reserved\n"));
    DestinationBuffer = MaxAddress;
    Status = AllocPages (
                    AllocateMaxAddress,
                    MemType,
                    PageCount,
                    &DestinationBuffer
                    );
    ASSERT_EFI_ERROR (Status);
  }

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
  
  mCommunicationData.MeasuredImageSize = GetMeasuredImageSize((UINTN)*ImageBase, (UINTN)*ImageSize);

  return EFI_SUCCESS;
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
  
  Status = GetImageFromFile (FileName, DeviceHandle, (UINT8 **)&SrcBuffer, &SrcSize);
  ASSERT_EFI_ERROR(Status);

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
  This service lets the caller enable or disable all APs from this point onward.
  This service may only be called from the BSP.

  @param[in] EnableAP          Specifies the new state for the processor for
                               enabled, FALSE for disabled.
**/
VOID
EnableDisableAllAps(
  IN  BOOLEAN                   EnableAP
  )
{
  EFI_STATUS  Status;
  UINTN       Index;
  UINTN       BspIndex;
  UINTN       NumberOfCPUs;
  UINTN       NumberOfEnabledCPUs;

  Status = mMpService->GetNumberOfProcessors(
                         mMpService,
                         &NumberOfCPUs,
                         &NumberOfEnabledCPUs
                         );
  ASSERT_EFI_ERROR(Status);

  Status = mMpService->WhoAmI(
                         mMpService,
                         &BspIndex
                         );
  ASSERT_EFI_ERROR(Status);

  for (Index = 0; Index < NumberOfCPUs; Index++) {
    if (Index == BspIndex) {
      continue;
    }
    Status = mMpService->EnableDisableAP(
                           mMpService,
                           Index,
                           EnableAP,
                           NULL
                           );
    ASSERT_EFI_ERROR(Status);
  }
}

/**

  This function launch FRM.

**/
VOID
LaunchFrm (
  VOID
  )
{
  FRM_ENTRYPOINT            FrmEntryPoint;
  EFI_TPL                   OldTpl;

  //
  // Because FRM will wake up AP, we need notify MP_CPU driver.
  // E.g. if MP_CPU driver puts APs to MONITOR state (not of HLT),
  // the MONITOR_WAKE does not work after FRM returns.
  //
  // So we disable all APs here, then enable all APs after FRM return.
  //
  EnableDisableAllAps(FALSE);

  //
  // RaiseTPL to disable interrupt
  //
  OldTpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

  //
  // Run FRM entrypoint
  //
  FrmEntryPoint = (FRM_ENTRYPOINT)(UINTN)mCommunicationData.ImageEntrypoint;
  FrmEntryPoint (&mCommunicationData);

  gBS->RestoreTPL(OldTpl);

  //
  // Re-enable all APs to notify MP_CPU that AP's state is changed.
  //
  EnableDisableAllAps(TRUE);
  return;
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
  EFI_CPU_ARCH_PROTOCOL     *CpuArch;
  UINTN                     NumberOfCPUs;
  UINTN                     NumberOfEnabledCPUs;
  UINT64                    TimerValue;
  UINT64                    TimerPeriod;
  VOID                      *SmMonitorServiceProtocol;
  EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;
  EFI_TCG2_PROTOCOL                *Tcg2;
  EFI_TCG_PROTOCOL                 *Tcg;
  EFI_TCG2_BOOT_SERVICE_CAPABILITY Tcg2ProtocolCapability;
  TCG_EFI_BOOT_SERVICE_CAPABILITY  TcgProtocolCapability;
  UINT32                           TCGFeatureFlags;
  EFI_PHYSICAL_ADDRESS             EventLogLocation;
  EFI_PHYSICAL_ADDRESS             EventLogLastEntry;

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
  if (Argc < 2) {
    return EFI_INVALID_PARAMETER;
  }
  
  mCommunicationData.TpmType = FRM_TPM_TYPE_NONE;
  Status = gBS->LocateProtocol(&gEfiTcg2ProtocolGuid, NULL, (VOID **)&Tcg2);
  if (!EFI_ERROR(Status)) {
    Tcg2ProtocolCapability.Size = sizeof(Tcg2ProtocolCapability);
    Status = Tcg2->GetCapability(Tcg2, &Tcg2ProtocolCapability);
    if ((!EFI_ERROR(Status)) && Tcg2ProtocolCapability.TPMPresentFlag) {
      mCommunicationData.TpmType = FRM_TPM_TYPE_TPM2;
      mCommunicationData.ActivePcrBanks = Tcg2ProtocolCapability.ActivePcrBanks;
      DEBUG((EFI_D_INFO, "TPM2.0\n"));
    }
  } else {
    Status = gBS->LocateProtocol(&gEfiTcgProtocolGuid, NULL, (VOID **)&Tcg);
    if (!EFI_ERROR(Status)) {
      TcgProtocolCapability.Size = sizeof(TcgProtocolCapability);
      Status = Tcg->StatusCheck(Tcg, &TcgProtocolCapability, &TCGFeatureFlags, &EventLogLocation, &EventLogLastEntry);
      if ((!EFI_ERROR(Status)) && TcgProtocolCapability.TPMPresentFlag && (!TcgProtocolCapability.TPMDeactivatedFlag)) {
        mCommunicationData.TpmType = FRM_TPM_TYPE_TPM12;
        DEBUG((EFI_D_INFO, "TPM1.2\n"));
      }
    }
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
  Status = gBS->LocateProtocol (&gEfiMpServiceProtocolGuid, NULL, (VOID **)&mMpService);
  ASSERT_EFI_ERROR (Status);
  Status = mMpService->GetNumberOfProcessors (
                         mMpService,
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
#if 0
  Address = 0xFFFFF;
  Status = gBS->AllocatePages (
                  AllocateMaxAddress,
                  EfiReservedMemoryType,
                  EFI_SIZE_TO_PAGES ((UINTN)mCommunicationData.LowMemorySize),
                  &Address
                  );
  ASSERT_EFI_ERROR (Status);
  ZeroMem ((VOID *)(UINTN)Address, EFI_PAGES_TO_SIZE (EFI_SIZE_TO_PAGES ((UINTN)mCommunicationData.LowMemorySize)));
#else
  Address = 0x8000;
#endif
  mCommunicationData.LowMemoryBase = Address;

  {
    VOID                              *SrcBuffer;
    UINTN                             SrcSize;

    if (Argc >= 3) {
      Status = GetImageFromFile(Argv[2], LoadedImage->DeviceHandle, (UINT8 **)&SrcBuffer, &SrcSize);
      ASSERT_EFI_ERROR (Status);
      if (!EFI_ERROR(Status)) {
        mCommunicationData.SinitAcmBase = (UINT64)(UINTN)SrcBuffer;
        mCommunicationData.SinitAcmSize = (UINT64)(UINTN)SrcSize;
      }

      if (Argc >= 4) {
        Status = GetImageFromFile(Argv[3], LoadedImage->DeviceHandle, (UINT8 **)&SrcBuffer, &SrcSize);
        ASSERT_EFI_ERROR (Status);
        if (!EFI_ERROR(Status)) {
          mCommunicationData.LcpPoBase = (UINT64)(UINTN)SrcBuffer;
          mCommunicationData.LcpPoSize = (UINT64)(UINTN)SrcSize;
        }
      }
      Status = LoadModule ();
      ASSERT_EFI_ERROR (Status);
    }
  }

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
  if (!EFI_ERROR (Status)) {
    mCommunicationData.SmMonitorServiceProtocol = (EFI_PHYSICAL_ADDRESS)(UINTN)SmMonitorServiceProtocol;
    Status = GetLoadedImageBaseAndSize (
               mCommunicationData.SmMonitorServiceProtocol,
               &mCommunicationData.SmMonitorServiceImageBase,
               &mCommunicationData.SmMonitorServiceImageSize
               );
    ASSERT_EFI_ERROR (Status);
  } else {
    mCommunicationData.SmMonitorServiceProtocol = 0;
    mCommunicationData.SmMonitorServiceImageBase = 0;
    mCommunicationData.SmMonitorServiceImageSize = 0;
  }

  LaunchFrm ();
  return Status;
}
