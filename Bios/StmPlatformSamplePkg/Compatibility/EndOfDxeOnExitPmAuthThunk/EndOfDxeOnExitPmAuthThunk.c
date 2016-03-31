/** @file
  EndOfDxe Protocol on ExitPmAuth Protocol Thunk driver.
  Some EDK platform uses ExitPmAuth and DxeSmmReadyToLock without EndOfDxe.

Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>

This program and the accompanying materials
are licensed and made available under the terms and conditions
of the BSD License which accompanies this distribution.  The
full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include  "EndOfDxeOnExitPmAuthThunk.h"

/**
  An empty dummy callback function.

  @param[in] Event    Event whose notification function is being invoked.
  @param[in] Context  Pointer to the notification function's context.
**/
VOID
EFIAPI
InternalEmptyCallbackFuntion (
  IN EFI_EVENT                Event,
  IN VOID                     *Context
  )
{
  return;
}

/**
  ExitPmAuth Protocol notification event handler.

  @param[in] Event    Event whose notification function is being invoked.
  @param[in] Context  Pointer to the notification function's context.
**/
VOID
EFIAPI
ExitPmAuthProtocolNotification (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS Status;
  VOID       *ExitPmAuth;
  EFI_EVENT  EndOfDxeEvent;

  //
  // Add more check to locate protocol after got event, because
  // ECP will signal this event immediately once it is register
  // just in case it is already installed.
  //
  Status = gBS->LocateProtocol (
                  &gExitPmAuthProtocolGuid,
                  NULL,
                  &ExitPmAuth
                  );
  if (EFI_ERROR (Status)) {
    return ;
  }
  
  //
  // Since PI1.2.1, we need signal EndOfDxe as ExitPmAuth
  //
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  InternalEmptyCallbackFuntion,
                  NULL,
                  &gEfiEndOfDxeEventGroupGuid,
                  &EndOfDxeEvent
                  );
  ASSERT_EFI_ERROR (Status);
  gBS->SignalEvent (EndOfDxeEvent);
  gBS->CloseEvent (EndOfDxeEvent);
  DEBUG((EFI_D_INFO,"All EndOfDxe callbacks have returned successfully\n"));
}

/**
  Entry Point for DxeSmmReadyToLock Protocol on ExitPmAuth Protocol Thunk driver.

  @param[in] ImageHandle  Image handle of this driver.
  @param[in] SystemTable  A Pointer to the EFI System Table.

  @retval EFI_SUCCESS  The entry point is executed successfully.
**/
EFI_STATUS
EFIAPI
EndOfDxeMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  VOID *Registration;

  //
  // Install notifications for required protocols
  //
  EfiCreateProtocolNotifyEvent (
    &gExitPmAuthProtocolGuid,
    TPL_CALLBACK,
    ExitPmAuthProtocolNotification,
    NULL,
    &Registration
    );

  return EFI_SUCCESS;
}
