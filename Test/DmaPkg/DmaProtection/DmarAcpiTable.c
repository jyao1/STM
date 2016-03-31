/** @file

  Copyright (c) 2014, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "DmaProtection.h"

#pragma pack(1)

typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER  Header;
  UINT32                       Entry;
} RSDT_TABLE;

typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER  Header;
  UINT64                       Entry;
} XSDT_TABLE;

#pragma pack()

EFI_ACPI_DMAR_DESCRIPTION_TABLE  *mAcpiDmarTable;

/**

  This function dump DMAR device scope entry.

  @param DmarDeviceScopeEntry  DMAR device scope entry.

**/
VOID
DumpDmarDeviceScopeEntry (
  IN EFI_ACPI_DMAR_DEVICE_SCOPE_ENTRY_STRUCTURE     *DmarDeviceScopeEntry
  )
{
  UINTN   PciPathNumber;
  UINTN   PciPathIndex;
  EFI_ACPI_DMAR_PCI_PATH  *PciPath;

  if (DmarDeviceScopeEntry == NULL) {
    return;
  }

  DEBUG ((EFI_D_INFO,
    "    *************************************************************************\n"
    "    *       DMA-Remapping Device Scope Entry Structure                      *\n"
    "    *************************************************************************\n"
    ));
  DEBUG ((EFI_D_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "    DMAR Device Scope Entry address ...................... 0x%016lx\n" :
    "    DMAR Device Scope Entry address ...................... 0x%08x\n",
    DmarDeviceScopeEntry
    ));
  DEBUG ((EFI_D_INFO,
    "      Device Scope Entry Type ............................ 0x%02x\n",
    DmarDeviceScopeEntry->DeviceScopeEntryType
    ));
  switch (DmarDeviceScopeEntry->DeviceScopeEntryType) {
  case EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_ENDPOINT:
    DEBUG ((EFI_D_INFO,
      "        PCI Endpoint Device\n"
      ));
    break;
  case EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_BRIDGE:
    DEBUG ((EFI_D_INFO,
      "        PCI Sub-hierachy\n"
      ));
    break;
  case EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_IOAPIC:
    DEBUG ((EFI_D_INFO,
      "        IOAPIC\n"
      ));
    break;
  case EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_MSI_CAPABLE_HPET:
    DEBUG ((EFI_D_INFO,
      "        MSI Capable HPET\n"
      ));
    break;
  case EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_ACPI_NAMESPACE_DEVICE:
    DEBUG ((EFI_D_INFO,
      "        ACPI Namespace Device\n"
      ));
    break;
  default:
    break;
  }
  DEBUG ((EFI_D_INFO,
    "      Length ............................................. 0x%02x\n",
    DmarDeviceScopeEntry->Length
    ));
  DEBUG ((EFI_D_INFO,
    "      Enumeration ID ..................................... 0x%02x\n",
    DmarDeviceScopeEntry->EnumerationID
    ));
  DEBUG ((EFI_D_INFO,
    "      Starting Bus Number ................................ 0x%02x\n",
    DmarDeviceScopeEntry->StartingBusNumber
    ));

  PciPathNumber = (DmarDeviceScopeEntry->Length - sizeof(EFI_ACPI_DMAR_DEVICE_SCOPE_ENTRY_STRUCTURE)) / sizeof(EFI_ACPI_DMAR_PCI_PATH);
  PciPath = (EFI_ACPI_DMAR_PCI_PATH *)(DmarDeviceScopeEntry + 1);
  for (PciPathIndex = 0; PciPathIndex < PciPathNumber; PciPathIndex++) {
    DEBUG ((EFI_D_INFO,
      "      Device ............................................. 0x%02x\n",
      PciPath[PciPathIndex].Device
      ));
    DEBUG ((EFI_D_INFO,
      "      Function ........................................... 0x%02x\n",
      PciPath[PciPathIndex].Function
      ));
  }

  DEBUG ((EFI_D_INFO,
    "    *************************************************************************\n\n"
    ));

  return;
}

