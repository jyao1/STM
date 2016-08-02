/** @file

  Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Base.h>
#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/TimerLib.h>

#include "DrtmTpm.h"
#include <Library/IoLib.h>
#include <Library/PcdLib.h>

#include "Dce.h"
#include "FrmCommon.h"

/**

  This function request use TPM.

  @param  Locality         TPM locality

  @retval EFI_SUCCESS        Command send successfully
  @retval EFI_DEVICE_ERROR   Something wrong with TPM

**/
EFI_STATUS
TpmCommRequestUseTpm(
  IN      TPM_LOCALITY_SELECTION    Locality
  )
{
  MLE_PRIVATE_DATA          *MlePrivateData;
  DCE_PRIVATE_DATA          *DcePrivateData;

  MlePrivateData = GetMlePrivateData();
  DcePrivateData = &MlePrivateData->DcePrivateData;

  PcdSet64 (PcdTpmBaseAddress, TPM_BASE_ADDRESS + TPM_ACCESS(Locality));

  switch (DcePrivateData->TpmType) {
  case FRM_TPM_TYPE_TPM12:
    return Tpm12RequestUseTpm ();
    break;
  case FRM_TPM_TYPE_TPM2:
    return Tpm2RequestUseTpm ();
    break;
  }
  return EFI_NOT_FOUND;
}

/**

  This function relinquish locality.

  @param  Locality         TPM locality

  @retval EFI_SUCCESS        Command send successfully
  @retval EFI_DEVICE_ERROR   Something wrong with TPM

**/
EFI_STATUS
TpmCommRelinquishLocality (
  IN      TPM_LOCALITY_SELECTION    Locality
  )
{
  MLE_PRIVATE_DATA          *MlePrivateData;
  DCE_PRIVATE_DATA          *DcePrivateData;

  MlePrivateData = GetMlePrivateData();
  DcePrivateData = &MlePrivateData->DcePrivateData;

  PcdSet64 (PcdTpmBaseAddress, TPM_BASE_ADDRESS + TPM_ACCESS(Locality));

  switch (DcePrivateData->TpmType) {
  case FRM_TPM_TYPE_TPM12:
    return Tpm12RelinquishTpm ();
    break;
  case FRM_TPM_TYPE_TPM2:
    return Tpm2RelinquishTpm ();
    break;
  }
  return EFI_NOT_FOUND;
}


/**

  This function log TPM12 event into log area.

  @param PcrIndex   event PCR index
  @param EventType  event type
  @param Digest     event digest
  @param EventSize  event size
  @param EventData  event data

  @retval EFI_SUCCESS           log is added
  @retval EFI_OUT_OF_RESOURCES  not enough memory for log

**/
EFI_STATUS
Tpm12LogEvent (
  IN TPM_PCRINDEX PcrIndex,
  IN UINT32       EventType,
  IN TPM_DIGEST   *Digest,
  IN UINT32       EventSize,
  IN UINT8        *EventData
  )
{
  MLE_PRIVATE_DATA                      *MlePrivateData;
  DCE_PRIVATE_DATA                      *DcePrivateData;
  TXT_HEAP_EVENTLOG_EXT_ELEMENT         *EventLogElement;
  TXT_EVENT_LOG_CONTAINER               *EventLog;
  TCG_PCR_EVENT_HDR                     *Event;
  UINTN                                 ThisEventLogSize;

  MlePrivateData = GetMlePrivateData();
  DcePrivateData = &MlePrivateData->DcePrivateData;

  ASSERT(DcePrivateData->TpmType == FRM_TPM_TYPE_TPM12);

  EventLogElement = DcePrivateData->EventLogElement;
  EventLog = (TXT_EVENT_LOG_CONTAINER *)(UINTN)EventLogElement->EventLogAddress;

  ASSERT(EventLog->NextEventOffset <= EventLog->Size);

  ThisEventLogSize = sizeof(TCG_PCR_EVENT_HDR) + EventSize;

  if (EventLog->Size - EventLog->NextEventOffset < ThisEventLogSize) {
    return EFI_OUT_OF_RESOURCES;
  }
  Event = (TCG_PCR_EVENT_HDR *)((UINTN)EventLogElement->EventLogAddress + EventLog->NextEventOffset);

  // Fill info
  Event->PCRIndex = (UINT16)PcrIndex;
  Event->EventType = EventType;
  CopyMem(&Event->Digest, Digest, sizeof(Event->Digest));
  Event->EventSize = EventSize;
  if (EventSize != 0) {
    CopyMem (Event + 1, Event, EventSize);
  }

  // Update event record
  EventLog->NextEventOffset = (UINT32)(EventLog->NextEventOffset + ThisEventLogSize);
  return EFI_SUCCESS;
}

