/** @file
  DlEntry dump information

  Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Dce.h"
#include "Frm.h"

#include "Dmar.h"
#include "DrtmTpm.h"
#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/MemoryMappedConfigurationSpaceAccessTable.h>

/**

  This function dump raw data.

  @param  Data  raw data
  @param  Size  raw data size

**/
VOID
DumpData (
  IN UINT8  *Data,
  IN UINT32 Size
  )
{
  UINT32 Index;
  for (Index = 0; Index < Size; Index++) {
    DEBUG ((EFI_D_INFO, "%02x", (UINTN)Data[Index]));
  }
}

/**

  This function dump raw data with colume format.

  @param  Data  raw data
  @param  Size  raw data size

**/
VOID
DumpHex (
  IN UINT8  *Data,
  IN UINT32 Size
  )
{
  UINT32  Index;
  UINT32  Count;
  UINT32  Left;

#define COLUME_SIZE  (16 * 2)

  Count = Size / COLUME_SIZE;
  Left  = Size % COLUME_SIZE;
  for (Index = 0; Index < Count; Index++) {
    DEBUG ((EFI_D_INFO, "(TXT) %04x: ", Index * COLUME_SIZE));
    DumpData (Data + Index * COLUME_SIZE, COLUME_SIZE);
    DEBUG ((EFI_D_INFO, "\n"));
  }

  if (Left != 0) {
    DEBUG ((EFI_D_INFO, "(TXT) %04x: ", Index * COLUME_SIZE));
    DumpData (Data + Index * COLUME_SIZE, Left);
    DEBUG ((EFI_D_INFO, "\n"));
  }
}

/**

  This function dump ACPI table header.

  @param  Header  ACPI table header

**/
VOID
DumpAcpiTableHeader (
  IN EFI_ACPI_DESCRIPTION_HEADER *Header
  )
{
  UINT8               *SignatureByte;
  UINT8               OemTableId[8];
  UINT8               *CreatorId;

  SignatureByte = (UINT8*)&Header->Signature;
  
  DEBUG ((EFI_D_INFO, "(TXT)   Table Header:\n"));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Signature ............................................ '%c%c%c%c'\n",
    SignatureByte[0],
    SignatureByte[1],
    SignatureByte[2],
    SignatureByte[3]
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Length ............................................... 0x%08x\n",
    (UINTN)Header->Length
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Revision ............................................. 0x%02x\n",
    (UINTN)Header->Revision
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Checksum ............................................. 0x%02x\n",
    (UINTN)Header->Checksum
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     OEMID ................................................ '%c%c%c%c%c%c'\n",
    Header->OemId[0],
    Header->OemId[1],
    Header->OemId[2],
    Header->OemId[3],
    Header->OemId[4],
    Header->OemId[5]
    ));
  CopyMem (&OemTableId, &Header->OemTableId, sizeof(OemTableId));
  DEBUG ((EFI_D_INFO,
    "(TXT)     OEM Table ID ......................................... '%c%c%c%c%c%c%c%c'\n",
    OemTableId[0],
    OemTableId[1],
    OemTableId[2],
    OemTableId[3],
    OemTableId[4],
    OemTableId[5],
    OemTableId[6],
    OemTableId[7]
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     OEM Revision ......................................... 0x%08x\n",
    (UINTN)Header->OemRevision
    ));
  CreatorId = (UINT8 *)&Header->CreatorId;
  DEBUG ((EFI_D_INFO,
    "(TXT)     Creator ID ........................................... '%c%c%c%c'\n",
    CreatorId[0],
    CreatorId[1],
    CreatorId[2],
    CreatorId[3]
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Creator Revision ..................................... 0x%08x\n",
    (UINTN)Header->CreatorRevision
    ));
  return;
}

/**

  This function dump ACPI RSDP.

  @param  Rsdp  ACPI RSDP

**/
VOID
DumpAcpiRSDP (
  IN EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER *Rsdp
  )
{
  UINT8  *Signature;

  if (Rsdp == NULL) {
    return;
  }
  
  DEBUG ((EFI_D_INFO,
    "(TXT) *****************************************************************************\n"
    "(TXT) *         Root System Description Pointer                                   *\n"
    "(TXT) *****************************************************************************\n"
    ));

  DEBUG ((EFI_D_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "(TXT) RSDP address ............................................. 0x%016lx\n" :
    "(TXT) RSDP address ............................................. 0x%08x\n",
    Rsdp
    ));
  Signature = (UINT8 *)&Rsdp->Signature;
  DEBUG ((EFI_D_INFO,
    "(TXT)   Signature .............................................. '%c%c%c%c%c%c%c%c'\n",
    Signature[0],
    Signature[1],
    Signature[2],
    Signature[3],
    Signature[4],
    Signature[5],
    Signature[6],
    Signature[7]
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)   Checksum ............................................... 0x%02x\n",
    Rsdp->Checksum
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)   OEMID .................................................. '%c%c%c%c%c%c'\n",
    Rsdp->OemId[0],
    Rsdp->OemId[1],
    Rsdp->OemId[2],
    Rsdp->OemId[3],
    Rsdp->OemId[4],
    Rsdp->OemId[5]
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)   Revision ............................................... 0x%02x\n",
    Rsdp->Revision
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)   RsdtAddress ............................................ 0x%08x\n",
    Rsdp->RsdtAddress
    ));
  if (Rsdp->Revision >= 2) {
    DEBUG ((EFI_D_INFO,
      "(TXT)   Length ................................................. 0x%08x\n",
      Rsdp->Length
      ));
    DEBUG ((EFI_D_INFO,
      "(TXT)   XsdtAddress ............................................ 0x%016lx\n",
      Rsdp->XsdtAddress
      ));
    DEBUG ((EFI_D_INFO,
      "(TXT)   Extended Checksum ...................................... 0x%02x\n",
      Rsdp->ExtendedChecksum
      ));
  }         
  
  DEBUG ((EFI_D_INFO,
    "(TXT) *****************************************************************************\n\n"
    ));

  return;
}

/**

  This function dump ACPI RSDT.

  @param  Rsdt  ACPI RSDT

**/
VOID
DumpAcpiRSDT (
  IN EFI_ACPI_DESCRIPTION_HEADER *Rsdt
  )
{
  UINTN                     EntryCount;
  UINTN                     Index;
  UINT32                    *EntryPtr;

  if (Rsdt == NULL) {
    return;
  }

  EntryCount = (Rsdt->Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / 4;
  
  DEBUG ((EFI_D_INFO,
    "(TXT) *****************************************************************************\n"
    "(TXT) *         Root System Description Table                                     *\n"
    "(TXT) *****************************************************************************\n"
    ));

  DEBUG ((EFI_D_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "(TXT) RSDT address ............................................. 0x%016lx\n" :
    "(TXT) RSDT address ............................................. 0x%08x\n",
    Rsdt
    ));
  
  DumpAcpiTableHeader (Rsdt);
  
  DEBUG ((EFI_D_INFO,
    "(TXT)   Table Contents:\n"
    ));
  
  EntryPtr = (UINT32 *)(Rsdt + 1);
  for (Index = 0; Index < EntryCount; Index ++, EntryPtr ++) {
    DEBUG ((EFI_D_INFO,
      "(TXT)     Entry %2d: ............................................ 0x%08x\n",
      Index + 1,
      *EntryPtr
      ));
  }

  DEBUG ((EFI_D_INFO,
    "(TXT) *****************************************************************************\n\n"
    ));

  return;
}

/**

  This function dump ACPI XSDT.

  @param  Rsdt  ACPI XSDT

**/
VOID
DumpAcpiXSDT (
  IN EFI_ACPI_DESCRIPTION_HEADER *Xsdt
  )
{
  UINTN                     EntryCount;
  UINTN                     Index;
  UINT64                    *EntryPtr;
  UINT64                    TempEntry;
    
  if (Xsdt == NULL) {
    return;
  }

  EntryCount = (Xsdt->Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / 8;
  
  DEBUG ((EFI_D_INFO,
    "(TXT) *****************************************************************************\n"
    "(TXT) *         Extended System Description Table                                 *\n"
    "(TXT) *****************************************************************************\n"
    ));

  DEBUG ((EFI_D_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "(TXT) XSDT address ............................................. 0x%016lx\n" :
    "(TXT) XSDT address ............................................. 0x%08x\n",
    Xsdt
    ));
  
  DumpAcpiTableHeader (Xsdt);
  
  DEBUG ((EFI_D_INFO,
    "(TXT)   Table Contents:\n"
    ));
  
  EntryPtr = (UINT64 *)(Xsdt + 1);
  for (Index = 0; Index < EntryCount; Index ++, EntryPtr ++) {
    CopyMem (&TempEntry, EntryPtr, sizeof(UINT64));
    DEBUG ((EFI_D_INFO,
      "(TXT)     Entry %2d: ............................................ 0x%016lx\n",
      Index + 1,
      TempEntry
      ));
  }

  DEBUG ((EFI_D_INFO,
    "(TXT) *****************************************************************************\n\n"
    ));

  return;
}

//
// This table defines the DMAR DeviceScopt entry type string
//
GLOBAL_REMOVE_IF_UNREFERENCED CHAR16 *mDeviceScopeEntryTypeStr[] = {
  L"PCI Endpoint Device",
  L"PCI Sub-hierachy",
  L"IOAPIC",
  L"MSI Capable HPET",
  L"ACPI Namespace Device",
};

/**

  This function dump DMAR Device Scopt Entry.

  @param  DmarDeviceScopeEntry   DMAR Device Scopt Entry

**/
VOID
DumpDmarDeviceScopeEntry (
  IN EFI_ACPI_DMAR_DEVICE_SCOPE_ENTRY_STRUCTURE     *DmarDeviceScopeEntry
  )
{
  UINTN                   PciPathNumber;
  UINTN                   Index;
  EFI_ACPI_DMAR_PCI_PATH  *PciPath;

  DEBUG ((EFI_D_INFO,
    "(TXT)     *************************************************************************\n"
    "(TXT)     *       DMA-Remapping Device Scope Entry Structure                      *\n"
    "(TXT)     *************************************************************************\n"
    ));
  DEBUG ((EFI_D_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "(TXT)     DMAR Device Scope Entry address ...................... 0x%016lx\n" :
    "(TXT)     DMAR Device Scope Entry address ...................... 0x%08x\n",
    DmarDeviceScopeEntry
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)       Device Scope Entry Type ............................ 0x%02x\n",
    (UINTN)DmarDeviceScopeEntry->DeviceScopeEntryType
    ));
  if (DmarDeviceScopeEntry->DeviceScopeEntryType < sizeof(mDeviceScopeEntryTypeStr)/sizeof(mDeviceScopeEntryTypeStr[0])) {
    DEBUG ((EFI_D_INFO,
      "(TXT)         %s\n",
      mDeviceScopeEntryTypeStr[DmarDeviceScopeEntry->DeviceScopeEntryType]
      ));
  }
  DEBUG ((EFI_D_INFO,
    "(TXT)       Length ............................................. 0x%02x\n",
    (UINTN)DmarDeviceScopeEntry->Length
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)       Enumeration ID ..................................... 0x%02x\n",
    (UINTN)DmarDeviceScopeEntry->EnumerationID
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)       Starting Bus Number ................................ 0x%02x\n",
    (UINTN)DmarDeviceScopeEntry->StartingBusNumber
    ));

  PciPathNumber = (DmarDeviceScopeEntry->Length - sizeof(EFI_ACPI_DMAR_DEVICE_SCOPE_ENTRY_STRUCTURE)) / sizeof(EFI_ACPI_DMAR_PCI_PATH);
  PciPath = (EFI_ACPI_DMAR_PCI_PATH *)(DmarDeviceScopeEntry + 1);
  for (Index = 0; Index < PciPathNumber; Index++) {
    DEBUG ((EFI_D_INFO,
      "(TXT)       Device ............................................. 0x%02x\n",
      (UINTN)PciPath[Index].Device
      ));
    DEBUG ((EFI_D_INFO,
      "(TXT)       Function ........................................... 0x%02x\n",
      (UINTN)PciPath[Index].Function
      ));
  }
 
  DEBUG ((EFI_D_INFO,
    "(TXT)     *************************************************************************\n\n"
    ));

  return;
}

/**

  This function dump DMAR DRHD structure.

  @param  Drhd   DMAR DRHD structure

**/
VOID
DumpDmarDrhd (
  IN EFI_ACPI_DMAR_HARDWARE_UNIT_DEFINITION_STRUCTURE *Drhd
  )
{
  EFI_ACPI_DMAR_DEVICE_SCOPE_ENTRY_STRUCTURE       *DmarDeviceScopeEntry;
  INTN                                             DrhdLen;

  DEBUG ((EFI_D_INFO,
    "(TXT)   ***************************************************************************\n"
    "(TXT)   *       DMA-Remapping Hardware Definition Structure                       *\n"
    "(TXT)   ***************************************************************************\n"
    ));
  DEBUG ((EFI_D_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "(TXT)   DRHD address ........................................... 0x%016lx\n" :
    "(TXT)   DRHD address ........................................... 0x%08x\n",
    Drhd
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Type ................................................. 0x%04x\n",
    (UINTN)Drhd->Type
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Length ............................................... 0x%04x\n",
    (UINTN)Drhd->Length
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Flags ................................................ 0x%02x\n",
    (UINTN)Drhd->Flags
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)       INCLUDE_PCI_ALL .................................... 0x%02x\n",
    (UINTN)(Drhd->Flags & 0x1)
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Segment Number ....................................... 0x%04x\n",
    (UINTN)Drhd->SegmentNumber
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Register Base Address ................................ 0x%016lx\n",
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
    "(TXT)   ***************************************************************************\n\n"
    ));

  return;
}

/**

  This function dump DMAR RMRR structure.

  @param  Rmrr   DMAR RMRR structure

**/
VOID
DumpDmarRmrr (
  EFI_ACPI_DMAR_RESERVED_MEMORY_REGION_REPORTING_STRUCTURE *Rmrr
  )
{
  EFI_ACPI_DMAR_DEVICE_SCOPE_ENTRY_STRUCTURE       *DmarDeviceScopeEntry;
  INTN                                             RmrrLen;

  DEBUG ((EFI_D_INFO,
    "(TXT)   ***************************************************************************\n"
    "(TXT)   *       Reserved Memory Region Reporting Structure                        *\n"
    "(TXT)   ***************************************************************************\n"
    ));
  DEBUG ((EFI_D_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "(TXT)   RMRR address ........................................... 0x%016lx\n" :
    "(TXT)   RMRR address ........................................... 0x%08x\n",
    Rmrr
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Type ................................................. 0x%04x\n",
    (UINTN)Rmrr->Type
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Length ............................................... 0x%04x\n",
    (UINTN)Rmrr->Length
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Segment Number ....................................... 0x%04x\n",
    (UINTN)Rmrr->SegmentNumber
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Reserved Memory Region Base Address .................. 0x%016lx\n",
    Rmrr->ReservedMemoryRegionBaseAddress
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Reserved Memory Region Limit Address ................. 0x%016lx\n",
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
    "(TXT)   ***************************************************************************\n\n"
    ));

  return;
}

/**

  This function dump DMAR ATSR structure.

  @param  Atsr   DMAR ATSR structure

**/
VOID
DumpDmarAtsr (
  EFI_ACPI_DMAR_ROOT_PORT_ATS_CAPABILITY_REPORTING_STRUCTURE *Atsr
  )
{
  EFI_ACPI_DMAR_DEVICE_SCOPE_ENTRY_STRUCTURE       *DmarDeviceScopeEntry;
  INTN                                             AtsrLen;

  DEBUG ((EFI_D_INFO,
    "(TXT)   ***************************************************************************\n"
    "(TXT)   *       Root Port ATS Capability Reporting Structure                      *\n"
    "(TXT)   ***************************************************************************\n"
    ));
  DEBUG ((EFI_D_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "(TXT)   ATSR address ........................................... 0x%016lx\n" :
    "(TXT)   ATSR address ........................................... 0x%08x\n",
    Atsr
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Type ................................................. 0x%04x\n",
    (UINTN)Atsr->Type
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Length ............................................... 0x%04x\n",
    (UINTN)Atsr->Length
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Flags ................................................ 0x%02x\n",
    (UINTN)Atsr->Flags
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)       ALL_PORTS .......................................... 0x%02x\n",
    (UINTN)(Atsr->Flags & 0x1)
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Segment Number ....................................... 0x%04x\n",
    (UINTN)Atsr->SegmentNumber
    ));

  AtsrLen  = Atsr->Length - sizeof(EFI_ACPI_DMAR_ROOT_PORT_ATS_CAPABILITY_REPORTING_STRUCTURE);
  DmarDeviceScopeEntry = (EFI_ACPI_DMAR_DEVICE_SCOPE_ENTRY_STRUCTURE *)(Atsr + 1);
  while (AtsrLen > 0) {
    DumpDmarDeviceScopeEntry (DmarDeviceScopeEntry);
    AtsrLen -= DmarDeviceScopeEntry->Length;
    DmarDeviceScopeEntry = (EFI_ACPI_DMAR_DEVICE_SCOPE_ENTRY_STRUCTURE *)((UINTN)DmarDeviceScopeEntry + DmarDeviceScopeEntry->Length);
  }

  DEBUG ((EFI_D_INFO,
    "(TXT)   ***************************************************************************\n\n"
    ));

  return;
}

