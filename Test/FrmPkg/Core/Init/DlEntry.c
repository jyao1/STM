/** @file
  DlEntry

  Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Dce.h"
#include "Frm.h"
#include "FrmInit.h"
#include "DrtmTpm.h"

/**
 1) Memory layout if enough DPR:

 +-----------+<-- DPR ------------------------------|
 | TxtHeap   |    TxtHeap Size (896K/0xE0000)       |
 |-----------|<-- TxtHeap Base                      |
 | SinitAcm  |    Sinit Size (128K/0x20000)         |
 +-----------+<-- Sinit Base                        |
 | MLE reg   |                                      |-----> DPR size (3M/0x300000)
 +-----------+--------------------------------------|

 2) Memory layout if no enough DPR:

 +-----------+<-- DPR ------------------------------|
 | TxtHeap   |    TxtHeap Size (896K/0xE0000)       |
 |-----------|<-- TxtHeap Base                      |
 | SinitAcm  |    Sinit Size (128K/0x20000)         |
 +-----------+<-- Sinit Base                        |
 |           |                                      |-----> DPR size (3M/0x300000)
 +-----------+--------------------------------------|

 +-----------+--------------------------------------|
 | MLE reg   |                                      |-----> PMR
 +-----------+--------------------------------------|

 3) MLE region
 +-----------+--------------------------------------|
 | LcpPo     |                                      |
 |-----------|                                      |
 | EventLog  |    EventLog Size (24K/0x6000)        |
 |-----------|                                      |
 | DCE       | <- Code/RoData section to be extended|
 |-----------|                                      |
 | MleHeader |    MleHeader Size (4K/0x1000)        |
 |-----------|                                      |
 | PageTable |    PageTable Size (12K/0x3000)       |
 +-----------+--------------------------------------|

 4) TXT heap
 +-----------+--------------------------------------|
 |-----------|                                      |
 | RlpStack  |                                      |
 |-----------|                                      |
 | IlpStack  |                                      |
 |-----------|                                      |
 | RlpHeap   |                                      |
 |-----------|                                      |
 |-----------|                                      |
 | SinitToMle|                                      |
 |-----------|                                      |
 | MleToSinit|                                      |
 |-----------|                                      |
 | OsToMle   | <- Data to be extended               |
 |-----------|                                      |
 | BiosToOs  |                                      |
 +-----------+--------------------------------------|

**/

TXT_UUID  gMleHeaderUuid = TXT_MLE_HEADER_UUID;

MLE_PRIVATE_DATA        mMlePrivateData;

//
// Dlme Parameter
//   Do not put it to Heap, because no need to measure them as DceData.
//   They will be measured seperately.
//
UINTN  mDlmeEntryPoint;
VOID   *mDlmeArgs;
UINTN  *mStackBufferTop;

TXT_UUID mChipsetAcmTableUuidV2 = TXT_CHIPSET_ACM_INFORMATION_TABLE_UUID_V02;
TXT_UUID mChipsetAcmTableUuidV3 = TXT_CHIPSET_ACM_INFORMATION_TABLE_UUID_V03;

extern UINT32 PostInitAddr;

/**

  This function prepare TXT environment.

  @param AcmBase  ACM base
  @param AcmSize  ACM size

**/
VOID
TxtPrepareEnvironment (
  IN UINT32   AcmBase,
  IN UINT32   AcmSize
  )
{
  MLE_PRIVATE_DATA    *MlePrivateData;

  DEBUG((EFI_D_INFO, "(TXT) TxtPrepareEnvironment ...\n"));

  MlePrivateData = GetMlePrivateData ();

  DEBUG((EFI_D_INFO, "(TXT) TxtSaveMtrr ...\n"));
  TxtSaveMtrr (MlePrivateData);
  DEBUG((EFI_D_INFO, "(TXT) TxtSaveMtrr Done\n"));
  MlePrivateData->Cr0 = AsmReadCr0 ();
  MlePrivateData->Cr4 = AsmReadCr4 ();
  MlePrivateData->Cr3 = (UINT32)AsmReadCr3 ();

  DEBUG((EFI_D_INFO, "(TXT) TxtConfigMtrr ...\n"));
  TxtConfigMtrr (
    AcmBase,
    AcmSize,
    MEMORY_TYPE_WB
    );
  DEBUG((EFI_D_INFO, "(TXT) TxtConfigMtrr Done\n"));

  DEBUG((EFI_D_INFO, "(TXT) AsmWriteCr0 ...\n"));
  AsmWriteCr0(AsmReadCr0 () | CR0_NE);
  DEBUG((EFI_D_INFO, "(TXT) AsmWriteCr0 Done\n"));

  DEBUG((EFI_D_INFO, "(TXT) TxtPrepareEnvironment Done\n"));
  return ;
}

/**

  This function restore TXT environment.

**/
VOID
TxtRestoreEnvironment (
  VOID
  )
{
  MLE_PRIVATE_DATA    *MlePrivateData;

  DEBUG((EFI_D_INFO, "(TXT) TxtRestoreEnvironment...\n"));
  MlePrivateData = GetMlePrivateData ();

  DEBUG((EFI_D_INFO, "(TXT) TxtRestoreMtrr...\n"));
  TxtRestoreMtrr (MlePrivateData);
  DEBUG((EFI_D_INFO, "(TXT) TxtRestoreMtrr Done\n"));

  DEBUG((EFI_D_INFO, "(TXT) AsmWriteCr3...\n"));
  AsmWriteCr3 (MlePrivateData->Cr3);
  DEBUG((EFI_D_INFO, "(TXT) AsmWriteCr4...\n"));
  AsmWriteCr4 (MlePrivateData->Cr4);
  DEBUG((EFI_D_INFO, "(TXT) AsmWriteCr0...\n"));
  AsmWriteCr0 (MlePrivateData->Cr0);
  DEBUG((EFI_D_INFO, "(TXT) TxtRestoreEnvironment Done\n"));

  return ;
}