/**

  This function log TPM2 event into log area.

  @param PcrIndex   event PCR index
  @param EventType  event type
  @param Digest     event digest
  @param EventSize  event size
  @param EventData  event data

  @retval EFI_SUCCESS           log is added
  @retval EFI_OUT_OF_RESOURCES  not enough memory for log

**/
EFI_STATUS
Tpm2LogEvent(
  IN TPMI_DH_PCR          PcrIndex,
  IN UINT32               EventType,
  IN TPML_DIGEST_VALUES   *DigestList,
  IN UINT32               EventSize,
  IN UINT8                *EventData
  )
{
  MLE_PRIVATE_DATA                      *MlePrivateData;
  DCE_PRIVATE_DATA                      *DcePrivateData;
  TXT_HEAP_EVENT_LOG_POINTER_ELEMENT2   *EventLogPointerElement2;
  TXT_HEAP_EVENT_LOG_POINTER_ELEMENT2_1 *EventLogPointerElement2_1;
  TXT_HEAP_EVENT_LOG_DESCR              *EventLogDesc;
  TCG_LOG_DESCRIPTOR                    *TcgLogDesc;
  TCG_PCR_EVENT_EX                      *EventEx;
  TCG_PCR_EVENT2                        *TcgPcrEvent2;
  UINTN                                 Index;
  UINTN                                 EventIndex;
  UINTN                                 DigestSize;
  UINTN                                 ThisEventLogSize;
  UINT8                                 *Buffer;

  MlePrivateData = GetMlePrivateData();
  DcePrivateData = &MlePrivateData->DcePrivateData;

  ASSERT(DcePrivateData->TpmType == FRM_TPM_TYPE_TPM2);

  if ((DcePrivateData->AcmCapabilities & TXT_MLE_SINIT_CAPABILITY_TCG2_COMPATIBILE_EVENTLOG) == 0) {
    EventLogPointerElement2 = DcePrivateData->EventLogPointerElement2;

    for (Index = 0; Index < DigestList->count; Index++) {
      EventLogDesc = (TXT_HEAP_EVENT_LOG_DESCR *)(EventLogPointerElement2 + 1);
      for (EventIndex = 0; EventIndex < EventLogPointerElement2->Count; EventIndex++, EventLogDesc++) {
        if (DigestList->digests[Index].hashAlg == EventLogDesc->HashAlgID) {
          ASSERT(EventLogDesc->NextRecordOffset <= EventLogDesc->AllocatedEventContainerSize);

          TcgLogDesc = (TCG_LOG_DESCRIPTOR *)((UINTN)EventLogDesc->PhysicalAddress + sizeof(TCG_PCR_EVENT_HDR));
          ASSERT(TcgLogDesc->DigestSize == GetHashSizeFromAlgo(DigestList->digests[Index].hashAlg));

          ThisEventLogSize = sizeof(TCG_PCR_EVENT_EX) + TcgLogDesc->DigestSize + sizeof(UINT32) + EventSize;

          if (EventLogDesc->AllocatedEventContainerSize - EventLogDesc->NextRecordOffset < ThisEventLogSize) {
            return EFI_OUT_OF_RESOURCES;
          }

          EventEx = (TCG_PCR_EVENT_EX *)((UINTN)EventLogDesc->PhysicalAddress + EventLogDesc->NextRecordOffset);

          // Fill info
          EventEx->PCRIndex = PcrIndex;
          EventEx->EventType = EventType;
          Buffer = (UINT8 *)(EventEx + 1);
          CopyMem(Buffer, &DigestList->digests[Index].digest, TcgLogDesc->DigestSize);
          Buffer += TcgLogDesc->DigestSize;
          CopyMem(Buffer, &EventSize, sizeof(UINT32));
          Buffer += sizeof(UINT32);
          if (EventSize != 0) {
            CopyMem(Buffer, EventData, EventSize);
          }

          // Update event record
          EventLogDesc->NextRecordOffset = (UINT32)(EventLogDesc->NextRecordOffset + ThisEventLogSize);
        }
      }
    }

  } else {
    EventLogPointerElement2_1 = DcePrivateData->EventLogPointerElement2_1;

    ThisEventLogSize = sizeof(TCG_PCRINDEX) + sizeof(TCG_EVENTTYPE) + sizeof(UINT32) + EventSize;

    ThisEventLogSize += sizeof(DigestList->count);
    for (Index = 0; Index < DigestList->count; Index++) {
      ThisEventLogSize += sizeof(DigestList->digests[Index].hashAlg);
      ThisEventLogSize += GetHashSizeFromAlgo(DigestList->digests[Index].hashAlg);
    }

    if (EventLogPointerElement2_1->AllocatedEventContainerSize - EventLogPointerElement2_1->NextRecordOffset < ThisEventLogSize) {
      return EFI_OUT_OF_RESOURCES;
    }

    TcgPcrEvent2 = (TCG_PCR_EVENT2 *)((UINTN)EventLogPointerElement2_1->PhysicalAddress + EventLogPointerElement2_1->NextRecordOffset);

    // Fill info
    TcgPcrEvent2->PCRIndex = PcrIndex;
    TcgPcrEvent2->EventType = EventType;
    Buffer = (UINT8 *)&TcgPcrEvent2->Digest;
    
    CopyMem(Buffer, &DigestList->count, sizeof(DigestList->count));
    Buffer = (UINT8 *)Buffer + sizeof(DigestList->count);
    for (Index = 0; Index < DigestList->count; Index++) {
      CopyMem(Buffer, &DigestList->digests[Index].hashAlg, sizeof(DigestList->digests[Index].hashAlg));
      Buffer = (UINT8 *)Buffer + sizeof(DigestList->digests[Index].hashAlg);
      DigestSize = GetHashSizeFromAlgo(DigestList->digests[Index].hashAlg);
      CopyMem(Buffer, &DigestList->digests[Index].digest, DigestSize);
      Buffer = (UINT8 *)Buffer + DigestSize;
    }

    CopyMem(Buffer, &EventSize, sizeof(UINT32));
    Buffer += sizeof(UINT32);
    if (EventSize != 0) {
      CopyMem(Buffer, EventData, EventSize);
    }

    // Update event record
    EventLogPointerElement2_1->NextRecordOffset = (UINT32)(EventLogPointerElement2_1->NextRecordOffset + ThisEventLogSize);
  }

  return EFI_SUCCESS;
}