/**

  This function dump DMAR RHSA structure.

  @param  Rhsa   DMAR RHSA structure

**/
VOID
DumpDmarRhsa (
  EFI_ACPI_DMAR_REMAPPING_HARDWARE_STATIC_AFFINITY_STRUCTURE *Rhsa
  )
{
  DEBUG ((EFI_D_INFO,
    "(TXT)   ***************************************************************************\n"
    "(TXT)   *       Remapping Hardware Static Affinity Structure                      *\n"
    "(TXT)   ***************************************************************************\n"
    ));
  DEBUG ((EFI_D_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "(TXT)   RHSA address ........................................... 0x%016lx\n" :
    "(TXT)   RHSA address ........................................... 0x%08x\n",
    Rhsa
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Type ................................................. 0x%04x\n",
    (UINTN)Rhsa->Type
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Length ............................................... 0x%04x\n",
    (UINTN)Rhsa->Length
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Register Base Address ................................ 0x%016lx\n",
    Rhsa->RegisterBaseAddress
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Proximity Domain ..................................... 0x%08x\n",
    (UINTN)Rhsa->ProximityDomain
    ));

  DEBUG ((EFI_D_INFO,
    "(TXT)   ***************************************************************************\n\n"
    ));

  return;
}

/**

  This function dump DMAR ANDD structure.

  @param  Andd   DMAR ANDD structure

**/
VOID
DumpDmarAndd (
  EFI_ACPI_DMAR_ACPI_NAME_SPACE_DEVICE_DECLARATION_STRUCTURE *Andd
  )
{
  DEBUG ((EFI_D_INFO,
    "(TXT)   ***************************************************************************\n"
    "(TXT)   *       ACPI Name-space Device Declaration Structure                      *\n"
    "(TXT)   ***************************************************************************\n"
    ));
  DEBUG ((EFI_D_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "(TXT)   ANDD address ........................................... 0x%016lx\n" :
    "(TXT)   ANDD address ........................................... 0x%08x\n",
    Andd
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Type ................................................. 0x%04x\n",
    Andd->Type
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Length ............................................... 0x%04x\n",
    Andd->Length
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     ACPI Device Number ................................... 0x%02x\n",
    Andd->ACPIDeviceNumber
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     ACPI Object Name ..................................... '%a'\n",
    (Andd + 1)
    ));

  DEBUG ((EFI_D_INFO,
    "(TXT)   ***************************************************************************\n\n"
    ));

  return;
}

/**

  This function dump DMAR ACPI table.

  @param  Dmar   DMAR ACPI table

**/
VOID
DumpDmar (
  IN EFI_ACPI_DMAR_DESCRIPTION_TABLE  *Dmar
  )
{
  EFI_ACPI_DMAR_STRUCTURE_HEADER *DmarHeader;
  INTN                           DmarLen;

  DEBUG ((EFI_D_INFO,
    "(TXT) *****************************************************************************\n"
    "(TXT) *         DMAR Table                                                        *\n"
    "(TXT) *****************************************************************************\n"
    ));

  DEBUG ((EFI_D_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "(TXT) DMAR address ............................................. 0x%016lx\n" :
    "(TXT) DMAR address ............................................. 0x%08x\n",
    Dmar
    ));

  DumpAcpiTableHeader ((EFI_ACPI_DESCRIPTION_HEADER *)Dmar);
   
  DEBUG ((EFI_D_INFO, "(TXT)   Table Contents:\n"));
  DEBUG ((EFI_D_INFO,
    "(TXT)     HostAddressWidth ..................................... 0x%02x\n",
    (UINTN)Dmar->HostAddressWidth
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Flags ................................................ 0x%02x\n",
    (UINTN)Dmar->Flags
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)       INTR_REMAP ......................................... 0x%02x\n",
    (UINTN)(Dmar->Flags & 0x1)
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)       X2APIC_OPT_OUT_SET ................................. 0x%02x\n",
    (UINTN)(Dmar->Flags & 0x2)
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
      DumpDmarRhsa ((EFI_ACPI_DMAR_REMAPPING_HARDWARE_STATIC_AFFINITY_STRUCTURE *)DmarHeader);
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
    "(TXT) *****************************************************************************\n\n"
    ));

  ASSERT (DmarLen == 0);

  return ;
}

#pragma pack(1)

typedef struct {
  UINT8                 Type;
  UINT8                 Length;
} APIC_STRUCT_HEADER;

#pragma pack()

/**

  This function dump MADT ProcessorLocalApic structure.

  @param  ProcessorLocalApic   MADT ProcessorLocalApic structure

**/
VOID
DumpApicProcessorLocalApic (
  EFI_ACPI_1_0_PROCESSOR_LOCAL_APIC_STRUCTURE           *ProcessorLocalApic
  )
{
  DEBUG ((EFI_D_INFO,
    "(TXT)   ***************************************************************************\n"
    "(TXT)   *       Processor Local APIC Structure                                    *\n"
    "(TXT)   ***************************************************************************\n"
    ));
  DEBUG ((EFI_D_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "(TXT)   Processor Local APIC address ........................... 0x%016lx\n" :
    "(TXT)   Processor Local APIC address ........................... 0x%08x\n",
    ProcessorLocalApic
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Type ................................................. 0x%02x\n",
    ProcessorLocalApic->Type
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Length ............................................... 0x%02x\n",
    ProcessorLocalApic->Length
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     ACPI Processor ID .................................... 0x%02x\n",
    ProcessorLocalApic->AcpiProcessorId
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     APIC ID .............................................. 0x%02x\n",
    ProcessorLocalApic->ApicId
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Flags ................................................ 0x%08x\n",
    ProcessorLocalApic->Flags
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)       Enabled ............................................ 0x%08x\n",
    ProcessorLocalApic->Flags & EFI_ACPI_1_0_LOCAL_APIC_ENABLED
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)   ***************************************************************************\n\n"
    ));

  return;
}

/**

  This function dump MADT IOApic structure.

  @param  IOApic   MADT IOApic structure

**/
VOID
DumpApicIOApic (
  EFI_ACPI_1_0_IO_APIC_STRUCTURE                        *IOApic
  )
{
  DEBUG ((EFI_D_INFO,
    "(TXT)   ***************************************************************************\n"
    "(TXT)   *       IO APIC Structure                                                 *\n"
    "(TXT)   ***************************************************************************\n"
    ));
  DEBUG ((EFI_D_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "(TXT)   IO APIC address ........................................ 0x%016lx\n" :
    "(TXT)   IO APIC address ........................................ 0x%08x\n",
    IOApic
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Type ................................................. 0x%02x\n",
    IOApic->Type
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Length ............................................... 0x%02x\n",
    IOApic->Length
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     IO APIC ID ........................................... 0x%02x\n",
    IOApic->IoApicId
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     IO APIC Address ...................................... 0x%08x\n",
    IOApic->IoApicAddress
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Global System Interrupt Base ......................... 0x%08x\n",
    IOApic->SystemVectorBase
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)   ***************************************************************************\n\n"
    ));

  return;
}

/**

  This function dump MADT InterruptSourceOverride structure.

  @param  InterruptSourceOverride   MADT InterruptSourceOverride structure

**/
VOID
DumpApicInterruptSourceOverride (
  EFI_ACPI_1_0_INTERRUPT_SOURCE_OVERRIDE_STRUCTURE      *InterruptSourceOverride
  )
{
  DEBUG ((EFI_D_INFO,
    "(TXT)   ***************************************************************************\n"
    "(TXT)   *       Interrupt Source Override Structure                               *\n"
    "(TXT)   ***************************************************************************\n"
    ));
  DEBUG ((EFI_D_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "(TXT)   Interrupt Source Override address ...................... 0x%016lx\n" :
    "(TXT)   Interrupt Source Override address ...................... 0x%08x\n",
    InterruptSourceOverride
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Type ................................................. 0x%02x\n",
    InterruptSourceOverride->Type
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Length ............................................... 0x%02x\n",
    InterruptSourceOverride->Length
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Bus .................................................. 0x%02x\n",
    InterruptSourceOverride->Bus
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Source ............................................... 0x%02x\n",
    InterruptSourceOverride->Source
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Global System Interrupt .............................. 0x%08x\n",
    InterruptSourceOverride->GlobalSystemInterruptVector
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Flags ................................................ 0x%04x\n",
    InterruptSourceOverride->Flags
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)       Polarity ........................................... 0x%04x\n",
    InterruptSourceOverride->Flags & 0x3
    ));
  switch (InterruptSourceOverride->Flags & 0x3) {
  case 0x0:
    DEBUG ((EFI_D_INFO,
     "(TXT)         Conforms to the specifications of the bus\n"
     ));
    break;
  case 0x1:
    DEBUG ((EFI_D_INFO,
     "(TXT)         Active high\n"
     ));
    break;
  case 0x3:
    DEBUG ((EFI_D_INFO,
     "(TXT)         Active low\n"
     ));
    break;
  default:
    break;
  }
  DEBUG ((EFI_D_INFO,
    "(TXT)       Trigger Mode ....................................... 0x%04x\n",
    InterruptSourceOverride->Flags & 0xc
    ));
  switch (InterruptSourceOverride->Flags & 0xc) {
  case 0x0:
    DEBUG ((EFI_D_INFO,
     "(TXT)         Conforms to the specifications of the bus\n"
     ));
    break;
  case 0x4:
    DEBUG ((EFI_D_INFO,
     "(TXT)         Edge-triggered\n"
     ));
    break;
  case 0xc:
    DEBUG ((EFI_D_INFO,
     "(TXT)         Level-triggered\n"
     ));
    break;
  default:
    break;
  }
  DEBUG ((EFI_D_INFO,
    "(TXT)   ***************************************************************************\n\n"
    ));

  return;
}

/**

  This function dump MADT NonMaskableInterruptSource structure.

  @param  NonMaskableInterruptSource   MADT NonMaskableInterruptSource structure

**/
VOID
DumpApicNonMaskableInterruptSource (
  EFI_ACPI_1_0_NON_MASKABLE_INTERRUPT_SOURCE_STRUCTURE  *NonMaskableInterruptSource
  )
{
  DEBUG ((EFI_D_INFO,
    "(TXT)   ***************************************************************************\n"
    "(TXT)   *       Non-Maskable Interrupt Source Structure                           *\n"
    "(TXT)   ***************************************************************************\n"
    ));
  DEBUG ((EFI_D_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "(TXT)   NMI Source address ..................................... 0x%016lx\n" :
    "(TXT)   NMI Source address ..................................... 0x%08x\n",
    NonMaskableInterruptSource
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Type ................................................. 0x%02x\n",
    NonMaskableInterruptSource->Type
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Length ............................................... 0x%02x\n",
    NonMaskableInterruptSource->Length
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Flags ................................................ 0x%04x\n",
    NonMaskableInterruptSource->Flags
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)       Polarity ........................................... 0x%04x\n",
    NonMaskableInterruptSource->Flags & 0x3
    ));
  switch (NonMaskableInterruptSource->Flags & 0x3) {
  case 0x0:
    DEBUG ((EFI_D_INFO,
     "(TXT)         Conforms to the specifications of the bus\n"
     ));
    break;
  case 0x1:
    DEBUG ((EFI_D_INFO,
     "(TXT)         Active high\n"
     ));
    break;
  case 0x3:
    DEBUG ((EFI_D_INFO,
     "(TXT)         Active low\n"
     ));
    break;
  default:
    break;
  }
  DEBUG ((EFI_D_INFO,
    "(TXT)       Trigger Mode ....................................... 0x%04x\n",
    NonMaskableInterruptSource->Flags & 0xc
    ));
  switch (NonMaskableInterruptSource->Flags & 0xc) {
  case 0x0:
    DEBUG ((EFI_D_INFO,
     "(TXT)         Conforms to the specifications of the bus\n"
     ));
    break;
  case 0x4:
    DEBUG ((EFI_D_INFO,
     "(TXT)         Edge-triggered\n"
     ));
    break;
  case 0xc:
    DEBUG ((EFI_D_INFO,
     "(TXT)         Level-triggered\n"
     ));
    break;
  default:
    break;
  }
  DEBUG ((EFI_D_INFO,
    "(TXT)     Global System Interrupt .............................. 0x%08x\n",
    NonMaskableInterruptSource->GlobalSystemInterruptVector
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)   ***************************************************************************\n\n"
    ));

  return;
}

/**

  This function dump MADT LocalApicNMI structure.

  @param  LocalApicNMI   MADT LocalApicNMI structure

**/
VOID
DumpApicLocalApicNMI (
  EFI_ACPI_1_0_LOCAL_APIC_NMI_STRUCTURE                 *LocalApicNMI
  )
{
  UINT16               Flags;

  DEBUG ((EFI_D_INFO,
    "(TXT)   ***************************************************************************\n"
    "(TXT)   *       Local APIC NMI Structure                                          *\n"
    "(TXT)   ***************************************************************************\n"
    ));
  DEBUG ((EFI_D_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "(TXT)   Local APIC NMI address ................................. 0x%016lx\n" :
    "(TXT)   Local APIC NMI address ................................. 0x%08x\n",
    LocalApicNMI
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Type ................................................. 0x%02x\n",
    LocalApicNMI->Type
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Length ............................................... 0x%02x\n",
    LocalApicNMI->Length
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     ACPI Processor ID .................................... 0x%02x\n",
    LocalApicNMI->AcpiProcessorId
    ));
  CopyMem (&Flags, &LocalApicNMI->Flags, sizeof(UINT16));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Flags ................................................ 0x%04x\n",
    Flags
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)       Polarity ........................................... 0x%04x\n",
    Flags & 0x3
    ));
  switch (Flags & 0x3) {
  case 0x0:
    DEBUG ((EFI_D_INFO,
     "(TXT)         Conforms to the specifications of the bus\n"
     ));
    break;
  case 0x1:
    DEBUG ((EFI_D_INFO,
     "(TXT)         Active high\n"
     ));
    break;
  case 0x3:
    DEBUG ((EFI_D_INFO,
     "(TXT)         Active low\n"
     ));
    break;
  default:
    break;
  }
  DEBUG ((EFI_D_INFO,
    "(TXT)       Trigger Mode ....................................... 0x%04x\n",
    Flags & 0xc
    ));
  switch (Flags & 0xc) {
  case 0x0:
    DEBUG ((EFI_D_INFO,
     "(TXT)         Conforms to the specifications of the bus\n"
     ));
    break;
  case 0x4:
    DEBUG ((EFI_D_INFO,
     "(TXT)         Edge-triggered\n"
     ));
    break;
  case 0xc:
    DEBUG ((EFI_D_INFO,
     "(TXT)         Level-triggered\n"
     ));
    break;
  default:
    break;
  }
  DEBUG ((EFI_D_INFO,
    "(TXT)     Local APIC INTI ...................................... 0x%02x\n",
    LocalApicNMI->LocalApicInti
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)   ***************************************************************************\n\n"
    ));

  return;
}

/**

  This function dump MADT LocalApicAddressOverride structure.

  @param  LocalApicAddressOverride   MADT LocalApicAddressOverride structure

**/
VOID
DumpApicLocalApicAddressOverride (
  EFI_ACPI_2_0_LOCAL_APIC_ADDRESS_OVERRIDE_STRUCTURE    *LocalApicAddressOverride
  )
{
  UINT64               LocalApicAddress;
  
  DEBUG ((EFI_D_INFO,
    "(TXT)   ***************************************************************************\n"
    "(TXT)   *       Local APIC Address Override Structure                             *\n"
    "(TXT)   ***************************************************************************\n"
    ));
  DEBUG ((EFI_D_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "(TXT)   Local APIC Address Override address .................... 0x%016lx\n" :
    "(TXT)   Local APIC Address Override address .................... 0x%08x\n",
    LocalApicAddressOverride
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Type ................................................. 0x%02x\n",
    LocalApicAddressOverride->Type
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Length ............................................... 0x%02x\n",
    LocalApicAddressOverride->Length
    ));
  CopyMem (&LocalApicAddress, &LocalApicAddressOverride->LocalApicAddress, sizeof(UINT64));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Local APIC Address ................................... 0x%016lx\n",
    LocalApicAddress
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)   ***************************************************************************\n\n"
    ));

  return;
}

