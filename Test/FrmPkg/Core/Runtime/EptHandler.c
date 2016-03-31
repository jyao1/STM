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
#include "FrmHandler.h"

#define PAGING_4K_MASK  0xFFF
#define PAGING_4M_MASK  0x3FFFFF
#define PAGING_2M_MASK  0x1FFFFF

#define PAGING_INDEX_MASK      0x3FF
#define PAGING_PAE_INDEX_MASK  0x1FF

#ifdef MDE_CPU_X64
#define PAGING_4K_ADDRESS_MASK_64 0x000FFFFFFFFFF000u
#define PAGING_2M_ADDRESS_MASK_64 0x000FFFFFFFE00000u
#else
#define PAGING_4K_ADDRESS_MASK_64 0x00000000FFFFF000u
#define PAGING_2M_ADDRESS_MASK_64 0x00000000FFE00000u
#endif

#define PAGING_4K_ADDRESS_MASK_32 0xFFFFF000u
#define PAGING_4M_ADDRESS_MASK_32 0xFFC00000u

/**

This function translate guest physical address to host address.

  @param Addr           Guest physical address
  @param EntryPtr       EPT entry pointer
                        NULL on output means Entry not found.

  @return Host physical address

**/
UINTN
TranslateEPTGuestToHost (
  IN UINTN       Addr,
  OUT EPT_ENTRY  **EntryPtr  OPTIONAL
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
  )
{
  UINT64                *L1PageTable;
  UINT64                *L2PageTable;
  UINT64                *L3PageTable;
#ifdef MDE_CPU_X64
  UINT64                *L4PageTable;
#endif
  UINT32                *L1PageTable32;
  UINT32                *L2PageTable32;
  UINTN                 Index1;
  UINTN                 Index2;
  UINTN                 Index3;
#ifdef MDE_CPU_X64
  UINTN                 Index4;
#endif
  UINTN                 Offset;
  UINTN                 PhysicalAddress;

  if (mGuestContextCommon.GuestContextPerCpu[CpuIndex].EFER & IA32_EFER_MSR_MLA) {
#ifdef MDE_CPU_X64
    // x64 paging
    Index4 = ((UINTN)GuestVirtualAddress >> 39) & PAGING_PAE_INDEX_MASK;
    Index3 = ((UINTN)GuestVirtualAddress >> 30) & PAGING_PAE_INDEX_MASK;
    Index2 = ((UINTN)GuestVirtualAddress >> 21) & PAGING_PAE_INDEX_MASK;
    Index1 = ((UINTN)GuestVirtualAddress >> 12) & PAGING_PAE_INDEX_MASK;
    Offset = ((UINTN)GuestVirtualAddress & PAGING_4K_MASK);

    L4PageTable = (UINT64 *)VmReadN (VMCS_N_GUEST_CR3_INDEX);
    if ((L4PageTable[Index4] & IA32_PG_P) == 0) {
      return 0;
    }

    L3PageTable = (UINT64 *)(UINTN)(L4PageTable[Index4] & PAGING_4K_ADDRESS_MASK_64);
    if ((L3PageTable[Index3] & IA32_PG_P) == 0) {
      return 0;
    }

    L2PageTable = (UINT64 *)(UINTN)(L3PageTable[Index3] & PAGING_4K_ADDRESS_MASK_64);
    if ((L2PageTable[Index2] & IA32_PG_P) == 0) {
      return 0;
    }

    // PS
    if ((L2PageTable[Index2] & IA32_PG_PS) != 0) {
      // 2M 
      Offset = (UINTN)GuestVirtualAddress & PAGING_2M_MASK;
      PhysicalAddress = L2PageTable[Index2] & PAGING_2M_ADDRESS_MASK_64;
      return PhysicalAddress + Offset;
    } else {
      // 4k
      L1PageTable = (UINT64 *)(UINTN)(L2PageTable[Index2] & PAGING_4K_ADDRESS_MASK_64);
      if ((L1PageTable[Index1] & IA32_PG_P) == 0) {
        return 0;
      }
      PhysicalAddress = L1PageTable[Index1] & PAGING_4K_ADDRESS_MASK_64;
      return PhysicalAddress + Offset;
    }
#else
    return 0;
#endif
  } else {
    // ia32 paging
    if (mGuestContextCommon.GuestContextPerCpu[CpuIndex].Cr0 & CR0_PG) {
      if (mGuestContextCommon.GuestContextPerCpu[CpuIndex].Cr4 & CR4_PAE) {
        // PAE
        Index3 = ((UINT32)GuestVirtualAddress >> 30) & 0x3;
        Index2 = ((UINT32)GuestVirtualAddress >> 21) & PAGING_PAE_INDEX_MASK;
        Index1 = ((UINT32)GuestVirtualAddress >> 12) & PAGING_PAE_INDEX_MASK;
        Offset = ((UINT32)GuestVirtualAddress & PAGING_4K_MASK);

        L3PageTable = (UINT64 *)VmReadN (VMCS_N_GUEST_CR3_INDEX);
        if ((L3PageTable[Index3] & IA32_PG_P) == 0) {
          return 0;
        }

        L2PageTable = (UINT64 *)(UINTN)(L3PageTable[Index3] & PAGING_4K_ADDRESS_MASK_64);
        if ((L2PageTable[Index2] & IA32_PG_P) == 0) {
          return 0;
        }

        // PS
        if ((L2PageTable[Index2] & IA32_PG_PS) != 0) {
          // 2M 
          Offset = (UINTN)GuestVirtualAddress & PAGING_2M_MASK;
          PhysicalAddress = (UINTN)L2PageTable[Index2] & PAGING_2M_ADDRESS_MASK_64;
          return PhysicalAddress + Offset;
        } else {
          // 4k
          L1PageTable = (UINT64 *)(UINTN)(L2PageTable[Index2] & PAGING_4K_ADDRESS_MASK_64);
          if ((L1PageTable[Index1] & IA32_PG_P) == 0) {
            return 0;
          }
          PhysicalAddress = (UINTN)L1PageTable[Index1] & PAGING_4K_ADDRESS_MASK_64;
          return PhysicalAddress + Offset;
        }
      } else {
        // Non-PAE
        Index2 = ((UINT32)GuestVirtualAddress >> 22) & PAGING_INDEX_MASK;
        Index1 = ((UINT32)GuestVirtualAddress >> 12) & PAGING_INDEX_MASK;
        Offset = ((UINT32)GuestVirtualAddress & PAGING_4K_MASK);

        L2PageTable32 = (UINT32 *)VmReadN (VMCS_N_GUEST_CR3_INDEX);
        if ((L2PageTable32[Index2] & IA32_PG_P) == 0) {
          return 0;
        }

        // PS
        if ((L2PageTable32[Index2] & IA32_PG_PS) != 0) {
          // 4M 
          Offset = (UINT32)GuestVirtualAddress & PAGING_4M_MASK;
          PhysicalAddress = (UINTN)L2PageTable32[Index2] & PAGING_4M_ADDRESS_MASK_32;
          return PhysicalAddress + Offset;
        } else {
          // 4k
          L1PageTable32 = (UINT32 *)(UINTN)(L2PageTable32[Index2] & PAGING_4K_ADDRESS_MASK_32);
          if ((L1PageTable32[Index1] & IA32_PG_P) == 0) {
            return 0;
          }
          PhysicalAddress = (UINTN)L1PageTable32[Index1] & PAGING_4K_ADDRESS_MASK_32;
          return PhysicalAddress + Offset;
        }
      } // end of Non-PAE
    } // end of ia32 page
  }

  // one-one mapping
  return GuestVirtualAddress;
}