/**

  This function enable TXT environment.

  @retval EFI_SUCCESS      TXT environment is enabled
  @retval EFI_UNSUPPORTED  TXT environment is not supported

**/
EFI_STATUS
EnableTxt (
  VOID
  )
{
  TXT_GETSEC_CAPABILITIES TxtCapabilities;
  UINT32                  Index;
  UINT32                  RegEax;
  UINT32                  RegEbx;
  UINT32                  RegEcx;
  BOOLEAN                 ClearMca;
  UINT32                  McaCount;

  //
  // Enable SMX
  //
  AsmWriteCr4(AsmReadCr4() | CR4_SMXE);
  
  //
  // Check TXT Chipset
  //
  Index = 0;
  TxtCapabilities.Uint32 = AsmGetSecCapabilities (Index);
  DumpGetSecCapabilities (Index, TxtCapabilities.Uint32);
  if (TxtCapabilities.Bits.ChipsetPresent == 0) {
    DEBUG ((EFI_D_ERROR, "(TXT) TXT Chipset not present!\n"));
    return EFI_UNSUPPORTED;
  }
  if (TxtCapabilities.Bits.Senter == 0) {
    DEBUG ((EFI_D_ERROR, "(TXT) SENTER not supported!\n"));
    return EFI_UNSUPPORTED;
  }
  while (TxtCapabilities.Bits.ExtendedLeafs != 0) {
    Index ++;
    TxtCapabilities.Uint32 = AsmGetSecCapabilities (Index);
    DumpGetSecCapabilities (Index, TxtCapabilities.Uint32);
  }

  //
  // Get parameters
  //
  ClearMca = TRUE;
  Index = 0;
  while (TRUE) {
    AsmGetSecParameters (Index, &RegEax, &RegEbx, &RegEcx);
    if ((RegEax & GETSEC_PARAMETER_TYPE_MASK) == 0) {
      break;
    }
    DumpGetSecParameters (RegEax, RegEbx, RegEcx);
    if ((RegEax & GETSEC_PARAMETER_TYPE_MASK) == GETSEC_PARAMETER_TYPE_EXTERNSION) {
      if ((RegEax & (1 << 6)) != 0) {
        // No need clear MCA
        ClearMca = FALSE;
      }
    }
    Index ++;
  }

  //
  // Clear MCA
  //
  if (ClearMca) {
    McaCount = (UINT32)AsmReadMsr64 (IA32_MCG_CAP) & 0xFF;
    for (Index = 0; Index < McaCount; Index++) {
      AsmWriteMsr64 (IA32_MC0_STATUS + Index * 4, 0);
    }
  }

  return EFI_SUCCESS;
}

/**

  This function get SINIT ACM information.

  @param SinitAcmBase SINIT ACM base
  @param SinitAcmSize SINIT ACM size

  @retval EFI_SUCCESS      SINIT ACM info is got and pass check
  @retval EFI_UNSUPPORTED  SINIT ACM is not supported

**/
EFI_STATUS
TxtSetupSinitAcmMemory (
  OUT UINT32  *SinitAcmBase,
  OUT UINT32  *SinitAcmSize
  )
{
  TXT_ACM_FORMAT                    *SinitAcm;
  TXT_BIOS_TO_OS_DATA               *BiosToOsData;
  TXT_CHIPSET_ID_LIST               *ChipsetIdList;
  TXT_ACM_CHIPSET_ID                *ChipsetID;
  TXT_PROCESSOR_ID_LIST             *ProcessorIdList;
  TXT_ACM_PROCESSOR_ID              *ProcessorID;
  UINTN                             Index;
  TXT_DID_VID                       DidVid;
  UINT32                            CurrentFMS;
  UINT64                            CurrentPlatformID;
  TXT_CHIPSET_ACM_INFORMATION_TABLE *ChipsetAcmInformationTable;
  MLE_PRIVATE_DATA                  *MlePrivateData;
  DCE_PRIVATE_DATA                  *DcePrivateData;
  TXT_ACM_TPM_INFO_LIST             *TpmInfoList;

  BiosToOsData = GetTxtBiosToOsData ();
  if (BiosToOsData->BiosSinitSize == 0) {
    DEBUG((EFI_D_ERROR, "(TXT) BiosSinitSize == 0\n"));
    return EFI_UNSUPPORTED;
  }

  *SinitAcmBase = TxtPubRead32 (TXT_SINIT_BASE);
  *SinitAcmSize = BiosToOsData->BiosSinitSize;

  SinitAcm = (TXT_ACM_FORMAT *)(UINTN)*SinitAcmBase;

  //
  // Dump ACM information
  //
  DumpAcm (SinitAcm);

  MlePrivateData = GetMlePrivateData();
  DcePrivateData = &MlePrivateData->DcePrivateData;

  ChipsetAcmInformationTable = (TXT_CHIPSET_ACM_INFORMATION_TABLE *) \
                               ((UINTN)(SinitAcm + 1) + 
                               SinitAcm->KeySize * 4 + 
                               sizeof(UINT32) + 
                               ACM_PKCS_1_5_RSA_SIGNATURE_SIZE + 
                               SinitAcm->ScratchSize * 4
                               );
  if (ChipsetAcmInformationTable->Version >= TXT_CHIPSET_ACM_INFORMATION_TABLE_VERSION_3) {
    DcePrivateData->AcmCapabilities = ChipsetAcmInformationTable->Capabilities;
  }
  if (ChipsetAcmInformationTable->Version >= TXT_CHIPSET_ACM_INFORMATION_TABLE_VERSION_5) {
    UINT16 TpmHashAlgo[] = { TPM_ALG_SHA1, TPM_ALG_SHA256, TPM_ALG_SHA384, TPM_ALG_SHA512, TPM_ALG_SM3_256 };
    UINTN  SubIndex;

    TpmInfoList = (TXT_ACM_TPM_INFO_LIST *)((UINTN)SinitAcm + ChipsetAcmInformationTable->TPMInfoList);
    DcePrivateData->AcmTpmCapabilities = TpmInfoList->Capabilities;
    for (Index = 0; Index < TpmInfoList->Count; Index++) {
      for (SubIndex = 0; SubIndex < sizeof(TpmHashAlgo) / sizeof(TpmHashAlgo[0]); SubIndex++) {
        if (TpmInfoList->AlgorithmID[Index] == TpmHashAlgo[SubIndex]) {
          // Only record hash algo
          DcePrivateData->AcmTpmHashAlgoID[DcePrivateData->AcmTpmHashAlgoIDCount] = TpmInfoList->AlgorithmID[Index];
          DcePrivateData->AcmTpmHashAlgoIDCount++;
        }
      }
    }
  }

  if (SinitAcm->ModuleType != TXT_ACM_MODULE_TYPE_CHIPSET_ACM) {
    DEBUG ((EFI_D_ERROR, "(TXT) ModuleType != TXT_ACM_MODULE_TYPE_CHIPSET_ACM (%08x)\n", (UINTN)SinitAcm->ModuleType));
    return EFI_UNSUPPORTED;
  }
  if (ChipsetAcmInformationTable->ChipsetACMType != TXT_CHIPSET_ACM_TYPE_SINIT) {
    DEBUG ((EFI_D_ERROR, "(TXT) ChipsetACMType != TXT_CHIPSET_ACM_TYPE_SINIT (%02x)\n", (UINTN)ChipsetAcmInformationTable->ChipsetACMType));
    return EFI_UNSUPPORTED;
  }
  switch (ChipsetAcmInformationTable->Version) {
  case TXT_CHIPSET_ACM_INFORMATION_TABLE_VERSION_2:
    if (CompareMem (&ChipsetAcmInformationTable->Uuid, &mChipsetAcmTableUuidV2, sizeof(mChipsetAcmTableUuidV2)) != 0) {
      DEBUG ((EFI_D_ERROR, "(TXT) ChipsetAcmTableUuid != TXT_CHIPSET_ACM_INFORMATION_TABLE_UUID_V02 (%g)\n", &ChipsetAcmInformationTable->Uuid));
      return EFI_UNSUPPORTED;
    }
    break;
  case TXT_CHIPSET_ACM_INFORMATION_TABLE_VERSION_3:
    if (CompareMem (&ChipsetAcmInformationTable->Uuid, &mChipsetAcmTableUuidV3, sizeof(mChipsetAcmTableUuidV3)) != 0) {
      DEBUG ((EFI_D_ERROR, "(TXT) ChipsetAcmTableUuid != TXT_CHIPSET_ACM_INFORMATION_TABLE_UUID_V03 (%g)\n", &ChipsetAcmInformationTable->Uuid));
      return EFI_UNSUPPORTED;
    }
    break;
  default:
    break;
  }

  if (ChipsetAcmInformationTable->MinMleHeaderVer > TXT_MLE_HEADER_VERSION) {
    DEBUG ((EFI_D_ERROR, "(TXT) MinMleHeaderVer != TXT_MLE_HEADER_VERSION (%08x)\n", (UINTN)ChipsetAcmInformationTable->MinMleHeaderVer));
    return EFI_UNSUPPORTED;
  }

  DidVid.Uint64 = TxtPubRead64 (TXT_DIDVID);
  ChipsetIdList = (TXT_CHIPSET_ID_LIST *)((UINTN)SinitAcm + ChipsetAcmInformationTable->ChipsetIDList);
  for (Index = 0; Index < ChipsetIdList->Count; Index++) {
    ChipsetID = &ChipsetIdList->ChipsetID[Index];
    if ((DidVid.Bits.VendorID == ChipsetID->VendorID) &&
        (DidVid.Bits.DeviceID == ChipsetID->DeviceID) &&
        ((((ChipsetID->Flags & TXT_ACM_CHIPSET_ID_REVISION_ID_MAKE) == 0) && (DidVid.Bits.RevisionID == ChipsetID->RevisionID)) ||
         (((ChipsetID->Flags & TXT_ACM_CHIPSET_ID_REVISION_ID_MAKE) != 0) && (DidVid.Bits.RevisionID & ChipsetID->RevisionID) != 0))) {
      break;
    }
    if (DidVid.Bits.VendorID != ChipsetID->VendorID) {
      DEBUG ((EFI_D_ERROR, "(TXT) DidVid.Bits.VendorID != ChipsetID->VendorID (%08x - %08x)\n", (UINTN)DidVid.Bits.VendorID, (UINTN)ChipsetID->VendorID));
    }
    if (DidVid.Bits.DeviceID != ChipsetID->DeviceID) {
      DEBUG ((EFI_D_ERROR, "(TXT) DidVid.Bits.DeviceID != ChipsetID->DeviceID (%08x - %08x)\n", (UINTN)DidVid.Bits.DeviceID, (UINTN)ChipsetID->DeviceID));
    }
    if ((ChipsetID->Flags & TXT_ACM_CHIPSET_ID_REVISION_ID_MAKE) == 0) {
      if (DidVid.Bits.RevisionID != ChipsetID->RevisionID) {
        DEBUG ((EFI_D_ERROR, "(TXT) DidVid.Bits.RevisionID != ChipsetID->RevisionID (%08x - %08x)\n", (UINTN)DidVid.Bits.RevisionID, (UINTN)ChipsetID->RevisionID));
      }
    } else {
      if ((DidVid.Bits.RevisionID & ChipsetID->RevisionID) == 0) {
        DEBUG ((EFI_D_ERROR, "(TXT) (DidVid.Bits.RevisionID & ChipsetID->RevisionID) == 0 (%08x - %08x)\n", (UINTN)DidVid.Bits.RevisionID, (UINTN)ChipsetID->RevisionID));
      }
    }
  }
  if (Index == ChipsetIdList->Count) {
    return EFI_UNSUPPORTED;
  }

  if (ChipsetAcmInformationTable->Version >= TXT_CHIPSET_ACM_INFORMATION_TABLE_VERSION_4) {
    AsmCpuid (CPUID_FEATURE_INFORMATION, &CurrentFMS, NULL, NULL, NULL);
    CurrentPlatformID = AsmReadMsr64(IA32_PLATFORM_ID_MSR_INDEX);

    ProcessorIdList = (TXT_PROCESSOR_ID_LIST *)((UINTN)SinitAcm + ChipsetAcmInformationTable->ProcessorIDList);
    for (Index = 0; Index < ProcessorIdList->Count; Index++) {
      ProcessorID = &ProcessorIdList->ProcessorID[Index];
      if ((ProcessorID->FMS & ProcessorID->FMSMask) != (CurrentFMS & ProcessorID->FMSMask)) {
        DEBUG ((EFI_D_ERROR, "(TXT) (ProcessorID->FMS & ProcessorID->FMSMask) != (CurrentFMS & ProcessorID->FMSMask) (%08x - %08x, current - %08x)\n", (UINTN)ProcessorID->FMS, (UINTN)ProcessorID->FMSMask, (UINTN)CurrentFMS));
        continue;
      }
      if ((ProcessorID->PlatformID & ProcessorID->PlatformMask) != (CurrentPlatformID & ProcessorID->PlatformMask)) {
        DEBUG ((EFI_D_ERROR, "(TXT) (ProcessorID->PlatformID & ProcessorID->PlatformMask) != (CurrentPlatformID & ProcessorID->PlatformMask) (%016lx - %016lx, current - %016lx)\n", ProcessorID->PlatformID, ProcessorID->PlatformMask, CurrentPlatformID));
        continue;
      }
      //
      // break if both check pass.
      //
      break;
    }
    if (Index == ProcessorIdList->Count) {
      return EFI_UNSUPPORTED;
    }
  }

  return EFI_SUCCESS;
}

