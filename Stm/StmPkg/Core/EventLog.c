/** @file
  STM event log

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Stm.h"

/**

  This function clear event log.

  @param EventLog Event log structure

**/
VOID
ClearEventLog (
  IN MLE_EVENT_LOG_STRUCTURE   *EventLog
  )
{
  UINT32 Index;

  for (Index = 0; Index < EventLog->PageCount; Index++) {
    ZeroMem ((VOID *)(UINTN)EventLog->Pages[Index], STM_PAGES_TO_SIZE(1));
  }
  return ;
}

/**

  This function get next empty log entry.

  @param EventLog Event log structure

  @return next empty log entry

**/
STM_LOG_ENTRY *
GetNextEmpty (
  IN MLE_EVENT_LOG_STRUCTURE   *EventLog
  )
{
  STM_LOG_ENTRY             *LogEntry;
  UINTN                     EventIndex;
  UINT32                    Index;

  //
  // Or just use EventSerialNumber as Index?
  //

  //
  // Find invalid entry
  //
  for (Index = 0; Index < EventLog->PageCount; Index++) {
    LogEntry = (STM_LOG_ENTRY *)(UINTN)EventLog->Pages[Index];
    for (EventIndex = 0; EventIndex < STM_PAGES_TO_SIZE(1) / STM_LOG_ENTRY_SIZE; EventIndex++, LogEntry = (STM_LOG_ENTRY *)((UINTN)LogEntry + STM_LOG_ENTRY_SIZE)) {
      if (AsmTestAndSet (16, &LogEntry->Hdr.Type) == 1) {
        continue;
      }
      if (LogEntry->Hdr.Valid == 0) {
        return LogEntry;
      } else {
        AsmTestAndReset (16, &LogEntry->Hdr.Type);
      }
    }
  }
  //
  // If we come here no empty slot is found, so we try to find the record ReadByMle
  //
  for (Index = 0; Index < EventLog->PageCount; Index++) {
    LogEntry = (STM_LOG_ENTRY *)(UINTN)EventLog->Pages[Index];
    for (EventIndex = 0; EventIndex < STM_PAGES_TO_SIZE(1) / STM_LOG_ENTRY_SIZE; EventIndex++, LogEntry = (STM_LOG_ENTRY *)((UINTN)LogEntry + STM_LOG_ENTRY_SIZE)) {
      if (AsmTestAndSet (16, &LogEntry->Hdr.Type) == 1) {
        continue;
      }
      if (LogEntry->Hdr.ReadByMle == 1) {
        return LogEntry;
      } else {
        AsmTestAndReset (16, &LogEntry->Hdr.Type);
      }
    }
  }
  //
  // No lucky. We have to override an event not read by MLE
  //
  for (Index = 0; Index < EventLog->PageCount; Index++) {
    LogEntry = (STM_LOG_ENTRY *)(UINTN)EventLog->Pages[Index];
    for (EventIndex = 0; EventIndex < STM_PAGES_TO_SIZE(1) / STM_LOG_ENTRY_SIZE; EventIndex++, LogEntry = (STM_LOG_ENTRY *)((UINTN)LogEntry + STM_LOG_ENTRY_SIZE)) {
      if (AsmTestAndSet (16, &LogEntry->Hdr.Type) == 1) {
        continue;
      }
      if (LogEntry->Hdr.Wrapped == 0) {
        return LogEntry;
      } else {
        AsmTestAndReset (16, &LogEntry->Hdr.Type);
      }
    }
  }
  //
  // Ohh, all the record are comsumed
  //
  for (Index = 0; Index < EventLog->PageCount; Index++) {
    LogEntry = (STM_LOG_ENTRY *)(UINTN)EventLog->Pages[Index];
    for (EventIndex = 0; EventIndex < STM_PAGES_TO_SIZE(1) / STM_LOG_ENTRY_SIZE; EventIndex++, LogEntry = (STM_LOG_ENTRY *)((UINTN)LogEntry + STM_LOG_ENTRY_SIZE)) {
      if (AsmTestAndSet (16, &LogEntry->Hdr.Type) == 1) {
        continue;
      }
      return LogEntry;
    }
  }
  //
  // No chance. :-(
  //
  return NULL;
}

