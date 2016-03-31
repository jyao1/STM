/** @file
  SMM EPT handler

  Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmRuntime.h"

#define BUS_FROM_PCIE_ADDRESS(PcieAddress)       (UINT8)(((UINTN)(PcieAddress) & 0x0FF00000) >> 20)
#define DEVICE_FROM_PCIE_ADDRESS(PcieAddress)    (UINT8)(((UINTN)(PcieAddress) & 0x000F8000) >> 15)
#define FUNCTION_FROM_PCIE_ADDRESS(PcieAddress)  (UINT8)(((UINTN)(PcieAddress) & 0x00007000) >> 12)
#define REGISTER_FROM_PCIE_ADDRESS(PcieAddress)  (UINT16)((UINTN)(PcieAddress) & 0x00000FFF)

/**

  This function translate guest physical address to host address.

  @param EptPointer     EPT pointer
  @param Addr           Guest physical address
  @param EntryPtr       EPT entry pointer
                        NULL on output means Entry not found.

  @return Host physical address
**/
UINTN
TranslateEPTGuestToHost (
  IN UINT64      EptPointer,
  IN UINTN       Addr,
  OUT EPT_ENTRY  **EntryPtr  OPTIONAL
  );

/**

  This function translate guest physical address to host address.

  @param EptPointer           EPT pointer
  @param GuestPhysicalAddress Guest physical address
  @param HostPhysicalAddress  Host physical address

  @retval TRUE  HostPhysicalAddress is found
  @retval FALSE HostPhysicalAddress is not found
**/
BOOLEAN
LookupSmiGuestPhysicalToHostPhysical (
  IN  UINT64  EptPointer,
  IN  UINTN   GuestPhysicalAddress,
  OUT UINTN   *HostPhysicalAddress
  )
{
  EPT_ENTRY  *EptEntry;

  EptEntry = NULL;
  *HostPhysicalAddress = TranslateEPTGuestToHost (EptPointer, GuestPhysicalAddress, &EptEntry);
  if (EptEntry == NULL) {
    return FALSE;
  } else {
    return TRUE;
  }
}

/**

  This function translate guest linear address to host address.

  @param CpuIndex           CPU index
  @param GuestLinearAddress Guest linear address

  @return Host physical address
**/
UINTN
GuestLinearToHostPhysical (
  IN UINT32  CpuIndex,
  IN UINTN   GuestLinearAddress
  )
{
  UINTN   GuestPhysicalAddress;

  GuestPhysicalAddress = (UINTN)GuestLinearToGuestPhysical (CpuIndex, GuestLinearAddress);
  return TranslateEPTGuestToHost (mGuestContextCommonSmm.EptPointer.Uint64, GuestPhysicalAddress, NULL);
}

/**

  This function translate guest physical address to host address.

  @param EptPointer     EPT pointer
  @param Addr           Guest physical address
  @param EntryPtr       EPT entry pointer.
                        NULL on output means Entry not found.

  @return Host physical address
**/
UINTN
TranslateEPTGuestToHost (
  IN UINT64      EptPointer,
  IN UINTN       Addr,
  OUT EPT_ENTRY  **EntryPtr  OPTIONAL
  )
{
  EPT_ENTRY                *L1PageTable;
  EPT_ENTRY                *L2PageTable;
  EPT_ENTRY                *L3PageTable;
  EPT_ENTRY                *L4PageTable;
  UINTN                    Index1;
  UINTN                    Index2;
  UINTN                    Index3;
  UINTN                    Index4;
  UINTN                    Offset;

  // Assume 4G
  Index4 = ((UINTN)RShiftU64 (Addr, 39)) & 0x1ff;
  Index3 = ((UINTN)Addr >> 30) & 0x1ff;
  Index2 = ((UINTN)Addr >> 21) & 0x1ff;
  Index1 = ((UINTN)Addr >> 12) & 0x1ff;
  Offset = ((UINTN)Addr & 0xFFF);

  if (EntryPtr != NULL) {
    *EntryPtr = NULL;
  }
  L4PageTable = (EPT_ENTRY *)((UINTN)mGuestContextCommonSmm.EptPointer.Uint64 & ~0xFFF);
  if ((L4PageTable[Index4].Bits32.Ra == 0) &&
      (L4PageTable[Index4].Bits32.Wa == 0) &&
      (L4PageTable[Index4].Bits32.Xa == 0)) {
    return 0;
  }
  L3PageTable = (EPT_ENTRY *)((UINTN)L4PageTable[Index4].Uint64 & ~0xFFF);
  if ((L3PageTable[Index3].Bits32.Ra == 0) &&
      (L3PageTable[Index3].Bits32.Wa == 0) &&
      (L3PageTable[Index3].Bits32.Xa == 0)) {
    return 0;
  }
  L2PageTable = (EPT_ENTRY *)((UINTN)L3PageTable[Index3].Uint64 & ~0xFFF);
  if ((L2PageTable[Index2].Bits32.Ra == 0) &&
      (L2PageTable[Index2].Bits32.Wa == 0) &&
      (L2PageTable[Index2].Bits32.Xa == 0)) {
    return 0;
  }

  if (L2PageTable[Index2].Bits32.Sp == 1) {
    if (EntryPtr != NULL) {
      *EntryPtr = &L2PageTable[Index2];
    }
    return ((UINTN)L2PageTable[Index2].Uint64 & ~0x1FFFFF) + ((UINTN)Addr & 0x1FFFFF);
  }

  L1PageTable = (EPT_ENTRY *)((UINTN)L2PageTable[Index2].Uint64 & ~0xFFF);
  if ((L1PageTable[Index1].Bits32.Ra == 0) &&
      (L1PageTable[Index1].Bits32.Wa == 0) &&
      (L1PageTable[Index1].Bits32.Xa == 0)) {
    // not check last one, since user may update it
//    return 0;
  }

  if (EntryPtr != NULL) {
    *EntryPtr = &L1PageTable[Index1];
  }
  return ((UINTN)L1PageTable[Index1].Uint64 & ~0xFFF) + Offset;
}

