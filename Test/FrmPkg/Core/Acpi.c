/** @file

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Base.h>
#include <Library\BaseLib.h>
#include <Library\IoLib.h>
#include <Library\DebugLib.h>
#include <IndustryStandard\Acpi.h>
#include <IndustryStandard\MemoryMappedConfigurationSpaceAccessTable.h>
#include "Frm.h"

extern FRM_COMMUNICATION_DATA    mCommunicationData;

/**

  This function find ACPI RSDPTR.

  @return ACPI RSDPTR

**/
VOID *
FindAcpiRsdPtr(
  VOID
  )
{
  return (VOID *)(UINTN)mCommunicationData.AcpiRsdp;
}

/**
  Get ACPI table via signature.

  @param Signature ACPI Table signature.

  @return ACPI table pointer.
**/
VOID *
GetAcpiTableViaSignature (
  IN UINT32 Signature
  )
{
  EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER  *Rsdp;
  EFI_ACPI_DESCRIPTION_HEADER                   *Rsdt;
  EFI_ACPI_DESCRIPTION_HEADER                   *Xsdt;
  UINTN                                         Index;
  UINTN                                         TableCount;
  EFI_ACPI_DESCRIPTION_HEADER                   *Table;
  UINT32                                        *EntryPtr;
  UINT64                                        *Entry64Ptr;
  UINT64                                        TempEntry;

  Rsdp = (EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER *)(UINTN)mCommunicationData.AcpiRsdp;
  if (Rsdp == NULL) {
    return NULL;
  }

  Xsdt = NULL;
  if (Rsdp->Revision >= 2) {
    Xsdt = (EFI_ACPI_DESCRIPTION_HEADER *) (UINTN) Rsdp->XsdtAddress;
  }
  if (Xsdt != NULL) {
    TableCount = (Xsdt->Length - sizeof(EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT64);
    Entry64Ptr = (UINT64 *)((UINTN)Xsdt + sizeof(EFI_ACPI_DESCRIPTION_HEADER));
    for (Index = 0; Index < TableCount; Index++, Entry64Ptr++) {
      CopyMem(&TempEntry, Entry64Ptr, sizeof(UINT64));
      Table = (EFI_ACPI_DESCRIPTION_HEADER *)(UINTN)TempEntry;
      if (Table->Signature == Signature) {
        return Table;
      }
    }
  }

  Rsdt = (EFI_ACPI_DESCRIPTION_HEADER *) (UINTN) Rsdp->RsdtAddress;
  TableCount = (Rsdt->Length - sizeof(EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT32);
  EntryPtr = (UINT32 *)((UINTN)Rsdt + sizeof(EFI_ACPI_DESCRIPTION_HEADER));
  for (Index = 0; Index < TableCount; Index++, EntryPtr++) {
    Table = (EFI_ACPI_DESCRIPTION_HEADER *)(UINTN)*EntryPtr;
    if (Table->Signature == Signature) {
      return Table;
    }
  }

  return NULL;
}

/**
  Get MADT ACPI table.

  @return MADT pointer.
**/
VOID *
GetMadt (
  VOID
  )
{
  return GetAcpiTableViaSignature (EFI_ACPI_1_0_APIC_SIGNATURE);
}

/**

  This function return CPU number from MADT.

  @return CPU number

**/
UINT32
GetCpuNumFromAcpi (
  VOID
  )
{
  UINT32                                               Index;
  EFI_ACPI_2_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER  *Madt;
  UINTN                                                Length;
  EFI_ACPI_2_0_PROCESSOR_LOCAL_APIC_STRUCTURE          *LocalApic;

  Madt = GetMadt ();
  if (Madt == NULL) {
    DEBUG ((EFI_D_INFO, "(FRM) CpuNumber - 1\n"));
    return 1;
  }

  Index = 0;
  Length = Madt->Header.Length;
  LocalApic = (EFI_ACPI_2_0_PROCESSOR_LOCAL_APIC_STRUCTURE *)(Madt + 1);
  while ((UINTN)LocalApic < (UINTN)Madt + Length) {
    if (LocalApic->Type == EFI_ACPI_2_0_PROCESSOR_LOCAL_APIC) {
      if ((LocalApic->Flags & EFI_ACPI_2_0_LOCAL_APIC_ENABLED) != 0) {
        Index++;
      }
    } else if (LocalApic->Type == EFI_ACPI_4_0_PROCESSOR_LOCAL_X2APIC) {
      if ((((EFI_ACPI_4_0_PROCESSOR_LOCAL_X2APIC_STRUCTURE *)LocalApic)->Flags & EFI_ACPI_4_0_LOCAL_APIC_ENABLED) != 0) {
        Index++;
      }
    }
    LocalApic = (EFI_ACPI_2_0_PROCESSOR_LOCAL_APIC_STRUCTURE *)((UINTN)LocalApic + LocalApic->Length);
  }

  DEBUG ((EFI_D_INFO, "(FRM) CpuNumber - %d\n", Index));
  return Index;
}

/**

  This function return APIC ID list from MADT.

  @param ApicIdList An array for APIC ID

**/
VOID
GetApicIdListFromAcpi (
  IN OUT UINT32  *ApicIdList
  )
{
  UINT32                                               Index;
  EFI_ACPI_2_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER  *Madt;
  UINTN                                                Length;
  EFI_ACPI_2_0_PROCESSOR_LOCAL_APIC_STRUCTURE          *LocalApic;

  Madt = GetMadt ();
  if (Madt == NULL) {
    *ApicIdList = ReadLocalApicId ();
    DEBUG ((EFI_D_INFO, "(FRM) ApicIdList(1) - %08x\n", *ApicIdList));
    return ;
  }

  Index = 0;
  Length = Madt->Header.Length;
  LocalApic = (EFI_ACPI_2_0_PROCESSOR_LOCAL_APIC_STRUCTURE *)(Madt + 1);
  while ((UINTN)LocalApic < (UINTN)Madt + Length) {
    if (LocalApic->Type == EFI_ACPI_2_0_PROCESSOR_LOCAL_APIC) {
      if ((LocalApic->Flags & EFI_ACPI_2_0_LOCAL_APIC_ENABLED) != 0) {
        ApicIdList[Index] = LocalApic->ApicId;
        DEBUG ((EFI_D_INFO, "(FRM) ApicIdList(%d) - %08x\n", Index, ApicIdList[Index]));
        Index++;
      }
    } else if (LocalApic->Type == EFI_ACPI_4_0_PROCESSOR_LOCAL_X2APIC) {
      if ((((EFI_ACPI_4_0_PROCESSOR_LOCAL_X2APIC_STRUCTURE *)LocalApic)->Flags & EFI_ACPI_4_0_LOCAL_APIC_ENABLED) != 0) {
        ApicIdList[Index] = ((EFI_ACPI_4_0_PROCESSOR_LOCAL_X2APIC_STRUCTURE *)LocalApic)->X2ApicId;
        DEBUG ((EFI_D_INFO, "(FRM) ApicIdList(%d) - %08x\n", Index, ApicIdList[Index]));
        Index++;
      }
    }
    LocalApic = (EFI_ACPI_2_0_PROCESSOR_LOCAL_APIC_STRUCTURE *)((UINTN)LocalApic + LocalApic->Length);
  }

  return ;
}

/**
  Get MCFG ACPI table.

  @return MCFG pointer.
**/
VOID *
GetMcfg (
  VOID
  )
{
  return GetAcpiTableViaSignature (EFI_ACPI_2_0_MEMORY_MAPPED_CONFIGURATION_BASE_ADDRESS_TABLE_SIGNATURE);
}

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
  )
{
  EFI_ACPI_MEMORY_MAPPED_CONFIGURATION_BASE_ADDRESS_TABLE_HEADER                         *Mcfg;
  UINTN                                                                                  Length;
  EFI_ACPI_MEMORY_MAPPED_ENHANCED_CONFIGURATION_SPACE_BASE_ADDRESS_ALLOCATION_STRUCTURE  *McfgStruct;

  Mcfg = GetMcfg ();
  if (Mcfg == NULL) {
    *PciExpressBaseAddress = 0;
    *PciExpressLength = 0;
    DEBUG ((EFI_D_INFO, "(FRM) PciExpressBaseAddress - 0, PciExpressLength - 0\n"));
    return 0;
  }

  Length = Mcfg->Header.Length;
  McfgStruct = (EFI_ACPI_MEMORY_MAPPED_ENHANCED_CONFIGURATION_SPACE_BASE_ADDRESS_ALLOCATION_STRUCTURE *)(Mcfg + 1);
  while ((UINTN)McfgStruct < (UINTN)Mcfg + Length) {
    if ((McfgStruct->PciSegmentGroupNumber == 0) && (McfgStruct->StartBusNumber == 0)) {
      *PciExpressBaseAddress = McfgStruct->BaseAddress;
      *PciExpressLength = (McfgStruct->EndBusNumber + 1) * SIZE_1MB;
      DEBUG ((EFI_D_INFO, "(FRM) PciExpressBaseAddress - 0x%016lx, PciExpressLength - 0x%016lx\n", *PciExpressBaseAddress, *PciExpressLength));
      return McfgStruct->BaseAddress;
    }
    McfgStruct ++;
  }

  *PciExpressBaseAddress = 0;
  *PciExpressLength = 0;
  DEBUG ((EFI_D_ERROR, "(FRM) PciExpressBaseAddress - 0, PciExpressLength - 0\n"));
  return 0;
}

/**
  Get FADT ACPI table.

  @return FADT pointer.
**/
VOID *
GetFadt (
  VOID
  )
{
  return GetAcpiTableViaSignature (EFI_ACPI_1_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE);
}

/**

  This function return ACPI timer port from FADT.

  @param AcpiTimerWidth  ACPI timer width

  @return ACPI timer port

**/
UINT16
GetAcpiTimerPort (
  OUT UINT8  *AcpiTimerWidth
  )
{
  EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE  *Fadt;

  Fadt = GetFadt ();
  if (Fadt == NULL) {
    DEBUG ((EFI_D_ERROR, "(FRM) FADT not found!\n"));
    *AcpiTimerWidth = 0;
    return 0;
  }

  if (Fadt->Header.Revision >= EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE_REVISION) {
    if (Fadt->XPmTmrBlk.Address != 0) {
      DEBUG ((EFI_D_INFO, "(FRM) ACPI XTimer port - 0x%08x\n", (UINT32)Fadt->XPmTmrBlk.Address));
      *AcpiTimerWidth= (UINT8)Fadt->XPmTmrBlk.RegisterBitWidth;
      //
      // It's possible that the PM_TMR_BLK.RegisterBitWidth is always 32,
      //  we need to set the correct RegisterBitWidth value according to the TMR_VAL_EXT
      //  A zero indicates TMR_VAL is implemented as a 24-bit value. 
      //  A one indicates TMR_VAL is implemented as a 32-bit value
      //
      if ((Fadt->Flags & EFI_ACPI_2_0_TMR_VAL_EXT) != 0) {
        *AcpiTimerWidth= 32;
      } else {
        *AcpiTimerWidth= 24;
      }
      return (UINT16)Fadt->XPmTmrBlk.Address;
    }
  }

  if (Fadt->PmTmrBlk != 0) {
    DEBUG ((EFI_D_INFO, "(FRM) ACPI Timer port - 0x%08x\n", Fadt->PmTmrBlk));
    //
    // It's possible that the PM_TMR_BLK.RegisterBitWidth is always 32,
    //  we need to set the correct RegisterBitWidth value according to the TMR_VAL_EXT
    //  A zero indicates TMR_VAL is implemented as a 24-bit value. 
    //  A one indicates TMR_VAL is implemented as a 32-bit value
    //
    *AcpiTimerWidth= (UINT8)(Fadt->PmTmrLen * 8);
    if ((Fadt->Flags & EFI_ACPI_2_0_TMR_VAL_EXT) != 0) {
      *AcpiTimerWidth= 32;
    } else {
      *AcpiTimerWidth= 24;
    }
    return (UINT16)Fadt->PmTmrBlk;
  }

  DEBUG ((EFI_D_ERROR, "(FRM) ACPI All Timer port - 0\n"));
  *AcpiTimerWidth = 0;
  return 0;
}

/**

  This function return ACPI PmControl port from FADT.

  @return ACPI PmControl port

**/
UINT16
GetAcpiPmControlPort (
  VOID
  )
{
  EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE  *Fadt;

  Fadt = GetFadt ();
  if (Fadt == NULL) {
    DEBUG ((EFI_D_ERROR, "(FRM) FADT not found!\n"));
    return 0;
  }

  if (Fadt->Header.Revision >= EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE_REVISION) {
    if (Fadt->XPm1aCntBlk.Address != 0) {
      DEBUG ((EFI_D_INFO, "(FRM) ACPI PmControl port - 0x%08x\n", (UINT32)Fadt->XPm1aCntBlk.Address));
      return (UINT16)Fadt->XPm1aCntBlk.Address;
    }
  }

  if (Fadt->Pm1aCntBlk != 0) {
    DEBUG ((EFI_D_INFO, "(FRM) ACPI PmControl port - 0x%08x\n", Fadt->Pm1aCntBlk));
    return (UINT16)Fadt->PmTmrBlk;
  }

  DEBUG ((EFI_D_ERROR, "(FRM) ACPI PmControl port - 0\n"));
  return 0;
}

/**

  This function return ACPI Reset port from FADT.

  @return ACPI Reset port

**/
UINT16
GetAcpiResetPort (
  VOID
  )
{
  EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE  *Fadt;

  Fadt = GetFadt ();
  if (Fadt == NULL) {
    DEBUG ((EFI_D_ERROR, "(FRM) FADT not found!\n"));
    return 0;
  }

  if (Fadt->Header.Revision >= EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE_REVISION) {
    if (Fadt->ResetReg.Address != 0) {
      DEBUG ((EFI_D_ERROR, "(FRM) ACPI Reset port - 0x%08x\n", (UINT32)Fadt->ResetReg.Address));
      return (UINT16)Fadt->ResetReg.Address;
    }
  }

  DEBUG ((EFI_D_ERROR, "(FRM) ACPI Reset port - 0\n"));
  return 0;
}
