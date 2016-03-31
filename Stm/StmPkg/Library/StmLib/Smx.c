/** @file
  SMX related function

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Base.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include "Library/StmLib.h"

/**

  This function read TXT public space.

  @param Offset TXT public space register

  @return TXT public space data

**/
UINT32
TxtPubRead32 (
  IN UINTN  Offset
  )
{
  return MmioRead32 (TXT_PUBLIC_SPACE + Offset);
}

/**

  This function write TXT public space.

  @param Offset TXT public space register
  @param Data   TXT public space data

**/
VOID
TxtPubWrite32 (
  IN UINTN  Offset,
  IN UINT32 Data
  )
{
  MmioWrite32 (TXT_PUBLIC_SPACE + Offset, Data);
}

/**

  This function read TXT public space.

  @param Offset TXT public space register

  @return TXT public space data

**/
UINT64
TxtPubRead64 (
  IN UINTN  Offset
  )
{
  return MmioRead64 (TXT_PUBLIC_SPACE + Offset);
}

/**

  This function write TXT public space.

  @param Offset TXT public space register
  @param Data   TXT public space data

**/
VOID
TxtPubWrite64 (
  IN UINTN  Offset,
  IN UINT64 Data
  )
{
  MmioWrite64 (TXT_PUBLIC_SPACE + Offset, Data);
}

/**

  This function read TXT private space.

  @param Offset TXT private space register

  @return TXT private space data

**/
UINT32
TxtPriRead32 (
  IN UINTN  Offset
  )
{
  return MmioRead32 (TXT_PRIVATE_SPACE + Offset);
}

/**

  This function write TXT private space.

  @param Offset TXT private space register
  @param Data   TXT private space data

**/
VOID
TxtPriWrite32 (
  IN UINTN  Offset,
  IN UINT32 Data
  )
{
  MmioWrite32 (TXT_PRIVATE_SPACE + Offset, Data);
}

/**

  This function read TXT private space.

  @param Offset TXT private space register

  @return TXT private space data

**/
UINT64
TxtPriRead64 (
  IN UINTN  Offset
  )
{
  return MmioRead64 (TXT_PRIVATE_SPACE + Offset);
}

/**

  This function write TXT private space.

  @param Offset TXT private space register
  @param Data   TXT private space data

**/
VOID
TxtPriWrite64 (
  IN UINTN  Offset,
  IN UINT64 Data
  )
{
  MmioWrite64 (TXT_PRIVATE_SPACE + Offset, Data);
}

/**

  This function open locality2.

**/
VOID
OpenLocality2 (
  VOID
  )
{
  TxtPriWrite32 (TXT_CMD_OPEN_LOCALITY2, 0x0);
  TxtPriRead32 (TXT_E2STS);
}

/**

  This function close locality2.

**/
VOID
CloseLocality2 (
  VOID
  )
{
  TxtPriWrite32 (TXT_CMD_CLOSE_LOCALITY2, 0x0);
  TxtPriRead32 (TXT_E2STS);
}

/**

  This function open locality1.

**/
VOID
OpenLocality1 (
  VOID
  )
{
  TxtPriWrite32 (TXT_CMD_OPEN_LOCALITY1, 0x0);
  TxtPriRead32 (TXT_E2STS);
}

/**

  This function close locality1.

**/
VOID
CloseLocality1 (
  VOID
  )
{
  TxtPriWrite32 (TXT_CMD_CLOSE_LOCALITY1, 0x0);
  TxtPriRead32 (TXT_E2STS);
}

/**

  This function set secrets.

**/
VOID
SetSecrets (
  VOID
  )
{
  TxtPriWrite32 (TXT_CMD_SECRETS, 0x0);
  TxtPriRead32 (TXT_E2STS);
}

/**

  This function set no-secrets.

**/
VOID
SetNoSecrets (
  VOID
  )
{
  TxtPriWrite32 (TXT_CMD_NO_SECRETS, 0x0);
  TxtPriRead32 (TXT_E2STS);
}

/**

  This function unlock memory configuration.

**/
VOID
UnlockMemConfig (
  VOID
  )
{
  TxtPriWrite32 (TXT_CMD_UNLOCK_MEM_CONFIG, 0x0);
  TxtPriRead32 (TXT_STS);
}

/**

  This function close private.

**/
VOID
ClosePrivate (
  VOID
  )
{
  TxtPriWrite32 (TXT_CMD_CLOSE_PRIVATE, 0x0);
}

/**

  This function return TXT heap.

  @return TXT heap

**/
VOID *
GetTxtHeap (
  VOID
  )
{
  VOID           *TxtHeap;

  TxtHeap = (VOID *)(UINTN)TxtPubRead32 (TXT_HEAP_BASE);

  return TxtHeap;
}

/**

  This function return TXT heap size.

  @return TXT heap size

**/
UINTN
GetTxtHeapSize (
  VOID
  )
{
  UINTN  TxtHeapSize;

  TxtHeapSize = TxtPubRead32 (TXT_HEAP_SIZE);

  return TxtHeapSize;
}

/**

  This function return TXT BiosToOs region.

  @return TXT BiosToOs region

**/
TXT_BIOS_TO_OS_DATA *
GetTxtBiosToOsData (
  VOID
  )
{
  UINT64 *Data;
  Data = GetTxtHeap ();
  Data += 1; // Skip size
  return (TXT_BIOS_TO_OS_DATA *)Data;
}

/**

  This function return TXT OsToMle region.

  @return TXT OsToMle region

**/
VOID *
GetTxtOsToMleData (
  VOID
  )
{
  UINT64 *Data;
  Data = (UINT64 *)GetTxtBiosToOsData ();
  Data -= 1; // Ptr to size
  Data = (UINT64 *)(UINTN)((UINTN)Data + *Data);
  Data += 1; // Skip size
  return (VOID *)Data;
}

/**

  This function return TXT OsToSinit region.

  @return TXT OsToSinit region

**/
TXT_OS_TO_SINIT_DATA *
GetTxtOsToSinitData (
  VOID
  )
{
  UINT64 *Data;
  Data = (UINT64 *)GetTxtOsToMleData ();
  Data -= 1; // Ptr to size
  Data = (UINT64 *)(UINTN)((UINTN)Data + *Data);
  Data += 1; // Skip size
  return (TXT_OS_TO_SINIT_DATA *)Data;
}

/**

  This function return TXT SinitToMle region.

  @return TXT SinitToMle region

**/
TXT_SINIT_TO_MLE_DATA *
GetTxtSinitToMleData (
  VOID
  )
{
  UINT64 *Data;
  Data = (UINT64 *)GetTxtOsToSinitData ();
  Data -= 1; // Ptr to size
  Data = (UINT64 *)(UINTN)((UINTN)Data + *Data);
  Data += 1; // Skip size
  return (TXT_SINIT_TO_MLE_DATA *)Data;
}

/**

  This function return TXT Heap occupied size.

  @return TXT Heap occupied size

**/
UINTN
GetTxtHeapOccupiedSize (
  VOID
  )
{
  UINT64 *Data;
  Data = (UINT64 *)GetTxtSinitToMleData ();
  Data -= 1; // Ptr to size
  Data = (UINT64 *)(UINTN)((UINTN)Data + *Data);

  return (UINTN)Data - (UINTN)GetTxtHeap ();
}
