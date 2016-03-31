/** @file
  STM relocation

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmInit.h"
#include <IndustryStandard/PeImage.h>

/**

  This function relocate image at ImageBase.

  @param ImageBase   Image base
  @param PeImageBase Image base field in PE/COFF header
  @param IsTeardown  If the relocation is for teardown.
                     FALSE means relocation for setup.
                     TRUE  means relocation for teardown.

**/
VOID
PeCoffRelocateImageOnTheSpot (
  IN  UINTN        ImageBase,
  IN  UINTN        PeImageBase,
  IN  BOOLEAN      IsTeardown
  )
{
  EFI_IMAGE_DOS_HEADER                *DosHdr;
  EFI_IMAGE_OPTIONAL_HEADER_PTR_UNION Hdr;
  UINT32                              NumberOfRvaAndSizes;
  EFI_IMAGE_DATA_DIRECTORY            *DataDirectory;
  EFI_IMAGE_DATA_DIRECTORY            *RelocDir;
  EFI_IMAGE_BASE_RELOCATION           *RelocBase;
  EFI_IMAGE_BASE_RELOCATION           *RelocBaseEnd;
  UINT16                              *Reloc;
  UINT16                              *RelocEnd;
  CHAR8                               *Fixup;
  CHAR8                               *FixupBase;
  UINT16                              *Fixup16;
  UINT32                              *Fixup32;
  UINT64                              *Fixup64;
  UINTN                               Adjust;
  UINT16                              Magic;

  if (!IsTeardown) {
    Adjust = ImageBase - PeImageBase;
  } else {
    Adjust = PeImageBase - ImageBase;
  }

  //
  // Find the image's relocate dir info
  //
  DosHdr = (EFI_IMAGE_DOS_HEADER *)ImageBase;
  if (DosHdr->e_magic == EFI_IMAGE_DOS_SIGNATURE) {
    //
    // Valid DOS header so get address of PE header
    //
    Hdr.Pe32 = (EFI_IMAGE_NT_HEADERS32 *)(((CHAR8 *)DosHdr) + DosHdr->e_lfanew);
  } else {
    //
    // No Dos header so assume image starts with PE header.
    //
    Hdr.Pe32 = (EFI_IMAGE_NT_HEADERS32 *)ImageBase;
  }

  if (Hdr.Pe32->Signature != EFI_IMAGE_NT_SIGNATURE) {
    //
    // Not a valid PE image so Exit
    //
    return ;
  }

  Magic = Hdr.Pe32->OptionalHeader.Magic;

  if (Magic == EFI_IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
    //
    // Use PE32 offset
    //
    NumberOfRvaAndSizes = Hdr.Pe32->OptionalHeader.NumberOfRvaAndSizes;
    DataDirectory = (EFI_IMAGE_DATA_DIRECTORY *)&(Hdr.Pe32->OptionalHeader.DataDirectory[0]);
  } else {
    //
    // Use PE32+ offset
    //
    NumberOfRvaAndSizes = Hdr.Pe32Plus->OptionalHeader.NumberOfRvaAndSizes;
    DataDirectory = (EFI_IMAGE_DATA_DIRECTORY *)&(Hdr.Pe32Plus->OptionalHeader.DataDirectory[0]);
  }

  //
  // Find the relocation block
  //
  // Per the PE/COFF spec, you can't assume that a given data directory
  // is present in the image. You have to check the NumberOfRvaAndSizes in
  // the optional header to verify a desired directory entry is there.
  //
  if (NumberOfRvaAndSizes > EFI_IMAGE_DIRECTORY_ENTRY_BASERELOC) {
    RelocDir      = DataDirectory + EFI_IMAGE_DIRECTORY_ENTRY_BASERELOC;
    RelocBase     = (EFI_IMAGE_BASE_RELOCATION *)(UINTN)(ImageBase + RelocDir->VirtualAddress);
    RelocBaseEnd  = (EFI_IMAGE_BASE_RELOCATION *)(UINTN)(ImageBase + RelocDir->VirtualAddress + RelocDir->Size);
  } else {
    //
    // Cannot find relocations, good just return.
    //
    return ;
  }
  
  //
  // ASSERT for the invalid image when RelocBase and RelocBaseEnd are both NULL.
  //
  ASSERT (RelocBase != NULL && RelocBaseEnd != NULL);

  //
  // Run the whole relocation block. And re-fixup data that has not been
  // modified. The FixupData is used to see if the image has been modified
  // since it was relocated. This is so data sections that have been updated
  // by code will not be fixed up, since that would set them back to
  // defaults.
  //
  while (RelocBase < RelocBaseEnd) {

    Reloc     = (UINT16 *) ((UINT8 *) RelocBase + sizeof (EFI_IMAGE_BASE_RELOCATION));
    RelocEnd  = (UINT16 *) ((UINT8 *) RelocBase + RelocBase->SizeOfBlock);
    FixupBase = (CHAR8 *) ((UINTN)ImageBase) + RelocBase->VirtualAddress;

    //
    // Run this relocation record
    //
    while (Reloc < RelocEnd) {

      Fixup = FixupBase + (*Reloc & 0xFFF);
      switch ((*Reloc) >> 12) {

      case EFI_IMAGE_REL_BASED_ABSOLUTE:
        break;

      case EFI_IMAGE_REL_BASED_HIGH:
        Fixup16  = (UINT16 *) Fixup;
        *Fixup16 = (UINT16) (*Fixup16 + ((UINT16) ((UINT32) Adjust >> 16)));
        break;

      case EFI_IMAGE_REL_BASED_LOW:
        Fixup16  = (UINT16 *) Fixup;
        *Fixup16 = (UINT16) (*Fixup16 + ((UINT16) Adjust & 0xffff));
        break;

      case EFI_IMAGE_REL_BASED_HIGHLOW:
        Fixup32  = (UINT32 *) Fixup;
        *Fixup32 = *Fixup32 + (UINT32) Adjust;
        break;

      case EFI_IMAGE_REL_BASED_DIR64:
        Fixup64  = (UINT64 *) Fixup;
        *Fixup64 = *Fixup64 + (UINT64)Adjust;
        break;

      case EFI_IMAGE_REL_BASED_HIGHADJ:
        //
        // Not valid Relocation type for UEFI image, ASSERT
        //
        ASSERT (FALSE);
        break;

      default:
        //
        // Only Itanium requires ConvertPeImage_Ex
        //
        ASSERT (FALSE);
        break;
      }
      //
      // Next relocation record
      //
      Reloc += 1;
    }
    //
    // next reloc block
    //
    RelocBase = (EFI_IMAGE_BASE_RELOCATION *) RelocEnd;
  }
}