/**

  This function dump MADT IOSapic structure.

  @param  IOSapic   MADT IOSapic structure

**/
VOID
DumpApicIOSapic (
  EFI_ACPI_2_0_IO_SAPIC_STRUCTURE                       *IOSapic
  )
{
  DEBUG ((EFI_D_INFO,
    "(TXT)   ***************************************************************************\n"
    "(TXT)   *       IO SAPIC Structure                                                *\n"
    "(TXT)   ***************************************************************************\n"
    ));
  DEBUG ((EFI_D_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "(TXT)   IO SAPIC address ....................................... 0x%016lx\n" :
    "(TXT)   IO SAPIC address ....................................... 0x%08x\n",
    IOSapic
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Type ................................................. 0x%02x\n",
    IOSapic->Type
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Length ............................................... 0x%02x\n",
    IOSapic->Length
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     IO SAPIC ID .......................................... 0x%02x\n",
    IOSapic->IoApicId
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Global System Interrupt Base ......................... 0x%08x\n",
    IOSapic->GlobalSystemInterruptBase
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     IO SAPIC Address ..................................... 0x%016lx\n",
    IOSapic->IoSapicAddress
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)   ***************************************************************************\n\n"
    ));

  return;
}

/**

  This function dump MADT ProcessorLocalSapic structure.

  @param  ProcessorLocalSapic   MADT ProcessorLocalSapic structure

**/
VOID
DumpApicProcessorLocalSapic (
  EFI_ACPI_2_0_PROCESSOR_LOCAL_SAPIC_STRUCTURE          *ProcessorLocalSapic
  )
{
  UINT32               Flags;

  DEBUG ((EFI_D_INFO,
    "(TXT)   ***************************************************************************\n"
    "(TXT)   *       Processor Local SAPIC Structure                                   *\n"
    "(TXT)   ***************************************************************************\n"
    ));
  DEBUG ((EFI_D_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "(TXT)   Processor Local SAPIC address .......................... 0x%016lx\n" :
    "(TXT)   Processor Local SAPIC address .......................... 0x%08x\n",
    ProcessorLocalSapic
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Type ................................................. 0x%02x\n",
    ProcessorLocalSapic->Type
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Length ............................................... 0x%02x\n",
    ProcessorLocalSapic->Length
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     ACPI Processor ID .................................... 0x%02x\n",
    ProcessorLocalSapic->AcpiProcessorId
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Local SAPIC ID ....................................... 0x%02x\n",
    ProcessorLocalSapic->LocalSapicId
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Local SAPIC EID ...................................... 0x%02x\n",
    ProcessorLocalSapic->LocalSapicEid
    ));
  CopyMem (&Flags, &ProcessorLocalSapic->Flags, sizeof(UINT32));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Flags ................................................ 0x%08x\n",
    Flags
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)       Enabled ............................................ 0x%08x\n",
    Flags & EFI_ACPI_2_0_LOCAL_APIC_ENABLED
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)   ***************************************************************************\n\n"
    ));

  return;
}

/**

  This function dump MADT PlatformInterruptSource structure.

  @param  PlatformInterruptSource   MADT PlatformInterruptSource structure

**/
VOID
DumpApicPlatformInterruptSource (
  EFI_ACPI_2_0_PLATFORM_INTERRUPT_SOURCES_STRUCTURE      *PlatformInterruptSource
  )
{
  DEBUG ((EFI_D_INFO,
    "(TXT)   ***************************************************************************\n"
    "(TXT)   *       Platform Interrupt Source Structure                               *\n"
    "(TXT)   ***************************************************************************\n"
    ));
  DEBUG ((EFI_D_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "(TXT)   Platform Interrupt Source address ...................... 0x%016lx\n" :
    "(TXT)   Platform Interrupt Source address ...................... 0x%08x\n",
    PlatformInterruptSource
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Type ................................................. 0x%02x\n",
    PlatformInterruptSource->Type
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Length ............................................... 0x%02x\n",
    PlatformInterruptSource->Length
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Flags ................................................ 0x%04x\n",
    PlatformInterruptSource->Flags
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)       Polarity ........................................... 0x%04x\n",
    PlatformInterruptSource->Flags & 0x3
    ));
  switch (PlatformInterruptSource->Flags & 0x3) {
  case 0x0:
    DEBUG ((EFI_D_INFO,
     "(TXT)         Conforms to the specifications of the bus\n"
     ));
    break;
  case 0x1:
    DEBUG ((EFI_D_INFO,
     "(TXT)         Active high\n"
     ));
    break;
  case 0x3:
    DEBUG ((EFI_D_INFO,
     "(TXT)         Active low\n"
     ));
    break;
  default:
    break;
  }
  DEBUG ((EFI_D_INFO,
    "(TXT)       Trigger Mode ....................................... 0x%04x\n",
    PlatformInterruptSource->Flags & 0xc
    ));
  switch (PlatformInterruptSource->Flags & 0xc) {
  case 0x0:
    DEBUG ((EFI_D_INFO,
     "(TXT)         Conforms to the specifications of the bus\n"
     ));
    break;
  case 0x4:
    DEBUG ((EFI_D_INFO,
     "(TXT)         Edge-triggered\n"
     ));
    break;
  case 0xc:
    DEBUG ((EFI_D_INFO,
     "(TXT)         Level-triggered\n"
     ));
    break;
  default:
    break;
  }
  DEBUG ((EFI_D_INFO,
    "(TXT)     Interrupt Type ....................................... 0x%02x\n",
    PlatformInterruptSource->InterruptType
    ));
  switch (PlatformInterruptSource->InterruptType) {
  case 1:
    DEBUG ((EFI_D_INFO,
      "(TXT)       PMI\n"
      ));
    break;
  case 2:
    DEBUG ((EFI_D_INFO,
      "(TXT)       INIT\n"
      ));
    break;
  case 3:
    DEBUG ((EFI_D_INFO,
      "(TXT)       Corrected Platform Error Interrupt\n"
      ));
    break;
  default:
    break;
  }
  DEBUG ((EFI_D_INFO,
    "(TXT)     Processor ID ......................................... 0x%02x\n",
    PlatformInterruptSource->ProcessorId
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Processor EID ........................................ 0x%02x\n",
    PlatformInterruptSource->ProcessorEid
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     IO SAPIC Vector ...................................... 0x%02x\n",
    PlatformInterruptSource->IoSapicVector
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Global System Interrupt .............................. 0x%08x\n",
    PlatformInterruptSource->GlobalSystemInterrupt
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)   ***************************************************************************\n\n"
    ));

  return;
}

/**

  This function dump MADT ProcessorLocalX2Apic structure.

  @param  ProcessorLocalX2Apic   MADT ProcessorLocalX2Apic structure

**/
VOID
DumpApicProcessorLocalX2Apic (
  EFI_ACPI_4_0_PROCESSOR_LOCAL_X2APIC_STRUCTURE           *ProcessorLocalX2Apic
  )
{
  DEBUG ((EFI_D_INFO,
    "(TXT)   ***************************************************************************\n"
    "(TXT)   *       Processor Local X2APIC Structure                                  *\n"
    "(TXT)   ***************************************************************************\n"
    ));
  DEBUG ((EFI_D_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "(TXT)   Processor Local X2APIC address ......................... 0x%016lx\n" :
    "(TXT)   Processor Local X2APIC address ......................... 0x%08x\n",
    ProcessorLocalX2Apic
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Type ................................................. 0x%02x\n",
    ProcessorLocalX2Apic->Type
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Length ............................................... 0x%02x\n",
    ProcessorLocalX2Apic->Length
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     X2ACPI ID ............................................ 0x%08x\n",
    ProcessorLocalX2Apic->X2ApicId
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Flags ................................................ 0x%08x\n",
    ProcessorLocalX2Apic->Flags
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)       Enabled ............................................ 0x%08x\n",
    ProcessorLocalX2Apic->Flags & EFI_ACPI_1_0_LOCAL_APIC_ENABLED
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     ACPI Processor UID ................................... 0x%08x\n",
    ProcessorLocalX2Apic->AcpiProcessorUid
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)   ***************************************************************************\n\n"
    ));

  return;
}

/**

  This function dump MADT LocalX2ApicNmi structure.

  @param  LocalX2ApicNmi   MADT LocalX2ApicNmi structure

**/
VOID
DumpApicLocalX2ApicNmi (
  EFI_ACPI_4_0_LOCAL_X2APIC_NMI_STRUCTURE                 *LocalX2ApicNmi
  )
{
  UINT16               Flags;

  DEBUG ((EFI_D_INFO,
    "(TXT)   ***************************************************************************\n"
    "(TXT)   *       Local X2APIC NMI Structure                                        *\n"
    "(TXT)   ***************************************************************************\n"
    ));
  DEBUG ((EFI_D_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "(TXT)   Local X2APIC NMI address ............................... 0x%016lx\n" :
    "(TXT)   Local X2APIC NMI address ............................... 0x%08x\n",
    LocalX2ApicNmi
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Type ................................................. 0x%02x\n",
    LocalX2ApicNmi->Type
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Length ............................................... 0x%02x\n",
    LocalX2ApicNmi->Length
    ));
  CopyMem (&Flags, &LocalX2ApicNmi->Flags, sizeof(UINT16));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Flags ................................................ 0x%04x\n",
    Flags
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)       Polarity ........................................... 0x%04x\n",
    Flags & 0x3
    ));
  switch (Flags & 0x3) {
  case 0x0:
    DEBUG ((EFI_D_INFO,
     "(TXT)         Conforms to the specifications of the bus\n"
     ));
    break;
  case 0x1:
    DEBUG ((EFI_D_INFO,
     "(TXT)         Active high\n"
     ));
    break;
  case 0x3:
    DEBUG ((EFI_D_INFO,
     "(TXT)         Active low\n"
     ));
    break;
  default:
    break;
  }
  DEBUG ((EFI_D_INFO,
    "(TXT)       Trigger Mode ....................................... 0x%04x\n",
    Flags & 0xc
    ));
  switch (Flags & 0xc) {
  case 0x0:
    DEBUG ((EFI_D_INFO,
     "(TXT)         Conforms to the specifications of the bus\n"
     ));
    break;
  case 0x4:
    DEBUG ((EFI_D_INFO,
     "(TXT)         Edge-triggered\n"
     ));
    break;
  case 0xc:
    DEBUG ((EFI_D_INFO,
     "(TXT)         Level-triggered\n"
     ));
    break;
  default:
    break;
  }
  DEBUG ((EFI_D_INFO,
    "(TXT)     ACPI Processor UID ................................... 0x%08x\n",
    LocalX2ApicNmi->AcpiProcessorUid
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Local X2APIC LINT .................................... 0x%02x\n",
    LocalX2ApicNmi->LocalX2ApicLint
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)   ***************************************************************************\n\n"
    ));

  return;
}

/**

  This function dump MADT ACPI Table.

  @param  Madt   MADT ACPI Table

**/
VOID
DumpMadt (
  IN EFI_ACPI_1_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER                            *Madt
  )
{
  APIC_STRUCT_HEADER        *ApicStructHeader;
  INTN                      MadtLen;
  INTN                      TableLen;

  //
  // Dump Madt table
  //
  DEBUG ((EFI_D_INFO,
    "(TXT) *****************************************************************************\n"
    "(TXT) *         Multiple APIC Description Table                                   *\n"
    "(TXT) *****************************************************************************\n"
    ));

  DEBUG ((EFI_D_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "(TXT) MADT address ............................................. 0x%016lx\n" :
    "(TXT) MADT address ............................................. 0x%08x\n",
    Madt
    ));
  
  DumpAcpiTableHeader ((EFI_ACPI_DESCRIPTION_HEADER *)Madt);
  
  DEBUG ((EFI_D_INFO,
    "(TXT)   Table Contents:\n"
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Local APIC Address ................................... 0x%08x\n",
    Madt->LocalApicAddress
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)     Flags ................................................ 0x%08x\n",
    Madt->Flags
    ));
  DEBUG ((EFI_D_INFO,
    "(TXT)       PCAT_COMPAT ........................................ 0x%08x\n",
    Madt->Flags & EFI_ACPI_1_0_PCAT_COMPAT
    ));

  MadtLen  = Madt->Header.Length - sizeof(EFI_ACPI_1_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER);
  TableLen = 0;
  ApicStructHeader = (APIC_STRUCT_HEADER *)(Madt + 1);
  while (MadtLen > 0) {
    switch (ApicStructHeader->Type) {
    case EFI_ACPI_1_0_PROCESSOR_LOCAL_APIC:
      DumpApicProcessorLocalApic ((EFI_ACPI_1_0_PROCESSOR_LOCAL_APIC_STRUCTURE *)ApicStructHeader);
      TableLen = sizeof(EFI_ACPI_1_0_PROCESSOR_LOCAL_APIC_STRUCTURE);
      break;
    case EFI_ACPI_1_0_IO_APIC:
      DumpApicIOApic ((EFI_ACPI_1_0_IO_APIC_STRUCTURE *)ApicStructHeader);
      TableLen = sizeof(EFI_ACPI_1_0_IO_APIC_STRUCTURE);
      break;
    case EFI_ACPI_1_0_INTERRUPT_SOURCE_OVERRIDE:
      DumpApicInterruptSourceOverride ((EFI_ACPI_1_0_INTERRUPT_SOURCE_OVERRIDE_STRUCTURE *)ApicStructHeader);
      TableLen = sizeof(EFI_ACPI_1_0_INTERRUPT_SOURCE_OVERRIDE_STRUCTURE);
      break;
    case EFI_ACPI_1_0_NON_MASKABLE_INTERRUPT_SOURCE:
      DumpApicNonMaskableInterruptSource ((EFI_ACPI_1_0_NON_MASKABLE_INTERRUPT_SOURCE_STRUCTURE *)ApicStructHeader);
      TableLen = sizeof(EFI_ACPI_1_0_NON_MASKABLE_INTERRUPT_SOURCE_STRUCTURE);
      break;
    case EFI_ACPI_1_0_LOCAL_APIC_NMI:
      DumpApicLocalApicNMI ((EFI_ACPI_1_0_LOCAL_APIC_NMI_STRUCTURE *)ApicStructHeader);
      TableLen = sizeof(EFI_ACPI_1_0_LOCAL_APIC_NMI_STRUCTURE);
      break;
    case EFI_ACPI_2_0_LOCAL_APIC_ADDRESS_OVERRIDE:
      DumpApicLocalApicAddressOverride ((EFI_ACPI_2_0_LOCAL_APIC_ADDRESS_OVERRIDE_STRUCTURE *)ApicStructHeader);
      TableLen = sizeof(EFI_ACPI_2_0_LOCAL_APIC_ADDRESS_OVERRIDE_STRUCTURE);
      break;
    case EFI_ACPI_2_0_IO_SAPIC:
      DumpApicIOSapic ((EFI_ACPI_2_0_IO_SAPIC_STRUCTURE *)ApicStructHeader);
      TableLen = sizeof(EFI_ACPI_2_0_IO_SAPIC_STRUCTURE);
      break;
    case EFI_ACPI_2_0_PROCESSOR_LOCAL_SAPIC:
      DumpApicProcessorLocalSapic ((EFI_ACPI_2_0_PROCESSOR_LOCAL_SAPIC_STRUCTURE *)ApicStructHeader);
      TableLen = sizeof(EFI_ACPI_2_0_PROCESSOR_LOCAL_SAPIC_STRUCTURE);
      break;
    case EFI_ACPI_2_0_PLATFORM_INTERRUPT_SOURCES:
      DumpApicPlatformInterruptSource ((EFI_ACPI_2_0_PLATFORM_INTERRUPT_SOURCES_STRUCTURE *)ApicStructHeader);
      TableLen = sizeof(EFI_ACPI_2_0_PLATFORM_INTERRUPT_SOURCES_STRUCTURE);
      break;
    case EFI_ACPI_4_0_PROCESSOR_LOCAL_X2APIC:
      DumpApicProcessorLocalX2Apic ((EFI_ACPI_4_0_PROCESSOR_LOCAL_X2APIC_STRUCTURE *)ApicStructHeader);
      TableLen = sizeof(EFI_ACPI_4_0_PROCESSOR_LOCAL_X2APIC_STRUCTURE);
      break;
    case EFI_ACPI_4_0_LOCAL_X2APIC_NMI:
      DumpApicLocalX2ApicNmi ((EFI_ACPI_4_0_LOCAL_X2APIC_NMI_STRUCTURE *)ApicStructHeader);
      TableLen = sizeof(EFI_ACPI_4_0_LOCAL_X2APIC_NMI_STRUCTURE);
      break;
    default:
      TableLen = ApicStructHeader->Length;
      break;
    }
    ASSERT(TableLen == ApicStructHeader->Length);
    MadtLen -= ApicStructHeader->Length;
    ApicStructHeader = (APIC_STRUCT_HEADER *)((UINT8 *)ApicStructHeader + ApicStructHeader->Length);
  }

  DEBUG ((EFI_D_INFO,
    "(TXT) *****************************************************************************\n\n"
    ));
  
  return;
}

