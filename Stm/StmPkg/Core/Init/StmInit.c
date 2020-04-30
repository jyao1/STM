/** @file
  STM initialization

  Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmInit.h"
#include "PeStm.h"
#include <Library/PcdLib.h>
#include <string.h>

extern PE_SMI_CONTROL PeSmiControl;

extern void InitPe();

STM_HOST_CONTEXT_COMMON         mHostContextCommon;
STM_GUEST_CONTEXT_COMMON        mGuestContextCommonSmi;
STM_GUEST_CONTEXT_COMMON        mGuestContextCommonSmm[NUM_PE_TYPE];


static SPIN_LOCK CpuReadyCountLock;
static unsigned int CpuReadyCount = 0;
void CpuReadySync(UINT32 Index);
VOID InitCpuReadySync();

volatile BOOLEAN                mIsBspInitialized;

extern volatile BOOLEAN         *mCpuInitStatus;

/*++
  STM runtime:

                           +------------+
                           | SMM handler|
  +-------+                +------------+
  | Guest | --                  ^ |
  +-------+  |       (2)VMResume| |(3)RSM
             |(1) SMI           | v
  +-------+  |-----------> +------------+
  |       |  |(4) VMResume |SMI-H  SMM-H|
  | MVMM  |  -<----------- |   STM      |
  |       | (0) Init       |STM-Init    |
  +-------+ -------------> +------------+

  Memory layout:
                        +--------------------+ --
                        | SMM VMCS           |  |
                        +--------------------+  |-> Per-Processor VMCS
                        | SMI VMCS           |  |
                        +--------------------+ --
                        | SMM VMCS           |  |
                        +--------------------+  |-> Per-Processor VMCS
                        | SMI VMCS           |  |
                        +--------------------+ --
                        | Stack              |  |-> Per-Processor Dynamic
                        +--------------------+ --
                        | Stack              |  |-> Per-Processor Dynamic
                  RSP-> +--------------------+ --
                        | Heap               |  |
                        +--------------------+  |-> Additional Dynamic
                        | Page Table (24K)   |  |
                  CR3-> +--------------------+ --
                  RIP-> | STM Code           |  |
                        +--------------------+  |
                        | GDT (4K)           |  |-> Static Image
                  GDT-> +--------------------+  |
                        | STM Header (4K)    |  |
                 MSEG-> +--------------------+ --
--*/

/**

  This function return 4K page aligned VMCS size.

  @return 4K page aligned VMCS size

**/
UINT32
GetVmcsSize (
  VOID
  )
{
  UINT64 Data64;
  UINT32 VmcsSize;

  Data64 = AsmReadMsr64 (IA32_VMX_BASIC_MSR_INDEX);
  VmcsSize = (UINT32)(RShiftU64 (Data64, 32) & 0xFFFF);
  VmcsSize = STM_PAGES_TO_SIZE (STM_SIZE_TO_PAGES (VmcsSize));

  return VmcsSize;
}

/**

  This function return if Senter executed.

  @retval TRUE  Senter executed
  @retval FALSE Sexit not executed

**/
BOOLEAN
IsSentryEnabled (
  VOID
  )
{
  UINT32  TxtStatus;

  TxtStatus = TxtPubRead32 (TXT_STS);
  if (((TxtStatus & TXT_STS_SENTER_DONE) != 0) &&
      ((TxtStatus & TXT_STS_SEXIT_DONE) == 0)) {
    return TRUE;
  } else {
    return FALSE;
  }
}

/**

  This function get CPU number in TXT heap region.

  @return CPU number in TXT heap region

**/
UINT32
GetCpuNumFromTxt (
  VOID
  )
{
  TXT_BIOS_TO_OS_DATA            *BiosToOsData;

  BiosToOsData = GetTxtBiosToOsData ();

  return BiosToOsData->NumLogProcs;
}

/**

  This function return PCI Express information in TXT heap region.
  Only one segment information is returned.

  @param  PciExpressBaseAddress  PCI Express base address
  @param  PciExpressLength       PCI Express length

  @return PciExpressBaseAddress

**/
UINT64
GetPciExpressInfoFromTxt (
  OUT UINT64  *PciExpressBaseAddress,
  OUT UINT64  *PciExpressLength
  )
{
  TXT_SINIT_TO_MLE_DATA               *SinitToMleData;
  TXT_SINIT_MEMORY_DESCRIPTOR_RECORD  *SinitMemoryDescriptor;
  UINTN                               Index;

  SinitToMleData = GetTxtSinitToMleData ();
  SinitMemoryDescriptor = (TXT_SINIT_MEMORY_DESCRIPTOR_RECORD *)((UINTN)SinitToMleData - sizeof(UINT64) + SinitToMleData->SinitMdrTableOffset);
  for (Index = 0; Index < SinitToMleData->NumberOfSinitMdrs; Index++) {
    if (SinitMemoryDescriptor[Index].Type == TXT_SINIT_MDR_TYPE_PCIE) {
      *PciExpressBaseAddress = SinitMemoryDescriptor[Index].Address;
      *PciExpressLength = SinitMemoryDescriptor[Index].Length;
      return SinitMemoryDescriptor[Index].Address;
    }
  }

  *PciExpressBaseAddress = 0;
  *PciExpressLength = 0;
  return 0;
}

#define EBDA_BASE_ADDRESS 0x40E

/**

  This function find ACPI RSDPTR in TXT heap region.

  @return ACPI RSDPTR in TXT heap region

**/
VOID *
FindTxtAcpiRsdPtr (
  VOID
  )
{
  TXT_OS_TO_SINIT_DATA            *OsSinitData;

  OsSinitData = GetTxtOsToSinitData ();
  if (OsSinitData->Version < 5) {
    return NULL;
  }

  return (VOID *)(UINTN)OsSinitData->RsdpPtr;
}

/**

  This function find ACPI RSDPTR in UEFI or legacy region.

  @return ACPI RSDPTR in UEFI or legacy region

**/
VOID *
FindAcpiRsdPtr (
  VOID
  )
{
  if (mHostContextCommon.AcpiRsdp != 0) {
	  DEBUG((EFI_D_INFO, "RSDP found in mHostContextCommon.AcpiRsdp 0x%016llx\n", mHostContextCommon.AcpiRsdp));
    return (VOID *)(UINTN)mHostContextCommon.AcpiRsdp;
  } else {
    UINTN                           Address;
	DEBUG((EFI_D_INFO, "Searching for RSDP\n"));
    //
    // Search EBDA
    //
    Address = (*(UINT16 *)(UINTN)(EBDA_BASE_ADDRESS)) << 4;
    for (; Address < 0xA0000 ; Address += 0x10) {
         if (*(UINT64 *)(Address) == EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER_SIGNATURE) {
		DEBUG((EFI_D_INFO, "RSDP found\n"));
		mHostContextCommon.AcpiRsdp = Address;     // save having to keep looking for it
		AsmWbinvd ();                              // let everone see it
        return (VOID *)(Address);
      }
    }

    //
    // First Seach 0x0e0000 - 0x0fffff for RSD Ptr
    //
    for (Address = 0xe0000; Address < 0xfffff; Address += 0x10) {
		//DEBUG((EFI_D_INFO, "0x%08lx  0x%016llx 0x%016llx - ", Address, *(UINT64 *)(Address), EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER_SIGNATURE));
        if (*(UINT64 *)(Address) == EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER_SIGNATURE) {
		DEBUG((EFI_D_INFO, "RSDP found\n"));
		mHostContextCommon.AcpiRsdp = Address;     // save having to keep looking for it
		AsmWbinvd ();
        return (VOID *)Address;
      }
    }

    //
    // Not found
    //
    return NULL;
  }
}

/**

  This function scan ACPI table in RSDT.

  @param Rsdt      ACPI RSDT
  @param Signature ACPI table signature

  @return ACPI table

**/
VOID *
ScanTableInRSDT (
  IN EFI_ACPI_DESCRIPTION_HEADER    *Rsdt,
  IN UINT32                         Signature
  )
{
  UINTN                              Index;
  UINT32                             EntryCount;
  UINT32                             *EntryPtr;
  EFI_ACPI_DESCRIPTION_HEADER        *Table;

  EntryCount = (Rsdt->Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT32);
  
  EntryPtr = (UINT32 *)(Rsdt + 1);
  for (Index = 0; Index < EntryCount; Index ++, EntryPtr ++) {
    Table = (EFI_ACPI_DESCRIPTION_HEADER *)((UINTN)(*EntryPtr));
    if (Table->Signature == Signature) {
      return Table;
    }
  }
  
  return NULL;
}