/**

  This function dump DMAR ANDD structure.

  @param Andd  DMAR ANDD structure.

**/
VOID
DumpDmarAndd (
  IN EFI_ACPI_DMAR_ACPI_NAME_SPACE_DEVICE_DECLARATION_STRUCTURE *Andd
  )
{
  if (Andd == NULL) {
    return;
  }

  DEBUG ((EFI_D_INFO,
    "  ***************************************************************************\n"
    "  *       ACPI Name-space Device Declaration Structure                      *\n"
    "  ***************************************************************************\n"
    ));
  DEBUG ((EFI_D_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "  ANDD address ........................................... 0x%016lx\n" :
    "  ANDD address ........................................... 0x%08x\n",
    Andd
    ));
  DEBUG ((EFI_D_INFO,
    "    Type ................................................. 0x%04x\n",
    Andd->Type
    ));
  DEBUG ((EFI_D_INFO,
    "    Length ............................................... 0x%04x\n",
    Andd->Length
    ));
  DEBUG ((EFI_D_INFO,
    "    ACPI Device Number ................................... 0x%02x\n",
    Andd->ACPIDeviceNumber
    ));
  DEBUG ((EFI_D_INFO,
    "    ACPI Object Name ..................................... '%a'\n",
    (Andd + 1)
    ));

  DEBUG ((EFI_D_INFO,
    "  ***************************************************************************\n\n"
    ));

  return;
}

/**

  This function dump DMAR RHSA structure.

  @param Rhsa  DMAR RHSA structure.

**/
VOID
DumpDmarRhsa (
  IN EFI_ACPI_DMAR_REMAPPING_HARDWARE_STATUS_AFFINITY_STRUCTURE *Rhsa
  )
{
  if (Rhsa == NULL) {
    return;
  }

  DEBUG ((EFI_D_INFO,
    "  ***************************************************************************\n"
    "  *       Remapping Hardware Status Affinity Structure                      *\n"
    "  ***************************************************************************\n"
    ));
  DEBUG ((EFI_D_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "  RHSA address ........................................... 0x%016lx\n" :
    "  RHSA address ........................................... 0x%08x\n",
    Rhsa
    ));
  DEBUG ((EFI_D_INFO,
    "    Type ................................................. 0x%04x\n",
    Rhsa->Type
    ));
  DEBUG ((EFI_D_INFO,
    "    Length ............................................... 0x%04x\n",
    Rhsa->Length
    ));
  DEBUG ((EFI_D_INFO,
    "    Register Base Address ................................ 0x%016lx\n",
    Rhsa->RegisterBaseAddress
    ));
  DEBUG ((EFI_D_INFO,
    "    Proximity Domain ..................................... 0x%08x\n",
    Rhsa->ProximityDomain
    ));

  DEBUG ((EFI_D_INFO,
    "  ***************************************************************************\n\n"
    ));

  return;
}

/**

  This function dump DMAR ATSR structure.

  @param Atsr  DMAR ATSR structure.

**/
VOID
DumpDmarAtsr (
  IN EFI_ACPI_DMAR_ROOT_PORT_ATS_CAPABILITY_REPORTING_STRUCTURE *Atsr
  )
{
  EFI_ACPI_DMAR_DEVICE_SCOPE_ENTRY_STRUCTURE       *DmarDeviceScopeEntry;
  INTN                                    AtsrLen;

  if (Atsr == NULL) {
    return;
  }

  DEBUG ((EFI_D_INFO,
    "  ***************************************************************************\n"
    "  *       Root Port ATS Capability Reporting Structure                      *\n"
    "  ***************************************************************************\n"
    ));
  DEBUG ((EFI_D_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "  ATSR address ........................................... 0x%016lx\n" :
    "  ATSR address ........................................... 0x%08x\n",
    Atsr
    ));
  DEBUG ((EFI_D_INFO,
    "    Type ................................................. 0x%04x\n",
    Atsr->Type
    ));
  DEBUG ((EFI_D_INFO,
    "    Length ............................................... 0x%04x\n",
    Atsr->Length
    ));
  DEBUG ((EFI_D_INFO,
    "    Flags ................................................ 0x%02x\n",
    Atsr->Flags
    ));
  DEBUG ((EFI_D_INFO,
    "      ALL_PORTS .......................................... 0x%02x\n",
    Atsr->Flags & EFI_ACPI_DMAR_ATSR_FLAGS_ALL_PORTS_SET
    ));
  DEBUG ((EFI_D_INFO,
    "    Segment Number ....................................... 0x%04x\n",
    Atsr->SegmentNumber
    ));

  AtsrLen  = Atsr->Length - sizeof(EFI_ACPI_DMAR_ROOT_PORT_ATS_CAPABILITY_REPORTING_STRUCTURE);
  DmarDeviceScopeEntry = (EFI_ACPI_DMAR_DEVICE_SCOPE_ENTRY_STRUCTURE *)(Atsr + 1);
  while (AtsrLen > 0) {
    DumpDmarDeviceScopeEntry (DmarDeviceScopeEntry);
    AtsrLen -= DmarDeviceScopeEntry->Length;
    DmarDeviceScopeEntry = (EFI_ACPI_DMAR_DEVICE_SCOPE_ENTRY_STRUCTURE *)((UINTN)DmarDeviceScopeEntry + DmarDeviceScopeEntry->Length);
  }

  DEBUG ((EFI_D_INFO,
    "  ***************************************************************************\n\n"
    ));

  return;
}

