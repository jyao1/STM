/** @file
  DMAR header file

  Copyright (c) 2010, Intel Corporation. All rights reserved. <BR>
  This software and associated documentation
  (if any) is furnished under a license and may only be used or
  copied in accordance with the terms of the license. Except as
  permitted by such license, no part of this software or
  documentation may be reproduced, stored in a retrieval system, or
  transmitted in any form or by any means without the express
  written consent of Intel Corporation.

**/

#ifndef _EFI_DMA_REMAPPING_REPORTING_TABLE_H_
#define _EFI_DMA_REMAPPING_REPORTING_TABLE_H_

#include <IndustryStandard/Acpi.h>

//
// DMAR Revision
//
#define EFI_ACPI_DMAR_DESCRIPTION_TABLE_REVISION   0x01

//
// Ensure proper structure formats
//
#pragma pack (1)

//
// Definition for DMA Remapping Structure Types
//
#define EFI_ACPI_DMA_REMAPPING_STRUCTURE_TYPE_DRHD  0
#define EFI_ACPI_DMA_REMAPPING_STRUCTURE_TYPE_RMRR  1
#define EFI_ACPI_DMA_REMAPPING_STRUCTURE_TYPE_ATSR  2
#define EFI_ACPI_DMA_REMAPPING_STRUCTURE_TYPE_RHSA  3
#define EFI_ACPI_DMA_REMAPPING_STRUCTURE_TYPE_ANDD  4

//
// Definition for DMA Remapping Structure Header
//
typedef struct {
  UINT16                                      Type;
  UINT16                                      Length;
} EFI_ACPI_DMAR_STRUCTURE_HEADER;

//
// Definition for DMA-Remapping PCI Path
//
typedef struct {
  UINT8         Device;
  UINT8         Function;
} EFI_ACPI_DMAR_PCI_PATH;

//
// Definition for DMA-Remapping Device Scope Entry Structure
//
#define EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_ENDPOINT                 0x01
#define EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_BRIDGE                   0x02
#define EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_IOAPIC                   0x03
#define EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_MSI_CAPABLE_HPET         0x04
#define EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_ACPI_NAMESPACE_DEVICE    0x05
typedef struct {
  UINT8                   DeviceScopeEntryType;
  UINT8                   Length;
  UINT16                  Reserved_2;
  UINT8                   EnumerationID;
  UINT8                   StartingBusNumber;
//EFI_ACPI_DMAR_PCI_PATH  PciPath[];
} EFI_ACPI_DMAR_DEVICE_SCOPE_ENTRY_STRUCTURE;

//
// Definition for DMA-Remapping Hardware Definition (DRHD) Structure
//
#define EFI_ACPI_DMAR_DRHD_FLAGS_INCLUDE_ALL_SET     0x1
#define EFI_ACPI_DMAR_DRHD_FLAGS_INCLUDE_ALL_CLEAR   0x0
typedef struct {
  UINT16                                      Type;
  UINT16                                      Length;
  UINT8                                       Flags;
  UINT8                                       Reserved_5;
  UINT16                                      SegmentNumber;
  UINT64                                      RegisterBaseAddress;
//EFI_ACPI_DMAR_DEVICE_SCOPE_ENTRY_STRUCTURE  DeviceScope[];
} EFI_ACPI_DMAR_HARDWARE_UNIT_DEFINITION_STRUCTURE;

//
// Definition for Reserved Memory Region Reporting (RMRR) Structure
//
typedef struct {
  UINT16                                      Type;
  UINT16                                      Length;
  UINT8                                       Reserved_4[2];
  UINT16                                      SegmentNumber;
  UINT64                                      ReservedMemoryRegionBaseAddress;
  UINT64                                      ReservedMemoryRegionLimitAddress;
//EFI_ACPI_DMAR_DEVICE_SCOPE_ENTRY_STRUCTURE  DeviceScope[];
} EFI_ACPI_DMAR_RESERVED_MEMORY_REGION_REPORTING_STRUCTURE;

//
// Definition for Root Port ATS Capability Reporting (ATSR) Structure
//
#define EFI_ACPI_DMAR_ATSR_FLAGS_ALL_PORTS_SET     0x1
#define EFI_ACPI_DMAR_ATSR_FLAGS_ALL_PORTS_CLEAR   0x0
typedef struct {
  UINT16                                      Type;
  UINT16                                      Length;
  UINT8                                       Flags;
  UINT8                                       Reserved_5;
  UINT16                                      SegmentNumber;
//EFI_ACPI_DMAR_DEVICE_SCOPE_ENTRY_STRUCTURE  DeviceScope[];
} EFI_ACPI_DMAR_ROOT_PORT_ATS_CAPABILITY_REPORTING_STRUCTURE;

//
// Definition for Remapping Hardware Static Affinity (RHSA) Structure
//
typedef struct {
  UINT16                                      Type;
  UINT16                                      Length;
  UINT32                                      Reserved;
  UINT64                                      RegisterBaseAddress;
  UINT32                                      ProximityDomain;
} EFI_ACPI_DMAR_REMAPPING_HARDWARE_STATIC_AFFINITY_STRUCTURE;

//
// Definition for ACPI Name-space Device Declaration (ANDD) Structure
//
typedef struct {
  UINT16                                      Type;
  UINT16                                      Length;
  UINT8                                       Reserved_4[3];
  UINT8                                       ACPIDeviceNumber;
//CHAR8                                       ACPIObjectName;
} EFI_ACPI_DMAR_ACPI_NAME_SPACE_DEVICE_DECLARATION_STRUCTURE;

//
// Definition for DMA Remapping Structure
//
typedef union {
  EFI_ACPI_DMAR_STRUCTURE_HEADER                               DMARStructureHeader;
  EFI_ACPI_DMAR_HARDWARE_UNIT_DEFINITION_STRUCTURE             DMARHardwareUnitDefinition;
  EFI_ACPI_DMAR_RESERVED_MEMORY_REGION_REPORTING_STRUCTURE     DMARReservedMemoryRegionReporting;
  EFI_ACPI_DMAR_ROOT_PORT_ATS_CAPABILITY_REPORTING_STRUCTURE   DMARRootPortATSCapabilityReporting;
  EFI_ACPI_DMAR_REMAPPING_HARDWARE_STATIC_AFFINITY_STRUCTURE   DMARRemappingHardwareStaticAffinity;
} EFI_ACPI_DMA_REMAPPING_STRUCTURE;

//
// Definition for DMA-Remapping Reporting ACPI Table
//
#define EFI_ACPI_DMAR_TABLE_FLAGS_INTR_REMAP_CLEAR          0x00
#define EFI_ACPI_DMAR_TABLE_FLAGS_INTR_REMAP_SET            0x01
typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER                               Header;
  UINT8                                                     HostAddressWidth;
  UINT8                                                     Flags;
  UINT8                                                     Reserved_38[10];
} EFI_ACPI_DMAR_DESCRIPTION_TABLE;

#pragma pack()

#endif

