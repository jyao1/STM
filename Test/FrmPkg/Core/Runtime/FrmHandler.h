/** @file

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _FRM_HANDLER_H_
#define _FRM_HANDLER_H_

#include <Base.h>
#include "Frm.h"

extern FRM_HOST_CONTEXT_COMMON   mHostContextCommon;
extern FRM_GUEST_CONTEXT_COMMON  mGuestContextCommon;

typedef
VOID
(* FRM_HANDLER) (
  IN UINT32  Index
  );

/**

  This function is unknown handler.

  @param Index CPU index

**/
VOID
UnknownHandler (
  IN UINT32 Index
  );

/**

  This function is CR access handler.

  @param Index CPU index

**/
VOID
CrHandler (
  IN UINT32 Index
  );

/**

  This function is EPT violation handler.

  @param Index CPU index

**/
VOID
EPTViolationHandler (
  IN UINT32 Index
  );

/**

  This function is EPT misconfiguration handler.

  @param Index CPU index

**/
VOID
EPTMisconfigurationHandler (
  IN UINT32  Index
  );

/**

  This function is INVEPT handler.

  @param Index CPU index

**/
VOID
InvEPTHandler (
  IN UINT32  Index
  );

/**

  This function is IO instruction handler.

  @param Index CPU index

**/
VOID
IoHandler (
  IN UINT32 Index
  );

/**

  This function is CPUID handler.

  @param Index CPU index

**/
VOID
CpuidHandler (
  IN UINT32 Index
  );

/**

  This function is RDMSR handler.

  @param Index CPU index

**/
VOID
ReadMsrHandler (
  IN UINT32 Index
  );

/**

  This function is WRMSR handler.

  @param Index CPU index

**/
VOID
WriteMsrHandler (
  IN UINT32 Index
  );

/**

  This function is INIT handler.

  @param Index CPU index

**/
VOID
InitHandler (
  IN UINT32 Index
  );

/**

  This function is SIPI handler.

  @param Index CPU index

**/
VOID
SipiHandler (
  IN UINT32 Index
  );

/**

  This function is INVD handler.

  @param Index CPU index

**/
VOID
InvdHandler (
  IN UINT32 Index
  );

/**

  This function is WBINVD handler.

  @param Index CPU index

**/
VOID
WbinvdHandler (
  IN UINT32 Index
  );

/**

  This function is VMCALL handler.

  @param Index CPU index

**/
VOID
VmcallHandler (
  IN UINT32 Index
  );

/**

  This function is VMX timer handler.

  @param Index CPU index

**/
VOID
VmxTimerHandler (
  IN UINT32 Index
  );

/**

  This function is external interrupt handler.

  @param Index CPU index

**/
VOID
ExternalInterruptHandler (
  IN UINT32 Index
  );

/**

  This function is NMI handler.

  @param Index CPU index

**/
VOID
ExceptionNMIHandler (
  IN UINT32 Index
  );

/**

  This function is interrupt windows handler.

  @param Index CPU index

**/
VOID
InterruptWindowHandler (
  IN UINT32 Index
  );

/**

  This function is task switch handler.

  @param Index CPU index

**/
VOID
TaskSwitchHandler (
  IN UINT32 Index
  );

/**

  This function is XSETBV handler.

  @param Index CPU index

**/
VOID
XsetbvHandler (
  IN UINT32 Index
  );

/**

  This function is IO handler for ACPI.

  @param Index CPU index
  @param Port  ACPI IO port address
  @param Data  ACPI IO port data

**/
VOID
IoAcpiHandler (
  IN UINT32  Index,
  IN UINT16  Port,
  IN UINT32  Data
  );

/**

  This function is IO handler for reset.

  @param Index CPU index
  @param Port  Reset IO port address
  @param Data  Reset IO port data

**/
VOID
IoResetHandler (
  IN UINT32  Index,
  IN UINT16  Port,
  IN UINT8   Data
  );

//
// Misc
//

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

  This function convert guest virtual address to guest physical address.

  @param CpuIndex            CPU index
  @param GuestVirtualAddress Guest virtual address

  @return Guest physical address
**/
UINTN
GuestVirtualToGuestPhysical (
  IN UINT32  CpuIndex,
  IN UINTN   GuestVirtualAddress
  );

/**

  This function convert guest virtual address to host physical address.

  @param CpuIndex            CPU index
  @param GuestVirtualAddress Guest virtual address

  @return Host physical address
**/
UINTN
GuestVirtualToHostPhysical (
  IN UINT32  CpuIndex,
  IN UINTN   GuestVirtualAddress
  );

/**

  This function set EPT page table attribute by address.

  @param Addr                     Memory base
  @param Ra                       Read access
  @param Wa                       Write access
  @param Xa                       Execute access

**/
VOID
EPTSetPageAttribute (
  IN UINTN  Addr,
  IN UINTN  Ra,
  IN UINTN  Wa,
  IN UINTN  Xa
  );

/**
  This function teardown STM.

  @param Index               CPU index
**/
VOID
TeardownStm (
  IN UINT32 Index
  );

/**
  This function teardown AP.

  @param Index               CPU index
**/
VOID
FrmTeardownAp (
  IN UINT32 Index
  );

/**
  This function teardown BSP.

  @param Index               CPU index
  @param SystemPowerState    An integer representing the system power state that the software will be transitioning in to.

**/
VOID
FrmTeardownBsp (
  IN UINT32 Index,
  IN UINT32 SystemPowerState
  );

#endif