/** @file
  SMM exception handler

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

  This function is exception handler for SMM.

  @param Index CPU index

**/
VOID
SmmExceptionHandler (
  IN UINT32  Index
  )
{
  //
  // Resume to BIOS Exceptin Handler
  //
  ResumeToBiosExceptionHandler (Index);
  //
  // Should not return, issue TXT.RESET or system reset.
  //
  StmTxtReset (STM_CRASH_PROTECTION_EXCEPTION);
}