/**

  This function relocate this STM image.

  @param IsTeardown  If the relocation is for teardown.
                     FALSE means relocation for setup.
                     TRUE  means relocation for teardown.

**/
VOID
RelocateStmImage (
  IN BOOLEAN   IsTeardown
  )
{
  UINTN                               StmImage;
  UINTN                               ImageBase;
  UINTN                               PeImageBase;
  EFI_IMAGE_DOS_HEADER                *DosHdr;
  EFI_IMAGE_OPTIONAL_HEADER_PTR_UNION Hdr;
  UINT16                              Magic;

  StmImage = (UINTN)((UINT32)AsmReadMsr64(IA32_SMM_MONITOR_CTL_MSR_INDEX) & 0xFFFFF000);

  ImageBase = StmImage + STM_CODE_OFFSET;
  
  //
  // Find the image's relocate dir info
  //
  DosHdr = (EFI_IMAGE_DOS_HEADER *)ImageBase;
  if (DosHdr->e_magic == EFI_IMAGE_DOS_SIGNATURE) {
    //
    // Valid DOS header so get address of PE header
    //
    Hdr.Pe32 = (EFI_IMAGE_NT_HEADERS32 *)(((CHAR8 *)DosHdr) + DosHdr->e_lfanew);
  } else {
    //
    // No Dos header so assume image starts with PE header.
    //
    Hdr.Pe32 = (EFI_IMAGE_NT_HEADERS32 *)ImageBase;
  }

  if (Hdr.Pe32->Signature != EFI_IMAGE_NT_SIGNATURE) {
    //
    // Not a valid PE image so Exit
    //
    return ;
  }

  Magic = Hdr.Pe32->OptionalHeader.Magic;

  if (Magic == EFI_IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
    //
    // Use PE32 offset
    //
    PeImageBase = (UINTN)Hdr.Pe32->OptionalHeader.ImageBase;
  } else {
    //
    // Use PE32+ offset
    //
    PeImageBase = (UINTN)Hdr.Pe32Plus->OptionalHeader.ImageBase;
  }

  //
  // Basic Check
  //
  if (!IsTeardown) {
    if (PeImageBase == ImageBase) {
      //
      // Relocated
      //
      CpuDeadLoop ();
      return ;
    }
    if (PeImageBase != 0) {
      //
      // Build tool need gurantee it is 0-base address.
      //
      CpuDeadLoop ();
    }
  } else {
    if (PeImageBase == 0) {
      //
      // Already Relocated back
      //
      CpuDeadLoop ();
      return ;
    }
    //
    // relocated back
    //
    PeImageBase = 0;
  }

  //
  // This is self-contain PE-COFF loader.
  //
  PeCoffRelocateImageOnTheSpot (ImageBase, PeImageBase, IsTeardown);

  AsmWbinvd ();

  //
  // Set value indicate we have already relocated
  //
  if (!IsTeardown) {
    if (Magic == EFI_IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
      //
      // Use PE32 offset
      //
      Hdr.Pe32->OptionalHeader.ImageBase = (UINT32)ImageBase;
    } else {
      //
      // Use PE32+ offset
      //
      Hdr.Pe32Plus->OptionalHeader.ImageBase = (UINT64)ImageBase;
    }
  } else {
    if (Magic == EFI_IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
      //
      // Use PE32 offset
      //
      Hdr.Pe32->OptionalHeader.ImageBase = (UINT32)0;
    } else {
      //
      // Use PE32+ offset
      //
      Hdr.Pe32Plus->OptionalHeader.ImageBase = (UINT64)0;
    }
  }

  return ;
}