/**

  This function set TXT HEAP.

  @param MleLoadAddress  MLE address
  @param MleLoadSize     MLE size
  @param PageTableBase   page table base

**/
VOID
TxtSetupHeap (
  IN UINT64  MleLoadAddress,
  IN UINT64  MleLoadSize,
  IN UINT64  PageTableBase
  )
{
  VOID                              *TxtOsMleData;
  MLE_PRIVATE_DATA                  *MlePrivateData;
  DCE_PRIVATE_DATA                  *DcePrivateData;
  TXT_OS_TO_SINIT_DATA              *OsSinitData;
  TXT_SINIT_TO_MLE_DATA             *SinitMleData;
  TXT_ACM_FORMAT                    *SinitAcm;
  TXT_CHIPSET_ACM_INFORMATION_TABLE *ChipsetAcmInformationTable;
  UINTN                             TxtHeapSize;
  UINTN                             TxtHeapOccupiedSize;

  //
  // MlePrivateData
  //
  TxtOsMleData = GetTxtOsToMleData();
  DEBUG((EFI_D_INFO, "(TXT) TxtOsMleData - 0x%x\n", TxtOsMleData));
  *((UINT64 *)TxtOsMleData - 1) = sizeof(UINT64) + sizeof(TXT_OS_TO_MLE_DATA_STRUCT);

  MlePrivateData = GetMlePrivateData ();
  DcePrivateData = &MlePrivateData->DcePrivateData;
  AsmReadGdtr (&MlePrivateData->Gdtr);
  AsmReadIdtr (&MlePrivateData->Idtr);
  MlePrivateData->Ds = AsmReadDs ();

  MlePrivateData->TempEsp = (UINT32)(UINT32)((UINTN)GetTxtHeap () + GetTxtHeapSize () - MLE_TEMP_STACK_SIZE_RLP - 0x20);

  //
  // Patch CS/Offset in stack
  //
  // +---------+
  // |  Offset |
  // +---------+
  // |    CS   |
  // +---------+
  // |  Dummy  |
  // +---------+
  // |  Dummy  |
  // +---------+
  // |  Dummy  |
  // +---------+
  // |  Dummy  |
  // +---------+ <- TempEsp
  if (PostInitAddr != 0) {
    MlePrivateData->PostSinitOffset  = (UINT32)((UINTN)AsmMleEntryPoint + PostInitAddr);
    MlePrivateData->PostSinitSegment = (UINT32)AsmReadCs();
  }

  MlePrivateData->Lock = 0;
  MlePrivateData->RlpInitializedNumber = 0;

  //
  // OsSinitData
  //
  OsSinitData = GetTxtOsToSinitData ();
  DEBUG((EFI_D_INFO, "(TXT) OsSinitData - 0x%x\n", OsSinitData));
  *((UINT64 *)OsSinitData - 1) = sizeof(UINT64) + sizeof(*OsSinitData);
  OsSinitData->Version = TXT_OS_TO_SINIT_DATA_VERSION;
  OsSinitData->Flags = 0;
  OsSinitData->MLEPageTableBase = PageTableBase;

  //
  // We copy MleHeader to DPR
  //
  {
    TXT_MLE_HEADER   *MleHeader;

    MleHeader = (TXT_MLE_HEADER *)(UINTN)MleLoadAddress;
    CopyMem (&MleHeader->Uuid, &gMleHeaderUuid, sizeof(gMleHeaderUuid));
    MleHeader->HeaderLen = sizeof(*MleHeader);
    MleHeader->Version = TXT_MLE_HEADER_VERSION;

    MleHeader->EntryPoint = (UINT32)(UINTN)MleHeader + sizeof(*MleHeader);
    if (MleHeader->Version < TXT_MLE_HEADER_VERSION_2_1) {
      MleHeader->EntryPoint -= (sizeof(MleHeader->CmdlineStart) + sizeof(MleHeader->CmdlineEnd));
    }
    //
    // Patch the instruction for ILP
    //
    *(UINT8 *)(UINTN)MleHeader->EntryPoint = 0x90; // nop
    *(UINT8 *)((UINTN)MleHeader->EntryPoint + 1) = 0xE9; // near jmp
    *(UINT32 *)((UINTN)MleHeader->EntryPoint + 2) = (UINT32)((UINTN)AsmMleEntryPoint - ((UINTN)MleHeader->EntryPoint + 6)); // minus next instruction
    //
    // Patch the instrution for RLPs: cli, hlt, and jmp $-2
    //
    *(UINT32 *)((UINTN)MleHeader->EntryPoint + 6) = 0xFCEBF4FA;

    MleHeader->EntryPoint -= (UINT32)PageTableBase;

    if (MleHeader->Version >= TXT_MLE_HEADER_VERSION_1_1) {
      MleHeader->FirstValidPage = (UINT32)MleLoadAddress;
      MleHeader->FirstValidPage -= (UINT32)PageTableBase;
      MleHeader->MleStart = MleHeader->FirstValidPage;
      //MleHeader->MleEnd = MleHeader->MleStart + sizeof(*MleHeader) + 10; // Offset (1 nop + 5 jmp + 4 deadloop)
      MleHeader->MleEnd = MleHeader->MleStart + (UINT32)MleLoadSize;
    }

    if (MleHeader->Version >= TXT_MLE_HEADER_VERSION_2) {
      MleHeader->Capabilities = TXT_MLE_SINIT_CAPABILITY_GETSET_WAKEUP | TXT_MLE_SINIT_CAPABILITY_MONITOR_ADDRESS_RLP_WAKEUP;
      MleHeader->Capabilities |= TXT_MLE_SINIT_CAPABILITY_ECX_HAS_PAGE_TABLE;
#ifdef STM_SUPPORT
      MleHeader->Capabilities |= TXT_MLE_SINIT_CAPABILITY_STM;
#endif
    }

    if (MleHeader->Version >= TXT_MLE_HEADER_VERSION_2_1) {
      MleHeader->CmdlineStart = 0; // Not use
      MleHeader->CmdlineEnd   = 0; // Not use
    }

    //
    // Done
    //
    OsSinitData->MLEHeaderBase = (UINT64)(UINTN)MleHeader;
    OsSinitData->MLEHeaderBase -= PageTableBase;
    OsSinitData->MLESize = (UINT64)(MleHeader->MleEnd - MleHeader->MleStart);
  }

  OsSinitData->PMRLowBase  = MlePrivateData->DcePrivateData.PmrLowBase;
  OsSinitData->PMRLowSize  = MlePrivateData->DcePrivateData.PmrLowSize;
  OsSinitData->PMRHighBase = MlePrivateData->DcePrivateData.PmrHighBase;
  OsSinitData->PMRHighSize = MlePrivateData->DcePrivateData.PmrHighSize;
  OsSinitData->LCPPOBase   = MlePrivateData->DcePrivateData.LcpPoBase;
  OsSinitData->LCPPOSize   = MlePrivateData->DcePrivateData.LcpPoSize;

  SinitAcm = (TXT_ACM_FORMAT *)(UINTN)TxtPubRead32 (TXT_SINIT_BASE);
  ChipsetAcmInformationTable = (TXT_CHIPSET_ACM_INFORMATION_TABLE *) \
                               ((UINTN)(SinitAcm + 1) + 
                               SinitAcm->KeySize * 4 + 
                               sizeof(UINT32) + 
                               ACM_PKCS_1_5_RSA_SIGNATURE_SIZE + 
                               SinitAcm->ScratchSize * 4
                               );

  if ((ChipsetAcmInformationTable->Capabilities & TXT_MLE_SINIT_CAPABILITY_MONITOR_ADDRESS_RLP_WAKEUP) != 0) {
    OsSinitData->Capabilities = TXT_MLE_SINIT_CAPABILITY_MONITOR_ADDRESS_RLP_WAKEUP;
  } else {
    OsSinitData->Capabilities = TXT_MLE_SINIT_CAPABILITY_GETSET_WAKEUP;
  }
  OsSinitData->Version = ChipsetAcmInformationTable->OsSinitTableVer;
  if (ChipsetAcmInformationTable->OsSinitTableVer >= TXT_OS_TO_SINIT_DATA_VERSION_5) {
    OsSinitData->RsdpPtr = (UINT64)(UINTN)FindAcpiRsdPtr ();
    if (OsSinitData->RsdpPtr == 0) {
      OsSinitData->RsdpPtr = (UINT64)(UINTN)&MlePrivateData->UefiRsdp;
    }
    if (ChipsetAcmInformationTable->OsSinitTableVer >= TXT_OS_TO_SINIT_DATA_VERSION_6) {
      if (ChipsetAcmInformationTable->OsSinitTableVer >= TXT_OS_TO_SINIT_DATA_VERSION_7) {
        OsSinitData->Flags = TXT_OS_TO_SINIT_DATA_FLAGS_MAX_AGILE_POLICY;
      }
      //
      // Fill Event Log data there
      //
      TXT_HEAP_EXT_DATA_ELEMENT             *Element;
      TXT_HEAP_EVENTLOG_EXT_ELEMENT         *EventLogElement;
      TXT_EVENT_LOG_CONTAINER               *EventLog;
      TXT_HEAP_EVENT_LOG_POINTER_ELEMENT2   *EventLogPointerElement2;
      TXT_HEAP_EVENT_LOG_POINTER_ELEMENT2_1 *EventLogPointerElement2_1;
      TXT_HEAP_EVENT_LOG_DESCR              *EventLogDesc;
      UINTN                                 Index;
      TCG_LOG_DESCRIPTOR                    *TcgLogDesc;
      TCG_PCR_EVENT_HDR                     *PcrEvent;

      *((UINT64 *)OsSinitData - 1) = sizeof(UINT64) + sizeof(*OsSinitData);

      Element = (TXT_HEAP_EXT_DATA_ELEMENT *)(OsSinitData + 1);

      if (DcePrivateData->TpmType == FRM_TPM_TYPE_TPM12) {
        // TPM1.2
        Element->Type = TXT_HEAP_EXTDATA_TYPE_EVENTLOG_PTR;
        Element->Size = sizeof(TXT_HEAP_EXT_DATA_ELEMENT) + sizeof(TXT_HEAP_EVENTLOG_EXT_ELEMENT);
        EventLogElement = (TXT_HEAP_EVENTLOG_EXT_ELEMENT *)(Element + 1);
        DcePrivateData->EventLogElement = EventLogElement;

        //
        // Init EventLogContainer
        //
        EventLog = (TXT_EVENT_LOG_CONTAINER *)(UINTN)(MlePrivateData->DcePrivateData.EventLogBase);
        ZeroMem(EventLog, MAX_EVENT_LOG_BUFFER_SIZE);
        CopyMem(EventLog->Signature, TXT_EVENTLOG_SIGNATURE, sizeof(TXT_EVENTLOG_SIGNATURE));
        EventLog->ContainerVersionMajor = TXT_EVENTLOG_CONTAINER_MAJOR_VERSION;
        EventLog->ContainerVersionMinor = TXT_EVENTLOG_CONTAINER_MINOR_VERSION;
        EventLog->PcrEventVersionMajor = TXT_EVENTLOG_EVENT_MAJOR_VERSION;
        EventLog->PcrEventVersionMinor = TXT_EVENTLOG_EVENT_MINOR_VERSION;
        EventLog->Size = MAX_EVENT_LOG_BUFFER_SIZE;
        EventLog->PcrEventsOffset = sizeof(*EventLog);
        EventLog->NextEventOffset = sizeof(*EventLog);

        EventLogElement->EventLogAddress = (UINT64)(UINTN)EventLog;

        *((UINT64 *)OsSinitData - 1) += Element->Size;
        Element = (TXT_HEAP_EXT_DATA_ELEMENT *)((UINTN)Element + Element->Size);

      }
      if (DcePrivateData->TpmType == FRM_TPM_TYPE_TPM2) {
        // TPM2.0
        UINT16 TpmHashAlgo[] = { TPM_ALG_SHA1, TPM_ALG_SHA256, TPM_ALG_SHA384, TPM_ALG_SHA512, TPM_ALG_SM3_256 };
        UINTN  SubIndex;

        DcePrivateData->EventHashAlgoIDCount = 0;
        for (Index = 0; Index < sizeof(TpmHashAlgo) / sizeof(TpmHashAlgo[0]); Index++) {
          if ((DcePrivateData->ActivePcrBanks & (1 << Index)) != 0) {
            for (SubIndex = 0; SubIndex < DcePrivateData->AcmTpmHashAlgoIDCount; SubIndex++) {
              if (DcePrivateData->AcmTpmHashAlgoID[SubIndex] == TpmHashAlgo[Index]) {
                // Both TPM and ACM support this hash algo.
                DcePrivateData->EventHashAlgoID[DcePrivateData->EventHashAlgoIDCount] = TpmHashAlgo[Index];
                DcePrivateData->EventHashAlgoIDCount++;
              }
            }
          }
        }

        if ((DcePrivateData->AcmCapabilities & TXT_MLE_SINIT_CAPABILITY_TCG2_COMPATIBILE_EVENTLOG) == 0) {
          Element->Type = TXT_HEAP_EXTDATA_TYPE_EVENT_LOG_POINTER2;
          EventLogPointerElement2 = (TXT_HEAP_EVENT_LOG_POINTER_ELEMENT2 *)(Element + 1);
          DcePrivateData->EventLogPointerElement2 = EventLogPointerElement2;
          EventLogPointerElement2->Count = DcePrivateData->EventHashAlgoIDCount;
          EventLogDesc = (TXT_HEAP_EVENT_LOG_DESCR *)(EventLogPointerElement2 + 1);

          Element->Size = sizeof(TXT_HEAP_EXT_DATA_ELEMENT) + sizeof(TXT_HEAP_EVENT_LOG_POINTER_ELEMENT2) + EventLogPointerElement2->Count * sizeof(TXT_HEAP_EVENT_LOG_DESCR);

          for (Index = 0; Index < EventLogPointerElement2->Count; Index++, EventLogDesc++) {
            EventLogDesc->HashAlgID = DcePrivateData->EventHashAlgoID[Index];
            EventLogDesc->Reserved = 0;
            EventLogDesc->PhysicalAddress = (UINT64)(UINTN)(MlePrivateData->DcePrivateData.EventLogBase + MAX_EVENT_LOG_BUFFER_SIZE * (Index + 1));
            ZeroMem((VOID *)(UINTN)EventLogDesc->PhysicalAddress, MAX_EVENT_LOG_BUFFER_SIZE);
            EventLogDesc->AllocatedEventContainerSize = MAX_EVENT_LOG_BUFFER_SIZE;
            if (EventLogDesc->HashAlgID == TPM_ALG_SHA) {
              PcrEvent = (TCG_PCR_EVENT_HDR *)(UINTN)EventLogDesc->PhysicalAddress;
              TcgLogDesc = (TCG_LOG_DESCRIPTOR *)(PcrEvent + 1);
              PcrEvent->PCRIndex = 0;
              PcrEvent->EventType = EV_NO_ACTION;
              ZeroMem(&PcrEvent->Digest, sizeof(PcrEvent->Digest));
              PcrEvent->EventSize = sizeof(TCG_LOG_DESCRIPTOR);
              CopyMem(TcgLogDesc->Signature, TCG_LOG_DESCRIPTOR_SIGNATURE, sizeof(TCG_LOG_DESCRIPTOR_SIGNATURE));
              TcgLogDesc->Revision    = TCG_LOG_DESCRIPTOR_REVISION;
              TcgLogDesc->DigestAlgID = DIGEST_ALG_ID_SHA_1;
              TcgLogDesc->DigestSize  = SHA1_DIGEST_SIZE;
              EventLogDesc->FirstRecordOffset = sizeof(TCG_PCR_EVENT_HDR) + PcrEvent->EventSize;
              EventLogDesc->NextRecordOffset = sizeof(TCG_PCR_EVENT_HDR) + PcrEvent->EventSize;
            } else {
              EventLogDesc->FirstRecordOffset = 0;
              EventLogDesc->NextRecordOffset = 0;
            }
          }
        } else {
          Element->Type = TXT_HEAP_EXTDATA_TYPE_EVENT_LOG_POINTER2_1;
          EventLogPointerElement2_1 = (TXT_HEAP_EVENT_LOG_POINTER_ELEMENT2_1 *)(Element + 1);
          DcePrivateData->EventLogPointerElement2_1 = EventLogPointerElement2_1;
          EventLogPointerElement2_1->PhysicalAddress = (UINT64)(UINTN)(MlePrivateData->DcePrivateData.EventLogBase + MAX_EVENT_LOG_BUFFER_SIZE);
          ZeroMem((VOID *)(UINTN)EventLogPointerElement2_1->PhysicalAddress, MAX_EVENT_LOG_BUFFER_SIZE);
          EventLogPointerElement2_1->AllocatedEventContainerSize = MAX_EVENT_LOG_BUFFER_SIZE * 5;
          EventLogPointerElement2_1->FirstRecordOffset = 0;
          EventLogPointerElement2_1->NextRecordOffset = 0;
        }

        *((UINT64 *)OsSinitData - 1) += Element->Size;
        Element = (TXT_HEAP_EXT_DATA_ELEMENT *)((UINTN)Element + Element->Size);

      }

      Element->Type = TXT_HEAP_EXTDATA_TYPE_END;
      Element->Size = sizeof(TXT_HEAP_END_ELEMENT);

      *((UINT64 *)OsSinitData - 1) += sizeof(TXT_HEAP_END_ELEMENT);
    }
  } else {
    // Version 4
    *((UINT64 *)OsSinitData - 1) = sizeof(UINT64) + sizeof(*OsSinitData) - sizeof(UINT64);
  }

  //
  // SinitMleData
  //
  SinitMleData = GetTxtSinitToMleData ();
  DEBUG((EFI_D_INFO, "(TXT) SinitMleData - 0x%x\n", SinitMleData));
  *((UINT64 *)SinitMleData - 1) = 0;
  ZeroMem (SinitMleData, sizeof(*SinitMleData));

  TxtHeapSize = GetTxtHeapSize ();
  TxtHeapOccupiedSize = GetTxtHeapOccupiedSize ();
  DEBUG ((EFI_D_INFO, "(TXT) TXT Heap base    - %08x\n", GetTxtHeap ()));
  DEBUG ((EFI_D_INFO, "(TXT) TXT Heap size    - %08x\n", TxtHeapSize));
  DEBUG ((EFI_D_INFO, "(TXT) TXT BiosOsData   - %08x\n", GetTxtBiosToOsData()));
  DEBUG ((EFI_D_INFO, "(TXT) TXT OsMleData    - %08x\n", (UINTN)TxtOsMleData));
  DEBUG ((EFI_D_INFO, "(TXT) TXT OsSinitData  - %08x\n", (UINTN)OsSinitData));
  DEBUG ((EFI_D_INFO, "(TXT) TXT SinitMleData - %08x\n", (UINTN)SinitMleData));
  DEBUG ((EFI_D_INFO, "(TXT) TXT OccupiedSize - %08x\n", TxtHeapOccupiedSize));

  //
  // Check heap
  //
  if (TxtHeapOccupiedSize >= TxtHeapSize) {
    DEBUG ((EFI_D_ERROR, "(TXT) ERROR: TXT Heap overflow - Occupied (%08x), Allocated (%08x)\n", TxtHeapOccupiedSize, TxtHeapSize));
    ASSERT(FALSE);
  }

  if (OsSinitData->RsdpPtr != 0) {
    EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER   *Rsdp;
    EFI_ACPI_DESCRIPTION_HEADER                    *Rsdt;
    EFI_ACPI_DESCRIPTION_HEADER                    *Xsdt;
    // validate ACPI RSDP/RSDT/XSDT
    Rsdp = (EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER *)(UINTN)OsSinitData->RsdpPtr;
    DEBUG ((EFI_D_INFO, "(TXT) RSDP Address  - %08x\n", Rsdp));
    DEBUG ((EFI_D_INFO, "(TXT) RSDP Checksum - %02x\n", CalculateCheckSum8((UINT8 *)Rsdp, sizeof(EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_POINTER))));
    if (Rsdp->Revision >= 2) {
      DEBUG ((EFI_D_INFO, "(TXT) RSDP Length   - %08x\n", Rsdp->Length));
      DEBUG ((EFI_D_INFO, "(TXT) RSDP ExtendedChecksum - %02x\n", CalculateCheckSum8((UINT8 *)Rsdp, Rsdp->Length)));
    }
    Rsdt = (EFI_ACPI_DESCRIPTION_HEADER *)(UINTN)Rsdp->RsdtAddress;
    DEBUG ((EFI_D_INFO, "(TXT) RSDT Address  - %08x\n", Rsdt));
    DEBUG ((EFI_D_INFO, "(TXT) RSDT Length   - %08x\n", Rsdt->Length));
    DEBUG ((EFI_D_INFO, "(TXT) RSDT Checksum - %02x\n", CalculateCheckSum8((UINT8 *)Rsdt, Rsdt->Length)));
    if (Rsdp->Revision >= 2) {
      Xsdt = (EFI_ACPI_DESCRIPTION_HEADER *)(UINTN)Rsdp->XsdtAddress;
      DEBUG ((EFI_D_INFO, "(TXT) XSDT Address  - %016lx\n", Xsdt));
      DEBUG ((EFI_D_INFO, "(TXT) XSDT Length   - %08x\n", Xsdt->Length));
      DEBUG ((EFI_D_INFO, "(TXT) XSDT Checksum - %02x\n", CalculateCheckSum8((UINT8 *)Xsdt, Xsdt->Length)));
    }
  }

  return ;
}