/**

  This function add event log.

  @param EventType        Event type
  @param LogEntryData     Log entry data
  @param LogEntryDataSize Log entry data size
  @param EventLog         Event log structure

**/
VOID
AddEventLog (
  IN UINT16                    EventType,
  IN LOG_ENTRY_DATA            *LogEntryData,
  IN UINTN                     LogEntryDataSize,
  IN MLE_EVENT_LOG_STRUCTURE   *EventLog
  )
{
  STM_LOG_ENTRY             *LogEntry;

  if (EventLog->State != EvtLogStarted) {
    return ;
  }
  if ((EventLog->EventEnableBitmap & (1 << EventType)) == 0) {
    return ;
  }
  //
  // Check total length, truncate any STM_LOG_ENTRY that is greater than 256 bytes.
  //
  if (sizeof(LogEntry->Hdr) + LogEntryDataSize > STM_LOG_ENTRY_SIZE) {
    LogEntryDataSize =  STM_LOG_ENTRY_SIZE - sizeof(LogEntry->Hdr);
  }

  AcquireSpinLock (&mHostContextCommon.EventLog.EventLogLock);

  LogEntry = GetNextEmpty (EventLog);
  if (LogEntry == NULL) {
    ReleaseSpinLock (&mHostContextCommon.EventLog.EventLogLock);
    return ;
  }
  if ((LogEntry->Hdr.Valid == 1) && (LogEntry->Hdr.ReadByMle == 0)) {
    LogEntry->Hdr.Wrapped = 1;
  } else {
    LogEntry->Hdr.Wrapped = 0;
  }
  LogEntry->Hdr.Valid = 1;
  LogEntry->Hdr.ReadByMle = 0;

  LogEntry->Hdr.EventSerialNumber = EventLog->EventSerialNumber ++;
  LogEntry->Hdr.Type = EventType;
  CopyMem (&LogEntry->Data, LogEntryData, LogEntryDataSize);
  AsmTestAndReset (16, &LogEntry->Hdr.Type);

  ReleaseSpinLock (&mHostContextCommon.EventLog.EventLogLock);
  return ;
}

/**

  This function add event log for invalid parameter.

  @param VmcallApiNumber    VMCALL API number which caused invalid parameter

**/
VOID
AddEventLogInvalidParameter (
  IN UINT32 VmcallApiNumber
  )
{
  LOG_ENTRY_DATA                     LogEntryData;

  LogEntryData.InvalidParam.VmcallApiNumber    = VmcallApiNumber;
  AddEventLog (EvtLogInvalidParameterDetected, &LogEntryData, sizeof(LogEntryData.InvalidParam), &mHostContextCommon.EventLog);
}

/**

  This function add event log for resource.

  @param EventType   EvtHandledProtectionException, EvtBiosAccessToUnclaimedResource,
                     EvtMleResourceProtectionGranted, EvtMleResourceProtectionDenied,
                     EvtMleResourceUnprotect, EvtMleResourceUnprotectError
  @param Resource    STM Resource

**/
VOID
AddEventLogForResource (
  IN EVENT_TYPE  EventType,
  IN STM_RSC     *Resource
  )
{
  LOG_ENTRY_DATA                     LogEntryData;

  if (!IsResourceNodeValid (Resource, FALSE, TRUE)) {
    DEBUG ((EFI_D_ERROR, "AddEventLogForResource - Invalid Resource!!!\n"));
    return ;
  }

  CopyMem (&LogEntryData.HandledProtectionException.Resource, Resource, Resource->Header.Length);
  AddEventLog (EventType, &LogEntryData, Resource->Header.Length, &mHostContextCommon.EventLog);
}

/**

  This function add event log for domain degration.

  @param VmcsPhysPointer    VmcsPhysPointer
  @param ExpectedDomainType Expected DomainType
  @param DegradedDomainType Degraded DomainType

**/
VOID
AddEventLogDomainDegration (
  IN UINT64 VmcsPhysPointer,
  IN UINT8  ExpectedDomainType,
  IN UINT8  DegradedDomainType
  )
{
  LOG_ENTRY_DATA                     LogEntryData;

  LogEntryData.MleDomainTypeDegraded.VmcsPhysPointer    = VmcsPhysPointer;
  LogEntryData.MleDomainTypeDegraded.ExpectedDomainType = ExpectedDomainType;
  LogEntryData.MleDomainTypeDegraded.DegradedDomainType = DegradedDomainType;
  AddEventLog (EvtMleDomainTypeDegraded, &LogEntryData, sizeof(LogEntryData.MleDomainTypeDegraded), &mHostContextCommon.EventLog);
}

/**

  This function initialize event log.

**/
VOID
InitializeEventLog (
  VOID
  )
{
  InitializeSpinLock (&mHostContextCommon.EventLog.EventLogLock);
  mHostContextCommon.EventLog.State = (UINT32)EvtInvalid;
  mHostContextCommon.EventLog.EventSerialNumber = 0;
  mHostContextCommon.EventLog.EventEnableBitmap = 0;
  mHostContextCommon.EventLog.PageCount = 0;
  mHostContextCommon.EventLog.Pages = AllocatePages (1);
  ZeroMem (mHostContextCommon.EventLog.Pages, STM_PAGES_TO_SIZE(1));
}

/**

  This function dump event log header.

  @param LogEntry    Log entry

**/
VOID
DumpEventLogHeader (
  IN STM_LOG_ENTRY             *LogEntry
  )
{
  DEBUG ((EFI_D_INFO, "  EventSerialNumber : %08x\n", LogEntry->Hdr.EventSerialNumber));
}