/**

  This function set EPT page table attribute by address.

  @param GuestPhysicalAddr        Guest physical address
  @param Sp                       Super page
  @param Ra                       Read access
  @param Wa                       Write access
  @param Xa                       Execute access
  @param EptPageAttributeSetting  EPT page attribute setting

**/
VOID
EPTSetPageAttribute (
  IN UINT64                     GuestPhysicalAddr,
  IN UINT32                     Sp,
  IN UINT32                     Ra,
  IN UINT32                     Wa,
  IN UINT32                     Xa,
  IN EPT_PAGE_ATTRIBUTE_SETTING EptPageAttributeSetting
  )
{
  EPT_ENTRY  *EptEntry;
  UINT_128   Data128;
  EPT_ENTRY  *L1PageTable;
  UINTN      Index1;
  EPT_ENTRY  *L2PageTable;
  UINT64     BaseAddress;

  EptEntry = NULL;
  TranslateEPTGuestToHost (mGuestContextCommonSmm.EptPointer.Uint64, (UINTN)GuestPhysicalAddr, &EptEntry);
  if (EptEntry == NULL) {
    DEBUG ((EFI_D_ERROR, "!!!EPTSetPageAttribute fail!!! Addr - %x, Sp - %x, Ra - %x, Wa - %x, Xa - %x\n", (UINTN)GuestPhysicalAddr, (UINTN)Sp, (UINTN)Ra, (UINTN)Wa, (UINTN)Xa));
    CpuDeadLoop ();
    return ;
  }

  if (Sp == 1) {
    //
    // Super page
    //
    if (EptEntry->Bits32.Sp == 1) {
      switch (EptPageAttributeSetting) {
      case EptPageAttributeSet:
        EptEntry->Bits32.Ra = (UINT32)Ra;
        EptEntry->Bits32.Wa = (UINT32)Wa;
        EptEntry->Bits32.Xa = (UINT32)Xa;
        break;
      case EptPageAttributeAnd:
        EptEntry->Bits32.Ra &= (UINT32)Ra;
        EptEntry->Bits32.Wa &= (UINT32)Wa;
        EptEntry->Bits32.Xa &= (UINT32)Xa;
        break;
      case EptPageAttributeOr:
        EptEntry->Bits32.Ra |= (UINT32)Ra;
        EptEntry->Bits32.Wa |= (UINT32)Wa;
        EptEntry->Bits32.Xa |= (UINT32)Xa;
        break;
      default:
        CpuDeadLoop ();
        break;
      }
    } else {
      for (BaseAddress = GuestPhysicalAddr; BaseAddress < GuestPhysicalAddr + SIZE_2MB; BaseAddress += SIZE_4KB) {
        EPTSetPageAttribute (BaseAddress, 0, Ra, Wa, Xa, EptPageAttributeSetting);
      }
    }

    return ;
  }

  if (EptEntry->Bits32.Sp == 1) {
    L2PageTable = EptEntry;
    BaseAddress = L2PageTable->Uint64 & ~0x1FFFFF;

    L1PageTable = (EPT_ENTRY *)AllocatePages (1);

    for (Index1 = 0; Index1 < 512; Index1 ++) {
      L1PageTable->Uint64       = BaseAddress;
      L1PageTable->Bits32.Ra    = EptEntry->Bits32.Ra;
      L1PageTable->Bits32.Wa    = EptEntry->Bits32.Wa;
      L1PageTable->Bits32.Xa    = EptEntry->Bits32.Xa;
      L1PageTable->Bits32.Emt   = EptEntry->Bits32.Emt;

      if (BaseAddress == (GuestPhysicalAddr & ~0xFFF)) {
        EptEntry = L1PageTable;
      }

      BaseAddress += SIZE_4KB;
      L1PageTable ++;
    }

    L2PageTable->Uint64       = (UINT64)((UINTN)L1PageTable - SIZE_4KB);
    L2PageTable->Bits32.Ra    = 1;
    L2PageTable->Bits32.Wa    = 1;
    L2PageTable->Bits32.Xa    = 1;
  }
  switch (EptPageAttributeSetting) {
  case EptPageAttributeSet:
    EptEntry->Bits32.Ra = (UINT32)Ra;
    EptEntry->Bits32.Wa = (UINT32)Wa;
    EptEntry->Bits32.Xa = (UINT32)Xa;
    break;
  case EptPageAttributeAnd:
    EptEntry->Bits32.Ra &= (UINT32)Ra;
    EptEntry->Bits32.Wa &= (UINT32)Wa;
    EptEntry->Bits32.Xa &= (UINT32)Xa;
    break;
  case EptPageAttributeOr:
    EptEntry->Bits32.Ra |= (UINT32)Ra;
    EptEntry->Bits32.Wa |= (UINT32)Wa;
    EptEntry->Bits32.Xa |= (UINT32)Xa;
    break;
  default:
    CpuDeadLoop ();
    break;
  }

  Data128.Lo = mGuestContextCommonSmm.EptPointer.Uint64;
  Data128.Hi = 0;
  AsmInvEpt (INVEPT_TYPE_SINGLE_CONTEXT_INVALIDATION, &Data128);
  return ;
}

