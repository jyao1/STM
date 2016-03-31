/** @file
  IO initialization

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmInit.h"

/**

  This function set/unset IO bitmap.

  @param Port IO port
  @param Set  TRUE means set IO bitmap, FALSE means unset IO bitmap

**/
VOID
SetIoBitmapEx (
  IN UINT16  Port,
  IN BOOLEAN Set
  )
{
  UINT8 *IoBitmap;
  UINTN Index;
  UINTN Offset;

  if (Port >= 0x8000) {
    IoBitmap = (UINT8 *)(UINTN)mGuestContextCommonSmm.IoBitmapB;
    Port -= 0x8000;
  } else {
    IoBitmap = (UINT8 *)(UINTN)mGuestContextCommonSmm.IoBitmapA;
  }

  Index = Port / 8;
  Offset = Port % 8;

  if (Set) {
    IoBitmap[Index] |= (UINT8)(1 << Offset);
  } else {
    IoBitmap[Index] &= (UINT8)~(1 << Offset);
  }

  return ;
}

/**

  This function set IO bitmap.

  @param Base   IO port base
  @param Length IO port length

**/
VOID
SetIoBitmapRange (
  IN UINT16  Base,
  IN UINT16  Length
  )
{
  UINT16  Port;
  for (Port = Base; Port < Base + Length; Port++) {
    SetIoBitmapEx (Port, TRUE);
  }
}

/**

  This function unset IO bitmap.

  @param Base   IO port base
  @param Length IO port length

**/
VOID
UnSetIoBitmapRange (
  IN UINT16  Base,
  IN UINT16  Length
  )
{
  UINT16  Port;
  for (Port = Base; Port < Base + Length; Port++) {
    SetIoBitmapEx (Port, FALSE);
  }
}

/**

  This function initialize IO bitmap.

**/
VOID
IoInit (
  VOID
  )
{
  mGuestContextCommonSmm.IoBitmapA = (UINT64)(UINTN)AllocatePages (1);
  mGuestContextCommonSmm.IoBitmapB = (UINT64)(UINTN)AllocatePages (1);
}