/**

  This function dump MCFG ACPI table.

  @param  Mcfg   MCFG ACPI table

**/
VOID
DumpMcfg (
  IN EFI_ACPI_MEMORY_MAPPED_CONFIGURATION_BASE_ADDRESS_TABLE_HEADER  *Mcfg
  )
{
  UINTN                                                                                   Index;
  UINTN                                                                                   Count;
  EFI_ACPI_MEMORY_MAPPED_ENHANCED_CONFIGURATION_SPACE_BASE_ADDRESS_ALLOCATION_STRUCTURE   *McfgStruct;


  DEBUG ((EFI_D_INFO,
    "(TXT) *****************************************************************************\n"
    "(TXT) *         MCFG Table                                                        *\n"
    "(TXT) *****************************************************************************\n"
    ));

  DEBUG ((EFI_D_INFO,
    (sizeof(UINTN) == sizeof(UINT64)) ?
    "(TXT) MCFG address ............................................. 0x%016lx\n" :
    "(TXT) MCFG address ............................................. 0x%08x\n",
    Mcfg
    ));

  DumpAcpiTableHeader ((EFI_ACPI_DESCRIPTION_HEADER *)Mcfg);
   
  DEBUG ((EFI_D_INFO, "(TXT)   Table Contents:\n"));

  Count = (Mcfg->Header.Length - sizeof(*Mcfg)) / sizeof(*McfgStruct);
  McfgStruct = (EFI_ACPI_MEMORY_MAPPED_ENHANCED_CONFIGURATION_SPACE_BASE_ADDRESS_ALLOCATION_STRUCTURE *)(Mcfg + 1);
  for (Index = 0; Index < Count; Index++) {
    DEBUG ((EFI_D_INFO,
      "(TXT)   McfgStruct[%d]:\n",
      Index
      ));
    DEBUG ((EFI_D_INFO,
      "(TXT)     Bass Address ......................................... 0x%016lx\n",
      McfgStruct->BaseAddress
      ));
    DEBUG ((EFI_D_INFO,
      "(TXT)     PCI Segment Group Number ............................. 0x%04x\n",
      McfgStruct->PciSegmentGroupNumber
      ));
    DEBUG ((EFI_D_INFO,
      "(TXT)     Start Bus Number ..................................... 0x%02x\n",
      McfgStruct->StartBusNumber
      ));
    DEBUG ((EFI_D_INFO,
      "(TXT)     End Bus Number ....................................... 0x%02x\n",
      McfgStruct->EndBusNumber
      ));
  }

  DEBUG ((EFI_D_INFO,
    "(TXT) *****************************************************************************\n\n"
    ));

  return ;
}

//
// This table defines the ACM type string
//
GLOBAL_REMOVE_IF_UNREFERENCED CHAR16 *mAcmTypeStr[] = {
  L"BIOS ACM",
  L"SINIT ACM",
};

//
// This table defines the ACM capability string
//
GLOBAL_REMOVE_IF_UNREFERENCED CHAR16 *mCapabilityStr[] = {
  L"GETSEC[WAKEUP] for RLP    ",
  L"MONITOR address for RLP   ",
  L"ECX for MLE PageTable     ",
  L"STM support               ",
  L"TPM12 PCR no legacy       ",
  L"TPM12 PCR detail authority",
  L"Platform Type Client      ",
  L"Platform Type Server      ",
  L"MAXPHYADDR supported      ",
  L"TCG2 compatible eventlog  ",
};

//
// This table defines the ACM TPM capability string
//
GLOBAL_REMOVE_IF_UNREFERENCED CHAR16 *mTpmCapabilityStr[] = {
  L"Maximum Agility Policy    ",
  L"Maximum performance Policy",
  L"Discrete TPM 1.2 Support  ",
  L"Discrete TPM 2.0 Support  ",
  NULL,
  L"Firmware TPM 2.0 Support  ",
  L"TCG2 compliant NV Index   ",
};

/**

  This function dump ACM binary info.

  @param  Acm   ACM binary

**/
VOID
DumpAcm (
  IN TXT_ACM_FORMAT                    *Acm
  )
{
  TXT_CHIPSET_ACM_INFORMATION_TABLE *ChipsetAcmInformationTable;
  TXT_CHIPSET_ID_LIST               *ChipsetIdList;
  TXT_PROCESSOR_ID_LIST             *ProcessorIdList;
  TXT_ACM_TPM_INFO_LIST             *TpmInfoList;
  UINTN                             Index;
  UINT8                             *Buffer;

  DEBUG ((EFI_D_INFO,
    "(TXT) *****************************************************************************\n"
    "(TXT) *         ACM                                                               *\n"
    "(TXT) *****************************************************************************\n"
    ));

  DEBUG ((EFI_D_INFO, "(TXT) ACM: (%08x)\n", Acm));
  DEBUG ((EFI_D_INFO, "(TXT)   ModuleType                 - %04x\n", (UINTN)Acm->ModuleType));
  if (Acm->ModuleType == TXT_ACM_MODULE_TYPE_CHIPSET_ACM) {
    DEBUG ((EFI_D_INFO, "(TXT)     Chipset ACM\n"));
  }
  DEBUG ((EFI_D_INFO, "(TXT)   ModuleSubType              - %04x\n", (UINTN)Acm->ModuleSubType));
  if (Acm->ModuleSubType == TXT_ACM_MODULE_SUBTYPE_CAPABLE_OF_EXECUTE_AT_RESET) {
    DEBUG ((EFI_D_INFO, "(TXT)     Capable of be Executed at Reset\n"));
  }
  DEBUG ((EFI_D_INFO, "(TXT)   HeaderLen                  - %08x\n", (UINTN)Acm->HeaderLen));
  DEBUG ((EFI_D_INFO, "(TXT)   HeaderVersion              - %08x\n", (UINTN)Acm->HeaderVersion));
  DEBUG ((EFI_D_INFO, "(TXT)   ChipsetID                  - %04x\n", (UINTN)Acm->ChipsetID));
  DEBUG ((EFI_D_INFO, "(TXT)   Flags                      - %04x\n", (UINTN)Acm->Flags));
  DEBUG ((EFI_D_INFO, "(TXT)     PreProduction            - %04x\n", (UINTN)Acm->Flags & TXT_ACM_MODULE_FLAG_PREPRODUCTION));
  DEBUG ((EFI_D_INFO, "(TXT)     Debug Signed             - %04x\n", (UINTN)Acm->Flags & TXT_ACM_MODULE_FLAG_DEBUG_SIGN));
  DEBUG ((EFI_D_INFO, "(TXT)   ModuleVendor               - %08x\n", (UINTN)Acm->ModuleVendor));
  DEBUG ((EFI_D_INFO, "(TXT)   Date                       - %08x\n", (UINTN)Acm->Date));
  DEBUG ((EFI_D_INFO, "(TXT)   Size                       - %08x\n", (UINTN)Acm->Size));
  DEBUG ((EFI_D_INFO, "(TXT)   CodeControl                - %08x\n", (UINTN)Acm->CodeControl));
  DEBUG ((EFI_D_INFO, "(TXT)   ErrorEntryPoint            - %08x\n", (UINTN)Acm->ErrorEntryPoint));
  DEBUG ((EFI_D_INFO, "(TXT)   GDTLimit                   - %08x\n", (UINTN)Acm->GDTLimit));
  DEBUG ((EFI_D_INFO, "(TXT)   GDTBasePtr                 - %08x\n", (UINTN)Acm->GDTBasePtr));
  DEBUG ((EFI_D_INFO, "(TXT)   SegSel                     - %08x\n", (UINTN)Acm->SegSel));
  DEBUG ((EFI_D_INFO, "(TXT)   EntryPoint                 - %08x\n", (UINTN)Acm->EntryPoint));
  DEBUG ((EFI_D_INFO, "(TXT)   KeySize                    - %08x\n", (UINTN)Acm->KeySize));
  DEBUG ((EFI_D_INFO, "(TXT)   ScratchSize                - %08x\n", (UINTN)Acm->ScratchSize));

  Buffer = (UINT8 *)(Acm + 1);
  DEBUG ((EFI_D_INFO, "(TXT)   RSAPubKey                  - \n"));
  DumpHex (Buffer, Acm->KeySize * 4);
  Buffer += Acm->KeySize * 4;

  DEBUG ((EFI_D_INFO, "(TXT)   RSAPubExp                  - %08x\n", (UINTN)*(UINT32 *)Buffer));
  Buffer += 4;

  DEBUG ((EFI_D_INFO, "(TXT)   RSASig                     - \n"));
  DumpHex (Buffer, ACM_PKCS_1_5_RSA_SIGNATURE_SIZE); // PKCS #1.5 RSA Signature
  Buffer += ACM_PKCS_1_5_RSA_SIGNATURE_SIZE;

  Buffer += Acm->ScratchSize * 4;

  ChipsetAcmInformationTable = (TXT_CHIPSET_ACM_INFORMATION_TABLE *)Buffer;
  DEBUG ((EFI_D_INFO, "(TXT) Chipset ACM info:\n"));
  DEBUG ((EFI_D_INFO,
    "(TXT)   Uuid                       - {%08x-%08x-%08x-%08x}\n",
    (UINTN)ChipsetAcmInformationTable->Uuid.Uuid0,
    (UINTN)ChipsetAcmInformationTable->Uuid.Uuid1,
    (UINTN)ChipsetAcmInformationTable->Uuid.Uuid2,
    (UINTN)ChipsetAcmInformationTable->Uuid.Uuid3
    ));
  DEBUG ((EFI_D_INFO, "(TXT)   ChipsetACMType             - %02x\n", (UINTN)ChipsetAcmInformationTable->ChipsetACMType));
  if (ChipsetAcmInformationTable->ChipsetACMType < sizeof(mAcmTypeStr)/sizeof(mAcmTypeStr[0])) {
    DEBUG ((EFI_D_INFO, "(TXT)     %s\n", mAcmTypeStr[ChipsetAcmInformationTable->ChipsetACMType]));
  }
  DEBUG ((EFI_D_INFO, "(TXT)   Version                    - %02x\n", (UINTN)ChipsetAcmInformationTable->Version));
  DEBUG ((EFI_D_INFO, "(TXT)   Length                     - %04x\n", (UINTN)ChipsetAcmInformationTable->Length));
  DEBUG ((EFI_D_INFO, "(TXT)   ChipsetIDList              - %08x\n", (UINTN)ChipsetAcmInformationTable->ChipsetIDList));
  DEBUG ((EFI_D_INFO, "(TXT)   OsSinitTableVer            - %08x\n", (UINTN)ChipsetAcmInformationTable->OsSinitTableVer));
  DEBUG ((EFI_D_INFO, "(TXT)   MinMleHeaderVer            - %08x\n", (UINTN)ChipsetAcmInformationTable->MinMleHeaderVer));
  if (ChipsetAcmInformationTable->Version >= TXT_CHIPSET_ACM_INFORMATION_TABLE_VERSION_3) {
    DEBUG ((EFI_D_INFO, "(TXT)   Capabilities               - %08x\n", (UINTN)ChipsetAcmInformationTable->Capabilities));
    for (Index = 0; Index < sizeof(mCapabilityStr)/sizeof(mCapabilityStr[0]); Index++) {
      if (mCapabilityStr[Index] == NULL) {
        continue;
      }
      DEBUG ((EFI_D_INFO,
        "(TXT)     %s- %08x\n",
        mCapabilityStr[Index],
        (UINTN)(ChipsetAcmInformationTable->Capabilities & (1 << Index))
        ));
    }
    DEBUG ((EFI_D_INFO, "(TXT)   AcmVersion                 - %02x\n", (UINTN)ChipsetAcmInformationTable->AcmVersion));
    if (ChipsetAcmInformationTable->Version >= TXT_CHIPSET_ACM_INFORMATION_TABLE_VERSION_6) {
      DEBUG ((EFI_D_INFO, "(TXT)   AcmRevision                - %02x.%02x.%02x\n",
        (UINTN)ChipsetAcmInformationTable->AcmRevision[0],
        (UINTN)ChipsetAcmInformationTable->AcmRevision[1],
        (UINTN)ChipsetAcmInformationTable->AcmRevision[2]
        ));
    }
  }
  if (ChipsetAcmInformationTable->Version >= TXT_CHIPSET_ACM_INFORMATION_TABLE_VERSION_4) {
    DEBUG ((EFI_D_INFO, "(TXT)   ProcessorIDList            - %08x\n", (UINTN)ChipsetAcmInformationTable->ProcessorIDList));
  }

  ChipsetIdList = (TXT_CHIPSET_ID_LIST *)((UINTN)Acm + ChipsetAcmInformationTable->ChipsetIDList);
  DEBUG ((EFI_D_INFO, "(TXT) Chipset ID List info:\n"));
  DEBUG ((EFI_D_INFO, "(TXT)   Count                      - %08x\n", (UINTN)ChipsetIdList->Count));
  for (Index = 0; Index < ChipsetIdList->Count; Index++) {
    DEBUG ((EFI_D_INFO, "(TXT)   ID[%d]:\n", (UINTN)Index));
    DEBUG ((EFI_D_INFO, "(TXT)     Flags                    - %08x\n", (UINTN)ChipsetIdList->ChipsetID[Index].Flags));
    DEBUG ((EFI_D_INFO, "(TXT)       RevisionIdMask         - %08x\n", (UINTN)ChipsetIdList->ChipsetID[Index].Flags & TXT_ACM_CHIPSET_ID_REVISION_ID_MAKE));
    DEBUG ((EFI_D_INFO, "(TXT)     VendorID                 - %04x\n", (UINTN)ChipsetIdList->ChipsetID[Index].VendorID));
    DEBUG ((EFI_D_INFO, "(TXT)     DeviceID                 - %04x\n", (UINTN)ChipsetIdList->ChipsetID[Index].DeviceID));
    DEBUG ((EFI_D_INFO, "(TXT)     RevisionID               - %04x\n", (UINTN)ChipsetIdList->ChipsetID[Index].RevisionID));
    DEBUG ((EFI_D_INFO, "(TXT)     ExtendedID               - %08x\n", (UINTN)ChipsetIdList->ChipsetID[Index].ExtendedID));
  }
  if (ChipsetAcmInformationTable->Version < TXT_CHIPSET_ACM_INFORMATION_TABLE_VERSION_4) {
    goto End;
  }
  ProcessorIdList = (TXT_PROCESSOR_ID_LIST *)((UINTN)Acm + ChipsetAcmInformationTable->ProcessorIDList);
  DEBUG ((EFI_D_INFO, "(TXT) Processor ID List info:\n"));
  DEBUG ((EFI_D_INFO, "(TXT)   Count                      - %08x\n", (UINTN)ProcessorIdList->Count));
  for (Index = 0; Index < ProcessorIdList->Count; Index++) {
    DEBUG ((EFI_D_INFO, "(TXT)   ID[%d]:\n", (UINTN)Index));
    DEBUG ((EFI_D_INFO, "(TXT)     FMS                      - %08x\n", (UINTN)ProcessorIdList->ProcessorID[Index].FMS));
    DEBUG ((EFI_D_INFO, "(TXT)     FMSMask                  - %08x\n", (UINTN)ProcessorIdList->ProcessorID[Index].FMSMask));
    DEBUG ((EFI_D_INFO, "(TXT)     PlatformID               - %016lx\n", ProcessorIdList->ProcessorID[Index].PlatformID));
    DEBUG ((EFI_D_INFO, "(TXT)     PlatformMask             - %016lx\n", ProcessorIdList->ProcessorID[Index].PlatformMask));
  }

  if (ChipsetAcmInformationTable->Version >= TXT_CHIPSET_ACM_INFORMATION_TABLE_VERSION_5) {
    DEBUG ((EFI_D_INFO, "(TXT)   TPMInfoList                - %08x\n", (UINTN)ChipsetAcmInformationTable->TPMInfoList));
    TpmInfoList = (TXT_ACM_TPM_INFO_LIST *)((UINTN)Acm + ChipsetAcmInformationTable->TPMInfoList);
    DEBUG ((EFI_D_INFO, "(TXT) TPM Info List info:\n"));
    DEBUG ((EFI_D_INFO, "(TXT)   Capabilities               - %08x\n", (UINTN)TpmInfoList->Capabilities));
    for (Index = 0; Index < sizeof(mTpmCapabilityStr) / sizeof(mTpmCapabilityStr[0]); Index++) {
      if (mTpmCapabilityStr[Index] == NULL) {
        continue;
      }
      DEBUG((EFI_D_INFO,
        "(TXT)     %s- %08x\n",
        mTpmCapabilityStr[Index],
        (UINTN)(TpmInfoList->Capabilities & (1 << Index))
        ));
    }
    DEBUG ((EFI_D_INFO, "(TXT)   Count                      - %04x\n", (UINTN)TpmInfoList->Count));
    for (Index = 0; Index < TpmInfoList->Count; Index++) {
      DEBUG ((EFI_D_INFO, "(TXT)   AlgorithmID                - %04x\n", (UINTN)TpmInfoList->AlgorithmID[Index]));
    }
  }

End:
  DEBUG ((EFI_D_INFO,
    "(TXT) *****************************************************************************\n\n"
    ));

  return ;
}