/**

  This function dump TPM PCR value.

  @param  Locality          TPM locality
  @param  PcrIndex          PCR index

**/
VOID
DumpPcr (
  IN TPM_LOCALITY_SELECTION    Locality,
  IN TPM_PCRINDEX              PcrIndex
  )
{
  TCG_DIGEST                PcrValue;
  TPML_PCR_SELECTION        PcrSelectionIn;
  UINT32                    PcrUpdateCounter;
  TPML_PCR_SELECTION        PcrSelectionOut;
  TPML_DIGEST               PcrValues;
  UINTN                     Index;
  UINTN                     HashIndex;
  EFI_STATUS                Status;
  MLE_PRIVATE_DATA          *MlePrivateData;
  DCE_PRIVATE_DATA          *DcePrivateData;

  MlePrivateData = GetMlePrivateData();
  DcePrivateData = &MlePrivateData->DcePrivateData;

  PcdSet64 (PcdTpmBaseAddress, TPM_BASE_ADDRESS + TPM_ACCESS(Locality));

  switch (DcePrivateData->TpmType) {
  case FRM_TPM_TYPE_TPM12:
    Status = Tpm12PcrRead (PcrIndex, &PcrValue);
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "(TPM) Tpm12PcrRead (%d) - %r\n", (UINTN)PcrIndex, Status));
      return ;
    }
    DEBUG ((EFI_D_INFO, "(TPM) PCR[%d] - ", (UINTN)PcrIndex));
    DumpData ((UINT8 *)&PcrValue, sizeof(PcrValue));
    DEBUG ((EFI_D_INFO, "\n"));
    break;
  case FRM_TPM_TYPE_TPM2:
    for (HashIndex = 0; HashIndex < DcePrivateData->EventHashAlgoIDCount; HashIndex++) {
      PcrSelectionIn.count = 1;
      PcrSelectionIn.pcrSelections[0].hash = DcePrivateData->EventHashAlgoID[HashIndex];
      PcrSelectionIn.pcrSelections[0].sizeofSelect = PCR_SELECT_MAX;
      PcrSelectionIn.pcrSelections[0].pcrSelect[PcrIndex / 8] = (1 << (PcrIndex % 8));
      Status = Tpm2PcrRead (&PcrSelectionIn, &PcrUpdateCounter, &PcrSelectionOut, &PcrValues);
      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "(TPM) Tpm2PcrRead (%d) (Hash:0x%x) - %r\n", (UINTN)PcrIndex, DcePrivateData->EventHashAlgoID[HashIndex], Status));
        return ;
      }
      for (Index = 0; Index < PcrValues.count; Index++) {
        DEBUG ((EFI_D_INFO, "(TPM) PCR[%d] (Hash:0x%x) - ", PcrIndex, DcePrivateData->EventHashAlgoID[HashIndex]));
        DumpData ((UINT8 *)&PcrValues.digests[Index].buffer, PcrValues.digests[Index].size);
        DEBUG ((EFI_D_INFO, "\n"));
      }
	}
    break;
  }

  return ;
}

