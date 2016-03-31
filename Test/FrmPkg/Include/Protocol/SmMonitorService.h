/** @file
  STM service protocol definition
  
  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _SM_MONITOR_SERVICE_PROTOCOL_H_
#define _SM_MONITOR_SERVICE_PROTOCOL_H_

#include <Uefi.h>
#include <StmApi.h>

#define EFI_SM_MONITOR_SERVICE_PROTOCOL_GUID \
    { 0xa022f30c, 0x1f3d, 0x4cd3, 0xbb, 0xfc, 0xcc, 0x5b, 0xef, 0xd4, 0xe8, 0xe7}

//
// STM service
//

/**

  Start STM.

  @retval EFI_SUCCESS            Start successfully.
**/
typedef
EFI_STATUS
(EFIAPI *EFI_SM_MONITOR_START) (
  VOID
  );

/**

  Stop STM.
  
  @retval EFI_SUCCESS            Stop successfully.
**/
typedef
EFI_STATUS
(EFIAPI *EFI_SM_MONITOR_STOP) (
  VOID
  );

/**

  Add resources in list to database.

  @param ResourceList  A pointer to resource list to be added
  @param NumEntries    Optional number of entries.
                       If 0, list must be terminated by END_OF_RESOURCES.

  @retval EFI_SUCCESS            If resources are added
  @retval EFI_INVALID_PARAMETER  If nested procedure detected resource failer
  @retval EFI_OUT_OF_RESOURCES   If nested procedure returned it and we cannot allocate more areas.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_SM_MONITOR_PROTECT_OS_RESOURCE) (
  IN STM_RSC *ResourceList,
  IN UINT32   NumEntries OPTIONAL
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
typedef
EFI_STATUS
(EFIAPI *EFI_SM_MONITOR_UNPROTECT_OS_RESOURCE) (
  IN STM_RSC *ResourceList OPTIONAL,
  IN UINT32   NumEntries OPTIONAL
  );

/**

  Get OS resources.

  @param ResourceList  A pointer to resource list to be filled
  @param ResourceSize  On input it means size of resource list input.
                       On output it means size of resource list filled,
                       or the size of resource list to be filled if size of too small.

  @retval EFI_SUCCESS            If resources are returned.
  @retval EFI_BUFFER_TOO_SMALL   If resource list buffer is too small to hold the whole resources.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_SM_MONITOR_GET_OS_RESOURCE) (
  OUT    STM_RSC *ResourceList,
  IN OUT UINT32  *ResourceSize
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
typedef
EFI_STATUS
(EFIAPI *EFI_SM_MONITOR_RETURN_PI_RESOURCE) (
  OUT    STM_RSC *ResourceList,
  IN OUT UINT32  *ResourceSize
  );

/**

  Initialize protection.
  
  @retval EFI_SUCCESS            Initialize successfully.
**/
typedef
EFI_STATUS
(EFIAPI *EFI_SM_MONITOR_INITIALIZE_PROTECTION) (
  VOID
  );

/**

  Add resources in list to database.

  @param ResourceList  A pointer to resource list to be added
  @param NumEntries    Optional number of entries.
                       If 0, list must be terminated by END_OF_RESOURCES.

  @retval EFI_SUCCESS            If resources are added
  @retval EFI_INVALID_PARAMETER  If nested procedure detected resource failer
  @retval EFI_OUT_OF_RESOURCES   If nested procedure returned it and we cannot allocate more areas.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_SM_MONITOR_ADD_OS_TRUSTED_REGION) (
  IN STM_RSC *ResourceList,
  IN UINT32   NumEntries OPTIONAL
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
typedef
EFI_STATUS
(EFIAPI *EFI_SM_MONITOR_DELETE_OS_TRUSTED_REGION) (
  IN STM_RSC *ResourceList OPTIONAL,
  IN UINT32   NumEntries OPTIONAL
  );

/**

  Get OS trusted region resources.

  @param ResourceList  A pointer to resource list to be filled
  @param ResourceSize  On input it means size of resource list input.
                       On output it means size of resource list filled,
                       or the size of resource list to be filled if size of too small.

  @retval EFI_SUCCESS            If resources are returned.
  @retval EFI_BUFFER_TOO_SMALL   If resource list buffer is too small to hold the whole resources.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_SM_MONITOR_GET_OS_TRUSTED_REGION) (
  OUT    STM_RSC *ResourceList,
  IN OUT UINT32  *ResourceSize
  );

typedef struct _EFI_SM_MONITOR_SERVICE_PROTOCOL {
  EFI_SM_MONITOR_START                      Start;
  EFI_SM_MONITOR_STOP                       Stop;
  EFI_SM_MONITOR_PROTECT_OS_RESOURCE        ProtectOsResource;
  EFI_SM_MONITOR_UNPROTECT_OS_RESOURCE      UnprotectOsResource;
  EFI_SM_MONITOR_GET_OS_RESOURCE            GetOsResource;
  EFI_SM_MONITOR_RETURN_PI_RESOURCE         ReturnPiResource;
  EFI_SM_MONITOR_INITIALIZE_PROTECTION      InitializeProtection;
  EFI_SM_MONITOR_ADD_OS_TRUSTED_REGION      AddOsTrustedRegion;
  EFI_SM_MONITOR_DELETE_OS_TRUSTED_REGION   DeleteOsTrustedRegion;
  EFI_SM_MONITOR_GET_OS_TRUSTED_REGION      GetOsTrustedRegion;
} EFI_SM_MONITOR_SERVICE_PROTOCOL;

extern EFI_GUID gEfiSmMonitorServiceProtocolGuid;

#endif