/**

  This function dump MLE header info.

  @param  MleHeader   MLE header

**/
VOID
DumpMleHeader (
  IN TXT_MLE_HEADER   *MleHeader
  )
{
  UINTN  Index;

  DEBUG ((EFI_D_INFO,
    "(TXT) *****************************************************************************\n"
    "(TXT) *         MLE Header                                                        *\n"
    "(TXT) *****************************************************************************\n"
    ));

  DEBUG ((EFI_D_INFO, "(TXT) MLE Header: (%08x)\n", MleHeader));
  DEBUG ((EFI_D_INFO,
    "(TXT)   Uuid                       - {%08x-%08x-%08x-%08x}\n",
    (UINTN)MleHeader->Uuid.Uuid0,
    (UINTN)MleHeader->Uuid.Uuid1,
    (UINTN)MleHeader->Uuid.Uuid2,
    (UINTN)MleHeader->Uuid.Uuid3
    ));
  DEBUG ((EFI_D_INFO, "(TXT)   HeaderLen                  - %08x\n", (UINTN)MleHeader->HeaderLen));
  DEBUG ((EFI_D_INFO, "(TXT)   Version                    - %08x\n", (UINTN)MleHeader->Version));
  DEBUG ((EFI_D_INFO, "(TXT)   EntryPoint                 - %08x\n", (UINTN)MleHeader->EntryPoint));
  if (MleHeader->Version < TXT_MLE_HEADER_VERSION_1_1) {
    goto End ;
  }
  DEBUG ((EFI_D_INFO, "(TXT)   FirstValidPage             - %08x\n", (UINTN)MleHeader->FirstValidPage));
  DEBUG ((EFI_D_INFO, "(TXT)   MleStart                   - %08x\n", (UINTN)MleHeader->MleStart));
  DEBUG ((EFI_D_INFO, "(TXT)   MleEnd                     - %08x\n", (UINTN)MleHeader->MleEnd));
  if (MleHeader->Version < TXT_MLE_HEADER_VERSION_2) {
    goto End ;
  }
  DEBUG ((EFI_D_INFO, "(TXT)   Capabilities               - %08x\n", (UINTN)MleHeader->Capabilities));
  for (Index = 0; Index < sizeof(mCapabilityStr)/sizeof(mCapabilityStr[0]); Index++) {
    if (mCapabilityStr[Index] == NULL) {
      continue;
    }
    DEBUG ((EFI_D_INFO,
      "(TXT)     %s- %08x\n",
      mCapabilityStr[Index],
      MleHeader->Capabilities & (1 << Index)
      ));
  }
  if (MleHeader->Version < TXT_MLE_HEADER_VERSION_2_1) {
    goto End ;
  }
  DEBUG ((EFI_D_INFO, "(TXT)   CmdlineStart               - %08x\n", (UINTN)MleHeader->CmdlineStart));
  DEBUG ((EFI_D_INFO, "(TXT)   CmdlineEnd                 - %08x\n", (UINTN)MleHeader->CmdlineEnd));

End:
  DEBUG ((EFI_D_INFO,
    "(TXT) *****************************************************************************\n\n"
    ));

  return ;
}

/**

  This function dump TXT Ext Data Element.

  @param  ExtDataElement   Ext Data Element
  @param  MaxSize          Max size of Ext Data Element

**/
VOID
DumpExtDataElement (
  IN TXT_HEAP_EXT_DATA_ELEMENT       *ExtDataElement,
  IN UINTN                           MaxSize
  )
{
  VOID                            *ExtDataElementEnd;
  TXT_HEAP_BIOS_SPEC_VER_ELEMENT  *ExtDataBiosSpecVer;
  TXT_HEAP_BIOSACM_ELEMENT        *ExtDataBiosAcm;
  UINT64                          *BiosAcmAddrs;
  TXT_HEAP_BIOS_EXT_ELEMENT       *ExtDataExtBios;
  TXT_HEAP_CUSTOM_ELEMENT         *ExtDataCustomer;
  TXT_HEAP_EVENTLOG_EXT_ELEMENT   *ExtDataEventLog;
  TXT_EVENT_LOG_CONTAINER         *EventLogContainer;
  TXT_HEAP_EVENT_LOG_DESCR        *EventLogDesc;
  UINTN                           Index;
  TXT_HEAP_EVENT_LOG_POINTER_ELEMENT2   *ExtDataEventLogPointerElement2;
  TXT_HEAP_EVENT_LOG_POINTER_ELEMENT2_1 *ExtDataEventLogPointerElement2_1;

  ExtDataElementEnd = (UINT8 *)ExtDataElement + MaxSize;

  while ((UINTN)ExtDataElement < (UINTN)ExtDataElementEnd) {
    switch (ExtDataElement->Type) {
    case TXT_HEAP_EXTDATA_TYPE_END:
      DEBUG ((EFI_D_INFO, "(TXT)   ExtDataTypeEnd:\n"));
      DEBUG ((EFI_D_INFO, "(TXT)     Type                     - %08x\n", (UINTN)ExtDataElement->Type));
      DEBUG ((EFI_D_INFO, "(TXT)     Size                     - %08x\n", (UINTN)ExtDataElement->Size));
      break;
    case TXT_HEAP_EXTDATA_TYPE_BIOS_SPEC_VER:
      ExtDataBiosSpecVer = (TXT_HEAP_BIOS_SPEC_VER_ELEMENT *)(ExtDataElement + 1);
      DEBUG ((EFI_D_INFO, "(TXT)   ExtDataTypeBiosSpecVer:\n"));
      DEBUG ((EFI_D_INFO, "(TXT)     Type                     - %08x\n", (UINTN)ExtDataElement->Type));
      DEBUG ((EFI_D_INFO, "(TXT)     Size                     - %08x\n", (UINTN)ExtDataElement->Size));
      DEBUG ((EFI_D_INFO, "(TXT)     SpecVer                  - %04x.%04x.%04x\n", (UINTN)ExtDataBiosSpecVer->SpecVerMajor, (UINTN)ExtDataBiosSpecVer->SpecVerMinor, (UINTN)ExtDataBiosSpecVer->SpecVerRevision));
      break;
    case TXT_HEAP_EXTDATA_TYPE_BIOSACM:
      ExtDataBiosAcm = (TXT_HEAP_BIOSACM_ELEMENT *)(ExtDataElement + 1);
      DEBUG ((EFI_D_INFO, "(TXT)   ExtDataTypeBiosAcm:\n"));
      DEBUG ((EFI_D_INFO, "(TXT)     Type                     - %08x\n", (UINTN)ExtDataElement->Type));
      DEBUG ((EFI_D_INFO, "(TXT)     Size                     - %08x\n", (UINTN)ExtDataElement->Size));
      DEBUG ((EFI_D_INFO, "(TXT)     NumAcms                  - %08x\n", (UINTN)ExtDataBiosAcm->NumAcms));
      BiosAcmAddrs = (UINT64 *)(ExtDataBiosAcm + 1);
      for (Index = 0; Index < ExtDataBiosAcm->NumAcms; Index++) {
        DEBUG ((EFI_D_INFO, "(TXT)     BiosAcmAddrs             - %016lx\n", BiosAcmAddrs[Index]));
      }
      break;
    case TXT_HEAP_EXTDATA_TYPE_BIOS_EXT:
      ExtDataExtBios = (TXT_HEAP_BIOS_EXT_ELEMENT *)(ExtDataElement + 1);
      DEBUG ((EFI_D_INFO, "(TXT)   ExtDataTypeBiosExt:\n"));
      DEBUG ((EFI_D_INFO, "(TXT)     Type                     - %08x\n", (UINTN)ExtDataElement->Type));
      DEBUG ((EFI_D_INFO, "(TXT)     Size                     - %08x\n", (UINTN)ExtDataElement->Size));
      DEBUG ((EFI_D_INFO, "(TXT)     StmSpecVer               - %02x.%02x\n", (UINTN)ExtDataExtBios->StmSpecVerMajor, (UINTN)ExtDataExtBios->StmSpecVerMinor));
      DEBUG ((EFI_D_INFO, "(TXT)     BiosSmmFlags             - %04x\n", (UINTN)ExtDataExtBios->BiosSmmFlags));
      DEBUG ((EFI_D_INFO, "(TXT)     StmFeatureFlags          - %04x\n", (UINTN)ExtDataExtBios->StmFeatureFlags));
      DEBUG ((EFI_D_INFO, "(TXT)     RequiredStmSmmRevId      - %08x\n", (UINTN)ExtDataExtBios->RequiredStmSmmRevId));
      DEBUG ((EFI_D_INFO, "(TXT)     GetBiosAcStatusCmd       - %02x\n", (UINTN)ExtDataExtBios->GetBiosAcStatusCmd));
      DEBUG ((EFI_D_INFO, "(TXT)     UpdateBiosAcCmd          - %02x\n", (UINTN)ExtDataExtBios->UpdateBiosAcCmd));
      DEBUG ((EFI_D_INFO, "(TXT)     GetSinitAcStatusCmd      - %02x\n", (UINTN)ExtDataExtBios->GetSinitAcStatusCmd));
      DEBUG ((EFI_D_INFO, "(TXT)     UpdateSinitAcCmd         - %02x\n", (UINTN)ExtDataExtBios->UpdateSinitAcCmd));
      DEBUG ((EFI_D_INFO, "(TXT)     GetStmStatusCmd          - %02x\n", (UINTN)ExtDataExtBios->GetStmStatusCmd));
      DEBUG ((EFI_D_INFO, "(TXT)     UpdateStmCmd             - %02x\n", (UINTN)ExtDataExtBios->UpdateStmCmd));
      DEBUG ((EFI_D_INFO, "(TXT)     HandleBiosResourcesCmd   - %02x\n", (UINTN)ExtDataExtBios->HandleBiosResourcesCmd));
      DEBUG ((EFI_D_INFO, "(TXT)     AccessResourcesCmd       - %02x\n", (UINTN)ExtDataExtBios->AccessResourcesCmd));
      DEBUG ((EFI_D_INFO, "(TXT)     LoadStmCmd               - %02x\n", (UINTN)ExtDataExtBios->LoadStmCmd));
      break;
    case TXT_HEAP_EXTDATA_TYPE_CUSTOM:
      ExtDataCustomer = (TXT_HEAP_CUSTOM_ELEMENT *)(ExtDataElement + 1);
      DEBUG ((EFI_D_INFO, "(TXT)   ExtDataTypeCustom:\n"));
      DEBUG ((EFI_D_INFO, "(TXT)     Type                     - %08x\n", (UINTN)ExtDataElement->Type));
      DEBUG ((EFI_D_INFO, "(TXT)     Size                     - %08x\n", (UINTN)ExtDataElement->Size));
      DEBUG ((EFI_D_INFO, "(TXT)     Uuid                     - %08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x\n",
        (UINTN)ExtDataCustomer->Uuid.Data1,
        (UINTN)ExtDataCustomer->Uuid.Data2,
        (UINTN)ExtDataCustomer->Uuid.Data3,
        (UINTN)ExtDataCustomer->Uuid.Data4,
        (UINTN)ExtDataCustomer->Uuid.Data5[0],
        (UINTN)ExtDataCustomer->Uuid.Data5[1],
        (UINTN)ExtDataCustomer->Uuid.Data5[2],
        (UINTN)ExtDataCustomer->Uuid.Data5[3],
        (UINTN)ExtDataCustomer->Uuid.Data5[4],
        (UINTN)ExtDataCustomer->Uuid.Data5[5]
        ));
      break;
    case TXT_HEAP_EXTDATA_TYPE_EVENTLOG_PTR:
      ExtDataEventLog = (TXT_HEAP_EVENTLOG_EXT_ELEMENT *)(ExtDataElement + 1);
      DEBUG ((EFI_D_INFO, "(TXT)   ExtDataTypeEventLog:\n"));
      DEBUG ((EFI_D_INFO, "(TXT)     Type                     - %08x\n", (UINTN)ExtDataElement->Type));
      DEBUG ((EFI_D_INFO, "(TXT)     Size                     - %08x\n", (UINTN)ExtDataElement->Size));
      DEBUG ((EFI_D_INFO, "(TXT)     EventLogAddress          - %016lx\n", (UINTN)ExtDataEventLog->EventLogAddress));
      EventLogContainer = (TXT_EVENT_LOG_CONTAINER *)(UINTN)ExtDataEventLog->EventLogAddress;
      DEBUG ((EFI_D_INFO, "(TXT)       EventLogContainer:\n"));
      DEBUG ((EFI_D_INFO, "(TXT)         Signature            - '"));
      for (Index = 0; Index < sizeof(EventLogContainer->Signature); Index++) {
        DEBUG ((EFI_D_INFO, "%c", EventLogContainer->Signature[Index]));
      }
      DEBUG ((EFI_D_INFO, "'\n"));
      DEBUG ((EFI_D_INFO, "(TXT)         ContainerVersion     - %02x.%02x\n", (UINTN)EventLogContainer->ContainerVersionMajor, (UINTN)EventLogContainer->ContainerVersionMinor));
      DEBUG ((EFI_D_INFO, "(TXT)         PcrEventVersion      - %02x.%02x\n", (UINTN)EventLogContainer->PcrEventVersionMajor, (UINTN)EventLogContainer->PcrEventVersionMinor));
      DEBUG ((EFI_D_INFO, "(TXT)         Size                 - %08x\n", (UINTN)EventLogContainer->Size));
      DEBUG ((EFI_D_INFO, "(TXT)         PcrEventsOffset      - %08x\n", (UINTN)EventLogContainer->PcrEventsOffset));
      DEBUG ((EFI_D_INFO, "(TXT)         NextEventOffset      - %08x\n", (UINTN)EventLogContainer->NextEventOffset));
      break;
    case TXT_HEAP_EXTDATA_TYPE_MADT:
      DEBUG ((EFI_D_INFO, "(TXT)   ExtDataEventMadt:\n"));
      DEBUG ((EFI_D_INFO, "(TXT)     Type                     - %08x\n", (UINTN)ExtDataElement->Type));
      DEBUG ((EFI_D_INFO, "(TXT)     Size                     - %08x\n", (UINTN)ExtDataElement->Size));
      DumpMadt ((EFI_ACPI_1_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER *)(ExtDataElement + 1));
      break;
    case TXT_HEAP_EXTDATA_TYPE_EVENT_LOG_POINTER2:
      ExtDataEventLogPointerElement2 = (TXT_HEAP_EVENT_LOG_POINTER_ELEMENT2 *)(ExtDataElement + 1);
      DEBUG ((EFI_D_INFO, "(TXT)   ExtDataEventLogPointerElement2:\n"));
      DEBUG ((EFI_D_INFO, "(TXT)     Type                     - %08x\n", (UINTN)ExtDataElement->Type));
      DEBUG ((EFI_D_INFO, "(TXT)     Size                     - %08x\n", (UINTN)ExtDataElement->Size));
      DEBUG ((EFI_D_INFO, "(TXT)     Count                    - %08x\n", (UINTN)ExtDataEventLogPointerElement2->Count));
      EventLogDesc = (TXT_HEAP_EVENT_LOG_DESCR *)(ExtDataEventLogPointerElement2 + 1);
      for (Index = 0; Index < ExtDataEventLogPointerElement2->Count; Index++, EventLogDesc++) {
        DEBUG ((EFI_D_INFO, "(TXT)     HashAlgID(%d)             - %04x\n", Index, EventLogDesc->HashAlgID));
        DEBUG ((EFI_D_INFO, "(TXT)     PhysicalAddress          - %016lx\n", EventLogDesc->PhysicalAddress));
        DEBUG ((EFI_D_INFO, "(TXT)     AllocatedEventContainSize- %08x\n", EventLogDesc->AllocatedEventContainerSize));
        DEBUG ((EFI_D_INFO, "(TXT)     FirstRecordOffset        - %08x\n", EventLogDesc->FirstRecordOffset));
        DEBUG ((EFI_D_INFO, "(TXT)     NextRecordOffset         - %08x\n", EventLogDesc->NextRecordOffset));
      }
      break;
    case TXT_HEAP_EXTDATA_TYPE_EVENT_LOG_POINTER2_1:
      ExtDataEventLogPointerElement2_1 = (TXT_HEAP_EVENT_LOG_POINTER_ELEMENT2_1 *)(ExtDataElement + 1);
      DEBUG ((EFI_D_INFO, "(TXT)   ExtDataEventLogPointerElement2_1:\n"));
      DEBUG ((EFI_D_INFO, "(TXT)     Type                     - %08x\n", (UINTN)ExtDataElement->Type));
      DEBUG ((EFI_D_INFO, "(TXT)     Size                     - %08x\n", (UINTN)ExtDataElement->Size));
      DEBUG ((EFI_D_INFO, "(TXT)     PhysicalAddress          - %016lx\n", ExtDataEventLogPointerElement2_1->PhysicalAddress));
      DEBUG ((EFI_D_INFO, "(TXT)     AllocatedEventContainSize- %08x\n", ExtDataEventLogPointerElement2_1->AllocatedEventContainerSize));
      DEBUG ((EFI_D_INFO, "(TXT)     FirstRecordOffset        - %08x\n", ExtDataEventLogPointerElement2_1->FirstRecordOffset));
      DEBUG ((EFI_D_INFO, "(TXT)     NextRecordOffset         - %08x\n", ExtDataEventLogPointerElement2_1->NextRecordOffset));
      break;
    case TXT_HEAP_EXTDATA_TYPE_MCFG:
      DEBUG ((EFI_D_INFO, "(TXT)   ExtDataEventMcfg:\n"));
      DEBUG ((EFI_D_INFO, "(TXT)     Type                     - %08x\n", (UINTN)ExtDataElement->Type));
      DEBUG ((EFI_D_INFO, "(TXT)     Size                     - %08x\n", (UINTN)ExtDataElement->Size));
      DumpMcfg ((EFI_ACPI_MEMORY_MAPPED_CONFIGURATION_BASE_ADDRESS_TABLE_HEADER *)(ExtDataElement + 1));
      break;
    default:
      break;
    }
    if (ExtDataElement->Type == TXT_HEAP_EXTDATA_TYPE_END) {
      break;
    } else {
      ExtDataElement = (TXT_HEAP_EXT_DATA_ELEMENT *)((UINTN)ExtDataElement + ExtDataElement->Size);
    }
  }
}

