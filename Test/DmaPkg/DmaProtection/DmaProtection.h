/** @file

  Copyright (c) 2014, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _DMAR_PROTECTION_H_
#define _DMAR_PROTECTION_H_

#include <Uefi.h>
#include <PiDxe.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/IoLib.h>
#include <Library/PciLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>

#include <Guid/EventGroup.h>
#include <Guid/Acpi.h>

#include <Protocol/DxeSmmReadyToLock.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/PciIo.h>
#include <Protocol/PciEnumerationComplete.h>

#include <IndustryStandard/Pci.h>
#include <IndustryStandard/DMARemappingReportingTable.h>
#include <IndustryStandard/Vtd.h>

//#define ALIGN_VALUE(Value, Alignment) ((Value) + (((Alignment) - (Value)) & ((Alignment) - 1)))
#define ALIGN_VALUE_UP(Value, Alignment)  (((Value) + (Alignment) - 1) & (~((Alignment) - 1)))
#define ALIGN_VALUE_LOW(Value, Alignment) ((Value) & (~((Alignment) - 1)))

#define UEFI_DOMAIN_ID                  1

#define MAX_VTD_UNITS                   4
#define MAX_PCI_DESCRIPTORS             0x100

typedef struct {
  UINT8          Bus;
  UINT8          Device;
  UINT8          Function;
} PCI_DESCRIPTOR;

typedef struct {
  BOOLEAN                IncludeAllFlag;
  UINT8                  PciDescriptorNumber;
  // Need add one, because the 0 is servered as not assigned
  PCI_DESCRIPTOR         PciDescriptors[MAX_PCI_DESCRIPTORS+1];
} PCI_DEVICE_INFORMATION;

typedef struct {
  UINT64                           VtdUnitBaseAddress;
  VTD_CAP_REG                      CapReg;
  VTD_ECAP_REG                     ECapReg;
  VTD_ROOT_ENTRY                   *RootEntryTable;
  VTD_EXT_ROOT_ENTRY               *ExtRootEntryTable;
  VTD_SECOND_LEVEL_PAGING_ENTRY    *SecondLevelPagingEntry;
  VTD_FIRST_LEVEL_PAGING_ENTRY     *FirstLevelPagingEntry;
  UINT8                            PASIDTableSize;
  VTD_PASID_ENTRY                  *PASIDTable;
  BOOLEAN                          HasDirtyPages;
  PCI_DEVICE_INFORMATION           PciDeviceInfo;
} VTD_UNIT_INFORMATION;

extern EFI_ACPI_DMAR_DESCRIPTION_TABLE  *mAcpiDmarTable;

extern UINT32                           mVtdUnitNumber;
extern VTD_UNIT_INFORMATION             mVtdUnitInformation[MAX_VTD_UNITS];

/**

  This function prepare VTd configuration.

  @return EFI_SUCCESS  prepare VTd configuration successfully.

**/
EFI_STATUS
PrepareVtdConfig (
  VOID
  );

/**

  This function setup VTd translation table.

  @param Below4GMemoryLimit  Below4G memory limit
  @param Above4GMemoryLimit  Above4G memory limit

  @retval EFI_SUCCESS setup VTd translation table successfully.

**/
EFI_STATUS
SetupTranslationTable (
  IN UINT64          Below4GMemoryLimit,
  IN UINT64          Above4GMemoryLimit
  );

/**

  This function enable DMAR.

  @return EFI_SUCCESS enable DMAR successfully.

**/
EFI_STATUS
EnableDmar (
  VOID
  );

/**

  This function disable DMAR.

  @return EFI_SUCCESS disable DMAR successfully.

**/
EFI_STATUS
DisableDmar (
  VOID
  );

/**

  This function invalidate VTd IOTLBPage.

  @param VtdIndex          VTd engine index
  @param Address           Address
  @param AddressMode       Address mode

  @retval EFI_SUCCESS invalidate VTd IOTLBPage successfully.
**/
EFI_STATUS
InvalidateVtdIOTLBPage (
  IN UINTN  VtdIndex,
  IN UINT64 Address,
  IN UINT8  AddressMode
  );

/**

  This function invalidate VTd IOTLBDomain.

  @param VtdIndex          VTd engine index
  @param DomainIdentifier  VTd Domain Identifier

  @retval EFI_SUCCESS invalidate VTd IOTLBDomain successfully.
**/
EFI_STATUS
InvalidateVtdIOTLBDomain (
  IN UINTN  VtdIndex,
  IN UINT16 DomainIdentifier
  );

/**

  This function invalidate VTd IOTLBGlobal.

  @param VtdIndex      VTd engine index

  @retval EFI_SUCCESS invalidate VTd IOTLBGlobal successfully.
**/
EFI_STATUS
InvalidateVtdIOTLBGlobal (
  IN UINTN  VtdIndex
  );

/**

  This function dump VTd register.

**/
VOID
DumpVtdRegs (
  VOID
  );

