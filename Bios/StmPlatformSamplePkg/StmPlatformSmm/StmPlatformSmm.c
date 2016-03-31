/** @file
  STM platform SMM API

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmPlatformSmm.h"

EFI_SM_MONITOR_INIT_PROTOCOL                *mSmMonitorInitProtocol;

/**
  SMM End Of Dxe event notification handler.

  Add system resource for STM.

  @param[in] Protocol   Points to the protocol's unique identifier.
  @param[in] Interface  Points to the interface instance.
  @param[in] Handle     The handle on which the interface was installed.

  @retval EFI_SUCCESS   Notification handler runs successfully.
**/
EFI_STATUS
EFIAPI
SmmEndOfDxeEventNotify (
  IN CONST EFI_GUID  *Protocol,
  IN VOID            *Interface,
  IN EFI_HANDLE      Handle
  )
{
  AddResourcesCmd ();
  return EFI_SUCCESS;
}

/**
  Load STM image.

  @retval EFI_SUCCESS           STM is loaded to MSEG
  @retval EFI_BUFFER_TOO_SMALL  MSEG is too small
  @retval EFI_UNSUPPORTED       MSEG is not enabled
**/
EFI_STATUS
LoadStmImage (
  VOID
  )
{
  EFI_STATUS  Status;
  VOID        *StmImageBuffer;
  UINTN       StmImageSize;

  //
  // Extract STM image from FV
  //
  StmImageBuffer = NULL;
  StmImageSize = 0;

  Status = GetSectionFromAnyFv (
             PcdGetPtr(PcdStmBinFile),
             EFI_SECTION_RAW,
             0,
             &StmImageBuffer,
             &StmImageSize
             );
  ASSERT_EFI_ERROR (Status);

  Status = mSmMonitorInitProtocol->LoadMonitor ((EFI_PHYSICAL_ADDRESS)(UINTN)StmImageBuffer, StmImageSize);
  DEBUG ((EFI_D_ERROR, "mSmMonitorInitProtocol->LoadMonitor - %r\n", Status));
  ASSERT_EFI_ERROR (Status);

  gBS->FreePool ((VOID *)((UINTN)StmImageBuffer));

  return Status;
}

/**

  STM platform SMM driver entry point function.

  @param ImageHandle   image handle for this driver image
  @param SystemTable   pointer to the EFI System Table

  @retval EFI_SUCCESS

**/
EFI_STATUS
EFIAPI
InstallStmPlatformSmm (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS                    Status;
  VOID                          *Registration;

  Status = gSmst->SmmLocateProtocol (
                    &gEfiSmMonitorInitProtocolGuid,
                    NULL,
                    (VOID **)&mSmMonitorInitProtocol
                    );
  if (EFI_ERROR(Status) || (mSmMonitorInitProtocol == NULL)) {
    return EFI_UNSUPPORTED;
  }

  Status = LoadStmImage ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // We have to add resource here because it depends on PCI bus enumeration.
  // So we use EndOfDxe event.
  //
  Status = gSmst->SmmRegisterProtocolNotify (
                    &gEfiSmmEndOfDxeProtocolGuid,
                    SmmEndOfDxeEventNotify,
                    &Registration
                    );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}