/**

  This function set EPT page table attribute by range.

  @param Base                     Memory base
  @param Length                   Memory length
  @param Ra                       Read access
  @param Wa                       Write access
  @param Xa                       Execute access
  @param EptPageAttributeSetting  EPT page attribute setting

**/
VOID
EPTSetPageAttributeRange (
  IN UINT64                     Base,
  IN UINT64                     Length,
  IN UINT32                     Ra,
  IN UINT32                     Wa,
  IN UINT32                     Xa,
  IN EPT_PAGE_ATTRIBUTE_SETTING EptPageAttributeSetting
  )
{
  UINT64   Address;

//  DEBUG ((EFI_D_INFO, "EPTSetPageAttributeRange - 0x%016lx - 0x%016lx\n", Base, Length));

  for (Address = Base; Address < Base + Length; ) {
    if (((Address & (SIZE_2MB - 1)) == 0) &&
        ((Base <= Address + SIZE_2MB) && (Address + SIZE_2MB < Base + Length))) {
      EPTSetPageAttribute (Address, 1, Ra, Wa, Xa, EptPageAttributeSetting);
      Address += SIZE_2MB;
    } else {
      EPTSetPageAttribute (Address, 0, Ra, Wa, Xa, EptPageAttributeSetting);
      Address += SIZE_4KB;
    }
  }
}