/**

  This function dump VTd cap register.

  @param CapReg VTd cap register

**/
VOID
DumpVtdCapRegs (
  IN VTD_CAP_REG *CapReg
  );

/**

  This function dump VTd Ecap register.

  @param ECapReg VTd Ecap register

**/
VOID
DumpVtdECapRegs (
  IN VTD_ECAP_REG *ECapReg
  );

/**

  This function register PCI device.

  @param VtdIndex      VTd engine index
  @param Bus           Pci bus
  @param Device        Pci device
  @param Function      Pci function
  @param CheckExist    If check PCI device exist

**/
EFI_STATUS
RegisterPciDevice (
  IN UINTN          VtdIndex,
  IN UINT8          Bus,
  IN UINT8          Device,
  IN UINT8          Function,
  IN BOOLEAN        CheckExist
  );

/**

  This function scan PCI bus.

  @param VtdIndex      VTd engine index
  @param Bus           Pci bus

  @retval EFI_SUCCESS  scan PCI bus successfully.

**/
EFI_STATUS
ScanPciBus (
  IN UINTN          VtdIndex,
  IN UINT8          Bus
  );

/**

  This function dump PCI device information.

  @param VtdIndex      VTd engine index

**/
VOID
DumpPciDeviceInfo (
  IN UINTN  VtdIndex
  );

/**

  This function return VTd engine index by PCI bus/device/function.

  @param Bus           Pci bus
  @param Device        Pci device
  @param Function      Pci function

  @return VTd engine index

**/
UINTN
FindVtdIndexByPciDevice (
  IN UINT8          Bus,
  IN UINT8          Device,
  IN UINT8          Function
  );

/**

  This function find DMAR ACPI table.

  @retval EFI_SUCCESS DMAR table is found.
**/
EFI_STATUS
GetDmarAcpiTable (
  VOID
  );

/**

  This function parse DMAR table.

  @retval EFI_SUCCESS DMAR table is parsed.
**/
EFI_STATUS
ParseDmarAcpiTable (
  VOID
  );

/**

  This function dump context entry table.

  @param RootEntry  root entry

**/
VOID
DumpDmarContextEntryTable (
  IN VTD_ROOT_ENTRY *RootEntry
  );

/**

  This function dump ext context entry table.

  @param ExtRootEntry  ext root entry

**/
VOID
DumpDmarExtContextEntryTable (
  IN VTD_EXT_ROOT_ENTRY *ExtRootEntry
  );

/**

  This function dump 1st level paging entry.

  @param FirstLevelPagingEntry  1st level paging entry

**/
VOID
DumpFirstLevelPagingEntry (
  IN VOID *FirstLevelPagingEntry
  );

/**

  This function dump 2nd level paging entry.

  @param SecondLevelPagingEntry  2nd level paging entry

**/
VOID
DumpSecondLevelPagingEntry (
  IN VOID *SecondLevelPagingEntry
  );

/**

  This function set page attribute on VTd engine.

  @param VtdIndex      VTd engine index
  @param BaseAddress   BaseAddress
  @param Length        Length
  @param Allow         If allow access

  @retval EFI_SUCCESS  set page attribute successfully

**/
EFI_STATUS
SetPageAttribute (
  IN UINTN                 VtdIndex,
  IN UINT64                BaseAddress,
  IN UINT64                Length,
  IN BOOLEAN               Allow
  );

/**

  This function set page attribute for PCI device.

  @param Bus           Pci bus
  @param Device        Pci device
  @param Function      Pci function
  @param BaseAddress   BaseAddress
  @param Length        Length
  @param Allow         If allow access

  @retval EFI_SUCCESS  set page attribute successfully

**/
EFI_STATUS
SetPageAttributeByPciDevice (
  IN UINT8                 Bus,
  IN UINT8                 Device,
  IN UINT8                 Function,
  IN UINT64                BaseAddress,
  IN UINT64                Length,
  IN BOOLEAN               Allow
  );

/**

  This function set access attribute for PCI device.

  @param Bus           Pci bus
  @param Device        Pci device
  @param Function      Pci function
  @param BaseAddress   BaseAddress
  @param Length        Length
  @param Allow         If allow access

  @retval EFI_SUCCESS  set access attribute successfully

**/
EFI_STATUS
SetAccessAttribute (
  IN UINT8                 Bus,
  IN UINT8                 Device,
  IN UINT8                 Function,
  IN UINT64                BaseAddress,
  IN UINT64                Length,
  IN BOOLEAN               Allow
  );

/**

  This function hook PCI related protocol.

  @return EFI_SUCCESS hook PCI related protocol successfully.

**/
EFI_STATUS
HookPciProtocol (
  VOID
  );

/**

  This function activate hook PCI protocol.

  @return EFI_SUCCESS activate hook PCI protocol successfully.

**/
EFI_STATUS
ActivateHookPciProtocol (
  VOID
  );

/**
  Initialization for CSM.
**/
VOID
CsmInit (
  VOID
  );

#endif