/**

  This function dump TXT BiosToOs data.

  @param  Data   TXT BiosToOs data

**/
VOID
DumpBiosToOsData (
  IN UINT64  *Data
  )
{
  TXT_BIOS_TO_OS_DATA             *BiosToOsData;
  TXT_HEAP_EXT_DATA_ELEMENT       *ExtDataElement;
  UINTN                           TotalBiosToOsDataSize;

  DEBUG ((EFI_D_INFO,
    "(TXT) *****************************************************************************\n"
    "(TXT) *         BIOS TO OS Data                                                   *\n"
    "(TXT) *****************************************************************************\n"
    ));

  DEBUG ((EFI_D_INFO, "(TXT) BiosToOsData: (%08x)\n", Data));
  DEBUG ((EFI_D_INFO, "(TXT)   Size                       - %016lx\n", *Data));
  BiosToOsData = (TXT_BIOS_TO_OS_DATA *)(Data + 1);
  DEBUG ((EFI_D_INFO, "(TXT)   Version                    - %08x\n", (UINTN)BiosToOsData->Version));
  DEBUG ((EFI_D_INFO, "(TXT)   BiosSinitSize              - %08x\n", (UINTN)BiosToOsData->BiosSinitSize));
  if (BiosToOsData->Version < TXT_BIOS_TO_OS_DATA_VERSION_2) {
    return ;
  }
  DEBUG ((EFI_D_INFO, "(TXT)   LcpPdBase                  - %08x\n", (UINTN)BiosToOsData->LcpPdBase));
  DEBUG ((EFI_D_INFO, "(TXT)   LcpPdSize                  - %08x\n", (UINTN)BiosToOsData->LcpPdSize));
  DEBUG ((EFI_D_INFO, "(TXT)   NumLogProcs                - %08x\n", (UINTN)BiosToOsData->NumLogProcs));
  if (BiosToOsData->Version < TXT_BIOS_TO_OS_DATA_VERSION_3) {
    return ;
  }
  if (BiosToOsData->Version < TXT_BIOS_TO_OS_DATA_VERSION_5) {
    DEBUG ((EFI_D_INFO, "(TXT)   SinitFlags                 - %08x\n", (UINTN)BiosToOsData->SinitFlags));
  }
  if (BiosToOsData->Version >= TXT_BIOS_TO_OS_DATA_VERSION_5) {
    DEBUG ((EFI_D_INFO, "(TXT)   MleFlags                   - %08x\n", (UINTN)BiosToOsData->MleFlags));
  }

  if (BiosToOsData->Version >= TXT_BIOS_TO_OS_DATA_VERSION_4) {
    TotalBiosToOsDataSize = (UINTN)*((UINT64 *)BiosToOsData - 1) - sizeof(UINT64);
    ExtDataElement = (TXT_HEAP_EXT_DATA_ELEMENT *)(BiosToOsData + 1);
    DumpExtDataElement (ExtDataElement, (UINTN)BiosToOsData + TotalBiosToOsDataSize - (UINTN)ExtDataElement);
  }

  DEBUG ((EFI_D_INFO,
    "(TXT) *****************************************************************************\n\n"
    ));

  return ;
}

/**

  This function dump TXT OsToSinit data.

  @param  Data   TXT OsToSinit data

**/
VOID
DumpOsToSinitData (
  IN UINT64  *Data
  )
{
  TXT_OS_TO_SINIT_DATA            *OsToSinitData;
  TXT_HEAP_EXT_DATA_ELEMENT       *ExtDataElement;
  UINTN                           TotalOsToSinitDataSize;
  UINTN                           Index;
  EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER   *Rsdp;

  DEBUG ((EFI_D_INFO,
    "(TXT) *****************************************************************************\n"
    "(TXT) *         OS TO SINIT Data                                                  *\n"
    "(TXT) *****************************************************************************\n"
    ));

  DEBUG ((EFI_D_INFO, "(TXT) OsToSinitData: (%08x)\n", Data));
  DEBUG ((EFI_D_INFO, "(TXT)   Size                       - %016lx\n", *Data));
  OsToSinitData = (TXT_OS_TO_SINIT_DATA *)(Data + 1);
  DEBUG ((EFI_D_INFO, "(TXT)   Version                    - %08x\n", (UINTN)OsToSinitData->Version));
  if (OsToSinitData->Version >= TXT_OS_TO_SINIT_DATA_VERSION_7) {
    DEBUG ((EFI_D_INFO, "(TXT)   Flags                      - %08x\n", (UINTN)OsToSinitData->Flags));
  }
  DEBUG ((EFI_D_INFO, "(TXT)   MLEPageTableBase           - %016lx\n", OsToSinitData->MLEPageTableBase));
  DEBUG ((EFI_D_INFO, "(TXT)   MLESize                    - %016lx\n", OsToSinitData->MLESize));
  DEBUG ((EFI_D_INFO, "(TXT)   MLEHeaderBase              - %016lx\n", OsToSinitData->MLEHeaderBase));
  if (OsToSinitData->Version < TXT_OS_TO_SINIT_DATA_VERSION_3) {
    return ;
  }
  DEBUG ((EFI_D_INFO, "(TXT)   PMRLowBase                 - %016lx\n", OsToSinitData->PMRLowBase));
  DEBUG ((EFI_D_INFO, "(TXT)   PMRLowSize                 - %016lx\n", OsToSinitData->PMRLowSize));
  DEBUG ((EFI_D_INFO, "(TXT)   PMRHighBase                - %016lx\n", OsToSinitData->PMRHighBase));
  DEBUG ((EFI_D_INFO, "(TXT)   PMRHighSize                - %016lx\n", OsToSinitData->PMRHighSize));
  DEBUG ((EFI_D_INFO, "(TXT)   LCPPOBase                  - %016lx\n", OsToSinitData->LCPPOBase));
  DEBUG ((EFI_D_INFO, "(TXT)   LCPPOSize                  - %016lx\n", OsToSinitData->LCPPOSize));
  if (OsToSinitData->Version < TXT_OS_TO_SINIT_DATA_VERSION_4) {
    return ;
  }
  DEBUG ((EFI_D_INFO, "(TXT)   Capabilities               - %08x\n", (UINTN)OsToSinitData->Capabilities));
  for (Index = 0; Index < sizeof(mCapabilityStr)/sizeof(mCapabilityStr[0]); Index++) {
    if (mCapabilityStr[Index] == NULL) {
      continue;
    }
    DEBUG ((EFI_D_INFO,
      "(TXT)     %s- %08x\n",
      mCapabilityStr[Index],
      (UINTN)(OsToSinitData->Capabilities & (1 << Index))
      ));
  }
  if (OsToSinitData->Version < TXT_OS_TO_SINIT_DATA_VERSION_5) {
    return ;
  }
  DEBUG ((EFI_D_INFO, "(TXT)   RsdpPtr                    - %016lx\n", OsToSinitData->RsdpPtr));

  if (OsToSinitData->Version >= TXT_OS_TO_SINIT_DATA_VERSION_6) {
    TotalOsToSinitDataSize = (UINTN)*((UINT64 *)OsToSinitData - 1) - sizeof(UINT64);
    ExtDataElement = (TXT_HEAP_EXT_DATA_ELEMENT *)(OsToSinitData + 1);
    DumpExtDataElement (ExtDataElement, (UINTN)OsToSinitData + TotalOsToSinitDataSize - (UINTN)ExtDataElement);
  }

  DEBUG ((EFI_D_INFO,
    "(TXT) *****************************************************************************\n\n"
    ));

  Rsdp = FindAcpiRsdPtr ();
  DEBUG ((EFI_D_INFO, "(TXT) Uefi RsdpPtr - %016lx\n", Rsdp));
  DumpAcpiRSDP (Rsdp);
  DumpAcpiRSDT ((VOID *)(UINTN)Rsdp->RsdtAddress);
  DumpAcpiXSDT ((VOID *)(UINTN)Rsdp->XsdtAddress);

  return ;
}

//
// This table defines the TXT policy control string
//
GLOBAL_REMOVE_IF_UNREFERENCED CHAR16 *mPolicyControlStr[] = {
  L"Unsigned LCP to PCR17    ",
  L"Allow PreProduction      ",
  L"Capabilities to PCR17    ",
  L"PO LCP required          ",
};

//
// This table defines the TXT SinitMdr type string
//
GLOBAL_REMOVE_IF_UNREFERENCED CHAR16 *mSinitMdrTypeStr[] = {
  L"Usable    - Good memory",
  L"SMRAM     - Overlayed",
  L"SMRAM     - Non-Overlayed",
  L"PCIe      - PCIe Extended Config Region",
  L"Protected - Protected memory",
};

/**

  This function dump TXT SinitToMle data.

  @param  Data   TXT SinitToMle data

**/
VOID
DumpSinitToMleData (
  IN UINT64  *Data
  )
{
  TXT_SINIT_TO_MLE_DATA              *SinitToMleData;
  TXT_SINIT_MEMORY_DESCRIPTOR_RECORD *SinitMemoryDescriptor;
  UINT32                             Index;
  TXT_HEAP_EXT_DATA_ELEMENT          *ExtDataElement;
  UINTN                              TotalSinitToMleDataSize;

  DEBUG ((EFI_D_INFO,
    "(TXT) *****************************************************************************\n"
    "(TXT) *         SINIT TO MLE Data                                                 *\n"
    "(TXT) *****************************************************************************\n"
    ));

  DEBUG ((EFI_D_INFO, "(TXT) SinitToMleData: (%08x)\n", Data));
  DEBUG ((EFI_D_INFO, "(TXT)   Size                       - %016lx\n", *Data));
  SinitToMleData = (TXT_SINIT_TO_MLE_DATA *)(Data + 1);
  DEBUG ((EFI_D_INFO, "(TXT)   Version                    - %08x\n", (UINTN)SinitToMleData->Version));
  if (SinitToMleData->Version <= TXT_SINIT_TO_MLE_DATA_VERSION_8) {
    DEBUG ((EFI_D_INFO, "(TXT)   BiosAcmID                  - "));
    DumpData (SinitToMleData->BiosAcmID, sizeof(SinitToMleData->BiosAcmID));
    DEBUG ((EFI_D_INFO, "\n"));
    DEBUG ((EFI_D_INFO, "(TXT)   EdxSenterFlags             - %08x\n", (UINTN)SinitToMleData->EdxSenterFlags));
    DEBUG ((EFI_D_INFO, "(TXT)   MsegValid                  - %016lx\n", SinitToMleData->MsegValid));
    DEBUG ((EFI_D_INFO, "(TXT)   SinitHash                  - "));
    DumpData (SinitToMleData->SinitHash, sizeof(SinitToMleData->SinitHash));
    DEBUG ((EFI_D_INFO, "\n"));
    DEBUG ((EFI_D_INFO, "(TXT)   MleHash                    - "));
    DumpData (SinitToMleData->MleHash, sizeof(SinitToMleData->MleHash));
    DEBUG ((EFI_D_INFO, "\n"));
    DEBUG ((EFI_D_INFO, "(TXT)   StmHash                    - "));
    DumpData (SinitToMleData->StmHash, sizeof(SinitToMleData->StmHash));
    DEBUG ((EFI_D_INFO, "\n"));
    if (SinitToMleData->Version < TXT_SINIT_TO_MLE_DATA_VERSION_3) {
      return ;
    }
    DEBUG ((EFI_D_INFO, "(TXT)   LcpPolicyHash              - "));
    DumpData (SinitToMleData->LcpPolicyHash, sizeof(SinitToMleData->LcpPolicyHash));
    DEBUG ((EFI_D_INFO, "\n"));
    DEBUG ((EFI_D_INFO, "(TXT)   PolicyControl              - %08x\n", (UINTN)SinitToMleData->PolicyControl));
    for (Index = 0; Index < sizeof(mPolicyControlStr)/sizeof(mPolicyControlStr[0]); Index++) {
      if (mPolicyControlStr[Index] == NULL) {
        continue;
      }
      DEBUG ((EFI_D_INFO,
        "(TXT)     %s- %08x\n",
        mPolicyControlStr[Index],
        (UINTN)(SinitToMleData->PolicyControl & (1 << Index))
        ));
    }
  }

  if (SinitToMleData->Version >= TXT_SINIT_TO_MLE_DATA_VERSION_5) {
    DEBUG ((EFI_D_INFO, "(TXT)   RlpWakeupAddr              - %08x\n", (UINTN)SinitToMleData->RlpWakeupAddr));
  }
  DEBUG ((EFI_D_INFO, "(TXT)   NumberOfSinitMdrs          - %08x\n", (UINTN)SinitToMleData->NumberOfSinitMdrs));
  DEBUG ((EFI_D_INFO, "(TXT)   SinitMdrTableOffset        - %08x\n", (UINTN)SinitToMleData->SinitMdrTableOffset));
  DEBUG ((EFI_D_INFO, "(TXT)   SinitVtdDmarTableSize      - %08x\n", (UINTN)SinitToMleData->SinitVtdDmarTableSize));
  DEBUG ((EFI_D_INFO, "(TXT)   SinitVtdDmarTableOffset    - %08x\n", (UINTN)SinitToMleData->SinitVtdDmarTableOffset));
  if (SinitToMleData->Version >= TXT_SINIT_TO_MLE_DATA_VERSION_8) {
    DEBUG ((EFI_D_INFO, "(TXT)   ProcessorSCRTMStatus       - %08x\n", (UINTN)SinitToMleData->ProcessorSCRTMStatus));
  }
  SinitMemoryDescriptor = (TXT_SINIT_MEMORY_DESCRIPTOR_RECORD *)((UINTN)Data + SinitToMleData->SinitMdrTableOffset);
  for (Index = 0; Index < SinitToMleData->NumberOfSinitMdrs; Index++) {
    DEBUG ((EFI_D_INFO, "(TXT)   SinitMdr[%d]:\n", (UINTN)Index));
    DEBUG ((EFI_D_INFO, "(TXT)     Address                  - %016lx\n", SinitMemoryDescriptor[Index].Address));
    DEBUG ((EFI_D_INFO, "(TXT)     Length                   - %016lx\n", SinitMemoryDescriptor[Index].Length));
    DEBUG ((EFI_D_INFO, "(TXT)     Type                     - %02x\n", (UINTN)SinitMemoryDescriptor[Index].Type));
    if (SinitMemoryDescriptor[Index].Type < sizeof(mSinitMdrTypeStr)/sizeof(mSinitMdrTypeStr[0])) {
      DEBUG ((EFI_D_INFO, "(TXT)       %s\n", mSinitMdrTypeStr[SinitMemoryDescriptor[Index].Type]));
    }
  }

  if (SinitToMleData->Version >= TXT_SINIT_TO_MLE_DATA_VERSION_9) {
    TotalSinitToMleDataSize = (UINTN)*((UINT64 *)SinitToMleData - 1) - sizeof(UINT64);
    ExtDataElement = (TXT_HEAP_EXT_DATA_ELEMENT *)(SinitToMleData + 1);
    DumpExtDataElement(ExtDataElement, (UINTN)SinitToMleData + TotalSinitToMleDataSize - (UINTN)ExtDataElement);
  }

  DEBUG ((EFI_D_INFO,
    "(TXT) *****************************************************************************\n\n"
    ));

  if (SinitToMleData->SinitVtdDmarTableOffset != 0) {
    DumpDmar ((EFI_ACPI_DMAR_DESCRIPTION_TABLE *)((UINTN)Data + SinitToMleData->SinitVtdDmarTableOffset));
  }

  return ;
}