/**

  This function is EPT violation handler for SMM.

  @param Index CPU index

**/
VOID
SmmEPTViolationHandler (
  IN UINT32 Index
  )
{
  VM_EXIT_QUALIFICATION   Qualification;
  STM_RSC_MEM_DESC        *MemDesc;
  UINT64                  Address;
  STM_RSC_PCI_CFG_DESC    *PciCfgDesc;
  UINT64                  PciExpressAddress;
  STM_RSC_MEM_DESC        LocalMemDesc;
  STM_RSC_PCI_CFG_DESC    *LocalPciCfgDescPtr;
  UINT8                   LocalPciCfgDescBuf[STM_LOG_ENTRY_SIZE];

  Qualification.UintN = VmReadN (VMCS_N_RO_EXIT_QUALIFICATION_INDEX);

  DEBUG ((EFI_D_ERROR, "!!!EPTViolationHandler (%d)!!!\n", (UINTN)Index));
  DEBUG ((EFI_D_ERROR, "Qualification - %016lx\n", (UINT64)Qualification.UintN));
  DEBUG ((EFI_D_ERROR, "GuestPhysicalAddress - %016lx\n", VmRead64 (VMCS_64_RO_GUEST_PHYSICAL_ADDR_INDEX)));

  if (Qualification.EptViolation.GlaValid == 0) {
    //
    // 0=Linear address invalid.
    //
  } else {
    if (Qualification.EptViolation.Gpa == 0) {
      //
      // 1=Linear address valid but does not match provided physical address. EPT violation occurred while performing a guest page walk.
      //   1) No-read EPT page encountered when trying to read from the guest IA32 page tables (e.g fetching a PML4, PDE, PTE).
      //   2) No-write EPT page encountered when trying to write an A or D bit.
      //
    } else {
      //
      // 3=Linear address valid and match provided physical address. This is the normal case.
      //
      Address = VmRead64 (VMCS_64_RO_GUEST_PHYSICAL_ADDR_INDEX);
      MemDesc = GetStmResourceMem (
                  mHostContextCommon.MleProtectedResource.Base,
                  Address,
                  (UINT32)(Qualification.UintN & 0x7)
                  );
      if (MemDesc != NULL) {
        DEBUG ((EFI_D_ERROR, "EPT violation!\n"));
        AddEventLogForResource (EvtHandledProtectionException, (STM_RSC *)MemDesc);
        SmmExceptionHandler (Index);
        CpuDeadLoop ();
      }
      
      MemDesc = GetStmResourceMem (
                  (STM_RSC *)(UINTN)mGuestContextCommonSmm.BiosHwResourceRequirementsPtr,
                  Address,
                  (UINT32)(Qualification.UintN & 0x7)
                  );
      if (MemDesc == NULL) {
        DEBUG((EFI_D_ERROR, "Add unclaimed MEM_RSC!\n"));
        ZeroMem (&LocalMemDesc, sizeof(LocalMemDesc));
        LocalMemDesc.Hdr.RscType = MEM_RANGE;
        LocalMemDesc.Hdr.Length = sizeof(LocalMemDesc);
        LocalMemDesc.Base = Address;
        LocalMemDesc.Length = 1;
        LocalMemDesc.RWXAttributes = (UINT8)(Qualification.UintN & 0x7);
        AddEventLogForResource (EvtBiosAccessToUnclaimedResource, (STM_RSC *)&LocalMemDesc);
        // BUGBUG: it should not happen?
        // TBD: We need create EPT mapping here, if so?
        CpuDeadLoop ();
      }

      // Check PCIE MMIO.
      if ((mHostContextCommon.PciExpressBaseAddress != 0) &&
          (Address >= mHostContextCommon.PciExpressBaseAddress) &&
          (Address < (mHostContextCommon.PciExpressBaseAddress + mHostContextCommon.PciExpressLength))) {
        PciExpressAddress = Address - mHostContextCommon.PciExpressBaseAddress;
        PciCfgDesc = GetStmResourcePci (
                       mHostContextCommon.MleProtectedResource.Base,
                       BUS_FROM_PCIE_ADDRESS(PciExpressAddress),
                       DEVICE_FROM_PCIE_ADDRESS(PciExpressAddress),
                       FUNCTION_FROM_PCIE_ADDRESS(PciExpressAddress),
                       REGISTER_FROM_PCIE_ADDRESS(PciExpressAddress),
                       (UINT8)(Qualification.UintN & 0x3)
                       );
        if (PciCfgDesc != NULL) {
          DEBUG ((EFI_D_ERROR, "EPT (PCIE) violation!\n"));
          AddEventLogForResource (EvtHandledProtectionException, (STM_RSC *)PciCfgDesc);
          SmmExceptionHandler (Index);
          CpuDeadLoop ();
        }

        PciCfgDesc = GetStmResourcePci (
                       (STM_RSC *)(UINTN)mGuestContextCommonSmm.BiosHwResourceRequirementsPtr,
                       BUS_FROM_PCIE_ADDRESS(PciExpressAddress),
                       DEVICE_FROM_PCIE_ADDRESS(PciExpressAddress),
                       FUNCTION_FROM_PCIE_ADDRESS(PciExpressAddress),
                       REGISTER_FROM_PCIE_ADDRESS(PciExpressAddress),
                       (UINT8)(Qualification.UintN & 0x3)
                       );
        if (PciCfgDesc == NULL) {
          DEBUG((EFI_D_ERROR, "Add unclaimed PCIE_RSC!\n"));
          LocalPciCfgDescPtr = (STM_RSC_PCI_CFG_DESC *)LocalPciCfgDescBuf;
          ZeroMem (LocalPciCfgDescBuf, sizeof(LocalPciCfgDescBuf));
          LocalPciCfgDescPtr->Hdr.RscType = PCI_CFG_RANGE;
          LocalPciCfgDescPtr->Hdr.Length = sizeof(STM_RSC_PCI_CFG_DESC); // BUGBUG: Just report this PCI device, it is hard to create PCI hierachy here.
          LocalPciCfgDescPtr->RWAttributes = (UINT8)(Qualification.UintN & 0x3);
          LocalPciCfgDescPtr->Base = REGISTER_FROM_PCIE_ADDRESS(PciExpressAddress);
          LocalPciCfgDescPtr->Length = 1;
          LocalPciCfgDescPtr->OriginatingBusNumber = BUS_FROM_PCIE_ADDRESS(PciExpressAddress);
          LocalPciCfgDescPtr->LastNodeIndex = 0;
          LocalPciCfgDescPtr->PciDevicePath[0].Type = 1;
          LocalPciCfgDescPtr->PciDevicePath[0].Subtype = 1;
          LocalPciCfgDescPtr->PciDevicePath[0].Length = sizeof(STM_PCI_DEVICE_PATH_NODE);
          LocalPciCfgDescPtr->PciDevicePath[0].PciFunction = FUNCTION_FROM_PCIE_ADDRESS(PciExpressAddress);
          LocalPciCfgDescPtr->PciDevicePath[0].PciDevice = DEVICE_FROM_PCIE_ADDRESS(PciExpressAddress);
          AddEventLogForResource (EvtBiosAccessToUnclaimedResource, (STM_RSC *)LocalPciCfgDescPtr);
        }

      }
    }
  }

  VmWriteN (VMCS_N_GUEST_RIP_INDEX, VmReadN(VMCS_N_GUEST_RIP_INDEX) + VmRead32(VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX));

  return ;
}