/**

  This function cap TPM PCR value.

  @param  Locality          TPM locality
  @param  PcrIndex          PCR index

**/
VOID
CapPcr (
  IN TPM_LOCALITY_SELECTION    Locality,
  IN TPM_PCRINDEX              PcrIndex
  )
{
  TPM_NONCE                 NonceData;
  TPM_DIGEST                DigestToEntend;
  TPM2B_DIGEST              RandomBytes;
  TPML_DIGEST_VALUES        DigestList;
  MLE_PRIVATE_DATA          *MlePrivateData;
  DCE_PRIVATE_DATA          *DcePrivateData;

  MlePrivateData = GetMlePrivateData();
  DcePrivateData = &MlePrivateData->DcePrivateData;

  PcdSet64 (PcdTpmBaseAddress, TPM_BASE_ADDRESS + TPM_ACCESS(Locality));

  switch (DcePrivateData->TpmType) {
  case FRM_TPM_TYPE_TPM12:
    Tpm12GetRandom(sizeof(NonceData), (UINT8 *)&NonceData);
    Tpm12HashAndExtend(PcrIndex, (UINT8 *)&NonceData, sizeof(NonceData), &DigestToEntend);
    Tpm12LogEvent(PcrIndex, TXT_EVTYPE_CAP_VALUE, &DigestToEntend, 0, NULL);
    break;
  case FRM_TPM_TYPE_TPM2:
    Tpm2GetRandom(sizeof(RandomBytes.buffer), &RandomBytes);
    HashAndExtend (PcrIndex, (UINT8 *)&RandomBytes.buffer, sizeof(RandomBytes.buffer), &DigestList);
    Tpm2LogEvent(PcrIndex, TXT_EVTYPE_CAP_VALUE, &DigestList, 0, NULL);
    break;
  }

  return ;
}

