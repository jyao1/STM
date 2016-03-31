/** @file
  SMM STM support

  Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _SMM_STM_H_
#define _SMM_STM_H_

#include <Protocol/SmMonitorInit.h>

#define IA32_VMX_BASIC_MSR_INDEX               0x480
#define IA32_VMX_MISC_MSR_INDEX                0x485
#define IA32_SMM_MONITOR_CTL_MSR_INDEX         0x9B
#define   IA32_SMM_MONITOR_VALID               BIT0

/**

  Get STM state.

  @return STM state

**/
EFI_SM_MONITOR_STATE
EFIAPI
GetMonitorState (
  VOID
  );

/**

  Load STM image to MSEG.

  @param StmImage      STM image
  @param StmImageSize  STM image size

  @retval EFI_SUCCESS            Load STM to MSEG successfully
  @retval EFI_BUFFER_TOO_SMALL   MSEG is smaller than minimal requirement of STM image

**/
EFI_STATUS
EFIAPI
LoadMonitor (
  IN EFI_PHYSICAL_ADDRESS StmImage,
  IN UINTN                StmImageSize
  );

/**

  Add resources in list to database. Allocate new memory areas as needed.

  @param ResourceList  A pointer to resource list to be added
  @param NumEntries    Optional number of entries.
                       If 0, list must be terminated by END_OF_RESOURCES.

  @retval EFI_SUCCESS            If resources are added
  @retval EFI_INVALID_PARAMETER  If nested procedure detected resource failer
  @retval EFI_OUT_OF_RESOURCES   If nested procedure returned it and we cannot allocate more areas.

**/
EFI_STATUS
EFIAPI
AddPiResource (
  IN  STM_RSC  *ResourceList,
  IN  UINT32    NumEntries OPTIONAL
  );

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
DeletePiResource (
  IN  STM_RSC    *ResourceList,
  IN  UINT32      NumEntries OPTIONAL
  );

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
GetPiResource (
  OUT    STM_RSC *ResourceList,
  IN OUT UINT32  *ResourceSize
  );

/**
  This functin initialize STM configuration table.
**/
VOID
StmSmmConfigurationTableInit (
  VOID
  );

/**
  This function notify STM resource change.

  @param StmResource BIOS STM resource

**/
VOID
NotifyStmResourceChange (
  IN VOID *StmResource
  );

/**
  This function return BIOS STM resource.

  @return BIOS STM resource

**/
VOID *
GetStmResource (
  VOID
  );

#endif