//
// This table defines the GETSEC capabilities string
//
GLOBAL_REMOVE_IF_UNREFERENCED CHAR16  *mCapabilitiesStr[] = {
  L"Chipset Present             ",
  NULL, // 1
  L"ENTERACCS                   ",
  L"EXITAC                      ",
  L"SENTER                      ",
  L"SEXIT                       ",
  L"PARAMETERS                  ",
  L"SMCTRL                      ",
  L"WAKEUP                      ",
  NULL, // 9
  NULL, // 10
  NULL, // 11
  NULL, // 12
  NULL, // 13
  NULL, // 14
  NULL, // 15
  NULL, // 16
  NULL, // 17
  NULL, // 18
  NULL, // 19
  NULL, // 20
  NULL, // 21
  NULL, // 22
  NULL, // 23
  NULL, // 24
  NULL, // 25
  NULL, // 26
  NULL, // 27
  NULL, // 28
  NULL, // 29
  NULL, // 30
  L"Extended Leafs              ",
};

/**

  This function dump GETSEC capabilities.

  @param  Index          GETSEC index
  @param  Capabilities   GETSEC capabilities

**/
VOID
DumpGetSecCapabilities (
  IN UINT32   Index,
  IN UINT32   Capabilities
  )
{
  DEBUG ((EFI_D_INFO, "(TXT) GETSEC Capabilities (%08x) - 0x%08x\n", (UINTN)Index, (UINTN)Capabilities));
  if (Index == 0) {
    // Capability 0
    for (Index = 0; Index < sizeof(mCapabilitiesStr)/sizeof(mCapabilitiesStr[0]); Index++) {
      if (mCapabilitiesStr[Index] == NULL) {
        continue;
      }
      DEBUG ((EFI_D_INFO,
        "(TXT)   %s - 0x%08x\n",
        mCapabilitiesStr[Index],
        (UINTN)(Capabilities & (1 << Index))
        ));
    }
  }

  return ;
}

//
// This table defines the GETSEC parameter string
//
GLOBAL_REMOVE_IF_UNREFERENCED CHAR16  *mParameterStr[] = {
  NULL, // 0
  L"Supported AC module versions",
  L"Max size of authenticated code execution area",
  L"External memory types supported during AC mode",
  L"Selective SENTER functionality control",
  L"TXT extensions support",
};

//
// This table defines the GETSEC extern memory type string
//
GLOBAL_REMOVE_IF_UNREFERENCED CHAR16  *mParameterExternMemTypeStr[] = {
  NULL, // 0
  NULL, // 1
  NULL, // 2
  NULL, // 3
  NULL, // 4
  NULL, // 5
  NULL, // 6
  NULL, // 7
  L"Uncacheable             ",
  L"Write Combining         ",
  NULL, // 10
  NULL, // 11
  L"Write-through           ",
  L"Write-protected         ",
  L"Write-back              ",
};

//
// This table defines the GETSEC senter disable control string
//
GLOBAL_REMOVE_IF_UNREFERENCED CHAR16  *mParameterSenterDisableControlStr[] = {
  NULL, // 0
  NULL, // 1
  NULL, // 2
  NULL, // 3
  NULL, // 4
  NULL, // 5
  NULL, // 6
  NULL, // 7
  L"CAPABILITIES            ",
  NULL, // 9
  L"ENTERACCS               ",
  L"EXITAC                  ",
  L"SENTER                  ",
  L"SEXIT                   ",
  L"PARAMETERS              ",
  L"SMCTRL                  ",
  L"WAKEUP                  ",
};

//
// This table defines the GETSEC extensions string
//
GLOBAL_REMOVE_IF_UNREFERENCED CHAR16  *mParameterTxtExtensionsStr[] = {
  NULL, // 0
  NULL, // 1
  NULL, // 2
  NULL, // 3
  NULL, // 4
  L"Processor CRTM support  ",
  L"Machine Check Handling  ",
};

/**

  This function dump GETSEC parameters.

  @param  RegEax          GETSEC parameters RegEax
  @param  RegEbx          GETSEC parameters RegEbx
  @param  RegEcx          GETSEC parameters RegEcx

**/
VOID
DumpGetSecParameters (
  IN UINT32   RegEax,
  IN UINT32   RegEbx,
  IN UINT32   RegEcx
  )
{
  UINT32   Index;
  UINT32   EaxValue;

  Index = RegEax & GETSEC_PARAMETER_TYPE_MASK;
  EaxValue = RegEax & ~GETSEC_PARAMETER_TYPE_MASK;
  DEBUG ((EFI_D_INFO,
    "(TXT) GETSEC Parameter    (%08x) - 0x%08x, 0x%08x, 0x%08x\n",
    (UINTN)Index,
    (UINTN)EaxValue,
    (UINTN)RegEbx,
    (UINTN)RegEcx
    ));
  if (Index < sizeof(mParameterStr)/sizeof(mParameterStr[0])) {
    if (mParameterStr[Index] != NULL) {
      DEBUG ((EFI_D_INFO,
        "(TXT)   %s\n",
        mParameterStr[Index]
        ));
    }
  }

  switch (Index) {
  case GETSEC_PARAMETER_TYPE_ACM_VERSION:
    DEBUG ((EFI_D_INFO, "(TXT)     Version comparison mask    - 0x%08x\n", (UINTN)RegEbx));
    DEBUG ((EFI_D_INFO, "(TXT)     Version numbers supported  - 0x%08x\n", (UINTN)RegEcx));
    break;
  case GETSEC_PARAMETER_TYPE_ACM_MAX_SIZE:
    DEBUG ((EFI_D_INFO, "(TXT)     AC execution region size   - 0x%08x\n", (UINTN)EaxValue));
    break;
  case GETSEC_PARAMETER_TYPE_EXTERN_MEM_TYPE:
    DEBUG ((EFI_D_INFO, "(TXT)     Memory type bit mask       - 0x%08x\n", (UINTN)EaxValue));
    for (Index = 0; Index < sizeof(mParameterExternMemTypeStr)/sizeof(mParameterExternMemTypeStr[0]); Index++) {
      if (mParameterExternMemTypeStr[Index] == NULL) {
        continue;
      }
      DEBUG ((EFI_D_INFO,
        "(TXT)       %s - 0x%08x\n",
        mParameterExternMemTypeStr[Index],
        (UINTN)(EaxValue & (1 << Index))
        ));
    }
    break;
  case GETSEC_PARAMETER_TYPE_SENTER_DIS_CONTOL:
    DEBUG ((EFI_D_INFO, "(TXT)     SENTER disable controls    - 0x%08x\n", (UINTN)EaxValue));
#if 0
    for (Index = 0; Index < sizeof(mParameterSenterDisableControlStr)/sizeof(mParameterSenterDisableControlStr[0]); Index++) {
      if (mParameterSenterDisableControlStr[Index] == NULL) {
        continue;
      }
      DEBUG ((EFI_D_INFO,
        "(TXT)       %s - 0x%08x\n",
        mParameterSenterDisableControlStr[Index],
        (UINTN)(EaxValue & (1 << Index))
        ));
    }
#endif
    break;
  case GETSEC_PARAMETER_TYPE_EXTERNSION:
    DEBUG ((EFI_D_INFO, "(TXT)     TXT Extensions Flags       - 0x%08x\n", (UINTN)EaxValue));
    for (Index = 0; Index < sizeof(mParameterTxtExtensionsStr)/sizeof(mParameterTxtExtensionsStr[0]); Index++) {
      if (mParameterTxtExtensionsStr[Index] == NULL) {
        continue;
      }
      DEBUG ((EFI_D_INFO,
        "(TXT)       %s - 0x%08x\n",
        mParameterTxtExtensionsStr[Index],
        (UINTN)(EaxValue & (1 << Index))
        ));
    }
    break;
  default:
    break;
  }

  return ;
}

GLOBAL_REMOVE_IF_UNREFERENCED CHAR16  *mEventTypeName[] = {
  NULL,
  NULL,
  NULL,
  L"NO_ACTION",
  L"SEPARATOR",
};

GLOBAL_REMOVE_IF_UNREFERENCED CHAR16  *mTxtEventTypeName[] = {
  NULL,
  L"PCRMAPPING",
  L"HASH_START",
  L"COMBINED_HASH",
  L"MLE_HASH",
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  L"BIOSAC_REG_DATA",
  L"CPU_SCRTM_STAT",
  L"LCP_CONTROL_HASH",
  L"ELEMENTS_HASH",
  L"STM_HASH",
  L"OSSINITDATA_CAP_HASH",
  L"SINIT_PUBKEY_HASH",
  L"LCP_HASH",
  L"LCP_DETAILS_HASH",
  L"LCP_AUTHORITIES_HASH",
  L"NV_INFO_HASH",
};

/**

  This function conver TCG Event Type to string.

  @param  EventType          TCG Event Type

  @return  TCG Event Type string

**/
CHAR16 *
EventTypeToString(
  IN UINT32  EventType
  )
{
  if ((EventType >= TXT_EVTYPE_BASE) &&
      (EventType < TXT_EVTYPE_BASE + sizeof(mTxtEventTypeName)/sizeof(mTxtEventTypeName[0]))) {
    return mTxtEventTypeName[EventType - TXT_EVTYPE_BASE];
  } else if (EventType == TXT_EVTYPE_CAP_VALUE) {
    return L"CAP_VALUE";
  } else {
    if (EventType < sizeof(mEventTypeName) / sizeof(mEventTypeName[0])) {
      return mEventTypeName[EventType];
    }
    return L"Unknown";
  }
}

/**

  This function dump TPM2 event log entry.

  @param  PcrEvent          PCR event
  @param  DigestSize        Digest size

**/
VOID
DumpTpm2EventLogEntry (
  IN TCG_PCR_EVENT_EX  *PcrEvent,
  IN UINT32            DigestSize
  )
{
  UINT8   *Buffer;
  UINT32  EventDataSize;

  DEBUG((EFI_D_INFO, "(TXT) Event: (0x%x)\n", PcrEvent));
  DEBUG((EFI_D_INFO, "(TXT)   PCRIndex  - 0x%08x\n", PcrEvent->PCRIndex));
  DEBUG((EFI_D_INFO, "(TXT)   EventType - 0x%08x (%s)\n", PcrEvent->EventType, EventTypeToString(PcrEvent->EventType)));
  DEBUG((EFI_D_INFO, "(TXT)   Digest    - "));
  Buffer = (UINT8 *)(PcrEvent + 1);
  DumpData(Buffer, DigestSize);
  Buffer += DigestSize;
  DEBUG((EFI_D_INFO, "\n"));
  EventDataSize = ReadUnaligned32((UINT32 *)Buffer);
  DEBUG((EFI_D_INFO, "(TXT)   EventSize - 0x%08x\n", EventDataSize));
  Buffer += sizeof(UINT32);
  DEBUG((EFI_D_INFO, "(TXT)   EventData - "));
  DumpData(Buffer, EventDataSize);
  Buffer += EventDataSize;
  DEBUG((EFI_D_INFO, "\n"));
}

/**

  This function dump TPM2 TCG log descriptor.

  @param  TcgLogDesc          TPM2 TCG log descriptor

**/
VOID
DumpTpm2TcgLogDesc(
  IN TCG_LOG_DESCRIPTOR   *TcgLogDesc
  )
{
  UINTN  Index;

  DEBUG((EFI_D_INFO, "(TXT) TcgLogDescriptor: (0x%x)\n", TcgLogDesc));
  DEBUG((EFI_D_INFO, "(TXT)   Signature     - '", TcgLogDesc->Signature));
  for (Index = 0; Index < sizeof(TcgLogDesc->Signature); Index++) {
    DEBUG((EFI_D_INFO, "%c", TcgLogDesc->Signature[Index]));
  }
  DEBUG((EFI_D_INFO, "'\n"));
  DEBUG((EFI_D_INFO, "(TXT)   Revision      - 0x%08x\n", TcgLogDesc->Revision));
  DEBUG((EFI_D_INFO, "(TXT)   DigestAlgID   - 0x%08x\n", TcgLogDesc->DigestAlgID));
  DEBUG((EFI_D_INFO, "(TXT)   DigestSize    - 0x%08x\n", TcgLogDesc->DigestSize));
}

/**

  This function dump TPM2 Event log.

  @param  EventLog          TPM2 event log
  @param  MaxSize           TPM2 event log max size

**/
VOID
DumpTpm2EventLog(
  IN VOID   *EventLog,
  IN UINTN  MaxSize
  )
{
  TCG_LOG_DESCRIPTOR                    *TcgLogDesc;
  TCG_PCR_EVENT_EX                      *PcrEvent;
  UINT8                                 *Buffer;
  UINT32                                EventDataSize;

  PcrEvent = (TCG_PCR_EVENT_EX *)EventLog;
  DumpTpm2EventLogEntry ((TCG_PCR_EVENT_EX *)PcrEvent, SHA1_DIGEST_SIZE);
  TcgLogDesc = (TCG_LOG_DESCRIPTOR *)((UINT8 *)PcrEvent + sizeof(TCG_PCR_EVENT_HDR));
  DumpTpm2TcgLogDesc(TcgLogDesc);

  PcrEvent = (TCG_PCR_EVENT_EX *)(TcgLogDesc + 1);
  while (PcrEvent->PCRIndex != 0 || PcrEvent->EventType != 0) {
    DumpTpm2EventLogEntry((TCG_PCR_EVENT_EX *)PcrEvent, TcgLogDesc->DigestSize);
    Buffer = (UINT8 *)(PcrEvent + 1);
    Buffer += TcgLogDesc->DigestSize;
    EventDataSize = ReadUnaligned32((UINT32 *)Buffer);
    Buffer += sizeof(UINT32) + EventDataSize;
    PcrEvent = (TCG_PCR_EVENT_EX *)(Buffer);
    if ((UINTN)PcrEvent >= (UINTN)EventLog + MaxSize) {
      break;
    }
  }
}

/**
  This function dump PCR event.

  @param[in]  EventHdr     TCG PCR event structure.
**/
VOID
DumpEvent (
  IN TCG_PCR_EVENT_HDR         *EventHdr
  )
{
  UINTN                     Index;

  DEBUG ((EFI_D_INFO, "(TXT) Event: (0x%x)\n", EventHdr));
  DEBUG ((EFI_D_INFO, "(TXT)   PCRIndex  - %d\n", EventHdr->PCRIndex));
  DEBUG ((EFI_D_INFO, "(TXT)   EventType - 0x%08x (%s)\n", EventHdr->EventType, EventTypeToString(EventHdr->EventType)));
  DEBUG ((EFI_D_INFO, "(TXT)   Digest    - "));
  for (Index = 0; Index < sizeof(TCG_DIGEST); Index++) {
    DEBUG ((EFI_D_INFO, "%02x ", EventHdr->Digest.digest[Index]));
  }
  DEBUG ((EFI_D_INFO, "\n"));
  DEBUG ((EFI_D_INFO, "(TXT)   EventSize - 0x%08x\n", EventHdr->EventSize));
  DEBUG ((EFI_D_INFO, "(TXT)   EventData - "));
  DumpData((UINT8 *)(EventHdr + 1), EventHdr->EventSize);
  DEBUG((EFI_D_INFO, "\n"));
}