/**

  This function dump DMAR RMRR structure.

  @param Rmrr  DMAR RMRR structure.

**/
VOID
DumpDmarRmrr (
  IN EFI_ACPI_DMAR_RESERVED_MEMORY_REGION_REPORTING_STRUCTURE *Rmrr
  )
{
  EFI_ACPI_DMAR_DEVICE_SCOPE_ENTRY_STRUCTURE       *DmarDeviceScopeEntry;
  INTN                                    RmrrLen;

  if (Rmrr == NULL) {
    return;
  }

  DEBUG ((EFI_D_INFO,
    "  ***************************************************************************\n"
    "  *       Reserved Memory Region Reporting Structure                        *\n"
    "  ***************************************************************************\n"
    ));
  DEBUG ((EFI_D_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "  RMRR address ........................................... 0x%016lx\n" :
    "  RMRR address ........................................... 0x%08x\n",
    Rmrr
    ));
  DEBUG ((EFI_D_INFO,
    "    Type ................................................. 0x%04x\n",
    Rmrr->Type
    ));
  DEBUG ((EFI_D_INFO,
    "    Length ............................................... 0x%04x\n",
    Rmrr->Length
    ));
  DEBUG ((EFI_D_INFO,
    "    Segment Number ....................................... 0x%04x\n",
    Rmrr->SegmentNumber
    ));
  DEBUG ((EFI_D_INFO,
    "    Reserved Memory Region Base Address .................. 0x%016lx\n",
    Rmrr->ReservedMemoryRegionBaseAddress
    ));
  DEBUG ((EFI_D_INFO,
    "    Reserved Memory Region Limit Address ................. 0x%016lx\n",
    Rmrr->ReservedMemoryRegionLimitAddress
    ));

  RmrrLen  = Rmrr->Length - sizeof(EFI_ACPI_DMAR_RESERVED_MEMORY_REGION_REPORTING_STRUCTURE);
  DmarDeviceScopeEntry = (EFI_ACPI_DMAR_DEVICE_SCOPE_ENTRY_STRUCTURE *)(Rmrr + 1);
  while (RmrrLen > 0) {
    DumpDmarDeviceScopeEntry (DmarDeviceScopeEntry);
    RmrrLen -= DmarDeviceScopeEntry->Length;
    DmarDeviceScopeEntry = (EFI_ACPI_DMAR_DEVICE_SCOPE_ENTRY_STRUCTURE *)((UINTN)DmarDeviceScopeEntry + DmarDeviceScopeEntry->Length);
  }

  DEBUG ((EFI_D_INFO,
    "  ***************************************************************************\n\n"
    ));

  return;
}

