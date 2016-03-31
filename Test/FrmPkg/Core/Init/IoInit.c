/** @file

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Base.h>
#include "FrmInit.h"

/**

  This function set IO bitmap.

  @param Port   IO port

**/
VOID
SetIoBitmap (
  IN UINT16  Port
  )
{
  UINT8 *IoBitmap;
  UINTN Index;
  UINTN Offset;

  if (Port >= 0x8000) {
    IoBitmap = (UINT8 *)(UINTN)mGuestContextCommon.IoBitmapB;
    Port -= 0x8000;
  } else {
    IoBitmap = (UINT8 *)(UINTN)mGuestContextCommon.IoBitmapA;
  }

  Index = Port / 8;
  Offset = Port % 8;

  IoBitmap[Index] |= (UINT8)(1 << Offset);

  return ;
}

/**

  This function initialize IO handler for ACPI.

**/
VOID
InitializeIoAcpiHandlers (
  VOID
  );

/**

  This function initialize IO handler for Reset.

**/
VOID
InitializeIoResetHandlers (
  VOID
  );

/**

  This function initialize IO bitmap.

**/
VOID
IoInit (
  VOID
  )
{
  mGuestContextCommon.IoBitmapA = (UINT64)(UINTN)AllocatePages (1);
  mGuestContextCommon.IoBitmapB = (UINT64)(UINTN)AllocatePages (1);

#if 1
  SetIoBitmap (mHostContextCommon.AcpiPmControlIoPortBaseAddress);
//  InitializeIoAcpiHandlers ();
#endif

#if 1
  if (mHostContextCommon.ResetIoPortBaseAddress != 0) {
    SetIoBitmap (mHostContextCommon.ResetIoPortBaseAddress);
//    InitializeIoResetHandlers ();
  }
#endif

}
