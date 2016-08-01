/** @file
  DlEntry AP wakeup

  Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Dce.h"

extern UINT32 PostInitAddrRlp;

/**

  This is AP wakeup entrypoint.

**/
VOID
AsmApWakeup (
  VOID
  );

extern UINT32 mApGdtrOffset;
extern UINT32 mApGdtBase;
extern UINT32 mCodeSel;

/**

  This function return RLP protected mode CS.

  @retval RLP protected mode CS

**/
UINT32
GetProtectedModeCs (
  VOID
  )
{
  return mCodeSel;
}

/**

  This function wait up RLP.

  @param ApEntry   AP entrypoint

**/
VOID
TxtWakeUpRlps (
  IN UINT32 ApEntry
  )
{
  TXT_SINIT_TO_MLE_DATA             *SinitMleData;
  MLE_PRIVATE_DATA                  *MlePrivateData;
  volatile UINTN                    RlpInitializedNumber;
  TXT_ACM_FORMAT                    *SinitAcm;
  TXT_CHIPSET_ACM_INFORMATION_TABLE *ChipsetAcmInformationTable;
  UINTN                             LinearEntryPoint;
  IA32_DESCRIPTOR                   *ApGdtr;

  SinitMleData = GetTxtSinitToMleData ();
  MlePrivateData    = GetMlePrivateData ();

  DEBUG((EFI_D_INFO, "(TXT) AsmApWakeup - %08x\n", (UINTN)AsmApWakeup));
  DEBUG((EFI_D_INFO, "(TXT) mApGdtrOffset - %08x\n", (UINTN)mApGdtrOffset));
  DEBUG((EFI_D_INFO, "(TXT) mApGdtBase - %08x\n", (UINTN)mApGdtBase));

  ApGdtr = (IA32_DESCRIPTOR *)((UINTN)AsmApWakeup + mApGdtrOffset);
  ApGdtr->Base = (UINTN)AsmApWakeup + mApGdtBase;

  DEBUG((EFI_D_INFO, "(TXT) ApGdtr - %08x\n", (UINTN)ApGdtr));
  DEBUG((EFI_D_INFO, "(TXT) ApGdtr->Limit - %08x\n", (UINTN)ApGdtr->Limit));
  DEBUG((EFI_D_INFO, "(TXT) ApGdtr->Base - %08x\n", (UINTN)ApGdtr->Base));

  MlePrivateData->MleJoinData.GDTLimit         = (UINT32)ApGdtr->Limit;
  MlePrivateData->MleJoinData.GDTBasePtr       = (UINT32)ApGdtr->Base;
  MlePrivateData->MleJoinData.Cs               = GetProtectedModeCs();
  LinearEntryPoint                        = (UINTN)(AsmRlpWakeUpCode);
  MlePrivateData->MleJoinData.LinearEntryPoint = (UINT32)LinearEntryPoint;

  DEBUG ((EFI_D_INFO, "(TXT) TXT Wakeup APs - %08x\n", (UINTN)&MlePrivateData->MleJoinData));
  DEBUG ((EFI_D_INFO, "(TXT)   GDTLimit         - %08x\n", (UINTN)MlePrivateData->MleJoinData.GDTLimit));
  DEBUG ((EFI_D_INFO, "(TXT)   GDTBasePtr       - %08x\n", (UINTN)MlePrivateData->MleJoinData.GDTBasePtr));
  DEBUG ((EFI_D_INFO, "(TXT)   Cs               - %08x\n", (UINTN)MlePrivateData->MleJoinData.Cs));
  DEBUG ((EFI_D_INFO, "(TXT)   LinearEntryPoint - %08x\n", (UINTN)MlePrivateData->MleJoinData.LinearEntryPoint));

  TxtPubWrite64 (TXT_MLE_JOIN, (UINT64)(UINTN)&MlePrivateData->MleJoinData);

  MlePrivateData->ApEntry = ApEntry;
  MlePrivateData->TempEspRlp = (UINT32)((UINTN)GetTxtHeap () + GetTxtHeapSize () - 0x20);

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
  if (PostInitAddrRlp != 0) {
    MlePrivateData->RlpPostSinitOffset  = (UINT32)((UINTN)AsmRlpWakeUpCode + PostInitAddrRlp);
    MlePrivateData->RlpPostSinitSegment = (UINT32)AsmReadCs();
  }
  MlePrivateData->RlpDs = AsmReadDs ();

  SinitAcm = (TXT_ACM_FORMAT *)(UINTN)TxtPubRead32 (TXT_SINIT_BASE);
  ChipsetAcmInformationTable = (TXT_CHIPSET_ACM_INFORMATION_TABLE *) \
                               ((UINTN)(SinitAcm + 1) + 
                               SinitAcm->KeySize * 4 + 
                               sizeof(UINT32) + 
                               ACM_PKCS_1_5_RSA_SIGNATURE_SIZE + 
                               SinitAcm->ScratchSize * 4
                               );

  DEBUG ((EFI_D_INFO, "(TXT) WAKEUP ...\n"));

  //
  // Wake up it
  //
  if ((SinitMleData->RlpWakeupAddr != 0) && ((ChipsetAcmInformationTable->Capabilities & TXT_MLE_SINIT_CAPABILITY_MONITOR_ADDRESS_RLP_WAKEUP) != 0)) {
    *(volatile UINT32 *)(UINTN)SinitMleData->RlpWakeupAddr = 1;
  } else {
    AsmGetSecWakeup();
  }

  //
  // Check the wakeup status
  //
  RlpInitializedNumber = MlePrivateData->RlpInitializedNumber;
  while ((RlpInitializedNumber + 1) != MlePrivateData->CpuNum)  {
    // wait
    RlpInitializedNumber = MlePrivateData->RlpInitializedNumber;
  }

  DEBUG ((EFI_D_INFO, "(TXT) WAKEUP Done!\n"));

  return ;
}