/**

  This function dump DMAR DRHD structure.

  @param Drhd  DMAR DRHD structure.

**/
VOID
DumpDmarDrhd (
  IN EFI_ACPI_DMAR_HARDWARE_UNIT_DEFINITION_STRUCTURE *Drhd
  )
{
  EFI_ACPI_DMAR_DEVICE_SCOPE_ENTRY_STRUCTURE       *DmarDeviceScopeEntry;
  INTN                                    DrhdLen;

  if (Drhd == NULL) {
    return;
  }

  DEBUG ((EFI_D_INFO,
    "  ***************************************************************************\n"
    "  *       DMA-Remapping Hardware Definition Structure                       *\n"
    "  ***************************************************************************\n"
    ));
  DEBUG ((EFI_D_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "  DRHD address ........................................... 0x%016lx\n" :
    "  DRHD address ........................................... 0x%08x\n",
    Drhd
    ));
  DEBUG ((EFI_D_INFO,
    "    Type ................................................. 0x%04x\n",
    Drhd->Type
    ));
  DEBUG ((EFI_D_INFO,
    "    Length ............................................... 0x%04x\n",
    Drhd->Length
    ));
  DEBUG ((EFI_D_INFO,
    "    Flags ................................................ 0x%02x\n",
    Drhd->Flags
    ));
  DEBUG ((EFI_D_INFO,
    "      INCLUDE_PCI_ALL .................................... 0x%02x\n",
    Drhd->Flags & EFI_ACPI_DMAR_DRHD_FLAGS_INCLUDE_ALL_SET
    ));
  DEBUG ((EFI_D_INFO,
    "    Segment Number ....................................... 0x%04x\n",
    Drhd->SegmentNumber
    ));
  DEBUG ((EFI_D_INFO,
    "    Register Base Address ................................ 0x%016lx\n",
    Drhd->RegisterBaseAddress
    ));

  DrhdLen  = Drhd->Length - sizeof(EFI_ACPI_DMAR_HARDWARE_UNIT_DEFINITION_STRUCTURE);
  DmarDeviceScopeEntry = (EFI_ACPI_DMAR_DEVICE_SCOPE_ENTRY_STRUCTURE *)(Drhd + 1);
  while (DrhdLen > 0) {
    DumpDmarDeviceScopeEntry (DmarDeviceScopeEntry);
    DrhdLen -= DmarDeviceScopeEntry->Length;
    DmarDeviceScopeEntry = (EFI_ACPI_DMAR_DEVICE_SCOPE_ENTRY_STRUCTURE *)((UINTN)DmarDeviceScopeEntry + DmarDeviceScopeEntry->Length);
  }

  DEBUG ((EFI_D_INFO,
    "  ***************************************************************************\n\n"
    ));

  return;
}

/**

  This function dump DMAR table.

  @param Dmar  DMAR table.

**/
VOID
DumpAcpiDMAR (
  IN EFI_ACPI_DMAR_DESCRIPTION_TABLE  *Dmar
  )
{
  EFI_ACPI_DMAR_STRUCTURE_HEADER *DmarHeader;
  INTN                  DmarLen;

  if (Dmar == NULL) {
    return;
  }

  //
  // Dump Dmar table
  //
  DEBUG ((EFI_D_INFO,
    "*****************************************************************************\n"
    "*         DMAR Table                                                        *\n"
    "*****************************************************************************\n"
    ));

  DEBUG ((EFI_D_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "DMAR address ............................................. 0x%016lx\n" :
    "DMAR address ............................................. 0x%08x\n",
    Dmar
    ));

  DEBUG ((EFI_D_INFO,
    "  Table Contents:\n"
    ));
  DEBUG ((EFI_D_INFO,
    "    Host Address Width ................................... 0x%02x\n",
    Dmar->HostAddressWidth
    ));
  DEBUG ((EFI_D_INFO,
    "    Flags ................................................ 0x%02x\n",
    Dmar->Flags
    ));
  DEBUG ((EFI_D_INFO,
    "      INTR_REMAP ......................................... 0x%02x\n",
    Dmar->Flags & EFI_ACPI_DMAR_TABLE_FLAGS_INTR_REMAP_SET
    ));
  DEBUG ((EFI_D_INFO,
    "      X2APIC_OPT_OUT_SET ................................. 0x%02x\n",
    Dmar->Flags & EFI_ACPI_DMAR_TABLE_FLAGS_X2APIC_OPT_OUT_SET
    ));

  DmarLen  = Dmar->Header.Length - sizeof(EFI_ACPI_DMAR_DESCRIPTION_TABLE);
  DmarHeader = (EFI_ACPI_DMAR_STRUCTURE_HEADER *)(Dmar + 1);
  while (DmarLen > 0) {
    switch (DmarHeader->Type) {
    case EFI_ACPI_DMA_REMAPPING_STRUCTURE_TYPE_DRHD:
      DumpDmarDrhd ((EFI_ACPI_DMAR_HARDWARE_UNIT_DEFINITION_STRUCTURE *)DmarHeader);
      break;
    case EFI_ACPI_DMA_REMAPPING_STRUCTURE_TYPE_RMRR:
      DumpDmarRmrr ((EFI_ACPI_DMAR_RESERVED_MEMORY_REGION_REPORTING_STRUCTURE *)DmarHeader);
      break;
    case EFI_ACPI_DMA_REMAPPING_STRUCTURE_TYPE_ATSR:
      DumpDmarAtsr ((EFI_ACPI_DMAR_ROOT_PORT_ATS_CAPABILITY_REPORTING_STRUCTURE *)DmarHeader);
      break;
    case EFI_ACPI_DMA_REMAPPING_STRUCTURE_TYPE_RHSA:
      DumpDmarRhsa ((EFI_ACPI_DMAR_REMAPPING_HARDWARE_STATUS_AFFINITY_STRUCTURE *)DmarHeader);
      break;
    case EFI_ACPI_DMA_REMAPPING_STRUCTURE_TYPE_ANDD:
      DumpDmarAndd ((EFI_ACPI_DMAR_ACPI_NAME_SPACE_DEVICE_DECLARATION_STRUCTURE *)DmarHeader);
      break;
    default:
      break;
    }
    DmarLen -= DmarHeader->Length;
    DmarHeader = (EFI_ACPI_DMAR_STRUCTURE_HEADER *)((UINTN)DmarHeader + DmarHeader->Length);
  }

  DEBUG ((EFI_D_INFO,
    "*****************************************************************************\n\n"
    ));

  return;
}