/**

  This function set TXT page table for MLE.

  @param MleLoadAddress  MLE address
  @param MleLoadSize     MLE size
  @param PageTableBase   page table base

**/
VOID
TxtSetupPageTable (
  IN UINT64  MleLoadAddress,
  IN UINT64  MleLoadSize,
  IN UINT64  PageTableBase
  )
{
  UINT64   *PageDirectoryPtrEntry;
  UINT64   *PageDirectoryEntry;
  UINT64   *PageTableEntry;
  UINTN    PDPEIndex;
  UINTN    PDEIndex;
  UINTN    PDELastIndex;
  UINTN    PTEIndex;
  UINTN    PTELastIndex;
  UINTN    Index;

  MleLoadAddress = MleLoadAddress - PageTableBase;

  ZeroMem ((VOID *)(UINTN)PageTableBase, EFI_PAGES_TO_SIZE (MLE_PAGE_TABLE_PAGES));

  PageDirectoryPtrEntry= (UINT64 *)(UINTN)PageTableBase;
  PageDirectoryEntry   = (UINT64 *)((UINTN)PageDirectoryPtrEntry + EFI_PAGE_SIZE);
  PageTableEntry       = (UINT64 *)((UINTN)PageDirectoryEntry + EFI_PAGE_SIZE);

  PDPEIndex = ((UINTN)MleLoadAddress & 0xC0000000) >> 30;
  PageDirectoryPtrEntry[PDPEIndex] = (UINT64)(UINTN)PageDirectoryEntry;
  PageDirectoryPtrEntry[PDPEIndex] |= IA32_PG_P;

  PDEIndex = ((UINTN)MleLoadAddress & 0x3FE00000) >> 21;
  PDELastIndex = ((UINTN)(MleLoadAddress + MleLoadSize - 1) & 0x3FE00000) >> 21;
  ASSERT(PDEIndex == PDELastIndex);
  PageDirectoryEntry[PDEIndex] = (UINT64)(UINTN)PageTableEntry;
  PageDirectoryEntry[PDEIndex] |= IA32_PG_P|IA32_PG_RW;

  PTEIndex = ((UINTN)MleLoadAddress & 0x001FF000) >> 12;
  PTELastIndex = ((UINTN)(MleLoadAddress + MleLoadSize - 1) & 0x001FF000) >> 12;

  DEBUG ((EFI_D_INFO, "(TXT) PTEIndex: %08x - %08x\n", (UINTN)PTEIndex, (UINTN)PTELastIndex));

  for (Index = PTEIndex; Index <= PTELastIndex; Index++) {
    PageTableEntry[Index] = (UINT64)(UINTN)((MleLoadAddress + (Index - PTEIndex) * EFI_PAGE_SIZE) & 0xFFFFF000);
    PageTableEntry[Index] += PageTableBase;
    PageTableEntry[Index] |= IA32_PG_P|IA32_PG_RW;
  }

  DEBUG ((EFI_D_INFO, "(TXT) TXT PageTable    - %08x\n", (UINTN)PageTableBase));

  return ;
}

