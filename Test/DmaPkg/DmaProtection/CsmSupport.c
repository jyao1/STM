/** @file

  Copyright (c) 2014, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "DmaProtection.h"
#include <Protocol/LegacyBios.h>

EFI_LEGACY_BIOS_PROTOCOL  *mLegacyBios;

/**

  This function is legacy bios protocol notification.

  @param Event    event.
  @param Context  event context.

**/
VOID
EFIAPI
LegacyBiosNotification (
  IN  EFI_EVENT                Event,
  IN  VOID                     *Context
  )
{
  EFI_STATUS  Status;

  Status = gBS->LocateProtocol (
                  &gEfiLegacyBiosProtocolGuid,
                  NULL,
                  (VOID **)&mLegacyBios
                  );
  if (EFI_ERROR (Status)) {
    return;
  }
  
  DEBUG((EFI_D_INFO, "Vtd LegacyBiosNotification\n"));

  //
  // EBDA/LowPmmMemory/OpromReservedMemory
  //
  SetAccessAttribute (
    0,
    0,
    0,
    0, // PhysicalAddress
    0xA0000,
    TRUE
    );

  //
  // HiPmmMemory
  //
  SetAccessAttribute (
    0,
    0,
    0,
    BASE_1MB,
    SIZE_16MB - BASE_1MB,
    TRUE
    );
  return ;
}

/**

  This function is legacy boot event notification.

  @param Event    event.
  @param Context  event context.

**/
VOID
EFIAPI
OnLegacyBoot (
  EFI_EVENT                               Event,
  VOID                                    *Context
  )
{
  DEBUG ((EFI_D_INFO, "Vtd OnLegacyBoot\n"));
  DumpVtdRegs();
  DisableDmar ();
}

/**
  Initialization for CSM.
**/
VOID
CsmInit (
  VOID
  )
{
  EFI_EVENT  NotifyEvent;
  VOID       *Registration;
  EFI_EVENT  LegacyBootEvent;
  EFI_STATUS Status;

  DEBUG((EFI_D_INFO, "CsmInit\n"));

  NotifyEvent = EfiCreateProtocolNotifyEvent (
                  &gEfiLegacyBiosProtocolGuid,
                  TPL_CALLBACK,
                  LegacyBiosNotification,
                  NULL,
                  &Registration
                  );
  ASSERT (NotifyEvent != NULL);

  Status = EfiCreateEventLegacyBootEx (
             TPL_NOTIFY,
             OnLegacyBoot,
             NULL,
             &LegacyBootEvent
             );
  ASSERT_EFI_ERROR (Status);  
}