/**

  This function dump DMAR table.

**/
VOID
VtdDumpDmarTable (
  VOID
  )
{
  DumpAcpiDMAR ((EFI_ACPI_DMAR_DESCRIPTION_TABLE *)(UINTN)mAcpiDmarTable);
}

/**

  This function return Pci bus/device/function from DMAR device scrope entry.

  @param  DmarDevScopeEntry  DMAR device scope entry.
  @param  Bus                Pci bus.
  @param  Device             Pci device.
  @param  Function           Pci function.

  @retval EFI_SUCCESS Pci bus/device/function is returned.
**/
EFI_STATUS
GetPciBusDeviceFunction (
  IN  EFI_ACPI_DMAR_DEVICE_SCOPE_ENTRY_STRUCTURE *DmarDevScopeEntry,
  OUT UINT8                                      *Bus,
  OUT UINT8                                      *Device,
  OUT UINT8                                      *Function
  )
{
  EFI_ACPI_DMAR_PCI_PATH                     *DmarPciPath;
  UINT8                                      MyBus;
  UINT8                                      MyDevice;
  UINT8                                      MyFunction;

  DmarPciPath = (EFI_ACPI_DMAR_PCI_PATH *)((UINTN)(DmarDevScopeEntry + 1));
  MyBus = DmarDevScopeEntry->StartingBusNumber;
  MyDevice = DmarPciPath->Device;
  MyFunction = DmarPciPath->Function;

  while ((UINTN)DmarPciPath < (UINTN)DmarDevScopeEntry + DmarDevScopeEntry->Length) {
    MyBus = PciRead8 (PCI_LIB_ADDRESS(MyBus, MyDevice, MyFunction, PCI_BRIDGE_SECONDARY_BUS_REGISTER_OFFSET));
    MyDevice = DmarPciPath->Device;
    MyFunction = DmarPciPath->Function;
    DmarPciPath ++;
  }

  *Bus = MyBus;
  *Device = MyDevice;
  *Function = MyFunction;

  return EFI_SUCCESS;
}