/**

  This function cap sensitive TPM PCR values.

**/
VOID
CapPcrs (
  VOID
  )
{
  TpmCommRelinquishLocality (TPM_LOCALITY_0);
  TpmCommRequestUseTpm (TPM_LOCALITY_2);

  DEBUG ((EFI_D_INFO, "(TPM) PCRs before cap:\n"));
  DumpPcr (TPM_LOCALITY_2, 17);
  DumpPcr (TPM_LOCALITY_2, 18);
  DumpPcr (TPM_LOCALITY_2, 19);

  CapPcr (TPM_LOCALITY_2, 17);
  CapPcr (TPM_LOCALITY_2, 18);
  CapPcr (TPM_LOCALITY_2, 19);

  DEBUG ((EFI_D_INFO, "(TPM) PCRs after cap:\n"));
  DumpPcr (TPM_LOCALITY_2, 17);
  DumpPcr (TPM_LOCALITY_2, 18);
  DumpPcr (TPM_LOCALITY_2, 19);

  TpmCommRelinquishLocality (TPM_LOCALITY_2);

  return ;
}


/**

  This function extend required items to PCR17.

  //
  // PCR17:
  // PCR.Details ¨C Used to record detail measurements of all components
  // involved in the D-RTM process, including components provided by the
  // chipset manufacturer, the platform manufacturer and the operating system
  // vendor.
  //

  @param  Locality         TPM locality

**/
VOID
Pcr17Measurement (
  IN TPM_LOCALITY_SELECTION    Locality
  )
{
  TPM_DIGEST                DigestToEntend;
  TPML_DIGEST_VALUES        DigestList;
  MLE_PRIVATE_DATA          *MlePrivateData;
  DCE_PRIVATE_DATA          *DcePrivateData;

  MlePrivateData = GetMlePrivateData();
  DcePrivateData = &MlePrivateData->DcePrivateData;

  PcdSet64(PcdTpmBaseAddress, TPM_BASE_ADDRESS + TPM_ACCESS(Locality));

  //
  // BUGBUG: Only show some sample code
  //
  switch (DcePrivateData->TpmType) {
  case FRM_TPM_TYPE_TPM12:
    Tpm12HashAndExtend(17, (UINT8 *)"TestFrm PCR.Details", sizeof("TestFrm PCR.Details"), &DigestToEntend);
    Tpm12LogEvent(17, EV_EVENT_TAG, &DigestToEntend, sizeof("TestFrm PCR.Details"), (UINT8 *)"TestFrm PCR.Details");
    break;
  case FRM_TPM_TYPE_TPM2:
    HashAndExtend(17, (UINT8 *)"TestFrm PCR.Details", sizeof("TestFrm PCR.Details"), &DigestList);
    Tpm2LogEvent(17, EV_EVENT_TAG, &DigestList, sizeof("TestFrm PCR.Details"), (UINT8 *)"TestFrm PCR.Details");
    break;
  }

  return ;
}

/**

  This function extend required items to PCR18.

  //
  // PCR18:
  // PCR.Authorities ¨C Used to record the authority measurements of the
  // chipset manufacturer components and the platform manufacturer components.
  // These measurements may include a special well known value for the case
  // the intended validation steps occur for the current firmware on the
  // hardware platform.  If some components in the D-RTM process are not
  // signed, well known error codes will be recorded in this PCR to invalidate
  // its measurements.
  //

  @param  Locality         TPM locality

**/
VOID
Pcr18Measurement (
  IN TPM_LOCALITY_SELECTION    Locality
  )
{
  TPM_DIGEST                DigestToEntend;
  TPML_DIGEST_VALUES        DigestList;
  MLE_PRIVATE_DATA          *MlePrivateData;
  DCE_PRIVATE_DATA          *DcePrivateData;

  MlePrivateData = GetMlePrivateData();
  DcePrivateData = &MlePrivateData->DcePrivateData;

  PcdSet64(PcdTpmBaseAddress, TPM_BASE_ADDRESS + TPM_ACCESS(Locality));

  //
  // BUGBUG: Only show some sample code
  //
  switch (DcePrivateData->TpmType) {
  case FRM_TPM_TYPE_TPM12:
    Tpm12HashAndExtend(18, (UINT8 *)"TestFrm PCR.Authorities", sizeof("TestFrm PCR.Authorities"), &DigestToEntend);
    Tpm12LogEvent(18, EV_EVENT_TAG, &DigestToEntend, sizeof("TestFrm PCR.Authorities"), (UINT8 *)"TestFrm PCR.Authorities");
    break;
  case FRM_TPM_TYPE_TPM2:
    HashAndExtend(18, (UINT8 *)"TestFrm PCR.Authorities", sizeof("TestFrm PCR.Authorities"), &DigestList);
    Tpm2LogEvent(18, EV_EVENT_TAG, &DigestList, sizeof("TestFrm PCR.Authorities"), (UINT8 *)"TestFrm PCR.Authorities");
    break;
  }

  return;
}