/**

  This function launch TXT environment.

  @retval EFI_UNSUPPORTED  It is not supported to launch TXT
  not return means launch successfully

**/
EFI_STATUS
LaunchTxtEnvironment (
  VOID
  )
{
  UINT64                  PageTableBase;
  UINT32                  SinitAcmBase;
  UINT32                  SinitAcmSize;
  EFI_STATUS              Status;
  UINT64                  MleLoadAddress;
  UINT64                  MleLoadSize;
  MLE_PRIVATE_DATA        *MlePrivateData;

  //
  // Fill MLE image base
  //
  MlePrivateData    = GetMlePrivateData ();
  MleLoadAddress = (UINT32)(MlePrivateData->DcePrivateData.DprBase + EFI_PAGES_TO_SIZE (MLE_PAGE_TABLE_PAGES));
  MleLoadSize = EFI_PAGES_TO_SIZE (MLE_LOADER_PAGES) + MlePrivateData->DcePrivateData.MeasuredImageSize;

  //
  // Enable TXT CPU
  //
  Status = EnableTxt ();
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Setup SINIT ACM memory
  //
  Status = TxtSetupSinitAcmMemory (&SinitAcmBase, &SinitAcmSize);
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Setup page table
  //
  PageTableBase = MlePrivateData->DcePrivateData.DprBase;
  DEBUG((EFI_D_INFO, "(TXT) TxtSetupPageTable:\n"));
  DEBUG((EFI_D_INFO, "(TXT) MleLoadAddress: 0x%x\n", MleLoadAddress));
  DEBUG((EFI_D_INFO, "(TXT) MleLoadSize: 0x%x\n", MleLoadSize));
  DEBUG((EFI_D_INFO, "(TXT) PageTableBase: 0x%x\n", PageTableBase));
  TxtSetupPageTable (MleLoadAddress, MleLoadSize, PageTableBase);

  //
  // Setup TXT heap
  //
  TxtSetupHeap (MleLoadAddress, MleLoadSize, PageTableBase);

  DumpMleHeader ((TXT_MLE_HEADER *)(UINTN)MleLoadAddress);
  DumpBiosToOsData ((UINT64 *)GetTxtBiosToOsData() - 1);
  DumpOsToSinitData ((UINT64 *)GetTxtOsToSinitData() - 1);

  //
  // Prepare environment
  //
  TxtPrepareEnvironment (SinitAcmBase, SinitAcmSize);

  DEBUG ((EFI_D_INFO, "(TXT) SENTER ...\n"));

  //
  // LaunchTxt
  //
  AsmGetSecSenter (SinitAcmBase, SinitAcmSize, 0);
  DEBUG ((EFI_D_ERROR, "(TXT) SENTER fail!\n"));

  return EFI_DEVICE_ERROR;
}

