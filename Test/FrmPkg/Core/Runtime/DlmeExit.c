/** @file

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
#include "DrtmTpm.h"

/**

  This function exit DLME environment.

  @param SystemPowerState - An integer representing the system power state that the software will be transitioning in to.

  @retval  0 exit DLME successfully

**/
UINT32
DLME_Exit (
  IN UINT32 SystemPowerState
  )
{
  //EFI_STATUS Status;

  if (!IsMleLaunched()) {
    return (UINT32)-1;
  }

  //
  // Avoid attack after SEXIT (TPM - TBD)
  //
  CapPcrs ();

  if (SystemPowerState == 3) {
    // Need encrypt memory
  }

  //
  // Clean up
  //
  SetNoSecrets ();
  UnlockMemConfig ();

  ClosePrivate ();

  AsmWbinvd ();

  DEBUG ((EFI_D_INFO, "(TXT) SEXIT ...\n"));
  AsmGetSecSexit ();
  DEBUG ((EFI_D_INFO, "(TXT) SEXIT Done!\n"));

  AsmWbinvd ();

  return 0;
}
