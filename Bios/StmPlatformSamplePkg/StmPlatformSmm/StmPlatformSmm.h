/** @file
  STM platform SMM API

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _STM_PLATFORM_SMM_H_
#define _STM_PLATFORM_SMM_H_

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/IoLib.h>
#include <Library/PciLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>

#include <Protocol/SmmReadyToLock.h>
#include <Protocol/SmmEndOfDxe.h>
#include <Protocol/SmmCpu.h>

#include <IndustryStandard/Pci30.h>

#include <Protocol/SmMonitorInit.h>

#include <Library/PcdLib.h>

#include "StmPlatformResource.h"

extern EFI_SM_MONITOR_INIT_PROTOCOL *mSmMonitorInitProtocol;

#endif
