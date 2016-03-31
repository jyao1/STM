/** @file
  STM performance

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Stm.h"
#include <Library/PcdLib.h>

#define STM_PERF_DATA_LENGTH_MAX              (SIZE_4KB * 16)

//
// Performance library propery mask bits
//
#define STM_PERFORMANCE_LIBRARY_PROPERTY_MEASUREMENT_ENABLED  0x00000001

/**
  Creates a record for the beginning of a performance measurement. 
  
  Creates a record that contains the CpuIndex and Token.
  This function reads the current time stamp and adds that time stamp value to the record as the start time.

  @param  CpuIndex                Index of CPU.
  @param  Reason                  Reason of this measurement.
  @param  Token                   Pointer to a Null-terminated ASCII string
                                  that identifies the component being measured.
  @param  Description             Pointer to a Null-terminated ASCII string
                                  that describe this measurement.

  @retval RETURN_SUCCESS          The start of the measurement was recorded.
  @retval RETURN_OUT_OF_RESOURCES There are not enough resources to record the measurement.

**/
RETURN_STATUS
EFIAPI
StmStartPerformanceMeasurement (
  IN UINT32                      CpuIndex,
  IN UINT32                      Reason,
  IN CONST CHAR8                 *Token,
  IN CONST CHAR8                 *Description OPTIONAL
  )
{
  UINT32                Index;
  STM_PERF_DATA_ENTRY   *DataEntry;
  RETURN_STATUS         Status;

  //
  // BUGBUG: Just record CPU0 data, filter others - too many data collected :(
  //
  if (CpuIndex != 0) {
    return RETURN_SUCCESS;
  }

  DataEntry = (STM_PERF_DATA_ENTRY *)(UINTN)mHostContextCommon.PerfData.Address;
  if (DataEntry == NULL) {
    return RETURN_OUT_OF_RESOURCES;
  }

  AcquireSpinLock (&mHostContextCommon.PerfData.PerfLock);

  Index = mHostContextCommon.PerfData.EntryCount;
  Status = RETURN_OUT_OF_RESOURCES;
  if (Index < mHostContextCommon.PerfData.TotalSize / sizeof(*DataEntry)) {
    //
    // Creates a record that contains the CpuIndex and Token.
    //
    DataEntry[Index].StartTimeStamp = AsmReadTsc ();
    DataEntry[Index].EndTimeStamp   = 0;
    DataEntry[Index].CpuIndex       = CpuIndex;
    DataEntry[Index].Reason         = Reason;
    AsciiStrnCpy (DataEntry[Index].Token, Token, sizeof(DataEntry[Index].Token) - 1);
    DataEntry[Index].Token[sizeof(DataEntry[Index].Token) - 1] = 0;
    if (Description != NULL) {
      AsciiStrnCpy (DataEntry[Index].StartDescription, Description, sizeof(DataEntry[Index].StartDescription) - 1);
      DataEntry[Index].StartDescription[sizeof(DataEntry[Index].StartDescription) - 1] = 0;
    }
    mHostContextCommon.PerfData.EntryCount ++;
    Status = RETURN_SUCCESS;
  }

  ReleaseSpinLock (&mHostContextCommon.PerfData.PerfLock);

#if 0
  if (Status != RETURN_SUCCESS) {
    AcquireSpinLock (&mHostContextCommon.DebugLock);
    DEBUG ((EFI_D_ERROR, "StmStartPerformanceMeasurement(%x) - %a, %a\n", (UINTN)CpuIndex, Token, Description));
    STM_PERF_DUMP;
    ReleaseSpinLock (&mHostContextCommon.DebugLock);
    CpuDeadLoop ();
  }
#endif

  return Status;
}

/**
  Fills in the end time of a performance measurement. 
  
  Looks up the last record that matches CpuIndex and Token.
  If the record can not be found then return RETURN_NOT_FOUND.
  If the record is found then TimeStamp is added to the record as the end time.
  If this function is called multiple times for the same record, then the end time is overwritten.

  @param  CpuIndex                Index of CPU.
  @param  Token                   Pointer to a Null-terminated ASCII string
                                  that identifies the component being measured.
  @param  Description             Pointer to a Null-terminated ASCII string
                                  that describe this measurement.

  @retval RETURN_SUCCESS          The end of  the measurement was recorded.
  @retval RETURN_NOT_FOUND        The specified measurement record could not be found.

**/
RETURN_STATUS
EFIAPI
StmEndPerformanceMeasurement (
  IN UINT32                      CpuIndex,
  IN CONST CHAR8                 *Token,
  IN CONST CHAR8                 *Description OPTIONAL
  )
{
  INT64                 Index;
  STM_PERF_DATA_ENTRY   *DataEntry;
  RETURN_STATUS         Status;

  //
  // BUGBUG: Just record CPU0 data, filter others - too many data collected :(
  //
  if (CpuIndex != 0) {
    return RETURN_SUCCESS;
  }

  DataEntry = (STM_PERF_DATA_ENTRY *)(UINTN)mHostContextCommon.PerfData.Address;
  if (DataEntry == NULL) {
    return RETURN_NOT_FOUND;
  }

  AcquireSpinLock (&mHostContextCommon.PerfData.PerfLock);

  //
  // Looks up the last record that matches CpuIndex and Token.
  //
  Status = RETURN_NOT_FOUND;
  for (Index = (INT64)(UINT64)(mHostContextCommon.PerfData.EntryCount - 1); Index >= 0; Index--) {
    if ((DataEntry[Index].CpuIndex == CpuIndex) &&
        (AsciiStrnCmp (DataEntry[Index].Token, Token, sizeof(DataEntry[Index].Token) - 1) == 0)) {
      //
      // Need check EndTimeStamp to not override old data, because StmStartPerformanceMeasurement may return OUT_OF_RESOURCES
      //
      if (DataEntry[Index].EndTimeStamp == 0) {
        DataEntry[Index].EndTimeStamp = AsmReadTsc ();
        DataEntry[Index].DeltaOfTimeStamp = DataEntry[Index].EndTimeStamp - DataEntry[Index].StartTimeStamp;
        if (Description != NULL) {
          AsciiStrnCpy (DataEntry[Index].EndDescription, Description, sizeof(DataEntry[Index].EndDescription) - 1);
          DataEntry[Index].EndDescription[sizeof(DataEntry[Index].EndDescription) - 1] = 0;
        }
        Status = RETURN_SUCCESS;
        break;
      }
    }
  }

  ReleaseSpinLock (&mHostContextCommon.PerfData.PerfLock);

#if 0
  if (Status != RETURN_SUCCESS) {
    AcquireSpinLock (&mHostContextCommon.DebugLock);
    DEBUG ((EFI_D_ERROR, "StmEndPerformanceMeasurement(%x) - %a, %a\n", (UINTN)CpuIndex, Token, Description));
    STM_PERF_DUMP;
    ReleaseSpinLock (&mHostContextCommon.DebugLock);
    CpuDeadLoop ();
  }
#endif

  return Status;
}

