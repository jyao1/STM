/** @file

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _STM_SERVICE_H_
#define _STM_SERVICE_H_

#include <Base.h>
#include <Uefi.h>
#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>

#include <Protocol/SmMonitorService.h>

/**
  This function launch STM.

  @param Eax eax register value
  @param Ebx ebx register value
  @param Ecx ecx register value
  @param Edx edx register value
**/
UINT64
EFIAPI
AsmLaunchStm (
  IN UINT32  Eax,
  IN UINT32  Ebx,
  IN UINT32  Ecx,
  IN UINT32  Edx
  );

/**
  This function teardown STM.

  @param Eax eax register value
**/
UINT64
EFIAPI
AsmTeardownStm (
  IN UINT32  Eax
  );

#endif