/**
  This function dump TCG_EfiSpecIDEventStruct.

  @param[in]  TcgEfiSpecIdEventStruct     A pointer to TCG_EfiSpecIDEventStruct.
**/
VOID
DumpTcgEfiSpecIdEventStruct (
  IN TCG_EfiSpecIDEventStruct   *TcgEfiSpecIdEventStruct
  )
{
  TCG_EfiSpecIdEventAlgorithmSize  *DigestSize;
  UINTN                            Index;
  UINT8                            *VendorInfoSize;
  UINT8                            *VendorInfo;
  UINT32                           NumberOfAlgorithms;

  DEBUG ((EFI_D_INFO, "(TXT) TCG_EfiSpecIDEventStruct: (0x%x)\n", TcgEfiSpecIdEventStruct));
  DEBUG ((EFI_D_INFO, "(TXT)   signature          - '"));
  for (Index = 0; Index < sizeof(TcgEfiSpecIdEventStruct->signature); Index++) {
    DEBUG ((EFI_D_INFO, "%c", TcgEfiSpecIdEventStruct->signature[Index]));
  }
  DEBUG ((EFI_D_INFO, "'\n"));
  DEBUG ((EFI_D_INFO, "(TXT)   platformClass      - 0x%08x\n", TcgEfiSpecIdEventStruct->platformClass));
  DEBUG ((EFI_D_INFO, "(TXT)   specVersion        - %d.%d%d\n", TcgEfiSpecIdEventStruct->specVersionMajor, TcgEfiSpecIdEventStruct->specVersionMinor, TcgEfiSpecIdEventStruct->specErrata));
  DEBUG ((EFI_D_INFO, "(TXT)   uintnSize          - 0x%02x\n", TcgEfiSpecIdEventStruct->uintnSize));

  CopyMem (&NumberOfAlgorithms, TcgEfiSpecIdEventStruct + 1, sizeof(NumberOfAlgorithms));
  DEBUG ((EFI_D_INFO, "(TXT)   NumberOfAlgorithms - 0x%08x\n", NumberOfAlgorithms));

  DigestSize = (TCG_EfiSpecIdEventAlgorithmSize *)((UINT8 *)TcgEfiSpecIdEventStruct + sizeof(*TcgEfiSpecIdEventStruct) + sizeof(NumberOfAlgorithms));
  for (Index = 0; Index < NumberOfAlgorithms; Index++) {
    DEBUG ((EFI_D_INFO, "(TXT)   digest(%d)\n", Index));
    DEBUG ((EFI_D_INFO, "(TXT)     algorithmId      - 0x%04x\n", DigestSize[Index].algorithmId));
    DEBUG ((EFI_D_INFO, "(TXT)     digestSize       - 0x%04x\n", DigestSize[Index].digestSize));
  }
  VendorInfoSize = (UINT8 *)&DigestSize[NumberOfAlgorithms];
  DEBUG ((EFI_D_INFO, "(TXT)   VendorInfoSize     - 0x%02x\n", *VendorInfoSize));
  VendorInfo = VendorInfoSize + 1;
  DEBUG ((EFI_D_INFO, "(TXT)   VendorInfo         - "));
  for (Index = 0; Index < *VendorInfoSize; Index++) {
    DEBUG ((EFI_D_INFO, "%02x ", VendorInfo[Index]));
  }
  DEBUG ((EFI_D_INFO, "\n"));
}

/**
  This function get size of TCG_EfiSpecIDEventStruct.

  @param[in]  TcgEfiSpecIdEventStruct     A pointer to TCG_EfiSpecIDEventStruct.
**/
UINTN
GetTcgEfiSpecIdEventStructSize (
  IN TCG_EfiSpecIDEventStruct   *TcgEfiSpecIdEventStruct
  )
{
  TCG_EfiSpecIdEventAlgorithmSize  *DigestSize;
  UINT8                            *VendorInfoSize;
  UINT32                           NumberOfAlgorithms;

  CopyMem (&NumberOfAlgorithms, TcgEfiSpecIdEventStruct + 1, sizeof(NumberOfAlgorithms));

  DigestSize = (TCG_EfiSpecIdEventAlgorithmSize *)((UINT8 *)TcgEfiSpecIdEventStruct + sizeof(*TcgEfiSpecIdEventStruct) + sizeof(NumberOfAlgorithms));
  VendorInfoSize = (UINT8 *)&DigestSize[NumberOfAlgorithms];
  return sizeof(TCG_EfiSpecIDEventStruct) + sizeof(UINT32) + (NumberOfAlgorithms * sizeof(TCG_EfiSpecIdEventAlgorithmSize)) + sizeof(UINT8) + (*VendorInfoSize);
}

/**
  This function dump PCR event 2.

  @param[in]  TcgPcrEvent2     TCG PCR event 2 structure.
**/
VOID
DumpEvent2 (
  IN TCG_PCR_EVENT2        *TcgPcrEvent2
  )
{
  UINTN                     Index;
  UINT32                    DigestIndex;
  UINT32                    DigestCount;
  TPMI_ALG_HASH             HashAlgo;
  UINT32                    DigestSize;
  UINT8                     *DigestBuffer;
  UINT32                    EventSize;
  UINT8                     *EventBuffer;

  DEBUG ((EFI_D_INFO, "(TXT) Event: (0x%x)\n", TcgPcrEvent2));
  DEBUG ((EFI_D_INFO, "(TXT)   PCRIndex  - %d\n", TcgPcrEvent2->PCRIndex));
  DEBUG ((EFI_D_INFO, "(TXT)   EventType     - 0x%08x (%s)\n", TcgPcrEvent2->EventType, EventTypeToString(TcgPcrEvent2->EventType)));
  DEBUG ((EFI_D_INFO, "(TXT)   DigestCount: 0x%08x\n", TcgPcrEvent2->Digest.count));

  DigestCount = TcgPcrEvent2->Digest.count;
  HashAlgo = TcgPcrEvent2->Digest.digests[0].hashAlg;
  DigestBuffer = (UINT8 *)&TcgPcrEvent2->Digest.digests[0].digest;
  for (DigestIndex = 0; DigestIndex < DigestCount; DigestIndex++) {
    DEBUG ((EFI_D_INFO, "(TXT)     HashAlgo : 0x%04x\n", HashAlgo));
    DEBUG ((EFI_D_INFO, "(TXT)     Digest(%d): ", DigestIndex));
    DigestSize = GetHashSizeFromAlgo (HashAlgo);
    for (Index = 0; Index < DigestSize; Index++) {
      DEBUG ((EFI_D_INFO, "%02x ", DigestBuffer[Index]));
    }
    DEBUG ((EFI_D_INFO, "\n"));
    //
    // Prepare next
    //
    CopyMem (&HashAlgo, DigestBuffer + DigestSize, sizeof(TPMI_ALG_HASH));
    DigestBuffer = DigestBuffer + DigestSize + sizeof(TPMI_ALG_HASH);
  }
  DEBUG ((EFI_D_INFO, "\n"));
  DigestBuffer = DigestBuffer - sizeof(TPMI_ALG_HASH);

  CopyMem (&EventSize, DigestBuffer, sizeof(TcgPcrEvent2->EventSize));
  DEBUG ((EFI_D_INFO, "(TXT)   EventSize - 0x%08x\n", EventSize));
  EventBuffer = DigestBuffer + sizeof(TcgPcrEvent2->EventSize);
  DEBUG ((EFI_D_INFO, "(TXT)   EventData - "));
  DumpData(EventBuffer, EventSize);
  DEBUG ((EFI_D_INFO, "\n"));
}

/**
  This function returns size of TCG PCR event 2.
  
  @param[in]  TcgPcrEvent2     TCG PCR event 2 structure.

  @return size of TCG PCR event 2.
**/
UINTN
GetPcrEvent2Size (
  IN TCG_PCR_EVENT2        *TcgPcrEvent2
  )
{
  UINT32                    DigestIndex;
  UINT32                    DigestCount;
  TPMI_ALG_HASH             HashAlgo;
  UINT32                    DigestSize;
  UINT8                     *DigestBuffer;
  UINT32                    EventSize;
  UINT8                     *EventBuffer;

  DigestCount = TcgPcrEvent2->Digest.count;
  HashAlgo = TcgPcrEvent2->Digest.digests[0].hashAlg;
  DigestBuffer = (UINT8 *)&TcgPcrEvent2->Digest.digests[0].digest;
  for (DigestIndex = 0; DigestIndex < DigestCount; DigestIndex++) {
    DigestSize = GetHashSizeFromAlgo (HashAlgo);
    //
    // Prepare next
    //
    CopyMem (&HashAlgo, DigestBuffer + DigestSize, sizeof(TPMI_ALG_HASH));
    DigestBuffer = DigestBuffer + DigestSize + sizeof(TPMI_ALG_HASH);
  }
  DigestBuffer = DigestBuffer - sizeof(TPMI_ALG_HASH);

  CopyMem (&EventSize, DigestBuffer, sizeof(TcgPcrEvent2->EventSize));
  EventBuffer = DigestBuffer + sizeof(TcgPcrEvent2->EventSize);

  return (UINTN)EventBuffer + EventSize - (UINTN)TcgPcrEvent2;
}

/**

  This function dump TPM2 TCG2 Event log.

  @param  EventLog          TPM2 TCG2 event log
  @param  MaxSize           TPM2 TCG2 event log max size

**/
VOID
DumpTpm2Tcg2EventLog(
  IN VOID   *EventLog,
  IN UINTN  MaxSize
  )
{
  TCG_PCR_EVENT_HDR         *EventHdr;
  TCG_PCR_EVENT2            *TcgPcrEvent2;
  TCG_EfiSpecIDEventStruct  *TcgEfiSpecIdEventStruct;

  //
  // Dump first event	
  //
  EventHdr = (TCG_PCR_EVENT_HDR *)(UINTN)EventLog;
  DumpEvent (EventHdr);

  TcgEfiSpecIdEventStruct = (TCG_EfiSpecIDEventStruct *)(EventHdr + 1);
  DumpTcgEfiSpecIdEventStruct (TcgEfiSpecIdEventStruct);

  TcgPcrEvent2 = (TCG_PCR_EVENT2 *)((UINTN)TcgEfiSpecIdEventStruct + GetTcgEfiSpecIdEventStructSize (TcgEfiSpecIdEventStruct));

  while (TcgPcrEvent2->PCRIndex != 0 || TcgPcrEvent2->EventType != 0) {
    DumpEvent2 (TcgPcrEvent2);
    TcgPcrEvent2 = (TCG_PCR_EVENT2 *)((UINTN)TcgPcrEvent2 + GetPcrEvent2Size (TcgPcrEvent2));
    if ((UINTN)TcgPcrEvent2 >= (UINTN)EventLog + MaxSize) {
      break;
    }
  }
}

/**

  This function dump TPM12 Event log.

  @param  EventLog          TPM12 event log
  @param  MaxSize           TPM12 event log max size

**/
VOID
DumpTpm12EventLog(
  IN VOID   *EventLog,
  IN UINTN  MaxSize
  )
{
  TXT_EVENT_LOG_CONTAINER   *EventLogContainer;
  TCG_PCR_EVENT_HDR         *EventHdr;
  UINTN                     Index;

  EventLogContainer = (TXT_EVENT_LOG_CONTAINER *)(UINTN)EventLog;
  DEBUG((EFI_D_INFO, "(TXT) EventLogContainer:\n"));
  DEBUG((EFI_D_INFO, "(TXT)   Signature            - '"));
  for (Index = 0; Index < sizeof(EventLogContainer->Signature); Index++) {
    DEBUG((EFI_D_INFO, "%c", EventLogContainer->Signature[Index]));
  }
  DEBUG((EFI_D_INFO, "'\n"));
  DEBUG((EFI_D_INFO, "(TXT)   ContainerVersion     - %02x.%02x\n", (UINTN)EventLogContainer->ContainerVersionMajor, (UINTN)EventLogContainer->ContainerVersionMinor));
  DEBUG((EFI_D_INFO, "(TXT)   PcrEventVersion      - %02x.%02x\n", (UINTN)EventLogContainer->PcrEventVersionMajor, (UINTN)EventLogContainer->PcrEventVersionMinor));
  DEBUG((EFI_D_INFO, "(TXT)   Size                 - %08x\n", (UINTN)EventLogContainer->Size));
  DEBUG((EFI_D_INFO, "(TXT)   PcrEventsOffset      - %08x\n", (UINTN)EventLogContainer->PcrEventsOffset));
  DEBUG((EFI_D_INFO, "(TXT)   NextEventOffset      - %08x\n", (UINTN)EventLogContainer->NextEventOffset));

  EventHdr = (TCG_PCR_EVENT_HDR *)(EventLogContainer + 1);
  while (EventHdr->PCRIndex != 0 || EventHdr->EventType != 0) {
    DumpEvent(EventHdr);
    EventHdr = (TCG_PCR_EVENT_HDR *)((UINTN)EventHdr + EventHdr->EventSize);
    if ((UINTN)EventHdr >= (UINTN)EventLog + MaxSize) {
      break;
    }
  }
}

/**

  This function dump TPM event log buffer.

**/
VOID
DumpTpmEventLogBuffer(
  VOID
  )
{
  MLE_PRIVATE_DATA                      *MlePrivateData;
  DCE_PRIVATE_DATA                      *DcePrivateData;
  TXT_EVENT_LOG_CONTAINER               *EventLog;
  UINTN                                 Index;
  TXT_HEAP_EVENTLOG_EXT_ELEMENT         *EventLogElement;
  TXT_HEAP_EVENT_LOG_POINTER_ELEMENT2   *EventLogPointerElement2;
  TXT_HEAP_EVENT_LOG_POINTER_ELEMENT2_1 *EventLogPointerElement2_1;
  TXT_HEAP_EVENT_LOG_DESCR              *EventLogDesc;

  MlePrivateData = GetMlePrivateData();
  DcePrivateData = &MlePrivateData->DcePrivateData;
  
  if (DcePrivateData->TpmType == FRM_TPM_TYPE_TPM12) {
    DEBUG((EFI_D_INFO, "(TXT) DumpEventLog (TPM1.2):\n"));
    EventLogElement = DcePrivateData->EventLogElement;
    EventLog = (TXT_EVENT_LOG_CONTAINER *)(UINTN)EventLogElement->EventLogAddress;
    DumpTpm12EventLog((UINT8 *)(UINTN)EventLogElement->EventLogAddress, EventLog->Size);
  }

  if (DcePrivateData->TpmType == FRM_TPM_TYPE_TPM2) {
    if ((DcePrivateData->AcmCapabilities & TXT_MLE_SINIT_CAPABILITY_TCG2_COMPATIBILE_EVENTLOG) == 0) {
      DEBUG((EFI_D_INFO, "(TXT) DumpEventLog (TPM2.0):\n"));
      EventLogPointerElement2 = DcePrivateData->EventLogPointerElement2;
      EventLogDesc = (TXT_HEAP_EVENT_LOG_DESCR *)(EventLogPointerElement2 + 1);
      for (Index = 0; Index < EventLogPointerElement2->Count; Index++, EventLogDesc++) {
        DEBUG((EFI_D_INFO, "(TXT) DumpEventLog (TPM2.0) (Algo - 0x%x):\n", EventLogDesc->HashAlgID));
        DEBUG((EFI_D_INFO, "(TXT)   FirstRecordOffset - 0x%x\n", EventLogDesc->FirstRecordOffset));
        DEBUG((EFI_D_INFO, "(TXT)   NextRecordOffset  - 0x%x\n", EventLogDesc->NextRecordOffset));
        DumpTpm2EventLog((UINT8 *)(UINTN)EventLogDesc->PhysicalAddress, EventLogDesc->AllocatedEventContainerSize);
      }
    } else {
      DEBUG((EFI_D_INFO, "(TXT) DumpEventLog (TPM2.0):\n"));
      EventLogPointerElement2_1 = DcePrivateData->EventLogPointerElement2_1;
      DEBUG((EFI_D_INFO, "(TXT)   FirstRecordOffset - 0x%x\n", EventLogPointerElement2_1->FirstRecordOffset));
      DEBUG((EFI_D_INFO, "(TXT)   NextRecordOffset  - 0x%x\n", EventLogPointerElement2_1->NextRecordOffset));
      DumpTpm2Tcg2EventLog((UINT8 *)(UINTN)EventLogPointerElement2_1->PhysicalAddress, EventLogPointerElement2_1->AllocatedEventContainerSize);
    }
  }
}