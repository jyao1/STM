/** @file

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _FRM_INIT_H_
#define _FRM_INIT_H_

#include <Base.h>
#include "Frm.h"

extern FRM_COMMUNICATION_DATA    mCommunicationData;
extern FRM_HOST_CONTEXT_COMMON   mHostContextCommon;
extern FRM_GUEST_CONTEXT_COMMON  mGuestContextCommon;

extern volatile BOOLEAN    *mApFinished;
extern volatile BOOLEAN    mApLaunch;

extern UINT32 mBspIndex;

extern volatile BOOLEAN    *mTeardownFinished;

/**

  This function create 4G page table for Host.

  @return Page table pointer

**/
UINTN
CreatePageTable (
  VOID
  );

/**

  This function create 4G page table for Guest.

  @return Page table pointer

**/
UINTN
CreateCompatiblePageTable (
  VOID
  );

/**

  This function create 4G PAE page table for Guest.

  @return Page table pointer

**/
UINTN
CreateCompatiblePageTablePae (
  VOID
  );

/**

  This function initialize EPT.

**/
VOID
EptInit (
  VOID
  );

/**

  This function initialize IO bitmap.

**/
VOID
IoInit (
  VOID
  );

/**

  This function initialize VMX Timer.

**/
VOID
VmxTimerInit (
  VOID
  );

/**

  This function initialize all APs.

**/
VOID
InitAllAps (
  VOID
  );

/**

  This function wakeup all APs.

**/
VOID
WakeupAllAps (
  VOID
  );

/**

  This function is AP wakeup C function.

**/
VOID
EFIAPI
ApWakeupC (
  IN UINT32  Index
  );

/**

  This function allocate pages for host.

  @param Pages the requested pages number

  @return pages address

**/
VOID *
AllocatePages (
  IN UINTN Pages
  );

/**

  This function free pages for host.

  @param Address pages address
  @param Pages   pages number

**/
VOID
FreePages (
  IN VOID  *Address,
  IN UINTN Pages
  );

/**

  This function initialize host context per CPU.

  @param Index   CPU Index

**/
VOID
InitHostContextPerCpu (
  IN UINT32 Index
  );

/**

  This function initialize guest context per CPU.

  @param Index   CPU Index

**/
VOID
InitGuestContextPerCpu (
  IN UINT32 Index
  );

/**

  This function is host entrypoint.

**/
VOID
AsmHostEntrypoint (
  VOID
  );

/**

  This function is guest entrypoint during initialization.

**/
VOID
GuestEntrypoint (
  VOID
  );

/**

  This function set VMCS host field.

  @param Index   CPU index

**/
VOID
SetVmcsHostField (
  IN UINT32 Index
  );

/**

  This function set VMCS control field.

  @param Index   CPU index

**/
VOID
SetVmcsControlField (
  IN UINT32 Index
  );

/**

  This function set VMCS guest field.

  @param Index   CPU index

**/
VOID
SetVmcsGuestField (
  IN UINT32 Index
  );

/**

  This function return CPU index according to APICID.

  @param ApicId APIC ID

  @return CPU index

**/
UINT32
ApicToIndex (
  IN UINT32  ApicId
  );

/**

  This function initialize FRM handler.

**/
VOID
InitFrmHandler (
  VOID
  );

/**

  This function is FRM exception handler.

**/
VOID
EFIAPI
AsmExceptionHandlers (
  VOID
  );

/**

  This function is FRM NMI exception handler.

**/
VOID
EFIAPI
AsmNmiExceptionHandler (
  VOID
  );

/**

  This function launch STM.

  @param CpuIndex   CPU Index

**/
VOID
LaunchStm (
  IN UINTN                           CpuIndex
  );

#endif