/**

  This function check if MLE is launched.

  @retval TRUE  MLE is launched
  @retval FALSE MLE is not launched

**/
BOOLEAN
IsMleLaunched (
  VOID
  )
{
  UINT32  TxtStatus;

  if ((AsmReadMsr64(IA32_FEATURE_CONTROL_MSR_INDEX) & (IA32_FEATURE_CONTROL_SMX | IA32_FEATURE_CONTROL_LCK)) !=
    (IA32_FEATURE_CONTROL_SMX | IA32_FEATURE_CONTROL_LCK)) {
    return FALSE;
  }

  TxtStatus = TxtPubRead32 (TXT_STS);

  return (BOOLEAN)((TxtStatus & TXT_STS_SENTER_DONE) != 0);
}

/**

  This function init TPM pre DL_Entry.

**/
VOID
PreDL_Entry (
  VOID
  )
{
  TpmCommRelinquishLocality (TPM_LOCALITY_0);

  return ;
}

/**

  This function init TPM post DL_Entry.

**/
VOID
PostDL_Entry (
  VOID
  )
{
  DumpTpmEventLogBuffer();

  //
  // Need open it explicit
  //
  OpenLocality1 ();

  //
  // Dump and measure (TPM - TBD)
  //
  DoMeasurement ();

  TpmCommRequestUseTpm (TPM_LOCALITY_0);

  return ;
}