/**
  Returns TRUE if the performance measurement macros are enabled. 
  
  This function returns TRUE if the PERFORMANCE_LIBRARY_PROPERTY_MEASUREMENT_ENABLED bit of
  PcdPerformanceLibraryPropertyMask is set.  Otherwise FALSE is returned.

  @retval TRUE                    The PERFORMANCE_LIBRARY_PROPERTY_MEASUREMENT_ENABLED bit of
                                  PcdPerformanceLibraryPropertyMask is set.
  @retval FALSE                   The PERFORMANCE_LIBRARY_PROPERTY_MEASUREMENT_ENABLED bit of
                                  PcdPerformanceLibraryPropertyMask is clear.

**/
BOOLEAN
EFIAPI
StmPerformanceMeasurementEnabled (
  VOID
  )
{
  return (BOOLEAN) ((PcdGet8(PcdPerformanceLibraryPropertyMask) & STM_PERFORMANCE_LIBRARY_PROPERTY_MEASUREMENT_ENABLED) != 0);
}

/**
  Initialize STM performance measurement.

  @retval RETURN_SUCCESS          Initialize measurement successfully.
  @retval RETURN_OUT_OF_RESOURCES  No enough resource to hold STM PERF data.

**/
RETURN_STATUS
EFIAPI
StmInitPerformanceMeasurement (
  VOID
  )
{
  InitializeSpinLock (&mHostContextCommon.PerfData.PerfLock);
  mHostContextCommon.PerfData.Address = (UINT64)(UINTN)AllocatePages (STM_SIZE_TO_PAGES(STM_PERF_DATA_LENGTH_MAX));
  mHostContextCommon.PerfData.TotalSize = STM_PERF_DATA_LENGTH_MAX;

  if (mHostContextCommon.PerfData.Address != 0) {
    return RETURN_SUCCESS;
  } else {
    return RETURN_OUT_OF_RESOURCES;
  }
}

/**
  Dump STM performance measurement.

  @retval RETURN_SUCCESS          Dump measurement successfully.
  @retval RETURN_NOT_FOUND        No STM PERF data.

**/
RETURN_STATUS
EFIAPI
StmDumpPerformanceMeasurement (
  VOID
  )
{
  UINT32                Index;
  STM_PERF_DATA_ENTRY   *DataEntry;

  DataEntry = (STM_PERF_DATA_ENTRY *)(UINTN)mHostContextCommon.PerfData.Address;
  if (DataEntry == NULL) {
    return RETURN_NOT_FOUND;
  }

  AcquireSpinLock (&mHostContextCommon.PerfData.PerfLock);

  DEBUG ((EFI_D_INFO, "StmPerfAddress: %016lx\n", mHostContextCommon.PerfData.Address));
  DEBUG ((EFI_D_INFO, "StmPerfEntryCount: %08x\n", mHostContextCommon.PerfData.EntryCount));

  for (Index = 0; Index < mHostContextCommon.PerfData.EntryCount; Index++) {
    DEBUG ((EFI_D_INFO, "StmPerfEntry:\n"));
    DEBUG ((EFI_D_INFO, "  StartTimeStamp   : %016lx\n", DataEntry[Index].StartTimeStamp));
    DEBUG ((EFI_D_INFO, "  EndTimeStamp     : %016lx\n", DataEntry[Index].EndTimeStamp));
    DEBUG ((EFI_D_INFO, "  DeltaOfTimeStamp : %016lx\n", DataEntry[Index].DeltaOfTimeStamp));
    DEBUG ((EFI_D_INFO, "  CpuIndex         : %08x\n",   (UINTN)DataEntry[Index].CpuIndex));
    DEBUG ((EFI_D_INFO, "  Reason           : %08x\n",   (UINTN)DataEntry[Index].Reason));
    DEBUG ((EFI_D_INFO, "  Token            : %a\n",     DataEntry[Index].Token));
    DEBUG ((EFI_D_INFO, "  StartDesc        : %a\n",     DataEntry[Index].StartDescription));
    DEBUG ((EFI_D_INFO, "  EndDesc          : %a\n",     DataEntry[Index].EndDescription));
  }

  ReleaseSpinLock (&mHostContextCommon.PerfData.PerfLock);

  if (mHostContextCommon.PerfData.EntryCount != 0) {
    return RETURN_SUCCESS;
  } else {
    return RETURN_NOT_FOUND;
  }
}
