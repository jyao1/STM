/** @file

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _FRM_H_
#define _FRM_H_

#include <Base.h>
#include <Library\BaseLib.h>
#include <Library\BaseMemoryLib.h>
#include <Library\IoLib.h>
#include <Library\DebugLib.h>
#include <Library\SynchronizationLib.h>

#include <Library\Vmx.h>
#include <Protocol\SmMonitorService.h>

#include "StmApi.h"

#include "Dump.h"
#include "CpuDef.h"
#include "FrmCommon.h"

#define FRM_PAGE_SIZE             0x1000
#define FRM_PAGE_MASK             0xFFF
#define FRM_PAGE_SHIFT            12

#define FRM_SIZE_TO_PAGES(Size)  (((Size) >> FRM_PAGE_SHIFT) + (((Size) & FRM_PAGE_MASK) ? 1 : 0))
#define FRM_PAGES_TO_SIZE(Pages)  ((Pages) << FRM_PAGE_SHIFT)

typedef struct _FRM_GUEST_CONTEXT_PER_CPU {
  X86_REGISTER                Register;
  IA32_DESCRIPTOR             Gdtr;
  IA32_DESCRIPTOR             Idtr;
  UINTN                       Cr0;
  UINTN                       Cr3;
  UINTN                       Cr4;
  UINTN                       Stack;
  UINT32                      LastPendingInterrupt[24]; // hold 24 IRQ, 0 is always 0xFF (not used)
  UINT32                      LastPendingInterruptTop;  // pointer to highest current pending IRQ, ptr to LastPendingInterrupt[0] means no pending VIF
  UINT64                      EFER;
  UINT64                      VmExitMsrStore;
  UINT64                      VmExitMsrLoad;
  UINT64                      VmEnterMsrLoad;
  UINT64                      Vmcs;
  BOOLEAN                     UnrestrictedGuest;
} FRM_GUEST_CONTEXT_PER_CPU;

typedef struct _FRM_GUEST_CONTEXT_COMMON {
  EPT_POINTER                 EptPointer;
  UINTN                       CompatiblePageTable;
  UINTN                       CompatiblePageTablePae;
  UINT64                      IoBitmapA;
  UINT64                      IoBitmapB;
  UINT64                      MsrBitmap;
  UINT32                      VmxTimerValue;
  FRM_GUEST_CONTEXT_PER_CPU   *GuestContextPerCpu;
} FRM_GUEST_CONTEXT_COMMON;

typedef struct _FRM_HOST_CONTEXT_PER_CPU {
  UINT32                      Index;
  UINT32                      ApicId;
  UINTN                       Stack;
  UINT64                      Vmcs;
} FRM_HOST_CONTEXT_PER_CPU;

typedef struct _FRM_HOST_CONTEXT_COMMON {
  SPIN_LOCK                   DebugLock;
  SPIN_LOCK                   MemoryLock;
  UINT32                      CpuNum;
  UINTN                       PageTable;
  IA32_DESCRIPTOR             Gdtr;
  IA32_DESCRIPTOR             Idtr;
  UINT64                      DmarPageTable;
  UINT64                      HeapBottom;
  UINT64                      HeapTop;
  UINT8                       PhysicalAddressBits;
  //
  // BUGBUG: Assume only one segment for client system.
  //
  UINT64                      PciExpressBaseAddress;
  UINT64                      PciExpressLength;
  UINT16                      AcpiTimerIoPortBaseAddress;
  UINT8                       AcpiTimerWidth;
  UINT16                      AcpiPmControlIoPortBaseAddress;
  UINT16                      ResetIoPortBaseAddress;
  // For S3
  UINT64                      LowMemoryBase;
  UINT64                      LowMemorySize;
  UINT64                      LowMemoryBackupBase;

  UINT64                      ImageBase;
  UINT64                      ImageSize;

  FRM_HOST_CONTEXT_PER_CPU    *HostContextPerCpu;
} FRM_HOST_CONTEXT_COMMON;

//
// IO Handler
//
#define IO_ACTION_NO_ACTION          0
#define IO_ACTION_PASSTHROUGH        1

/**

  This is IO read handler.

  @param Context Context for IO read handler
  @param Port    IO port
  @param Value   IO port value
  @param Action  IO read action

**/
typedef
VOID
(* IO_READ_HANDLER) (
  IN VOID     *Context,
  IN UINT16   Port,
  OUT UINT32  *Value,
  OUT UINT32  *Action
  );