/**

  This function extend required items to PCR19.

  //
  // PCR19:
  // PCR.DLME ¨C Used to record the authority of the DLME launched at the end
  // of the D-RTM process.  There are advantages to separating the authority
  // measurement of the DLME from the authority measurement for the chipset
  // manufacturer and the platform manufacturer. An example advantage is a
  // platform manufacturer may seal data to the PCR.Authorities without regard
  // for what DLME may access the data.
  //

  @param  Locality         TPM locality

**/
VOID
Pcr19Measurement (
  IN TPM_LOCALITY_SELECTION    Locality
  )
{
  TPM_DIGEST                DigestToEntend;
  TPML_DIGEST_VALUES        DigestList;
  MLE_PRIVATE_DATA          *MlePrivateData;
  DCE_PRIVATE_DATA          *DcePrivateData;

  MlePrivateData = GetMlePrivateData();
  DcePrivateData = &MlePrivateData->DcePrivateData;

  PcdSet64(PcdTpmBaseAddress, TPM_BASE_ADDRESS + TPM_ACCESS(Locality));

  //
  // BUGBUG: Only show some sample code
  //
  switch (DcePrivateData->TpmType) {
  case FRM_TPM_TYPE_TPM12:
    Tpm12HashAndExtend(19, (UINT8 *)"TestFrm PCR.DLME", sizeof("TestFrm PCR.DLME"), &DigestToEntend);
    Tpm12LogEvent(19, EV_EVENT_TAG, &DigestToEntend, sizeof("TestFrm PCR.DLME"), (UINT8 *)"TestFrm PCR.DLME");
    break;
  case FRM_TPM_TYPE_TPM2:
    HashAndExtend(19, (UINT8 *)"TestFrm PCR.DLME", sizeof("TestFrm PCR.DLME"), &DigestList);
    Tpm2LogEvent(19, EV_EVENT_TAG, &DigestList, sizeof("TestFrm PCR.DLME"), (UINT8 *)"TestFrm PCR.DLME");
    break;
  }

  return;
}

/**

  This function extend required items to PCRs.

**/
VOID
DoMeasurement (
  VOID
  )
{
  TpmCommRelinquishLocality (TPM_LOCALITY_0);
  TpmCommRequestUseTpm (TPM_LOCALITY_2);

  DEBUG ((EFI_D_INFO, "(TPM) PCRs before extend:\n"));
  DumpPcr (TPM_LOCALITY_2, 17);
  DumpPcr (TPM_LOCALITY_2, 18);
  DumpPcr (TPM_LOCALITY_2, 19);

  Pcr17Measurement (TPM_LOCALITY_2);
  Pcr18Measurement (TPM_LOCALITY_2);
  Pcr19Measurement (TPM_LOCALITY_2);

  DEBUG ((EFI_D_INFO, "(TPM) PCRs after extend:\n"));
  DumpPcr (TPM_LOCALITY_2, 17);
  DumpPcr (TPM_LOCALITY_2, 18);
  DumpPcr (TPM_LOCALITY_2, 19);

  TpmCommRelinquishLocality (TPM_LOCALITY_2);

  return;
}