/**

  This function dump event log entry.

  @param LogEntry    Log entry

**/
VOID
DumpEventLogEntry (
  IN STM_LOG_ENTRY             *LogEntry
  )
{
  switch (LogEntry->Hdr.Type) {
  case EvtLogStarted:
    DEBUG ((EFI_D_INFO, "EVT_LOG_STARTED:\n"));
    DumpEventLogHeader (LogEntry);
    break;
  case EvtLogStopped:
    DEBUG ((EFI_D_INFO, "EVT_LOG_STOPPED:\n"));
    DumpEventLogHeader (LogEntry);
    break;
  case EvtLogInvalidParameterDetected:
    DEBUG ((EFI_D_INFO, "EVT_LOG_INVALID_PARAMETER_DETECTED:\n"));
    DumpEventLogHeader (LogEntry);
    DEBUG ((EFI_D_INFO, "  VmcallApiNumber   : %08x\n", LogEntry->Data.InvalidParam.VmcallApiNumber));
    break;
  case EvtHandledProtectionException:
    DEBUG ((EFI_D_INFO, "EVT_HANDLED_PROTECTION_EXCEPTION:\n"));
    DumpEventLogHeader (LogEntry);
    DumpStmResourceNode (&LogEntry->Data.HandledProtectionException.Resource);
    break;
  case EvtBiosAccessToUnclaimedResource:
    DEBUG ((EFI_D_INFO, "EVT_BIOS_ACCESS_TO_UNCLAIMED_RESOURCE:\n"));
    DumpEventLogHeader (LogEntry);
    DumpStmResourceNode (&LogEntry->Data.BiosUnclaimedRsc.Resource);
    break;
  case EvtMleResourceProtectionGranted:
    DEBUG ((EFI_D_INFO, "EVT_MLE_RESOURCE_PROTECTION_GRANTED:\n"));
    DumpEventLogHeader (LogEntry);
    DumpStmResourceNode (&LogEntry->Data.MleRscProtGranted.Resource);
    break;
  case EvtMleResourceProtectionDenied:
    DEBUG ((EFI_D_INFO, "EVT_MLE_RESOURCE_PROTECTION_DENIED:\n"));
    DumpEventLogHeader (LogEntry);
    DumpStmResourceNode (&LogEntry->Data.MleRscProtDenied.Resource);
    break;
  case EvtMleResourceUnprotect:
    DEBUG ((EFI_D_INFO, "EVT_MLE_RESOURCE_UNPROTECT:\n"));
    DumpEventLogHeader (LogEntry);
    DumpStmResourceNode (&LogEntry->Data.MleRscUnprot.Resource);
    break;
  case EvtMleResourceUnprotectError:
    DEBUG ((EFI_D_INFO, "EVT_MLE_RESOURCE_UNPROTECT_ERROR:\n"));
    DumpEventLogHeader (LogEntry);
    DumpStmResourceNode (&LogEntry->Data.MleRscUnprotError.Resource);
    break;
  case EvtMleDomainTypeDegraded:
    DEBUG ((EFI_D_INFO, "EVT_MLE_DOMAIN_TYPE_DEGRADED:\n"));
    DumpEventLogHeader (LogEntry);
    DEBUG ((EFI_D_INFO, "  VmcsPhysPointer    : 0x%016lx:\n", LogEntry->Data.MleDomainTypeDegraded.VmcsPhysPointer));
    DEBUG ((EFI_D_INFO, "  ExpectedDomainType : 0x%02x:\n",   (UINTN)LogEntry->Data.MleDomainTypeDegraded.ExpectedDomainType));
    DEBUG ((EFI_D_INFO, "  DegradedDomainType : 0x%02x:\n",   (UINTN)LogEntry->Data.MleDomainTypeDegraded.DegradedDomainType));
    break;
  default:
    break;
  }
}

/**

  This function dump event log.

  @param EventLog Event log structure

**/
VOID
DumpEventLog (
  IN MLE_EVENT_LOG_STRUCTURE   *EventLog
  )
{
  STM_LOG_ENTRY             *LogEntry;
  UINTN                     EventIndex;
  UINT32                    Index;

  for (Index = 0; Index < EventLog->PageCount; Index++) {
    LogEntry = (STM_LOG_ENTRY *)(UINTN)EventLog->Pages[Index];
    for (EventIndex = 0; EventIndex < STM_PAGES_TO_SIZE(1) / STM_LOG_ENTRY_SIZE; EventIndex++, LogEntry = (STM_LOG_ENTRY *)((UINTN)LogEntry + STM_LOG_ENTRY_SIZE)) {
      if (LogEntry->Hdr.Valid == 0) {
        continue ;
      }
      DumpEventLogEntry (LogEntry);
    }
  }
}