/**

  This is IO write handler.

  @param Context Context for IO write handler
  @param Port    IO port
  @param Value   IO port value
  @param Action  IO write action

**/
typedef
VOID
(* IO_WRITE_HANDLER) (
  IN VOID     *Context,
  IN UINT16   Port,
  IN UINT32   Value,
  OUT UINT32  *Action
  );

/**

  This function register IO read handler.

  @param Port    IO port
  @param Size    IO port size
  @param Handler IO read handler
  @param Context Context for IO read handler

  @retval RETURN_SUCCESS           IO read handler is registered.
  @retval RETURN_BUFFER_TOO_SMALL  IO read handler is not registered.

**/
RETURN_STATUS
RegisterIoRead (
  IN UINT16             Port,
  IN UINTN              Size,
  IN IO_READ_HANDLER    Handler,
  IN VOID               *Context
  );

/**

  This function register IO write handler.

  @param Port    IO port
  @param Size    IO port size
  @param Handler IO write handler
  @param Context Context for IO read handler

  @retval RETURN_SUCCESS           IO write handler is registered.
  @retval RETURN_BUFFER_TOO_SMALL  IO write handler is not registered.

**/
RETURN_STATUS
RegisterIoWrite (
  IN UINT16             Port,
  IN UINTN              Size,
  IN IO_WRITE_HANDLER   Handler,
  IN VOID               *Context
  );

/**

  This function return local APIC ID.

  @return Local APIC ID

**/
UINT32
ReadLocalApicId (
  VOID
  );

/**

  This function return if it is BSP.

  @retval TRUE  It is BSP
  @retval FALSE It is AP

**/
BOOLEAN
IsBsp (
  VOID
  );

/**
  Send SIPI to APs.

  @param Vector  The AP startup vector
**/
VOID
SendSipi (
  UINT8  Vector
  );

/**

  This function return if processor support XState.

  @retval TRUE XState is supported
  @retval FALSE XState is supported

**/
BOOLEAN
IsXStateSupoprted (
  VOID
  );

/**

  This function find ACPI RSDPTR.

  @return ACPI RSDPTR

**/
VOID *
FindAcpiRsdPtr(
  VOID
  );

/**
  Get ACPI table via signature.

  @param Signature ACPI Table signature.

  @return ACPI table pointer.
**/
VOID *
GetAcpiTableViaSignature(
  IN UINT32 Signature
  );

/**

  This function return CPU number from MADT.

  @return CPU number

**/
UINT32
GetCpuNumFromAcpi (
  VOID
  );

/**

  This function return APIC ID list from MADT.

  @param ApicIdList An array for APIC ID

**/
VOID
GetApicIdListFromAcpi (
  IN OUT UINT32  *ApicIdList
  );

/**

  This function return PCI Express information from MCFG.
  Only one segment information is returned.

  @param  PciExpressBaseAddress  PCI Express base address
  @param  PciExpressLength       PCI Express length

  @return PciExpressBaseAddress

**/
UINT64
GetPciExpressInfoFromAcpi (
  OUT UINT64  *PciExpressBaseAddress,
  OUT UINT64  *PciExpressLength
  );

/**

  This function return ACPI timer port from FADT.

  @param AcpiTimerWidth  ACPI timer width

  @return ACPI timer port

**/
UINT16
GetAcpiTimerPort (
  OUT UINT8  *AcpiTimerWidth
  );

/**

  This function return ACPI PmControl port from FADT.

  @return ACPI PmControl port

**/
UINT16
GetAcpiPmControlPort (
  VOID
  );

/**

  This function return ACPI Reset port from FADT.

  @return ACPI Reset port

**/
UINT16
GetAcpiResetPort (
  VOID
  );

#endif
