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
#include "FrmHandler.h"
#include "StmApi.h"

extern UINTN mExitCount[VmExitReasonMax];

extern UINT32    mSrmGuestId;
CHAR16  *Parameter = L"SrmGuest Parameter";

/**

  This function dump VMExit data.

**/
VOID
DumpPerf (
  VOID
  )
{
  UINTN  Index;
  DEBUG ((EFI_D_ERROR, "(FRM) VmExitCountStart:\n"));
  for (Index = 0; Index < sizeof(mExitCount)/sizeof(mExitCount[0]); Index++) {
    if (mExitCount[Index] == 0) {
      continue;
    }
    DEBUG ((EFI_D_INFO, "(FRM)   %08x - %08x\n", Index, mExitCount[Index]));
  }
  DEBUG ((EFI_D_INFO, "(FRM) VmExitCountEnd!\n"));
}

/**

  This function is VMX timer handler.

  @param Index CPU index

**/
VOID
VmxTimerHandler (
  IN UINT32 Index
  )
{
//DEBUG ((EFI_D_INFO, "(FRM) !!!VmxTimerHandler - %d (%ld)\n", (UINTN)Index, (UINTN)AsmReadTsc()));

  VmWrite32 (VMCS_32_GUEST_VMX_PREEMPTION_TIMER_VALUE_INDEX, mGuestContextCommon.VmxTimerValue);

  if (Index == 0) {
    AcquireSpinLock (&mHostContextCommon.DebugLock);
//    DumpPerf ();
    ReleaseSpinLock (&mHostContextCommon.DebugLock);
  }

  FrmTeardownAp (Index);

  return ;
}
