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

  This function initialize VMX Timer.

**/
VOID
VmxTimerInit (
  VOID
  )
{
  UINT64  VmxTimerValue;
  UINT64  Frequency;

  Frequency = DivU64x32 (1000000000000ull, (UINT32) mCommunicationData.TimerPeriod); // KHz
  VmxTimerValue = RShiftU64 (Frequency, (UINTN)AsmReadMsr64 (IA32_VMX_MISC_MSR_INDEX) & 0x1F); // KTimerValue
  VmxTimerValue = MultU64x32 (VmxTimerValue, 1000); // 1 second VmxTimerValue
  mGuestContextCommon.VmxTimerValue = (UINT32)MultU64x32 (VmxTimerValue, 4); // 4 seconds

  return ;
}
