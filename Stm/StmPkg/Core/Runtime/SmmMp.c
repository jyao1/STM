/** @file
  SMM MP support

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmRuntime.h"

/**

  This function return CPU index according to APICID.

  @param ApicId APIC ID

  @return CPU index

**/
UINT32
ApicToIndex (
  IN UINT32  ApicId
  )
{
  UINT32 Index;

  for (Index = 0; Index < mHostContextCommon.CpuNum; Index++) {
    if (mHostContextCommon.HostContextPerCpu[Index].ApicId == ApicId) {
      return Index;
    }
  }
  if (ApicId == 0xFF) {
    // standard PC
    return 0;
  }
  DEBUG ((EFI_D_ERROR, "ApicToIndex fail\n"));
  CpuDeadLoop ();
  return 0;
}

volatile BOOLEAN  *mCpuInitStatus;

/**

  This function wait all processor rendez-vous.

  @param CurrentIndex Current CPU Index

**/
VOID
WaitAllProcessorRendezVous (
  IN UINT32   CurrentIndex
  )
{
  UINTN  Index;

  // NOTE: This routine assume that when SMI happen all the Processors will enter SMI.
  // This can only be invoed at RSM and TearDown handler.
  if (CurrentIndex == 0) {
    //
    // Let BSP initialize itself
    //
    mCpuInitStatus[0] = TRUE;
    //
    // BSP wait for all AP initialization
    //
    for (Index = 1; Index < mHostContextCommon.CpuNum; Index++) {
      while (!mCpuInitStatus[Index]) {
        //
        // Wait here
        //
      }
    }

    //
    // RendezVous here
    //

    //
    // BSP re-initialize itself
    //
    mCpuInitStatus[0] = FALSE;
    //
    // BSP re-initialize AP
    //
    for (Index = 1; Index < mHostContextCommon.CpuNum; Index++) {
      mCpuInitStatus[Index] = FALSE;
    }
    //
    // BSP leave this routine
    //
  } else {
    //
    // Let AP wait for BSP initialization
    //
    while (!mCpuInitStatus[0]) {
      //
      // Wait here
      //
    }
    //
    // BSP done, AP initialize itself
    //
    mCpuInitStatus[CurrentIndex] = TRUE;

    //
    // RendezVous here
    //

    //
    // Wait for BSP re-initialization
    //
    while (mCpuInitStatus[CurrentIndex]) {
      //
      // Wait here
      //
    }
    //
    // AP leave this routine
    //
  }
  return ;
}