/**

  This function convert guest physical address to host physical address.

  @param GuestPhysicalAddress Guest physical address

  @return host physical address

**/
UINTN
GuestPhysicallToHostPhysical (
  UINTN   GuestPhysicalAddress
  )
{
  return TranslateEPTGuestToHost (GuestPhysicalAddress, NULL);
}

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
  )
{
  UINTN   GuestPhysicalAddress;

  GuestPhysicalAddress = GuestVirtualToGuestPhysical (CpuIndex, GuestVirtualAddress);
  return GuestPhysicallToHostPhysical (GuestPhysicalAddress);
}

/**

  This function translate guest physical address to host address.

  @param Addr           Guest physical address
  @param EntryPtr       EPT entry pointer
                        NULL on output means Entry not found.

  @return Host physical address

**/
UINTN
TranslateEPTGuestToHost (
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

  L4PageTable = (EPT_ENTRY *)((UINTN)mGuestContextCommon.EptPointer.Uint64 & PAGING_4K_ADDRESS_MASK_64);
  if ((L4PageTable[Index4].Bits32.Ra == 0) &&
      (L4PageTable[Index4].Bits32.Wa == 0) &&
      (L4PageTable[Index4].Bits32.Xa == 0)) {
    return 0;
  }
  L3PageTable = (EPT_ENTRY *)((UINTN)L4PageTable[Index4].Uint64 & PAGING_4K_ADDRESS_MASK_64);
  if ((L3PageTable[Index3].Bits32.Ra == 0) &&
      (L3PageTable[Index3].Bits32.Wa == 0) &&
      (L3PageTable[Index3].Bits32.Xa == 0)) {
    return 0;
  }
  L2PageTable = (EPT_ENTRY *)((UINTN)L3PageTable[Index3].Uint64 & PAGING_4K_ADDRESS_MASK_64);
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

  L1PageTable = (EPT_ENTRY *)((UINTN)L2PageTable[Index2].Uint64 & PAGING_4K_ADDRESS_MASK_64);
  if ((L1PageTable[Index1].Bits32.Ra == 0) &&
      (L1PageTable[Index1].Bits32.Wa == 0) &&
      (L1PageTable[Index1].Bits32.Xa == 0)) {
    // not check last one, since user may update it
//    return 0;
  }

  if (EntryPtr != NULL) {
    *EntryPtr = &L1PageTable[Index1];
  }
  return ((UINTN)L1PageTable[Index1].Uint64 & PAGING_4K_ADDRESS_MASK_64) + Offset;
}