/**

  This function process DHRD structure.

  @param  VtdIndex  VTD engine index.
  @param  DmarDrhd  DMAR DRHD structure.

  @retval EFI_SUCCESS DHRD table is processed.
**/
EFI_STATUS
ProcessDhrd (
  IN UINTN                                             VtdIndex,
  IN EFI_ACPI_DMAR_HARDWARE_UNIT_DEFINITION_STRUCTURE  *DmarDrhd
  )
{
  STATIC BOOLEAN                                    IncludeAllFlag = FALSE;
  EFI_ACPI_DMAR_DEVICE_SCOPE_ENTRY_STRUCTURE        *DmarDevScopeEntry;
  UINT8                                             Bus;
  UINT8                                             Device;
  UINT8                                             Function;
  UINT8                                             SecondaryBusNumber;
  EFI_STATUS                                        Status;

  if (IncludeAllFlag) {
    DEBUG ((EFI_D_INFO,"  DRHD after INCLUDE_ALL is not allowed\n"));
    return EFI_DEVICE_ERROR;
  }

  mVtdUnitInformation[VtdIndex].VtdUnitBaseAddress = DmarDrhd->RegisterBaseAddress;
  DEBUG ((EFI_D_INFO,"  VTD (%d) BaseAddress -  0x%016lx\n", VtdIndex, DmarDrhd->RegisterBaseAddress));

  if ((DmarDrhd->Flags & EFI_ACPI_DMAR_DRHD_FLAGS_INCLUDE_ALL_SET) != 0) {
    mVtdUnitInformation[VtdIndex].PciDeviceInfo.IncludeAllFlag = TRUE;
    IncludeAllFlag = TRUE;
    DEBUG ((EFI_D_INFO,"  ProcessDhrd: with INCLUDE ALL\n"));

    Status = ScanPciBus(VtdIndex, 0);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    return EFI_SUCCESS;
  }

  //
  // No INCLUDE_ALL
  //
  DEBUG ((EFI_D_INFO,"  ProcessDhrd: without INCLUDE ALL\n"));
  DmarDevScopeEntry = (EFI_ACPI_DMAR_DEVICE_SCOPE_ENTRY_STRUCTURE *)((UINTN)(DmarDrhd + 1));
  while ((UINTN)DmarDevScopeEntry < (UINTN)DmarDrhd + DmarDrhd->Length) {

    Status = GetPciBusDeviceFunction (DmarDevScopeEntry, &Bus, &Device, &Function);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Status = RegisterPciDevice (VtdIndex, Bus, Device, Function, TRUE);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    switch (DmarDevScopeEntry->DeviceScopeEntryType) {
    case EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_ENDPOINT:
      DEBUG ((EFI_D_INFO,"  ProcessDhrd: PCI Endpoint B%02x D%02x F%02x\n", Bus, Device, Function));
      break;
    case EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_BRIDGE:
      DEBUG ((EFI_D_INFO,"  ProcessDhrd: PCI-PCI bridge B%02x D%02x F%02x\n", Bus, Device, Function));
      SecondaryBusNumber = PciRead8 (PCI_LIB_ADDRESS(Bus, Device, Function, PCI_BRIDGE_SECONDARY_BUS_REGISTER_OFFSET));
      Status = ScanPciBus (VtdIndex, SecondaryBusNumber);
      if (EFI_ERROR (Status)) {
        return Status;
      }
      break;
    default:
      break;
    }

    DmarDevScopeEntry = (EFI_ACPI_DMAR_DEVICE_SCOPE_ENTRY_STRUCTURE *)((UINTN)DmarDevScopeEntry + DmarDevScopeEntry->Length);
  }

  return EFI_SUCCESS;
}