/**

  This function is invoked after SENTER by MLE entrypoint.

**/
VOID
DL_Entry_Back (
  VOID
  )
{
  DEBUG ((EFI_D_INFO, "(TXT) SENTER Done!\n"));

  //
  // Restore enviroment
  //
  TxtRestoreEnvironment ();

  //
  // AP wakeup, re-enable SMI
  //
  AsmGetSecSmctrl (0);

  DumpSinitToMleData((UINT64 *)GetTxtSinitToMleData() - 1);

  //
  // Set Secret flags
  // @bug - set when release - unset when debug
  //
//  SetSecrets ();

  PostDL_Entry ();

  AsmLaunchDlmeMain (mDlmeEntryPoint, mDlmeArgs, mStackBufferTop);

  //
  // Never come here
  //
  CpuDeadLoop ();

  return ;
}

/**

  This function entry DL environment.

  @param DlmeEntryPoint    A pointer to the entrypoint of DLME.
  @param DlmeArgs          A pointer to the Args of DLME.
  @param StackBufferTop    A pointer to the starting virtual address of a buffer to be used for the DLME's stack.

  @retval  non-0 fail to entry DL environment

**/
UINT32
DL_Entry (
  IN UINTN                  DlmeEntryPoint,
  IN VOID                   *DlmeArgs,
  IN UINTN                  *StackBufferTop
  )
{
  EFI_STATUS                 Status;

  if (IsMleLaunched ()) {
    return (UINT32)-1;
  }

  mDlmeEntryPoint = DlmeEntryPoint;
  mDlmeArgs = DlmeArgs;
  mStackBufferTop = StackBufferTop;

  PreDL_Entry ();

  Status = LaunchTxtEnvironment ();
  if (EFI_ERROR (Status)) {
    return (UINT32)-1;
  }

  //
  // Never come here
  //
  CpuDeadLoop ();

  return (UINT32)-1;
}