/**

  This function is EPT misconfiguration handler for SMM.

  @param Index CPU index

**/
VOID
SmmEPTMisconfigurationHandler (
  IN UINT32  Index
  )
{
  //
  // Should not happen
  //
  DEBUG ((EFI_D_ERROR, "!!!EPTMisconfigurationHandler!!!\n"));
  DumpVmcsAllField ();

  CpuDeadLoop ();

  return ;
}

/**

  This function is INVEPT handler for SMM.

  @param Index CPU index

**/
VOID
SmmInvEPTHandler (
  IN UINT32  Index
  )
{
  DEBUG ((EFI_D_ERROR, "!!!InvEPTHandler!!!\n"));
  DumpVmcsAllField ();

  CpuDeadLoop ();

  return ;
}

/**

  This function sync Ia32PAE page table for EPT.

  @param Index CPU index

**/
VOID
Ia32PAESync (
  IN UINT32  Index
  )
{
  UINTN              Cr0;
  UINTN              Cr3;
  UINTN              Cr4;

  //
  // If EPT is enabled and Guest is in IA32 PAE Mode, we need to write PDPTR.
  //
  Cr0 = VmReadN (VMCS_N_GUEST_CR0_INDEX);
  Cr3 = VmReadN (VMCS_N_GUEST_CR3_INDEX);
  Cr4 = VmReadN (VMCS_N_GUEST_CR4_INDEX);
  if (((Cr4 & CR4_PAE) != 0) &&
      ((Cr0 & CR0_PG) != 0) &&
      ((mGuestContextCommonSmm.GuestContextPerCpu[Index].Efer & IA32_EFER_MSR_MLA) == 0)) {
    VmWrite64 (VMCS_64_GUEST_PDPTE0_INDEX, *(UINT64 *)(Cr3 + sizeof(UINT64) * 0));
    VmWrite64 (VMCS_64_GUEST_PDPTE1_INDEX, *(UINT64 *)(Cr3 + sizeof(UINT64) * 1));
    VmWrite64 (VMCS_64_GUEST_PDPTE2_INDEX, *(UINT64 *)(Cr3 + sizeof(UINT64) * 2));
    VmWrite64 (VMCS_64_GUEST_PDPTE3_INDEX, *(UINT64 *)(Cr3 + sizeof(UINT64) * 3));
  }

  return ;
}