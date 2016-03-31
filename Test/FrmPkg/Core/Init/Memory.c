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

  This function allocate pages for host.

  @param Pages the requested pages number

  @return pages address

**/
VOID *
AllocatePages (
  IN UINTN Pages
  )
{
  PHYSICAL_ADDRESS  Address;

  AcquireSpinLock (&mHostContextCommon.MemoryLock);
  if (mHostContextCommon.HeapBottom + FRM_PAGES_TO_SIZE(Pages) > mHostContextCommon.HeapTop) {
    DEBUG ((EFI_D_ERROR, "(FRM) AllocatePages fail\n"));
    ReleaseSpinLock (&mHostContextCommon.MemoryLock);
    CpuDeadLoop ();
  }
  Address = mHostContextCommon.HeapTop - FRM_PAGES_TO_SIZE(Pages);
  mHostContextCommon.HeapTop = Address;

  ZeroMem ((VOID *)(UINTN)Address, FRM_PAGES_TO_SIZE (Pages));
  ReleaseSpinLock (&mHostContextCommon.MemoryLock);
  return (VOID *)(UINTN)Address;
}

/**

  This function free pages for host.

  @param Address pages address
  @param Pages   pages number

**/
VOID
FreePages (
  IN VOID  *Address,
  IN UINTN Pages
  )
{
  if ((UINT64)(UINTN)Address == mHostContextCommon.HeapTop) {
    mHostContextCommon.HeapTop += FRM_PAGES_TO_SIZE(Pages);
  }
  return ;
}
