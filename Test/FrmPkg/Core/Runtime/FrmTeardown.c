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
#include "Dce.h"

volatile BOOLEAN    mReadyForTeardown;
volatile UINT32     mTeardownSystemPowerState;
volatile BOOLEAN    *mTeardownFinished;

/**
  This function teardown BSP.

  @param Index               CPU index
  @param SystemPowerState    An integer representing the system power state that the software will be transitioning in to.
**/
VOID
FrmTeardownBsp (
  IN UINT32 Index,
  IN UINT32 SystemPowerState
  )
{
  DEBUG ((EFI_D_INFO, "(FRM) !!!ReadyForTeardown - %d\n", (UINTN)Index));

  mTeardownSystemPowerState = SystemPowerState;
  mReadyForTeardown = TRUE;

  TeardownStm (Index);

  AsmWbinvd ();
  AsmVmClear (&mGuestContextCommon.GuestContextPerCpu[Index].Vmcs);
  AsmWbinvd ();
  AsmVmxOff ();
  AsmWbinvd ();
  mTeardownFinished[Index] = TRUE;

  // Wait AP finish
  for (Index = 0; Index < mHostContextCommon.CpuNum; Index++) {
    while (!mTeardownFinished[Index]) {
      ; // WAIT
    }
  }

  if (IsBsp()) {
    DLME_Exit(SystemPowerState);
  }

  // re-init
  mReadyForTeardown = FALSE;
  mTeardownSystemPowerState = 0;
  for (Index = 0; Index < mHostContextCommon.CpuNum; Index++) {
    mTeardownFinished[Index] = FALSE;
  }

  return ;
}

/**
  This function teardown AP.

  @param Index               CPU index
**/
VOID
FrmTeardownAp (
  IN UINT32 Index
  )
{
  if (mReadyForTeardown) {
    DEBUG ((EFI_D_INFO, "(FRM) !!!ReadyForTeardown - %d\n", (UINTN)Index));
    
    TeardownStm (Index);

    AsmWbinvd ();
    AsmVmClear (&mGuestContextCommon.GuestContextPerCpu[Index].Vmcs);
    AsmWbinvd ();
    AsmVmxOff ();
    AsmWbinvd ();

    if (IsBsp()) {
      DLME_Exit(mTeardownSystemPowerState);
    }

    mTeardownFinished[Index] = TRUE;
    while (TRUE);
  }
  return ;
}
