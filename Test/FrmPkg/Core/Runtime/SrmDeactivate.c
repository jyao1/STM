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

extern FRM_COMMUNICATION_DATA    mCommunicationData;

extern BOOLEAN   mStmBspDone;
extern UINT32    mApFinishCount;

extern UINT32    mSrmGuestId;

/**
  This function teardown STM.

  @param Index               CPU index
**/
VOID
TeardownStm (
  IN UINT32 Index
  )
{
  UINT64 Data64;
  EFI_SM_MONITOR_SERVICE_PROTOCOL    *SmMonitorServiceProtocol;

  Data64 = AsmReadMsr64 (IA32_SMM_MONITOR_CTL_MSR_INDEX);
  if ((Data64 & IA32_SMM_MONITOR_VALID) == 0) {
    return ;
  }
  
  SmMonitorServiceProtocol = (EFI_SM_MONITOR_SERVICE_PROTOCOL *)(UINTN)mCommunicationData.SmMonitorServiceProtocol;
  if (SmMonitorServiceProtocol == NULL) {
    return ;
  }

  AsmWbinvd ();

  SmMonitorServiceProtocol->Stop ();

  AsmWbinvd ();

  if (Index == 0) {
    mStmBspDone = FALSE;
    mApFinishCount = 0;
  }

  return ;
}