/**

  This function process RMRR structure.

  @param  DmarRmrr  DMAR RMRR structure.

  @retval EFI_SUCCESS RMRR table is processed.
**/
EFI_STATUS
ProcessRmrr (
  IN EFI_ACPI_DMAR_RESERVED_MEMORY_REGION_REPORTING_STRUCTURE  *DmarRmrr
  )
{
  EFI_ACPI_DMAR_DEVICE_SCOPE_ENTRY_STRUCTURE        *DmarDevScopeEntry;
  UINT8                                             Bus;
  UINT8                                             Device;
  UINT8                                             Function;
  EFI_STATUS                                        Status;

  DEBUG ((EFI_D_INFO,"  RMRR (Base 0x%016lx, Limit 0x%016lx)\n", DmarRmrr->ReservedMemoryRegionBaseAddress, DmarRmrr->ReservedMemoryRegionLimitAddress));

  DmarDevScopeEntry = (EFI_ACPI_DMAR_DEVICE_SCOPE_ENTRY_STRUCTURE *)((UINTN)(DmarRmrr + 1));
  while ((UINTN)DmarDevScopeEntry < (UINTN)DmarRmrr + DmarRmrr->Length) {
    if (DmarDevScopeEntry->DeviceScopeEntryType != EFI_ACPI_DEVICE_SCOPE_ENTRY_TYPE_ENDPOINT) {
      DEBUG ((EFI_D_INFO,"RMRR DevScopeEntryType is not endpoint, type[0x%x] \n", DmarDevScopeEntry->DeviceScopeEntryType));
      return EFI_DEVICE_ERROR;
    }

    Status = GetPciBusDeviceFunction (DmarDevScopeEntry, &Bus, &Device, &Function);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    DEBUG ((EFI_D_INFO,"RMRR B%02x D%02x F%02x\n", Bus, Device, Function));

    Status = SetAccessAttribute (
               Bus,
               Device,
               Function,
               DmarRmrr->ReservedMemoryRegionBaseAddress,
               DmarRmrr->ReservedMemoryRegionLimitAddress + 1 - DmarRmrr->ReservedMemoryRegionBaseAddress,
               TRUE
               );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    DmarDevScopeEntry = (EFI_ACPI_DMAR_DEVICE_SCOPE_ENTRY_STRUCTURE *)((UINTN)DmarDevScopeEntry + DmarDevScopeEntry->Length);
  }

  return EFI_SUCCESS;
}

/**

  This function parse DMAR table.

  @retval EFI_SUCCESS DMAR table is parsed.
**/
EFI_STATUS
ParseDmarAcpiTable (
  VOID
  )
{
  EFI_ACPI_DMAR_STRUCTURE_HEADER                    *DmarHeader;
  EFI_STATUS                                        Status;
  UINTN                                             VtdIndex;

  ZeroMem (&mVtdUnitInformation, sizeof(mVtdUnitInformation));
  mVtdUnitNumber = 0;

  DmarHeader = (EFI_ACPI_DMAR_STRUCTURE_HEADER *)((UINTN)(mAcpiDmarTable + 1));
  while ((UINTN)DmarHeader < (UINTN)mAcpiDmarTable + mAcpiDmarTable->Header.Length) {
    switch (DmarHeader->Type) {
    case EFI_ACPI_DMA_REMAPPING_STRUCTURE_TYPE_DRHD:
      ASSERT (mVtdUnitNumber < MAX_VTD_UNITS);
      if (mVtdUnitNumber >= MAX_VTD_UNITS) {
        DEBUG ((EFI_D_INFO,"  ParseDmarAcpiTable: VtdUnitNumber exceed - %d \n", mVtdUnitNumber));
        return EFI_OUT_OF_RESOURCES;
      }
      Status = ProcessDhrd (mVtdUnitNumber, (EFI_ACPI_DMAR_HARDWARE_UNIT_DEFINITION_STRUCTURE *)DmarHeader);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      mVtdUnitNumber++;
      break;

    case EFI_ACPI_DMA_REMAPPING_STRUCTURE_TYPE_RMRR:
      Status = ProcessRmrr ((EFI_ACPI_DMAR_RESERVED_MEMORY_REGION_REPORTING_STRUCTURE *)DmarHeader);
      if (EFI_ERROR (Status)) {
        return Status;
      }
      break;
    default:
      break;
    }
    DmarHeader = (EFI_ACPI_DMAR_STRUCTURE_HEADER *)((UINTN)DmarHeader + DmarHeader->Length);
  }

  DEBUG ((EFI_D_INFO,"  VtdUnitNumber - %d\n", mVtdUnitNumber));
  if (mVtdUnitNumber == 0) {
    return EFI_DEVICE_ERROR;
  }
  for (VtdIndex = 0; VtdIndex < mVtdUnitNumber; VtdIndex++) {
    DumpPciDeviceInfo (VtdIndex);
  }
  return EFI_SUCCESS ;
}