/**

  This function scan ACPI table in XSDT.

  @param Xsdt      ACPI XSDT
  @param Signature ACPI table signature

  @return ACPI table

**/
VOID *
ScanTableInXSDT (
  IN EFI_ACPI_DESCRIPTION_HEADER    *Xsdt,
  IN UINT32                         Signature
  )
{
  UINTN                          Index;
  UINT32                         EntryCount;
  UINT64                         EntryPtr;
  UINTN                          BasePtr;
  EFI_ACPI_DESCRIPTION_HEADER    *Table;

  EntryCount = (Xsdt->Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT64);
  
  BasePtr = (UINTN)(Xsdt + 1);
  for (Index = 0; Index < EntryCount; Index ++) {
    CopyMem (&EntryPtr, (VOID *)(BasePtr + Index * sizeof(UINT64)), sizeof(UINT64));
    Table = (EFI_ACPI_DESCRIPTION_HEADER *)((UINTN)(EntryPtr));
    if (Table->Signature == Signature) {
      return Table;
    }
  }
  
  return NULL;
}

/**

  This function find ACPI table according to signature.

  @param RsdPtr    ACPI RSDPTR
  @param Signature ACPI table signature

  @return ACPI table

**/
VOID *
FindAcpiPtr (
  VOID             *RsdPtr,
  UINT32           Signature
  )
{
  EFI_ACPI_DESCRIPTION_HEADER                             *AcpiTable;
  EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER            *Rsdp;
  EFI_ACPI_DESCRIPTION_HEADER                             *Rsdt;
  EFI_ACPI_DESCRIPTION_HEADER                             *Xsdt;

  if (RsdPtr == NULL) {
    return NULL;
  }
 
  AcpiTable = NULL;

  Rsdp = (EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER *)RsdPtr;
  Rsdt = (EFI_ACPI_DESCRIPTION_HEADER *)(UINTN)Rsdp->RsdtAddress;
  Xsdt = NULL;
  if ((Rsdp->Revision >= 2) && (Rsdp->XsdtAddress < (UINT64)(UINTN)-1)) {
    Xsdt = (EFI_ACPI_DESCRIPTION_HEADER *)(UINTN)Rsdp->XsdtAddress;
  }
  //
  // Check Xsdt
  //
  if (Xsdt != NULL) {
    AcpiTable = ScanTableInXSDT (Xsdt, Signature);
  }
  //
  // Check Rsdt
  //
  if ((AcpiTable == NULL) && (Rsdt != NULL)) {
    AcpiTable = ScanTableInRSDT (Rsdt, Signature);
  }

  return AcpiTable;
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

  Madt = FindAcpiPtr (FindAcpiRsdPtr (), EFI_ACPI_2_0_MULTIPLE_APIC_DESCRIPTION_TABLE_SIGNATURE);
  if (Madt == NULL) {
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

  return Index;
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

  Mcfg = FindAcpiPtr (FindAcpiRsdPtr (), EFI_ACPI_2_0_MEMORY_MAPPED_CONFIGURATION_BASE_ADDRESS_TABLE_SIGNATURE);
  if (Mcfg == NULL) {
    *PciExpressBaseAddress = 0;
    *PciExpressLength = 0;
    return 0;
  }

  Length = Mcfg->Header.Length;
  McfgStruct = (EFI_ACPI_MEMORY_MAPPED_ENHANCED_CONFIGURATION_SPACE_BASE_ADDRESS_ALLOCATION_STRUCTURE *)(Mcfg + 1);
  while ((UINTN)McfgStruct < (UINTN)Mcfg + Length) {
    if ((McfgStruct->PciSegmentGroupNumber == 0) && (McfgStruct->StartBusNumber == 0)) {
      *PciExpressBaseAddress = McfgStruct->BaseAddress;
      *PciExpressLength = (McfgStruct->EndBusNumber + 1) * SIZE_1MB;
      return McfgStruct->BaseAddress;
    }
    McfgStruct ++;
  }

  *PciExpressBaseAddress = 0;
  *PciExpressLength = 0;
  return 0;
}

/**

  This function return minimal MSEG size required by STM.

  @param StmHeader  Stm header

  @return minimal MSEG size

**/
UINTN
GetMinMsegSize (
  IN STM_HEADER                     *StmHeader
  )
{
  UINTN                     MinMsegSize;

  //MinMsegSize = (STM_PAGES_TO_SIZE (STM_SIZE_TO_PAGES (StmHeader->SwStmHdr.StaticImageSize)) +
  /* we use the page table offset in this calculation because the static memory size does
     not account for the data and bss locations which come before the page tables and 
     are cleared by sinit */
  MinMsegSize = StmHeader->HwStmHdr.Cr3Offset +
                 StmHeader->SwStmHdr.AdditionalDynamicMemorySize +
                 ((StmHeader->SwStmHdr.PerProcDynamicMemorySize + GetVmcsSize () * 2) * mHostContextCommon.CpuNum);

  return MinMsegSize;
}

/**

  This function return the index of CPU according to stack.

  @param Register  stack value of this CPU

  @return CPU index

**/
UINT32
GetIndexFromStack (
  IN X86_REGISTER *Register
  )
{
  STM_HEADER                     *StmHeader;
  UINTN                          ThisStackTop;
  UINTN                          StackBottom;
  UINTN                          Index;

  StmHeader = (STM_HEADER *)(UINTN)((UINT32)AsmReadMsr64(IA32_SMM_MONITOR_CTL_MSR_INDEX) & 0xFFFFF000);

  //
  // Stack top of this CPU
  //
  ThisStackTop = ((UINTN)Register + SIZE_4KB - 1) & ~(SIZE_4KB - 1);

  //
  // EspOffset pointer to bottom of 1st CPU
  //
  StackBottom = (UINTN)StmHeader + StmHeader->HwStmHdr.EspOffset;
  Index = (ThisStackTop - StackBottom) / StmHeader->SwStmHdr.PerProcDynamicMemorySize;

  //
  // Need minus one for 0-based CPU index
  //
  return (UINT32)(Index - 1);
}


UINT64
GetMsegInfoFromTxt (
                    OUT UINT64  *MsegBase,
                    OUT UINT64  *MsegLength
                    )
{
    *MsegBase = TxtPubRead64(TXT_MSEG_BASE);
    *MsegLength = TxtPubRead64(TXT_MSEG_SIZE);
    return 0;
}

/**
 
 This function return MSEG information from MSR.
 
 @param  MsegBase    MSEG base address
 @param  
 MSEG length
 
 @return MsegBase
 
 **/

extern MRTT_INFO  mMtrrInfo;

UINT64
GetMsegInfoFromMsr (
                    OUT UINT64  *MsegBase,
                    OUT UINT64  *MsegLength
                    )
{
    UINT32                        SmrrBase;
    UINT32                        SmrrLength;
    
    SmrrBase = (UINT32)mMtrrInfo.SmrrBase & (UINT32)mMtrrInfo.SmrrMask & 0xFFFFF000;
    SmrrLength = (UINT32)mMtrrInfo.SmrrMask & 0xFFFFF000;
    SmrrLength = ~SmrrLength + 1;
    
    *MsegBase = (UINT64)((UINT32)AsmReadMsr64(IA32_SMM_MONITOR_CTL_MSR_INDEX) & 0xFFFFF000);
    
    *MsegLength = SmrrLength - (*MsegBase - SmrrBase);

    return *MsegLength;
}

/**

  This function initialize STM heap.

  @param StmHeader MSEG STM header

**/
VOID
InitHeap (
  IN STM_HEADER   *StmHeader
  )
{
  HEAP_HEADER *HeaderPointer;

  UINT64 MsegBase;
  UINT64 MsegLength;

  if (IsSentryEnabled()) {
    GetMsegInfoFromTxt (&MsegBase, &MsegLength);
    DEBUG ((EFI_D_INFO, "TXT MsegBase- %08x\n", (UINTN)MsegBase));
    DEBUG ((EFI_D_INFO, "TXT MsegLength - %08x\n", (UINTN)MsegLength));
  } else {
    GetMsegInfoFromMsr (&MsegBase, &MsegLength);
  }
  DEBUG ((EFI_D_INFO, "MsegBase (MSR) - %08x\n", (UINTN)MsegBase));
  DEBUG ((EFI_D_INFO, "MsegLength (end of TSEG) - %08x\n", (UINTN)MsegLength));
  if (MsegBase == 0) {
    DEBUG ((EFI_D_ERROR, "MsegBase == 0\n"));
    CpuDeadLoop ();
  }

  DEBUG ((EFI_D_INFO, "Cr30Offset - %08x\n", StmHeader->HwStmHdr.Cr3Offset));
  DEBUG ((EFI_D_INFO, "Page Table Start - %08x\n", MsegBase + StmHeader->HwStmHdr.Cr3Offset));


  // make sure that the tseg size was set correctly
  // right now we will assume a max size of 3mb for mseg, bug, bug

  mHostContextCommon.HeapBottom = (UINT64)((UINTN)StmHeader + 
                                            StmHeader->HwStmHdr.Cr3Offset +
                                            STM_PAGES_TO_SIZE(6)); // reserve 6 page for page table

  mHostContextCommon.HeapTop    = (UINT64)((UINTN)StmHeader + 
                                            STM_PAGES_TO_SIZE (STM_SIZE_TO_PAGES (StmHeader->SwStmHdr.StaticImageSize)) + 
                                            StmHeader->SwStmHdr.AdditionalDynamicMemorySize);

  mHostContextCommon.HeapFree = mHostContextCommon.HeapBottom;
  HeaderPointer = (HEAP_HEADER *)((UINTN) mHostContextCommon.HeapFree);

  HeaderPointer->NextBlock = 0L;
  HeaderPointer->BlockLength = (mHostContextCommon.HeapTop - mHostContextCommon.HeapBottom) >> 12;

}

/**

  This function initialize basic context for STM.

**/
VOID
InitBasicContext (
  VOID
  )
{
  mHostContextCommon.HostContextPerCpu = (STM_HOST_CONTEXT_PER_CPU *)AllocatePages (STM_SIZE_TO_PAGES(sizeof(STM_HOST_CONTEXT_PER_CPU)) * mHostContextCommon.CpuNum);
  mGuestContextCommonSmi.GuestContextPerCpu = (STM_GUEST_CONTEXT_PER_CPU *) AllocatePages (STM_SIZE_TO_PAGES(sizeof(STM_GUEST_CONTEXT_PER_CPU)) * mHostContextCommon.CpuNum);
  mGuestContextCommonSmm[SMI_HANDLER].GuestContextPerCpu = AllocatePages (STM_SIZE_TO_PAGES(sizeof(STM_GUEST_CONTEXT_PER_CPU)) * mHostContextCommon.CpuNum);
}

extern void GetMtrr();  // found in eptinit.c...

/**

  This function initialize BSP.

  @param Register X86 register context

**/
VOID
BspInit (
  IN X86_REGISTER *Register
  )
{
  STM_HEADER                     *StmHeader;
  UINTN                          VmcsDatabasePage;
  VMCS_RECORD_STRUCTURE          *VmcsRecord;
  TXT_PROCESSOR_SMM_DESCRIPTOR   *TxtProcessorSmmDescriptor;
  X86_REGISTER                   *Reg;
  IA32_IDT_GATE_DESCRIPTOR       *IdtGate;
  UINT32                         SubIndex;
  UINTN                          XStateSize;
  UINT32                         RegEax;
  IA32_VMX_MISC_MSR              VmxMisc;
  UINT32			 BiosStmVer = 100;   // initially assume that the BIOS supports v1.0 of the Intel ref
  IA32_DESCRIPTOR IdtrLoad;

  GetMtrr();  //Needed in various inits
  AsmWbinvd();  // make sure it gets out
 
  StmHeader = (STM_HEADER *)(UINTN)((UINT32)AsmReadMsr64(IA32_SMM_MONITOR_CTL_MSR_INDEX) & 0xFFFFF000);

    // on a platform that does not start with TXT, cannot assume the data space has been set to zero
  ZeroMem(&mHostContextCommon, sizeof(STM_HOST_CONTEXT_COMMON));
  ZeroMem(&mGuestContextCommonSmi, sizeof(STM_HOST_CONTEXT_COMMON));
  ZeroMem(&mGuestContextCommonSmm, sizeof(STM_HOST_CONTEXT_COMMON) * NUM_PE_TYPE);

  InitHeap (StmHeader);
  // after that we can use mHostContextCommon
 
  InitializeSpinLock (&mHostContextCommon.DebugLock);
 // after that we can use DEBUG

  DEBUG ((EFI_D_INFO, "   ********************** STM/PE *********************\n"));
  DEBUG ((EFI_D_INFO, "!!!STM build time - %a %a!!!\n", (CHAR8 *)__DATE__, (CHAR8 *)__TIME__));
  DEBUG ((EFI_D_INFO, "!!!STM Relocation DONE!!!\n"));
  DEBUG ((EFI_D_INFO, "!!!Enter StmInit (BSP)!!! - %d (%x)\n", (UINTN)0, (UINTN)ReadUnaligned32 ((UINT32 *)&Register->Rax)));

   // Check Signature and size
  VmxMisc.Uint64 = AsmReadMsr64 (IA32_VMX_MISC_MSR_INDEX);
  if ((VmxMisc.Uint64 & BIT15) != 0) {
    TxtProcessorSmmDescriptor = (TXT_PROCESSOR_SMM_DESCRIPTOR *)(UINTN)(AsmReadMsr64 (IA32_SMBASE_INDEX) + SMM_TXTPSD_OFFSET);
  } else {
    TxtProcessorSmmDescriptor = (TXT_PROCESSOR_SMM_DESCRIPTOR *)(UINTN)(VmRead32 (VMCS_32_GUEST_SMBASE_INDEX) + SMM_TXTPSD_OFFSET);
  }
  
  DEBUG((EFI_D_INFO, "TxtProcessorSmmDescriptor: 0x%016llx\n", TxtProcessorSmmDescriptor));

  /*debug - used to make sure that the bios properly sets up the Smm Descriptors*********************debug*/

  DEBUG ((EFI_D_INFO, "TxtProcessorSmmDescriptor     - %08x\n",   (UINTN)TxtProcessorSmmDescriptor));
  DEBUG ((EFI_D_INFO, "  Signature                   - %016lx\n", TxtProcessorSmmDescriptor->Signature));
  DEBUG ((EFI_D_INFO, "  Size                        - %04x\n",   (UINTN)TxtProcessorSmmDescriptor->Size));
  DEBUG ((EFI_D_INFO, "  SmmDescriptorVerMajor       - %02x\n",   (UINTN)TxtProcessorSmmDescriptor->SmmDescriptorVerMajor));
  DEBUG ((EFI_D_INFO, "  SmmDescriptorVerMinor       - %02x\n",   (UINTN)TxtProcessorSmmDescriptor->SmmDescriptorVerMinor));
  DEBUG ((EFI_D_INFO, "  LocalApicId                 - %08x\n",   (UINTN)TxtProcessorSmmDescriptor->LocalApicId));
  DEBUG ((EFI_D_INFO, "  ExecutionDisableOutsideSmrr - %02x\n",   (UINTN)TxtProcessorSmmDescriptor->SmmEntryState.ExecutionDisableOutsideSmrr));
  DEBUG ((EFI_D_INFO, "  Intel64Mode                 - %02x\n",   (UINTN)TxtProcessorSmmDescriptor->SmmEntryState.Intel64Mode));
  DEBUG ((EFI_D_INFO, "  Cr4Pae                      - %02x\n",   (UINTN)TxtProcessorSmmDescriptor->SmmEntryState.Cr4Pae));
  DEBUG ((EFI_D_INFO, "  Cr4Pse                      - %02x\n",   (UINTN)TxtProcessorSmmDescriptor->SmmEntryState.Cr4Pse));
  DEBUG ((EFI_D_INFO, "  SmramToVmcsRestoreRequired  - %02x\n",   (UINTN)TxtProcessorSmmDescriptor->SmmResumeState.SmramToVmcsRestoreRequired));
  DEBUG ((EFI_D_INFO, "  ReinitializeVmcsRequired    - %02x\n",   (UINTN)TxtProcessorSmmDescriptor->SmmResumeState.ReinitializeVmcsRequired));
  DEBUG ((EFI_D_INFO, "  DomainType                  - %02x\n",   (UINTN)TxtProcessorSmmDescriptor->StmSmmState.DomainType));
  DEBUG ((EFI_D_INFO, "  XStatePolicy                - %02x\n",   (UINTN)TxtProcessorSmmDescriptor->StmSmmState.XStatePolicy));
  DEBUG ((EFI_D_INFO, "  EptEnabled                  - %02x\n",   (UINTN)TxtProcessorSmmDescriptor->StmSmmState.EptEnabled));
  DEBUG ((EFI_D_INFO, "  SmmCs                       - %04x\n",   (UINTN)TxtProcessorSmmDescriptor->SmmCs));
  DEBUG ((EFI_D_INFO, "  SmmDs                       - %04x\n",   (UINTN)TxtProcessorSmmDescriptor->SmmDs));
  DEBUG ((EFI_D_INFO, "  SmmSs                       - %04x\n",   (UINTN)TxtProcessorSmmDescriptor->SmmSs));
  DEBUG ((EFI_D_INFO, "  SmmOtherSegment             - %04x\n",   (UINTN)TxtProcessorSmmDescriptor->SmmOtherSegment));
  DEBUG ((EFI_D_INFO, "  SmmTr                       - %04x\n",   (UINTN)TxtProcessorSmmDescriptor->SmmTr));
  DEBUG ((EFI_D_INFO, "  SmmCr3                      - %016lx\n", TxtProcessorSmmDescriptor->SmmCr3));
  DEBUG ((EFI_D_INFO, "  SmmStmSetupRip              - %016lx\n", TxtProcessorSmmDescriptor->SmmStmSetupRip));
  DEBUG ((EFI_D_INFO, "  SmmStmTeardownRip           - %016lx\n", TxtProcessorSmmDescriptor->SmmStmTeardownRip));
  DEBUG ((EFI_D_INFO, "  SmmSmiHandlerRip            - %016lx\n", TxtProcessorSmmDescriptor->SmmSmiHandlerRip));
  DEBUG ((EFI_D_INFO, "  SmmSmiHandlerRsp            - %016lx\n", TxtProcessorSmmDescriptor->SmmSmiHandlerRsp));
  DEBUG ((EFI_D_INFO, "  SmmGdtPtr                   - %016lx\n", TxtProcessorSmmDescriptor->SmmGdtPtr));
  DEBUG ((EFI_D_INFO, "  SmmGdtSize                  - %08x\n",   (UINTN)TxtProcessorSmmDescriptor->SmmGdtSize));
  DEBUG ((EFI_D_INFO, "  RequiredStmSmmRevId         - %08x\n",   (UINTN)TxtProcessorSmmDescriptor->RequiredStmSmmRevId));
  DEBUG ((EFI_D_INFO, "  StmProtectionExceptionHandler:\n"));
  DEBUG ((EFI_D_INFO, "    SpeRip                    - %016lx\n", TxtProcessorSmmDescriptor->StmProtectionExceptionHandler.SpeRip));
  DEBUG ((EFI_D_INFO, "    SpeRsp                    - %016lx\n", TxtProcessorSmmDescriptor->StmProtectionExceptionHandler.SpeRsp));
  DEBUG ((EFI_D_INFO, "    SpeSs                     - %04x\n",   (UINTN)TxtProcessorSmmDescriptor->StmProtectionExceptionHandler.SpeSs));
  DEBUG ((EFI_D_INFO, "    PageViolationException    - %04x\n",   (UINTN)TxtProcessorSmmDescriptor->StmProtectionExceptionHandler.PageViolationException));
  DEBUG ((EFI_D_INFO, "    MsrViolationException     - %04x\n",   (UINTN)TxtProcessorSmmDescriptor->StmProtectionExceptionHandler.MsrViolationException));
  DEBUG ((EFI_D_INFO, "    RegisterViolationException- %04x\n",   (UINTN)TxtProcessorSmmDescriptor->StmProtectionExceptionHandler.RegisterViolationException));
  DEBUG ((EFI_D_INFO, "    IoViolationException      - %04x\n",   (UINTN)TxtProcessorSmmDescriptor->StmProtectionExceptionHandler.IoViolationException));
  DEBUG ((EFI_D_INFO, "    PciViolationException     - %04x\n",   (UINTN)TxtProcessorSmmDescriptor->StmProtectionExceptionHandler.PciViolationException));
  DEBUG ((EFI_D_INFO, "  BiosHwResourceRequirements  - %016lx\n", TxtProcessorSmmDescriptor->BiosHwResourceRequirementsPtr));
  DEBUG ((EFI_D_INFO, "  AcpiRsdp                    - %016lx\n", TxtProcessorSmmDescriptor->AcpiRsdp));
  DEBUG ((EFI_D_INFO, "  PhysicalAddressBits         - %02x\n",   (UINTN)TxtProcessorSmmDescriptor->PhysicalAddressBits));

  /************************************debug**********************************************************************/

  // We have to know CpuNum, or we do not know where VMCS will be.
  if (IsSentryEnabled ()) {
    mHostContextCommon.CpuNum = GetCpuNumFromTxt ();
    DEBUG ((EFI_D_INFO, "CpuNumber from TXT Region - %d\n", (UINTN)mHostContextCommon.CpuNum));
  } else {
    {
      EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER  *Rsdp;
      EFI_ACPI_DESCRIPTION_HEADER                   *Rsdt;
      EFI_ACPI_DESCRIPTION_HEADER                   *Xsdt;

	mHostContextCommon.AcpiRsdp = TxtProcessorSmmDescriptor->AcpiRsdp;
      Rsdp = FindAcpiRsdPtr ();
      DEBUG ((EFI_D_INFO, "Rsdp - %08x\n", Rsdp));
	  if (Rsdp == NULL) {
		DEBUG ((EFI_D_INFO, "Null Rsdp - Can not continue\n", Rsdp));
        CpuDeadLoop ();
      }
      Rsdt = (EFI_ACPI_DESCRIPTION_HEADER *)(UINTN)Rsdp->RsdtAddress;
      DEBUG ((EFI_D_INFO, "Rsdt - %08x\n", Rsdt));
      DEBUG ((EFI_D_INFO, "RsdtLen - %08x\n", Rsdt->Length));
      if ((Rsdp->Revision >= 2) && (Rsdp->XsdtAddress < (UINT64)(UINTN)-1)) {
        Xsdt = (EFI_ACPI_DESCRIPTION_HEADER *)(UINTN)Rsdp->XsdtAddress;
        DEBUG ((EFI_D_INFO, "Xsdt - %016lx\n", Xsdt));
        DEBUG ((EFI_D_INFO, "XsdtLen - %08x\n", Xsdt->Length));
      }
    }

    mHostContextCommon.CpuNum = GetCpuNumFromAcpi ();
    DEBUG ((EFI_D_INFO, "CpuNumber from ACPI MADT - %d\n", (UINTN)mHostContextCommon.CpuNum));
  }

  InterlockedIncrement (&mHostContextCommon.JoinedCpuNum);
  mHostContextCommon.StmShutdown = 0;     // used by Stm/Pe to know when to stop the timer
  InitializeSpinLock (&mHostContextCommon.MemoryLock);
  InitializeSpinLock (&mHostContextCommon.SmiVmcallLock);
  InitializeSpinLock (&mHostContextCommon.PciLock);

  DEBUG ((EFI_D_INFO, "HeapBottom - %08x\n", mHostContextCommon.HeapBottom));
  DEBUG ((EFI_D_INFO, "HeapTop    - %08x\n", mHostContextCommon.HeapTop));

    // initialize PE state

  PeSmiControl.PeExec     = 0;
  PeSmiControl.PeNmiBreak = 0;
  PeSmiControl.PeCpuIndex = -1;

  // Initialize CpuSync

  CpuReadyCount = 0;
  InitializeSpinLock(&CpuReadyCountLock);

  if (TxtProcessorSmmDescriptor->Signature != TXT_PROCESSOR_SMM_DESCRIPTOR_SIGNATURE) {
    DEBUG ((EFI_D_INFO, "TXT Descriptor Signature ERROR - %016lx!\n", TxtProcessorSmmDescriptor->Signature));
    CpuDeadLoop ();
  }
  if(TxtProcessorSmmDescriptor->Size == sizeof(TXT_PROCESSOR_SMM_DESCRIPTOR) - 9)  // are we dealing with a .99 Bios
  {
	  BiosStmVer = 99; // version .99 has nine less bytes, etc
	  DEBUG((EFI_D_INFO, "Version .99 Bios detected Found Size: %08x SizeOf %08x\n", TxtProcessorSmmDescriptor->Size, sizeof(TXT_PROCESSOR_SMM_DESCRIPTOR)));
  }
  else
  {
	if (TxtProcessorSmmDescriptor->Size != sizeof(TXT_PROCESSOR_SMM_DESCRIPTOR)) {
		DEBUG ((EFI_D_INFO, "TXT Descriptor Size ERROR - %08x! Found %08x\n", TxtProcessorSmmDescriptor->Size, sizeof(TXT_PROCESSOR_SMM_DESCRIPTOR) ));
		CpuDeadLoop ();
	}
  }
  InitBasicContext ();

  DEBUG ((EFI_D_INFO, "Register(%d) - %08x\n", (UINTN)0, Register));
  Reg = &mGuestContextCommonSmi.GuestContextPerCpu[0].Register;
  Register->Rsp = VmReadN (VMCS_N_GUEST_RSP_INDEX);
  CopyMem (Reg, Register, sizeof(X86_REGISTER));

  mHostContextCommon.StmHeader = StmHeader;
  DEBUG ((EFI_D_INFO, "StmHeader                     - %08x\n", (UINTN)mHostContextCommon.StmHeader));
  DEBUG ((EFI_D_INFO, "Hardware field:\n"));
  DEBUG ((EFI_D_INFO, "  StmHeaderRevision           - %08x\n", (UINTN)StmHeader->HwStmHdr.StmHeaderRevision));
  DEBUG ((EFI_D_INFO, "  MonitorFeatures             - %08x\n", (UINTN)StmHeader->HwStmHdr.MonitorFeatures));
  DEBUG ((EFI_D_INFO, "  GdtrLimit                   - %08x\n", (UINTN)StmHeader->HwStmHdr.GdtrLimit));
  DEBUG ((EFI_D_INFO, "  GdtrBaseOffset              - %08x\n", (UINTN)StmHeader->HwStmHdr.GdtrBaseOffset));
  DEBUG ((EFI_D_INFO, "  CsSelector                  - %08x\n", (UINTN)StmHeader->HwStmHdr.CsSelector));
  DEBUG ((EFI_D_INFO, "  EipOffset                   - %08x\n", (UINTN)StmHeader->HwStmHdr.EipOffset));
  DEBUG ((EFI_D_INFO, "  EspOffset                   - %08x\n", (UINTN)StmHeader->HwStmHdr.EspOffset));
  DEBUG ((EFI_D_INFO, "  Cr3Offset                   - %08x\n", (UINTN)StmHeader->HwStmHdr.Cr3Offset));
  DEBUG ((EFI_D_INFO, "Software field:\n"));
  DEBUG ((EFI_D_INFO, "  StmSpecVerMajor             - %02x\n", (UINTN)StmHeader->SwStmHdr.StmSpecVerMajor));
  DEBUG ((EFI_D_INFO, "  StmSpecVerMinor             - %02x\n", (UINTN)StmHeader->SwStmHdr.StmSpecVerMinor));
  DEBUG ((EFI_D_INFO, "  StaticImageSize             - %08x\n", (UINTN)StmHeader->SwStmHdr.StaticImageSize));
  DEBUG ((EFI_D_INFO, "  PerProcDynamicMemorySize    - %08x\n", (UINTN)StmHeader->SwStmHdr.PerProcDynamicMemorySize));
  DEBUG ((EFI_D_INFO, "  AdditionalDynamicMemorySize - %08x\n", (UINTN)StmHeader->SwStmHdr.AdditionalDynamicMemorySize));
  DEBUG ((EFI_D_INFO, "  Intel64ModeSupported        - %08x\n", (UINTN)StmHeader->SwStmHdr.StmFeatures.Intel64ModeSupported));
  DEBUG ((EFI_D_INFO, "  EptSupported                - %08x\n", (UINTN)StmHeader->SwStmHdr.StmFeatures.EptSupported));
  DEBUG ((EFI_D_INFO, "  NumberOfRevIDs              - %08x\n", (UINTN)StmHeader->SwStmHdr.NumberOfRevIDs));
  for (SubIndex = 0; SubIndex < StmHeader->SwStmHdr.NumberOfRevIDs; SubIndex++) {
    DEBUG ((EFI_D_INFO, "  StmSmmRevID(%02d)             - %08x\n", (UINTN)SubIndex, (UINTN)StmHeader->SwStmHdr.StmSmmRevID[SubIndex]));
  }

  mHostContextCommon.AcpiRsdp = TxtProcessorSmmDescriptor->AcpiRsdp;

  //
  // Check MSEG BASE/SIZE in TXT region
  //
  mHostContextCommon.StmSize = GetMinMsegSize (StmHeader);
  {
	UINT64 MsegBase, MsegLength;
	INT32 AvailMseg;

    if (IsSentryEnabled()) {
      GetMsegInfoFromTxt (&MsegBase, &MsegLength);
    } else {
      GetMsegInfoFromMsr (&MsegBase, &MsegLength);
    }
    AvailMseg = MsegLength - mHostContextCommon.StmSize;

    DEBUG ((EFI_D_INFO, "MinMsegSize - 0x%08x MsegLength 0x%08x AvailMseg 0x%08x \n", 
				(UINTN)mHostContextCommon.StmSize,
				MsegLength,
				AvailMseg));
  }

  if(BiosStmVer == 99)
  {
	  mHostContextCommon.PhysicalAddressBits = 36; // for v.99 use this value for now. CPUID value might be too big
  }
  else
  {
	  mHostContextCommon.PhysicalAddressBits = TxtProcessorSmmDescriptor->PhysicalAddressBits;
  }

  AsmCpuid(CPUID_EXTENDED_INFORMATION, &RegEax, NULL, NULL, NULL);
  if (RegEax >= CPUID_EXTENDED_ADDRESS_SIZE) {
    AsmCpuid(CPUID_EXTENDED_ADDRESS_SIZE, &RegEax, NULL, NULL, NULL);
    RegEax = (UINT8)RegEax;
    DEBUG((EFI_D_INFO, "CPUID - PhysicalAddressBits - 0x%02x\n", (UINT8)RegEax));
  } else {
    RegEax = 36;
  }
  if ((mHostContextCommon.PhysicalAddressBits == 0) || (mHostContextCommon.PhysicalAddressBits > (UINT8)RegEax)) {
    mHostContextCommon.PhysicalAddressBits = (UINT8)RegEax;
  }
  if (sizeof(UINTN) == sizeof(UINT32)) {
    if (mHostContextCommon.PhysicalAddressBits > 32) {
      mHostContextCommon.PhysicalAddressBits = 32;
    }
  }
  mHostContextCommon.MaximumSupportAddress = (LShiftU64(1, mHostContextCommon.PhysicalAddressBits) - 1);

  mHostContextCommon.PageTable = AsmReadCr3 ();
  AsmReadGdtr (&mHostContextCommon.Gdtr);

  //
  // Set up STM host IDT to catch exception
  //
  mHostContextCommon.Idtr.Limit = (UINT16)(STM_MAX_IDT_NUM * sizeof (IA32_IDT_GATE_DESCRIPTOR) - 1);
  mHostContextCommon.Idtr.Base = (UINTN)AllocatePages (STM_SIZE_TO_PAGES (mHostContextCommon.Idtr.Limit + 1));
  IdtGate = (IA32_IDT_GATE_DESCRIPTOR *)mHostContextCommon.Idtr.Base;
  InitializeExternalVectorTablePtr (IdtGate);

  //IA32_DESCRIPTOR IdtrLoad;
  
  IdtrLoad = mHostContextCommon.Idtr;

  AsmWriteIdtr(&IdtrLoad);

  //
  // Add more paging for Host CR3.
  /////////
  //CreateHostPaging ();
  // make host paging dynamic to save space for the PE/VMs
  SetupStmPageFault();

  // VMCS database: One CPU one page should be enough
  VmcsDatabasePage = mHostContextCommon.CpuNum;
  mHostContextCommon.VmcsDatabase = (UINT64)(UINTN)AllocatePages (VmcsDatabasePage);
  // Set last entry
  ZeroMem ((VOID *)(UINTN)mHostContextCommon.VmcsDatabase, STM_PAGES_TO_SIZE(VmcsDatabasePage));
  VmcsRecord = (VMCS_RECORD_STRUCTURE *)(UINTN)mHostContextCommon.VmcsDatabase;
  VmcsRecord[STM_PAGES_TO_SIZE(VmcsDatabasePage) / sizeof(VMCS_RECORD_STRUCTURE) - 1].Type = VMCS_RECORD_LAST;
//  DumpVmcsRecord (mHostContextCommon.VmcsDatabase);

  // EventLog
  InitializeEventLog ();

  mCpuInitStatus = AllocatePages (STM_SIZE_TO_PAGES (mHostContextCommon.CpuNum));

  mGuestContextCommonSmm[SMI_HANDLER].GuestContextPerCpu[0].Cr3 = (UINTN)TxtProcessorSmmDescriptor->SmmCr3;
  mGuestContextCommonSmm[SMI_HANDLER].GuestContextPerCpu[0].Actived = FALSE;

  //
  // CompatiblePageTable for IA32 flat mode only
  //
  mGuestContextCommonSmm[SMI_HANDLER].CompatiblePageTable = CreateCompatiblePageTable ();
  mGuestContextCommonSmm[SMI_HANDLER].CompatiblePaePageTable = CreateCompatiblePaePageTable ();

  //
  // Allocate XState buffer
  //
  XStateSize = CalculateXStateSize ();
  mGuestContextCommonSmi.ZeroXStateBuffer = (UINTN)AllocatePages (STM_SIZE_TO_PAGES(XStateSize));
  for (SubIndex = 0; SubIndex < mHostContextCommon.CpuNum; SubIndex++) {
    mGuestContextCommonSmi.GuestContextPerCpu[SubIndex].XStateBuffer = (UINTN)AllocatePages (STM_SIZE_TO_PAGES(XStateSize));
  }

  EptInit ();
  IoInit ();
  MsrInit ();

  //
  // Get PciExpressBaseAddress
  //
  if (IsSentryEnabled()) {
    GetPciExpressInfoFromTxt(&mHostContextCommon.PciExpressBaseAddress, &mHostContextCommon.PciExpressLength);
    DEBUG((EFI_D_INFO, "PCIExpressBase   from TXT Region - %x\n", (UINTN)mHostContextCommon.PciExpressBaseAddress));
    DEBUG((EFI_D_INFO, "PCIExpressLength from TXT Region - %x\n", (UINTN)mHostContextCommon.PciExpressLength));
  } else {
    GetPciExpressInfoFromAcpi(&mHostContextCommon.PciExpressBaseAddress, &mHostContextCommon.PciExpressLength);
    DEBUG((EFI_D_INFO, "PCIExpressBase   from ACPI MCFG - %x\n", (UINTN)mHostContextCommon.PciExpressBaseAddress));
    DEBUG((EFI_D_INFO, "PCIExpressLength from ACPI MCFG - %x\n", (UINTN)mHostContextCommon.PciExpressLength));
  }
  if (mHostContextCommon.PciExpressBaseAddress == 0) {
    DEBUG((EFI_D_INFO, "mHostContextCommon.PciExpressBaseAddress == 0\n"));
    CpuDeadLoop();
  }
  if ((mHostContextCommon.PciExpressBaseAddress > mHostContextCommon.MaximumSupportAddress) ||
      (mHostContextCommon.PciExpressLength > mHostContextCommon.MaximumSupportAddress - mHostContextCommon.PciExpressBaseAddress)) {
    DEBUG((EFI_D_INFO, "mHostContextCommon.PciExpressBaseAddress overflow MaximumSupportAddress\n"));
    CpuDeadLoop();
  }
  if (IsOverlap (mHostContextCommon.PciExpressBaseAddress, mHostContextCommon.PciExpressLength, mHostContextCommon.TsegBase, mHostContextCommon.TsegLength)) {
    DEBUG((EFI_D_INFO, "mHostContextCommon.PciExpressBaseAddress overlap with TSEG\n"));
    CpuDeadLoop();
  }
  PcdSet64(PcdPciExpressBaseAddress, mHostContextCommon.PciExpressBaseAddress);

  for (SubIndex = 0; SubIndex < mHostContextCommon.CpuNum; SubIndex++) {
    mHostContextCommon.HostContextPerCpu[SubIndex].HostMsrEntryCount = 1;
    mGuestContextCommonSmi.GuestContextPerCpu[SubIndex].GuestMsrEntryCount = 1;
    mGuestContextCommonSmm[SMI_HANDLER].GuestContextPerCpu[SubIndex].GuestMsrEntryCount = 1;
    mHostContextCommon.HostContextPerCpu[SubIndex].GuestVmType = SMI_HANDLER;
  }
  mHostContextCommon.HostContextPerCpu[0].HostMsrEntryAddress = (UINT64)(UINTN)AllocatePages (STM_SIZE_TO_PAGES (sizeof(VM_EXIT_MSR_ENTRY) * mHostContextCommon.HostContextPerCpu[0].HostMsrEntryCount * mHostContextCommon.CpuNum));
  mGuestContextCommonSmi.GuestContextPerCpu[0].GuestMsrEntryAddress = (UINT64)(UINTN)AllocatePages (STM_SIZE_TO_PAGES (sizeof(VM_EXIT_MSR_ENTRY) * mGuestContextCommonSmi.GuestContextPerCpu[0].GuestMsrEntryCount * mHostContextCommon.CpuNum));
  mGuestContextCommonSmm[SMI_HANDLER].GuestContextPerCpu[SubIndex].GuestMsrEntryAddress = (UINT64)(UINTN)AllocatePages (STM_SIZE_TO_PAGES(sizeof(VM_EXIT_MSR_ENTRY) * mGuestContextCommonSmm[SMI_HANDLER].GuestContextPerCpu[0].GuestMsrEntryCount * mHostContextCommon.CpuNum));
  for (SubIndex = 0; SubIndex < mHostContextCommon.CpuNum; SubIndex++) {
    mHostContextCommon.HostContextPerCpu[SubIndex].HostMsrEntryAddress = mHostContextCommon.HostContextPerCpu[0].HostMsrEntryAddress + sizeof(VM_EXIT_MSR_ENTRY) * mGuestContextCommonSmi.GuestContextPerCpu[0].GuestMsrEntryCount * SubIndex;
    mGuestContextCommonSmi.GuestContextPerCpu[SubIndex].GuestMsrEntryAddress = mGuestContextCommonSmi.GuestContextPerCpu[0].GuestMsrEntryAddress + sizeof(VM_EXIT_MSR_ENTRY) * mGuestContextCommonSmi.GuestContextPerCpu[0].GuestMsrEntryCount * SubIndex;
    mGuestContextCommonSmm[SMI_HANDLER].GuestContextPerCpu[SubIndex].GuestMsrEntryAddress = mGuestContextCommonSmm[SMI_HANDLER].GuestContextPerCpu[0].GuestMsrEntryAddress + sizeof(VM_EXIT_MSR_ENTRY) * mGuestContextCommonSmm[SMI_HANDLER].GuestContextPerCpu[0].GuestMsrEntryCount * SubIndex;
  }

  DEBUG ((EFI_D_INFO, "DumpStmResource - %x\n", TxtProcessorSmmDescriptor->BiosHwResourceRequirementsPtr));
  DumpStmResource ((STM_RSC *)(UINTN)TxtProcessorSmmDescriptor->BiosHwResourceRequirementsPtr);
  DEBUG ((EFI_D_INFO, "RegisterBiosResource - %x\n", TxtProcessorSmmDescriptor->BiosHwResourceRequirementsPtr));
  RegisterBiosResource ((STM_RSC *)(UINTN)TxtProcessorSmmDescriptor->BiosHwResourceRequirementsPtr);

  InitStmHandlerSmi ();
  InitStmHandlerSmm ();

  InitPe();                    // Initialize protected execution
  //STM_PERF_INIT;

  //
  // Initialization done
  //

  mIsBspInitialized = TRUE;
   AsmWbinvd ();  // let everyone else know 
  return ;
}

/**

  This function initialize AP.

  @param Index    CPU index
  @param Register X86 register context

**/
VOID
ApInit (
  IN UINT32       Index,
  IN X86_REGISTER *Register
  )
{
  X86_REGISTER        *Reg;
  IA32_DESCRIPTOR IdtrLoad;

  while (!mIsBspInitialized) {
    //
    // Wait here
    //
  }

  DEBUG ((EFI_D_INFO, "%ld !!!Enter StmInit (AP done)!!! (%x)\n", (UINTN)Index, (UINTN)ReadUnaligned32 ((UINT32 *)&Register->Rax)));

  if (Index >= mHostContextCommon.CpuNum) {
    DEBUG ((EFI_D_INFO, "%ld !!!Index(0x%x) >= mHostContextCommon.CpuNum(0x%x)\n", Index, (UINTN)Index, (UINTN)mHostContextCommon.CpuNum));
    CpuDeadLoop ();
    Index = GetIndexFromStack (Register);
  }

  // do this here to make sure that we can handle a page fault
  IdtrLoad = mHostContextCommon.Idtr;
  AsmWriteIdtr(&IdtrLoad);  
  
  InterlockedIncrement (&mHostContextCommon.JoinedCpuNum);

  //DEBUG ((EFI_D_INFO, "%ld Register - %08x\n", (UINTN)Index, Register));
  Reg = &mGuestContextCommonSmi.GuestContextPerCpu[Index].Register;
  Register->Rsp = VmReadN (VMCS_N_GUEST_RSP_INDEX);
  CopyMem (Reg, Register, sizeof(X86_REGISTER));

  if (mHostContextCommon.JoinedCpuNum > mHostContextCommon.CpuNum) {
    DEBUG ((EFI_D_ERROR, "%ld JoinedCpuNum(%d) > CpuNum(%d)\n", Index, (UINTN)mHostContextCommon.JoinedCpuNum, (UINTN)mHostContextCommon.CpuNum));
    // Reset system
    CpuDeadLoop ();
  }

  return ;
}

/**

  This function initialize common part for BSP and AP.

  @param Index    CPU index

**/
VOID
CommonInit (
  IN UINT32  Index
  )
{
  UINTN                              StackBase;
  UINTN                              StackSize;
  STM_HEADER                         *StmHeader;
  UINT32                             RegEdx;
  IA32_VMX_MISC_MSR                  VmxMisc;

  AsmWriteCr4 (AsmReadCr4 () | CR4_OSFXSR | CR4_OSXMMEXCPT);
  if (IsXStateSupoprted()) {
    AsmWriteCr4 (AsmReadCr4 () | CR4_OSXSAVE);
  }

  VmxMisc.Uint64 = AsmReadMsr64 (IA32_VMX_MISC_MSR_INDEX);
  RegEdx = ReadUnaligned32 ((UINT32 *)&mGuestContextCommonSmi.GuestContextPerCpu[Index].Register.Rdx);
  if ((RegEdx & STM_CONFIG_SMI_UNBLOCKING_BY_VMX_OFF) != 0) {
    if (VmxMisc.Bits.VmxOffUnblockSmiSupport != 0) {
      AsmWriteMsr64 (IA32_SMM_MONITOR_CTL_MSR_INDEX, AsmReadMsr64(IA32_SMM_MONITOR_CTL_MSR_INDEX) | IA32_SMM_MONITOR_SMI_UNBLOCKING_BY_VMX_OFF);
    }
  }

  mHostContextCommon.HostContextPerCpu[Index].Index = Index;
  mHostContextCommon.HostContextPerCpu[Index].ApicId = (UINT8)ReadLocalApicId ();
  mHostContextCommon.HostContextPerCpu[Index].Vmxon = VmRead64(VMCS_64_CONTROL_EXECUTIVE_VMCS_PTR_INDEX);

  StmHeader = mHostContextCommon.StmHeader;
  StackBase = (UINTN)StmHeader + 
                     STM_PAGES_TO_SIZE (STM_SIZE_TO_PAGES (StmHeader->SwStmHdr.StaticImageSize)) + 
                     StmHeader->SwStmHdr.AdditionalDynamicMemorySize;
  StackSize = StmHeader->SwStmHdr.PerProcDynamicMemorySize;
  mHostContextCommon.HostContextPerCpu[Index].Stack = (UINTN)(StackBase + StackSize * (Index + 1)); // Stack Top

  if ((VmxMisc.Uint64 & BIT15) != 0) {
    mHostContextCommon.HostContextPerCpu[Index].Smbase = (UINT32)AsmReadMsr64 (IA32_SMBASE_INDEX);
  } else {
    mHostContextCommon.HostContextPerCpu[Index].Smbase = VmRead32 (VMCS_32_GUEST_SMBASE_INDEX);
  }
  mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor = (TXT_PROCESSOR_SMM_DESCRIPTOR *)(UINTN)(mHostContextCommon.HostContextPerCpu[Index].Smbase + SMM_TXTPSD_OFFSET);
  mGuestContextCommonSmm[SMI_HANDLER].GuestContextPerCpu[Index].Cr3 = (UINTN)mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->SmmCr3;
  mGuestContextCommonSmm[SMI_HANDLER].GuestContextPerCpu[Index].Efer = AsmReadMsr64 (IA32_EFER_MSR_INDEX);
  
  mGuestContextCommonSmi.GuestContextPerCpu[Index].Efer = AsmReadMsr64 (IA32_EFER_MSR_INDEX);
}

/**

  This function initialize VMCS.

  @param Index    CPU index

**/
VOID
VmcsInit (
  IN UINT32  Index
  )
{
  UINT64                             CurrentVmcs; 
  UINTN                              VmcsBase;
  UINT32                             VmcsSize;
  STM_HEADER                         *StmHeader;
  UINTN                              Rflags;

  StmHeader = mHostContextCommon.StmHeader;
	/* have to use Cr3Offset because StaticImageSize ignores BSS and Data sections */
  VmcsBase = (UINTN)StmHeader + 
                    //STM_PAGES_TO_SIZE (STM_SIZE_TO_PAGES (StmHeader->SwStmHdr.StaticImageSize)) + 
		    StmHeader->HwStmHdr.Cr3Offset + 
                    StmHeader->SwStmHdr.AdditionalDynamicMemorySize + 
                    StmHeader->SwStmHdr.PerProcDynamicMemorySize * mHostContextCommon.CpuNum;
  VmcsSize = GetVmcsSize();

  mGuestContextCommonSmi.GuestContextPerCpu[Index].Vmcs = (UINT64)(VmcsBase + VmcsSize * (Index * 2));
  mGuestContextCommonSmm[SMI_HANDLER].GuestContextPerCpu[Index].Vmcs = (UINT64)(VmcsBase + VmcsSize * (Index * 2 + 1));

  DEBUG ((EFI_D_INFO, "%d SmiVmcsPtr - %016lx\n", (UINTN)Index, mGuestContextCommonSmi.GuestContextPerCpu[Index].Vmcs));
  DEBUG ((EFI_D_INFO, "%d SmmVmcsPtr - %016lx\n", (UINTN)Index, mGuestContextCommonSmm[SMI_HANDLER].GuestContextPerCpu[Index].Vmcs));

  AsmVmPtrStore (&CurrentVmcs);
  DEBUG ((EFI_D_INFO, "%d CurrentVmcs - %016lx VmcsSize %x\n", (UINTN)Index, CurrentVmcs, VmcsSize));
  if (IsOverlap (CurrentVmcs, VmcsSize, mHostContextCommon.TsegBase, mHostContextCommon.TsegLength)) {
    // Overlap TSEG
    DEBUG ((EFI_D_ERROR, "%d CurrentVmcs violation - %016lx\n", (UINTN)Index, CurrentVmcs));
    DumpVmcsAllField(Index);
    CpuDeadLoop() ;
  }
  Rflags = AsmVmClear (&CurrentVmcs);
  if ((Rflags & (RFLAGS_CF | RFLAGS_ZF)) != 0) {
    DEBUG ((EFI_D_ERROR, "%d ERROR: AsmVmClear - %016lx : %08x\n", (UINTN)Index, CurrentVmcs, Rflags));
    CpuDeadLoop ();
  }

  CopyMem (
    (VOID *)(UINTN)mGuestContextCommonSmi.GuestContextPerCpu[Index].Vmcs,
    (VOID *)(UINTN)CurrentVmcs,
    (UINTN)VmcsSize
    );
  CopyMem (
    (VOID *)(UINTN)mGuestContextCommonSmm[SMI_HANDLER].GuestContextPerCpu[Index].Vmcs,
    (VOID *)(UINTN)CurrentVmcs,
    (UINTN)VmcsSize
    );

  *(UINT32 *)(UINTN)mGuestContextCommonSmm[SMI_HANDLER].GuestContextPerCpu[Index].Vmcs = (UINT32)AsmReadMsr64 (IA32_VMX_BASIC_MSR_INDEX) & 0xFFFFFFFF;

  AsmWbinvd ();

  Rflags = AsmVmPtrLoad (&mGuestContextCommonSmm[SMI_HANDLER].GuestContextPerCpu[Index].Vmcs);
  if ((Rflags & (RFLAGS_CF | RFLAGS_ZF)) != 0) {
    DEBUG ((EFI_D_ERROR, "%d ERROR: AsmVmPtrLoad - %016lx : %08x\n", (UINTN)Index, mGuestContextCommonSmm[SMI_HANDLER].GuestContextPerCpu[Index].Vmcs, Rflags));
    CpuDeadLoop ();
  }
  InitializeSmmVmcs (Index, &mGuestContextCommonSmm[SMI_HANDLER].GuestContextPerCpu[Index].Vmcs);
  Rflags = AsmVmClear (&mGuestContextCommonSmm[SMI_HANDLER].GuestContextPerCpu[Index].Vmcs);
  if ((Rflags & (RFLAGS_CF | RFLAGS_ZF)) != 0) {
    DEBUG ((EFI_D_ERROR, "%d ERROR: AsmVmClear - %016lx : %08x\n", (UINTN)Index, mGuestContextCommonSmm[SMI_HANDLER].GuestContextPerCpu[Index].Vmcs, Rflags));
    CpuDeadLoop ();
  }

  AsmWbinvd ();

  Rflags = AsmVmPtrLoad (&mGuestContextCommonSmi.GuestContextPerCpu[Index].Vmcs);
  if ((Rflags & (RFLAGS_CF | RFLAGS_ZF)) != 0) {
    DEBUG ((EFI_D_ERROR, "%d ERROR: AsmVmPtrLoad - %016lx : %08x\n", (UINTN)Index, mGuestContextCommonSmi.GuestContextPerCpu[Index].Vmcs, Rflags));
    CpuDeadLoop ();
  }
  InitializeSmiVmcs (Index, &mGuestContextCommonSmi.GuestContextPerCpu[Index].Vmcs);
}

/**

  This function launch back to MLE.

  @param Index    CPU index

**/
VOID
LaunchBack (
  IN UINT32  Index
  )
{
  UINTN             Rflags;
  X86_REGISTER      *Reg;

  Reg = &mGuestContextCommonSmi.GuestContextPerCpu[Index].Register;
#if 0
  //
  // Dump BIOS resource - already dumped
  //
  if ((Index == 0) && (ReadUnaligned32 ((UINT32 *)&Reg->Rax) == STM_API_INITIALIZE_PROTECTION)) {
    DEBUG ((EFI_D_INFO, "BIOS resource:\n"));
    DumpStmResource ((STM_RSC *)(UINTN)mHostContextCommon.HostContextPerCpu[0].TxtProcessorSmmDescriptor->BiosHwResourceRequirementsPtr);
  }
#endif
  if (ReadUnaligned32 ((UINT32 *)&Reg->Rax) == STM_API_START) {
    // We need do additional thing for STM_API_START
    mGuestContextCommonSmm[SMI_HANDLER].GuestContextPerCpu[Index].Actived = TRUE;
    SmmSetup (Index);
  }
  
  if(!IsResourceListValid ((STM_RSC *)(UINTN)mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->BiosHwResourceRequirementsPtr, FALSE)) {
    DEBUG ((EFI_D_INFO, "%ld LaunchBack - ValidateBiosResourceList fail!\n", Index));
    WriteUnaligned32 ((UINT32 *)&Reg->Rax, ERROR_STM_MALFORMED_RESOURCE_LIST);
    VmWriteN (VMCS_N_GUEST_RFLAGS_INDEX, VmReadN(VMCS_N_GUEST_RFLAGS_INDEX) | RFLAGS_CF);
  } else {
    WriteUnaligned32 ((UINT32 *)&Reg->Rax, STM_SUCCESS);
    VmWriteN (VMCS_N_GUEST_RFLAGS_INDEX, VmReadN(VMCS_N_GUEST_RFLAGS_INDEX) & ~RFLAGS_CF);
  }
  WriteUnaligned32 ((UINT32 *)&Reg->Rbx, 0); // Not support STM_RSC_BGM or STM_RSC_BGI or STM_RSC_MSR

  DEBUG ((EFI_D_INFO, "%ld !!!LaunchBack!!!\n", (UINTN)Index));

  if(Index != 0)
  {
	  // syncs the AP CPUS - all will wait until the BSP has completed setting up the API
      CpuReadySync(Index);
  }
  else
  {
     // DumpVmcsAllField();
  }
  AsmWbinvd();  // flush caches
  Rflags = AsmVmLaunch (Reg);

  AcquireSpinLock (&mHostContextCommon.DebugLock);
  DEBUG ((EFI_D_ERROR, "%ld !!!LaunchBack FAIL!!!\n", Index));
  DEBUG ((EFI_D_ERROR, "%ld Rflags: %08x\n", Index, Rflags));
  DEBUG ((EFI_D_ERROR, "%ld VMCS_32_RO_VM_INSTRUCTION_ERROR: %08x\n", Index, (UINTN)VmRead32 (VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX)));
  ReleaseSpinLock (&mHostContextCommon.DebugLock);

  CpuDeadLoop ();
}

/**

  This function initialize STM.

  @param Register X86 register context

**/

VOID
InitializeSmmMonitor (
  IN X86_REGISTER *Register
  )
{
  UINT32  Index;

  Index = GetIndexFromStack (Register);
  if (Index == 0) {
    // The build process should make sure "virtual address" is same as "file pointer to raw data",
    // in final PE/COFF image, so that we can let StmLoad load binrary to memory directly.
    // If no, GenStm tool will "load image". So here, we just need "relocate image"
    RelocateStmImage (FALSE);
    BspInit (Register);
  } else {
    Index = GetIndexFromStack (Register);
    ApInit (Index, Register);
  }

  CommonInit (Index);

  VmcsInit (Index);
   //
  AsmWbinvd();  // flush caches
  LaunchBack (Index);
  return ;
}

static unsigned int CpuSynched;

VOID InitCpuReadySync()
{
	CpuReadyCount = 0;   // count of CPUs waiting in the loop
	CpuSynched = 0;       // when 0 - not synched yet; when 1 CPUs are synched, but all have not exited the loop
					     // need to prevent CPUs from entering the loop until all CPUs have exited
	AsmWbinvd ();        // force it out
}

VOID CpuReadySync(UINT32 Index)
{
	while(InterlockedCompareExchange32(&CpuSynched, 1, 1) == 1 /*CpuSynched == 1*/) {}   // prevent processors from entering the synch loop until all the previous processors have left

    InterlockedIncrement(&CpuReadyCount);

    //DEBUG ((EFI_D_ERROR, "%ld CpuReadySync - CpuReadyCount: %d CpuNum %d\n", Index, CpuReadyCount, mHostContextCommon.CpuNum));
    while(InterlockedCompareExchange32(&CpuSynched, 0, 0) == 0)//( CpuReadyCount < mHostContextCommon.CpuNum)
    {
        // spin until all CPUs are synced

		if((InterlockedCompareExchange32(&CpuSynched, 0, 0) == 0 /*CpuSynched == 0*/) && 
			InterlockedCompareExchange32(&CpuReadyCount, mHostContextCommon.CpuNum, mHostContextCommon.CpuNum)== mHostContextCommon.CpuNum)
		{
			InterlockedCompareExchange32(&CpuSynched, 0, 1); //CpuSynched = 1;
			//DEBUG((EFI_D_ERROR, "%ld CpuReadySync - CpuSynched set to 1\n", Index));
		}
    }

	if(InterlockedDecrement(&CpuReadyCount) == 0)
	{
		InterlockedCompareExchange32(&CpuSynched, 1, 0); // CpuSynched = 0;
	//	DEBUG((EFI_D_ERROR, "%ld CpuReadySync - CpuSynched set to 0\n", Index));
	}
	
   // DEBUG((EFI_D_ERROR, "%ld CpuReadySync - Cpu Released - CpuReadyCount: %d, \n", Index, CpuReadyCount));//could cause problems
}