/**

  This function return CPU number in TXT BiosToOs region.

  @return CPU number in TXT BiosToOs region

**/
UINT32
GetTxtCpuNumber (
  VOID
  )
{
  TXT_BIOS_TO_OS_DATA               *BiosToOsData;

  BiosToOsData = GetTxtBiosToOsData ();
  return BiosToOsData->NumLogProcs;
}

/**

  This function is Dce entrypoint.

  @retval EFI_SUCCESS           Dce entrypoint run successfully
  @retval EFI_UNSUPPORTED       not support run TXT

**/
EFI_STATUS
DceEntryPoint (
  VOID
  )
{
  VOID                  *Rsdp;
  MLE_PRIVATE_DATA      *MlePrivateData;

  if ((AsmReadMsr64 (IA32_FEATURE_CONTROL_MSR_INDEX) & (IA32_FEATURE_CONTROL_SMX | IA32_FEATURE_CONTROL_LCK)) !=
      (IA32_FEATURE_CONTROL_SMX | IA32_FEATURE_CONTROL_LCK)) {
    DEBUG ((EFI_D_ERROR, "(TXT) !!!SMX not enabled!\n"));
    return EFI_UNSUPPORTED;
  }

  MlePrivateData = GetMlePrivateData ();

  DEBUG ((EFI_D_INFO, "(TXT) DprBase      - %08x\n", (UINTN)MlePrivateData->DcePrivateData.DprBase));
  DEBUG ((EFI_D_INFO, "(TXT) DprSize      - %08x\n", (UINTN)MlePrivateData->DcePrivateData.DprSize));
  DEBUG ((EFI_D_INFO, "(TXT) PmrLowBase   - %08x\n", (UINTN)MlePrivateData->DcePrivateData.PmrLowBase));
  DEBUG ((EFI_D_INFO, "(TXT) PmrLowSize   - %08x\n", (UINTN)MlePrivateData->DcePrivateData.PmrLowSize));
  DEBUG ((EFI_D_INFO, "(TXT) LcpPoBase    - %08x\n", (UINTN)MlePrivateData->DcePrivateData.LcpPoBase));
  DEBUG ((EFI_D_INFO, "(TXT) LcpPoSize    - %08x\n", (UINTN)MlePrivateData->DcePrivateData.LcpPoSize));

  MlePrivateData->CpuNum = GetTxtCpuNumber ();

  MlePrivateData->DcePrivateData.DL_Entry  = (EFI_PHYSICAL_ADDRESS)(UINTN)DL_Entry;
  MlePrivateData->DcePrivateData.DLME_Exit = (EFI_PHYSICAL_ADDRESS)(UINTN)DLME_Exit;

  Rsdp = FindAcpiRsdPtr ();
  DEBUG ((EFI_D_INFO, "(TXT) UefiRsdp     - %08x\n", (UINTN)Rsdp));
  if (Rsdp != NULL) {
    CopyMem (&MlePrivateData->UefiRsdp, Rsdp, sizeof(MlePrivateData->UefiRsdp));
  }

  return EFI_SUCCESS;
}

/**

  This function get MLE private data.

  @return MLE private data

**/
VOID *
GetMlePrivateData(
  VOID
  )
{
  TXT_OS_TO_MLE_DATA_STRUCT          *TxtOsToMleDataStruct;

  TxtOsToMleDataStruct = GetTxtOsToMleData();

  if (TxtOsToMleDataStruct->Signature != TXT_OS_TO_MLE_DATA_STRUCT_SIGNATURE) {
    return NULL;
  }
  return (VOID *)(UINTN)TxtOsToMleDataStruct->MlePrivateDataAddress;
}

/**

  This function set MLE private data.

  @param MLE private data

**/
VOID
SetMlePrivateData(
  IN MLE_PRIVATE_DATA  *MlePrivateData
  )
{
  TXT_OS_TO_MLE_DATA_STRUCT          *TxtOsToMleDataStruct;

  TxtOsToMleDataStruct = GetTxtOsToMleData();

  if (TxtOsToMleDataStruct->Signature != TXT_OS_TO_MLE_DATA_STRUCT_SIGNATURE) {
    TxtOsToMleDataStruct->Signature = TXT_OS_TO_MLE_DATA_STRUCT_SIGNATURE;
    TxtOsToMleDataStruct->MlePrivateDataAddress = (UINT64)(UINTN)MlePrivateData;
  }
}

/**

  This fuction is entrypoint of DCE.

  @retval EFI_SUCCESS           load Dce successfully
  @retval EFI_UNSUPPORTED       not support load Dce

**/
EFI_STATUS
DceLoaderEntrypoint (
  VOID
  )
{
  EFI_STATUS                Status;
  DCE_PRIVATE_DATA          *DcePrivateData;
  MLE_PRIVATE_DATA          *MlePrivateData;

  MlePrivateData = GetMlePrivateData();
  if (MlePrivateData == NULL) {
    MlePrivateData = &mMlePrivateData;
    SetMlePrivateData(MlePrivateData);
  }

  DcePrivateData = &MlePrivateData->DcePrivateData;
  DcePrivateData->Signature = DCE_PRIVATE_DATA_SIGNATURE;
  DcePrivateData->TpmType        = mCommunicationData.TpmType;
  DcePrivateData->ActivePcrBanks = mCommunicationData.ActivePcrBanks;

  DcePrivateData->MeasuredImageSize = mCommunicationData.MeasuredImageSize;

  DcePrivateData->ImageBase    = mCommunicationData.ImageBase;
  DcePrivateData->ImageSize    = mCommunicationData.ImageSize;
  DcePrivateData->LcpPoBase    = mCommunicationData.LcpPoBase;
  DcePrivateData->LcpPoSize    = mCommunicationData.LcpPoSize;
  DcePrivateData->DprBase      = mCommunicationData.DprBase;
  DcePrivateData->DprSize      = mCommunicationData.DprSize;
  DcePrivateData->PmrLowBase   = mCommunicationData.PmrLowBase;
  DcePrivateData->PmrLowSize   = mCommunicationData.PmrLowSize;
  DcePrivateData->PmrHighBase  = mCommunicationData.PmrHighBase;
  DcePrivateData->PmrHighSize  = mCommunicationData.PmrHighSize;
  DcePrivateData->EventLogBase = mCommunicationData.EventLogBase;
  DcePrivateData->EventLogSize = mCommunicationData.EventLogSize;

  Status = DceEntryPoint ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}