/**

  This function scan table in RSDT.

  @param Rsdt       RSDT table
  @param Signature  ACPI table signature
  @param FoundTable ACPI table found

**/
VOID
ScanTableInRSDT (
  IN RSDT_TABLE                    *Rsdt,
  IN UINT32                        Signature,
  OUT EFI_ACPI_DESCRIPTION_HEADER  **FoundTable
  )
{
  UINTN                         Index;
  UINT32                        EntryCount;
  UINT32                        *EntryPtr;
  EFI_ACPI_DESCRIPTION_HEADER   *Table;

  *FoundTable = NULL;

  EntryCount = (Rsdt->Header.Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT32);

  EntryPtr = &Rsdt->Entry;
  for (Index = 0; Index < EntryCount; Index ++, EntryPtr ++) {
    Table = (EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)(*EntryPtr));
    if (Table->Signature == Signature) {
      *FoundTable = Table;
      break;
    }
  }

  return;
}

/**

  This function scan table in XSDT.

  @param Xsdt       RSDT table
  @param Signature  ACPI table signature
  @param FoundTable ACPI table found

**/
VOID
ScanTableInXSDT (
  IN XSDT_TABLE                    *Xsdt,
  IN UINT32                        Signature,
  OUT EFI_ACPI_DESCRIPTION_HEADER  **FoundTable
  )
{
  UINTN                        Index;
  UINT32                       EntryCount;
  UINT64                       EntryPtr;
  UINTN                        BasePtr;
  EFI_ACPI_DESCRIPTION_HEADER  *Table;

  *FoundTable = NULL;

  EntryCount = (Xsdt->Header.Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT64);

  BasePtr = (UINTN)(&(Xsdt->Entry));
  for (Index = 0; Index < EntryCount; Index ++) {
    CopyMem (&EntryPtr, (VOID *)(BasePtr + Index * sizeof(UINT64)), sizeof(UINT64));
    Table = (EFI_ACPI_DESCRIPTION_HEADER*)((UINTN)(EntryPtr));
    if (Table->Signature == Signature) {
      *FoundTable = Table;
      break;
    }
  }

  return;
}

/**

  This function find ACPI table.

  @param Rsdp       RSDP pointer
  @param Signature  ACPI table signature

  @return ACPI table found
**/
VOID *
FindAcpiPtr (
  IN EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER *Rsdp,
  IN UINT32                                       Signature
  )
{
  EFI_ACPI_DESCRIPTION_HEADER                    *AcpiTable;
  RSDT_TABLE                                     *Rsdt;
  XSDT_TABLE                                     *Xsdt;

  AcpiTable = NULL;

  //
  // Check ACPI2.0 table
  //
  Rsdt = (RSDT_TABLE *)(UINTN)Rsdp->RsdtAddress;
  Xsdt = NULL;
  if ((Rsdp->Revision >= 2) && (Rsdp->XsdtAddress < (UINT64)(UINTN)-1)) {
    Xsdt = (XSDT_TABLE *)(UINTN)Rsdp->XsdtAddress;
  }
  //
  // Check Xsdt
  //
  if (Xsdt != NULL) {
    ScanTableInXSDT (Xsdt, Signature, &AcpiTable);
  }
  //
  // Check Rsdt
  //
  if ((AcpiTable == NULL) && (Rsdt != NULL)) {
    ScanTableInRSDT (Rsdt, Signature, &AcpiTable);
  }

  return AcpiTable;
}

/**

  This function find DMAR ACPI table.

  @retval EFI_SUCCESS DMAR table is found.
**/
EFI_STATUS
GetDmarAcpiTable (
  VOID
  )
{
  VOID                              *AcpiTable;
  EFI_STATUS                        Status;

  AcpiTable = NULL;
  Status = EfiGetSystemConfigurationTable (
             &gEfiAcpi20TableGuid,
             &AcpiTable
             );
  if (EFI_ERROR (Status)) {
    Status = EfiGetSystemConfigurationTable (
               &gEfiAcpiTableGuid,
               &AcpiTable
               );
  }
  ASSERT (AcpiTable != NULL);

  mAcpiDmarTable = FindAcpiPtr (
                      (EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER *)AcpiTable,
                      EFI_ACPI_4_0_DMA_REMAPPING_TABLE_SIGNATURE
                      );
  DEBUG ((EFI_D_INFO,"DMAR Table - 0x%08x\n", mAcpiDmarTable));
  if (mAcpiDmarTable == NULL) {
    return EFI_UNSUPPORTED;
  }
  VtdDumpDmarTable();

  return EFI_SUCCESS;
}