/**

  This function set EPT page table attribute by address.

  @param GuestPhysicalAddr        Memory base
  @param Ra                       Read access
  @param Wa                       Write access
  @param Xa                       Execute access

**/
VOID
EPTSetPageAttribute (
  IN UINTN  GuestPhysicalAddr,
  IN UINTN  Ra,
  IN UINTN  Wa,
  IN UINTN  Xa
  )
{
  EPT_ENTRY  *EptEntry;
  UINT_128   Data128;

  if (TranslateEPTGuestToHost (GuestPhysicalAddr, &EptEntry) == 0) {
    DEBUG ((EFI_D_ERROR, "(FRM) !!!EPTSetPageAttribute fail!!!\n"));
    CpuDeadLoop ();
  }

  EptEntry->Bits32.Ra = (UINT32)Ra;
  EptEntry->Bits32.Wa = (UINT32)Wa;
  EptEntry->Bits32.Xa = (UINT32)Xa;

  Data128.Lo = mGuestContextCommon.EptPointer.Uint64;
  Data128.Hi = 0;
  AsmInvEpt (INVEPT_TYPE_SINGLE_CONTEXT_INVALIDATION, &Data128);
  return ;
}

/**

  This function is EPT violation handler.

  @param Index CPU index

**/
VOID
EPTViolationHandler (
  IN UINT32 Index
  )
{
  VM_EXIT_QUALIFICATION   Qualification;

  Qualification.UintN = VmReadN (VMCS_N_RO_EXIT_QUALIFICATION_INDEX);

  DEBUG ((EFI_D_ERROR, "(FRM) !!!EPTViolationHandler!!!\n"));
  DEBUG ((EFI_D_ERROR, "(FRM) Qualification - %08x\n", Qualification.UintN));
  DEBUG ((EFI_D_ERROR, "(FRM) GuestPhysicalAddress - %016lx\n", VmRead64 (VMCS_64_RO_GUEST_PHYSICAL_ADDR_INDEX)));

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
    }
  }

  VmWriteN (VMCS_N_GUEST_RIP_INDEX, VmReadN(VMCS_N_GUEST_RIP_INDEX) + VmRead32(VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX));

  return ;
}

/**

  This function is EPT misconfiguration handler.

  @param Index CPU index

**/
VOID
EPTMisconfigurationHandler (
  IN  UINT32  Index
  )
{
  //
  // Should not happen
  //
  DEBUG ((EFI_D_ERROR, "(FRM) !!!EPTMisconfigurationHandler!!!\n"));
  DumpVmcsAllField ();

  CpuDeadLoop ();

  return ;
}

/**

  This function is INVEPT handler.

  @param Index CPU index

**/
VOID
InvEPTHandler (
  IN UINT32  Index
  )
{
  DEBUG ((EFI_D_ERROR, "(FRM) !!!InvEPTHandler!!!\n"));
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
      ((VmRead64 (VMCS_64_GUEST_IA32_EFER_INDEX) & IA32_EFER_MSR_MLA) == 0)) {
    VmWrite64 (VMCS_64_GUEST_PDPTE0_INDEX, *(UINT64 *)(Cr3 + sizeof(UINT64) * 0));
    VmWrite64 (VMCS_64_GUEST_PDPTE1_INDEX, *(UINT64 *)(Cr3 + sizeof(UINT64) * 1));
    VmWrite64 (VMCS_64_GUEST_PDPTE2_INDEX, *(UINT64 *)(Cr3 + sizeof(UINT64) * 2));
    VmWrite64 (VMCS_64_GUEST_PDPTE3_INDEX, *(UINT64 *)(Cr3 + sizeof(UINT64) * 3));
  }

  return ;
}