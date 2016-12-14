/** @file
  EPT initialization

  Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmInit.h"

MRTT_INFO  mMtrrInfo;

typedef struct {
  UINT32  Base;
  UINT32  Stepping;
} FIXED_MTRR_STRUCT_INFO;

FIXED_MTRR_STRUCT_INFO mFixedMtrrStructInfo[] = {
  {0x00000, SIZE_64KB},
  {0x80000, SIZE_16KB},
  {0xA0000, SIZE_16KB},
  {0xC0000, SIZE_4KB},
  {0xC8000, SIZE_4KB},
  {0xD0000, SIZE_4KB},
  {0xD8000, SIZE_4KB},
  {0xE0000, SIZE_4KB},
  {0xE8000, SIZE_4KB},
  {0xF0000, SIZE_4KB},
  {0xF8000, SIZE_4KB},
};

/**

  This function dump EPT page table.

  @param EptPointer EPT pointer

**/
VOID
EptDumpPageTable (
  IN EPT_POINTER              *EptPointer
  );

/**

  This function return TSEG information in TXT heap region.

  @param  TsegBase    TSEG base address
  @param  TsegLength  TSEG length

  @return TsegBase

**/
UINT64
GetTsegInfoFromTxt (
  OUT UINT64  *TsegBase,
  OUT UINT64  *TsegLength
  )
{
  TXT_SINIT_TO_MLE_DATA               *SinitToMleData;
  TXT_SINIT_MEMORY_DESCRIPTOR_RECORD  *SinitMemoryDescriptor;
  UINTN                               Index;

  SinitToMleData = GetTxtSinitToMleData ();
  SinitMemoryDescriptor = (TXT_SINIT_MEMORY_DESCRIPTOR_RECORD *)((UINTN)SinitToMleData - sizeof(UINT64) + SinitToMleData->SinitMdrTableOffset);
  for (Index = 0; Index < SinitToMleData->NumberOfSinitMdrs; Index++) {
    if (SinitMemoryDescriptor[Index].Type == TXT_SINIT_MDR_TYPE_NON_OVERLAYED_SMRAM) {
      *TsegBase = SinitMemoryDescriptor[Index].Address;
      *TsegLength = SinitMemoryDescriptor[Index].Length;
      return SinitMemoryDescriptor[Index].Address;
    }
  }

  *TsegBase = 0;
  *TsegLength = 0;
  return 0;
}

/**

  This function return TSEG information from SMRR.

  @param  TsegBase    TSEG base address
  @param  TsegLength  TSEG length

  @return TsegBase

**/
UINT64
GetTsegInfoFromSmrr (
  OUT UINT64  *TsegBase,
  OUT UINT64  *TsegLength
  )
{
  UINT32                        SmrrBase;
  UINT32                        SmrrLength;

  SmrrBase = (UINT32)mMtrrInfo.SmrrBase & (UINT32)mMtrrInfo.SmrrMask & 0xFFFFF000;
  SmrrLength = (UINT32)mMtrrInfo.SmrrMask & 0xFFFFF000;
  SmrrLength = ~SmrrLength + 1;

  *TsegBase = SmrrBase;
  *TsegLength = SmrrLength;

  return SmrrBase;
}

/**

  This function return FixedMtrr struct information according to BaseAddress.

  @param BaseAddress Base address < 1MB

  @return FixedMtrr struct index

**/
UINT32
GetFixedMtrrStructInfo (
  IN UINT32  BaseAddress
  )
{
  INT32  Index;

  ASSERT (BaseAddress < BASE_1MB);

  for (Index = sizeof(mFixedMtrrStructInfo)/sizeof(mFixedMtrrStructInfo[0]) - 1; Index >= 0; Index--) {
    if (BaseAddress >= mFixedMtrrStructInfo[Index].Base) {
      return (UINT32)Index;
    }
  }
  ASSERT (FALSE);
  return 0xFFFFFFFF;
}

/**

  This function get and save MTRR.

**/
VOID
GetMtrr (
  VOID
  )
{
  UINT32  Count;
  UINT32  Index;

  mMtrrInfo.MtrrCap = AsmReadMsr64 (IA32_MTRRCAP_MSR_INDEX);
  mMtrrInfo.MtrrDefType = AsmReadMsr64 (IA32_MTRR_DEF_TYPE_MSR_INDEX);

  mMtrrInfo.FixedMtrr[0] = AsmReadMsr64 (IA32_MTRR_FIX64K_00000_MSR_INDEX);
  mMtrrInfo.FixedMtrr[1] = AsmReadMsr64 (IA32_MTRR_FIX16K_80000_MSR_INDEX);
  mMtrrInfo.FixedMtrr[2] = AsmReadMsr64 (IA32_MTRR_FIX16K_A0000_MSR_INDEX);
  for (Index = 0; Index < 8; Index++) {
    mMtrrInfo.FixedMtrr[3 + Index] = AsmReadMsr64 (IA32_MTRR_FIX4K_C0000_MSR_INDEX + Index);
  }

  Count = (UINT32)mMtrrInfo.MtrrCap & 0xFF;
  ASSERT (Count <= MAX_VARIABLE_MTRR_NUMBER);
  if (Count > MAX_VARIABLE_MTRR_NUMBER) {
    Count = MAX_VARIABLE_MTRR_NUMBER;
  }
  for (Index = 0; Index < Count; Index++) {
    mMtrrInfo.VariableMtrrBase[Index] = AsmReadMsr64 (IA32_MTRR_PHYSBASE0_MSR_INDEX + Index * 2);
    mMtrrInfo.VariableMtrrMask[Index] = AsmReadMsr64 (IA32_MTRR_PHYSMASK0_MSR_INDEX + Index * 2);
  }

  mMtrrInfo.SmrrBase = AsmReadMsr64 (EFI_MSR_NEHALEM_SMRR_PHYS_BASE);
  mMtrrInfo.SmrrMask = AsmReadMsr64 (EFI_MSR_NEHALEM_SMRR_PHYS_MASK);
}

/**

  This function get memory cache type according BaseAddress.

  @param BaseAddress memory base address

  @return memory cache type

**/
UINT8
GetMemoryType (
  IN UINT64 BaseAddress
  )
{
  UINT32  Count;
  UINT32  Index;
  UINT8   PossibleMemoryType[MAX_VARIABLE_MTRR_NUMBER];
  UINT8   PossibleMemoryTypeNumber;
  UINT8   FinalMemoryType;

  //
  // If the physical address falls within the first 1 MByte of physical memory and
  // fixed MTRRs are enabled, the processor uses the memory type stored for the
  // appropriate fixed-range MTRR.
  //
  if (BaseAddress < BASE_1MB) {
    Index = GetFixedMtrrStructInfo ((UINT32)BaseAddress);
    if (Index >= sizeof(mFixedMtrrStructInfo)/sizeof(mFixedMtrrStructInfo[0])) {
      return MEMORY_TYPE_UC;
    }
    return (UINT8)RShiftU64 (
                    mMtrrInfo.FixedMtrr[Index],
                    ((UINT32)BaseAddress - mFixedMtrrStructInfo[Index].Base) / (mFixedMtrrStructInfo[Index].Stepping) * 8
                    );
  }
  Count = (UINT32)mMtrrInfo.MtrrCap & 0xFF;
  ASSERT (Count <= MAX_VARIABLE_MTRR_NUMBER);
  if (Count > MAX_VARIABLE_MTRR_NUMBER) {
    Count = MAX_VARIABLE_MTRR_NUMBER;
  }
  PossibleMemoryTypeNumber = 0;
  for (Index = 0; Index < Count; Index++) {
    if ((mMtrrInfo.VariableMtrrMask[Index] & 0x800) == 0) {
      continue;
    }
    //
    // We can NOT return it directly, we need get all possible types and check the precedences.
    //
    if ((mMtrrInfo.VariableMtrrBase[Index] & mMtrrInfo.VariableMtrrMask[Index] & ~0xFFF) == (BaseAddress & mMtrrInfo.VariableMtrrMask[Index] & ~0xFFF)) {
      PossibleMemoryType[PossibleMemoryTypeNumber] = (UINT8)mMtrrInfo.VariableMtrrBase[Index] & 0xFF;
      PossibleMemoryTypeNumber++;
    }
  }

  //
  // Check SMRR
  //
  if ((mMtrrInfo.SmrrMask & EFI_MSR_SMRR_PHYS_MASK_VALID) != 0) {
    if ((mMtrrInfo.SmrrBase & mMtrrInfo.SmrrMask & ~0xFFF) == (BaseAddress & mMtrrInfo.SmrrMask & ~0xFFF)) {
      return (UINT8)mMtrrInfo.SmrrBase & 0xFF;
    }
  }

  //
  // If no fixed or variable memory range matches, the processor uses the default
  // memory type.
  //
  if (PossibleMemoryTypeNumber == 0) {
    return (UINT8)mMtrrInfo.MtrrDefType & 0xFF;
  }

  //
  // 1. If one variable memory range matches, the processor uses the memory type
  // stored in the IA32_MTRR_PHYSBASEn register for that range.
  // 2. If two or more variable memory ranges match and the memory types are
  // identical, then that memory type is used.
  // 3. If two or more variable memory ranges match and one of the memory types
  // is UC, the UC memory type used.
  // 4. If two or more variable memory ranges match and the memory types are WT
  // and WB, the WT memory type is used.
  // 5. For overlaps not defined by the above rules, processor behavior is undefined.
  //
  FinalMemoryType = PossibleMemoryType[0];
  for (Index = 1; Index < PossibleMemoryTypeNumber; Index++) {
    if (PossibleMemoryType[Index] == FinalMemoryType) {
      continue;
    }
    if (PossibleMemoryType[Index] == MEMORY_TYPE_UC) {
      FinalMemoryType = MEMORY_TYPE_UC;
      continue;
    }
    if (((PossibleMemoryType[Index] == MEMORY_TYPE_WB) && (FinalMemoryType == MEMORY_TYPE_WT)) ||
        ((PossibleMemoryType[Index] == MEMORY_TYPE_WT) && (FinalMemoryType == MEMORY_TYPE_WB))) {
      FinalMemoryType = MEMORY_TYPE_WT;
      continue;
    }
    //
    // Something wrong
    //
    DEBUG ((EFI_D_ERROR, "MTRR error for 0x%016lx: %02x VS %02x\n", BaseAddress, (UINTN)FinalMemoryType, (UINTN)PossibleMemoryType[Index]));
  }

  return FinalMemoryType;
}

/**

  This function create EPT page table for SMM guest.

  @param EptPointer EPT pointer
  @param Xa         Execute access

**/
VOID
EptCreatePageTable (
  OUT EPT_POINTER              *EptPointer,
  IN UINT32                    Xa
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
  UINT64                   BaseAddress;
  UINT8                    MemoryType;
  UINT32                   SmrrBase;
  UINT32                   SmrrLength;
  UINTN                    NumberOfPml4EntriesNeeded;
  UINTN                    NumberOfPdpEntriesNeeded;

  if (mHostContextCommon.PhysicalAddressBits <= 39) {
    NumberOfPml4EntriesNeeded = 1;
    NumberOfPdpEntriesNeeded = (UINTN)LShiftU64 (1, mHostContextCommon.PhysicalAddressBits - 30);
  } else {
    NumberOfPml4EntriesNeeded = (UINTN)LShiftU64 (1, mHostContextCommon.PhysicalAddressBits - 39);
    NumberOfPdpEntriesNeeded = 512;
  }

  SmrrBase = (UINT32)mMtrrInfo.SmrrBase & (UINT32)mMtrrInfo.SmrrMask & 0xFFFFF000;
  SmrrLength = (UINT32)mMtrrInfo.SmrrMask & 0xFFFFF000;
  SmrrLength = ~SmrrLength + 1;

  //
  // Setup below 4G
  //
  L4PageTable = (EPT_ENTRY *)AllocatePages (6);
  ZeroMem (L4PageTable, STM_PAGES_TO_SIZE(6));

  EptPointer->Uint64 = (UINT64)(UINTN)L4PageTable;
  EptPointer->Bits32.Gaw = EPT_GAW_48BIT;
  EptPointer->Bits32.Etmt = MEMORY_TYPE_WB;

  L3PageTable = (EPT_ENTRY *)((UINTN)L4PageTable + SIZE_4KB);
  L2PageTable = (EPT_ENTRY *)((UINTN)L3PageTable + SIZE_4KB);

  BaseAddress = 0;
  for (Index4 = 0; Index4 < 1; Index4 ++) {
    L4PageTable->Uint64 = (UINT64)(UINTN)L3PageTable;
    L4PageTable->Bits32.Ra    = 1;
    L4PageTable->Bits32.Wa    = 1;
    L4PageTable->Bits32.Xa    = 1;
    L4PageTable ++;
    for (Index3 = 0; Index3 < 4; Index3 ++) {
      L3PageTable->Uint64 = (UINT64)(UINTN)L2PageTable;
      L3PageTable->Bits32.Ra    = 1;
      L3PageTable->Bits32.Wa    = 1;
      L3PageTable->Bits32.Xa    = 1;
      L3PageTable ++;
      for (Index2 = 0; Index2 < 512; Index2 ++) {

        if (BaseAddress >= BASE_2MB) {
          if (TRUE) {
            // Use super page
            L2PageTable->Uint64 = BaseAddress;
            L2PageTable->Bits32.Ra    = 1;
            L2PageTable->Bits32.Wa    = 1;
            L2PageTable->Bits32.Xa    = Xa;
            L2PageTable->Bits32.Sp    = 1;

            // BUGBUG: Do we need set UC for STM region???
            MemoryType = GetMemoryType (BaseAddress);
            L2PageTable->Bits32.Emt = MemoryType;
            if ((BaseAddress >= SmrrBase) && (BaseAddress < SmrrBase + SmrrLength)) {
              DEBUG ((EFI_D_INFO, "EPT init: %x - %x\n", (UINTN)BaseAddress, (UINTN)L2PageTable->Bits32.Emt));
              L2PageTable->Bits32.Xa    = 1;
            }

            L2PageTable ++;

            BaseAddress += SIZE_2MB;
            continue;
          }
        }

        //
        // [0, 2MB)
        //
        L1PageTable = (EPT_ENTRY *)AllocatePages (1);

        L2PageTable->Uint64 = (UINT64)(UINTN)L1PageTable;
        L2PageTable->Bits32.Ra    = 1;
        L2PageTable->Bits32.Wa    = 1;
        L2PageTable->Bits32.Xa    = 1;
        L2PageTable ++;
        for (Index1 = 0; Index1 < 512; Index1 ++) {
          L1PageTable->Uint64 = BaseAddress;
          L1PageTable->Bits32.Ra    = 1;
          L1PageTable->Bits32.Wa    = 1;
          L1PageTable->Bits32.Xa    = Xa;

          MemoryType = GetMemoryType (BaseAddress);
          L1PageTable->Bits32.Emt = MemoryType;
          L1PageTable ++;
          BaseAddress += SIZE_4KB;
        }
      }
    }
  }

  //
  // Setup above 4G
  //
  if (sizeof(UINTN) == sizeof(UINT64)) {
    ASSERT(BaseAddress == BASE_4GB);
    L4PageTable = (EPT_ENTRY *)(UINTN)(EptPointer->Uint64 & ~(SIZE_4KB - 1));
    for (Index4 = 0; Index4 < NumberOfPml4EntriesNeeded; Index4 ++) {
      if (Index4 > 0) {
        L3PageTable = (EPT_ENTRY *)(UINTN)AllocatePages (1);
        L4PageTable[Index4].Uint64 = (UINT64)(UINTN)L3PageTable;
        L4PageTable[Index4].Bits32.Ra = 1;
        L4PageTable[Index4].Bits32.Wa = 1;
        L4PageTable[Index4].Bits32.Xa = 1;
        Index3 = 0;
      } else {
        // Start from 4G - L4PageTable[0] already allocated.
        L3PageTable = (EPT_ENTRY *)(UINTN)(L4PageTable[0].Uint64 & ~(SIZE_4KB - 1));
        Index3 = 4;
      }
      
      if (Is1GPageSupport()) {
        for (; Index3 < NumberOfPdpEntriesNeeded; Index3 ++) {
          L3PageTable[Index3].Uint64 = BaseAddress;
          L3PageTable[Index3].Bits32.Ra = 1;
          L3PageTable[Index3].Bits32.Wa = 1;
          L3PageTable[Index3].Bits32.Xa = Xa;
          L3PageTable[Index3].Bits32.Sp = 1;
          MemoryType = GetMemoryType (BaseAddress);
          L3PageTable[Index3].Bits32.Emt = MemoryType;
          BaseAddress += SIZE_1GB;
        }
      } else {
        for (; Index3 < NumberOfPdpEntriesNeeded; Index3 ++) {
          L2PageTable = (EPT_ENTRY *)(UINTN)AllocatePages (1);
          L3PageTable[Index3].Uint64 = (UINT64)(UINTN)L2PageTable;
          L3PageTable[Index3].Bits32.Ra = 1;
          L3PageTable[Index3].Bits32.Wa = 1;
          L3PageTable[Index3].Bits32.Xa = 1;
          for (Index2 = 0; Index2 < 512; Index2 ++) {
            L2PageTable[Index2].Uint64 = BaseAddress;
            L2PageTable[Index2].Bits32.Ra = 1;
            L2PageTable[Index2].Bits32.Wa = 1;
            L2PageTable[Index2].Bits32.Xa = Xa;
            L2PageTable[Index2].Bits32.Sp = 1;
            MemoryType = GetMemoryType (BaseAddress);
            L2PageTable[Index2].Bits32.Emt = MemoryType;
            BaseAddress += SIZE_2MB;
          }
        }
      }
    }
  }

  return ;
}

/**

  This function dump EPT page table.

  @param EptPointer EPT pointer

**/
VOID
EptDumpPageTable (
  IN EPT_POINTER              *EptPointer
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
  UINTN                    NumberOfPml4EntriesNeeded;
  UINTN                    NumberOfPdpEntriesNeeded;
  
  DEBUG ((EFI_D_INFO, "EptDumpPageTable\n"));
  DEBUG ((EFI_D_INFO, "EptPointer - 0x%016lx\n", EptPointer->Uint64));

  if (mHostContextCommon.PhysicalAddressBits <= 39) {
    NumberOfPml4EntriesNeeded = 1;
    NumberOfPdpEntriesNeeded = (UINTN)LShiftU64 (1, mHostContextCommon.PhysicalAddressBits - 30);
  } else {
    NumberOfPml4EntriesNeeded = (UINTN)LShiftU64 (1, mHostContextCommon.PhysicalAddressBits - 39);
    NumberOfPdpEntriesNeeded = 512;
  }

  L4PageTable = (EPT_ENTRY *)(UINTN)(EptPointer->Uint64 & (~(SIZE_4KB - 1)));
  for (Index4 = 0; Index4 < NumberOfPml4EntriesNeeded; Index4 ++, L4PageTable++) {
    if (L4PageTable->Uint64 != 0) {
      DEBUG ((EFI_D_INFO, "  L4PageTable(0x%x) - 0x%016lx\n", Index4, L4PageTable->Uint64));
    } else {
      continue;
    }
    L3PageTable = (EPT_ENTRY *)(UINTN)(L4PageTable->Uint64 & (~(SIZE_4KB - 1)));
    for (Index3 = 0; Index3 < NumberOfPdpEntriesNeeded; Index3 ++, L3PageTable++) {
      if (L3PageTable->Uint64 != 0) {
        DEBUG ((EFI_D_INFO, "    L3PageTable(0x%x) - 0x%016lx\n", Index3, L3PageTable->Uint64));
      } else {
        continue;
      }
      if (L3PageTable->Bits32.Sp == 0) {
        L2PageTable = (EPT_ENTRY *)(UINTN)(L3PageTable->Uint64 & (~(SIZE_4KB - 1)));
        for (Index2 = 0; Index2 < 512; Index2 ++, L2PageTable++) {
          if (L2PageTable->Uint64 != 0) {
            DEBUG ((EFI_D_INFO, "      L2PageTable(0x%x) - 0x%016lx\n", Index2, L2PageTable->Uint64));
          } else {
            continue;
          }
          if (L2PageTable->Bits32.Sp == 0) {
            L1PageTable = (EPT_ENTRY *)(UINTN)(L2PageTable->Uint64 & (~(SIZE_4KB - 1)));
            for (Index1 = 0; Index1 < 512; Index1 ++, L1PageTable++) {
              if (L1PageTable->Uint64 != 0) {
                DEBUG ((EFI_D_INFO, "        L1PageTable(0x%x) - 0x%016lx\n", Index1, L1PageTable->Uint64));
              }
            }
          }
        }
      }
    }
  }
}


/**

  This function initialize EPT.

**/
VOID
EptInit (
  VOID
  )
{
  TXT_PROCESSOR_SMM_DESCRIPTOR  *TxtProcessorSmmDescriptor;
  UINT32                        SmrrBase;
  UINT32                        SmrrLength;
  BOOLEAN                       ExecutionDisableOutsideSmrr;
  UINT32                        Xa;
  UINT64                        TsegBase;
  UINT64                        TsegLength;
  IA32_VMX_MISC_MSR             VmxMisc;

  AsmWbinvd ();
  GetMtrr ();
  
  VmxMisc.Uint64 = AsmReadMsr64 (IA32_VMX_MISC_MSR_INDEX);
  if ((VmxMisc.Uint64 & BIT15) != 0) {
    TxtProcessorSmmDescriptor = (TXT_PROCESSOR_SMM_DESCRIPTOR *)(UINTN)(AsmReadMsr64 (IA32_SMBASE_INDEX) + SMM_TXTPSD_OFFSET);
  } else {
    TxtProcessorSmmDescriptor = (TXT_PROCESSOR_SMM_DESCRIPTOR *)(UINTN)(VmRead32 (VMCS_32_GUEST_SMBASE_INDEX) + SMM_TXTPSD_OFFSET);
  }
  ExecutionDisableOutsideSmrr = TxtProcessorSmmDescriptor->SmmEntryState.ExecutionDisableOutsideSmrr;
  DEBUG ((EFI_D_INFO, "ExecutionDisableOutsideSmrr - %02x\n", (UINTN)ExecutionDisableOutsideSmrr));
  if (ExecutionDisableOutsideSmrr) {
    Xa = 0;
  } else {
    Xa = 1;
  }

  EptCreatePageTable (&mGuestContextCommonSmm.EptPointer, Xa);

  SmrrBase = (UINT32)mMtrrInfo.SmrrBase & (UINT32)mMtrrInfo.SmrrMask & 0xFFFFF000;
  SmrrLength = (UINT32)mMtrrInfo.SmrrMask & 0xFFFFF000;
  SmrrLength = ~SmrrLength + 1;
  DEBUG ((EFI_D_INFO, "SMRR_PHYS_BASE - %016lx\n", AsmReadMsr64 (EFI_MSR_NEHALEM_SMRR_PHYS_BASE)));
  DEBUG ((EFI_D_INFO, "SMRR_PHYS_MASK - %016lx\n", AsmReadMsr64 (EFI_MSR_NEHALEM_SMRR_PHYS_MASK)));
  DEBUG ((EFI_D_INFO, "SmrrBase - %08x\n", (UINTN)SmrrBase));
  DEBUG ((EFI_D_INFO, "SmrrLength - %08x\n", (UINTN)SmrrLength));
  DEBUG ((EFI_D_INFO, "StmHeader - %08x\n", (UINTN)mHostContextCommon.StmHeader));
  DEBUG ((EFI_D_INFO, "StmSize - %08x\n", (UINTN)mHostContextCommon.StmSize));
  DEBUG ((EFI_D_INFO, "GuestCr3(0) - %08x\n", (UINTN)mGuestContextCommonSmm.GuestContextPerCpu[0].Cr3));

  if (IsSentryEnabled()) {
    GetTsegInfoFromTxt (&TsegBase, &TsegLength);
    DEBUG ((EFI_D_INFO, "SMM NonOverlayed Base   - %08x\n", (UINTN)TsegBase));
    DEBUG ((EFI_D_INFO, "SMM NonOverlayed Length - %08x\n", (UINTN)TsegLength));
  } else {
    GetTsegInfoFromSmrr (&TsegBase, &TsegLength);
  }
  DEBUG ((EFI_D_INFO, "TsegBase - %08x\n", (UINTN)TsegBase));
  DEBUG ((EFI_D_INFO, "TsegLength - %08x\n", (UINTN)TsegLength));
  if (TsegBase == 0) {
    DEBUG ((EFI_D_ERROR, "TsegBase == 0\n"));
    CpuDeadLoop ();
  }
  if (TsegBase != SmrrBase) {
    DEBUG ((EFI_D_ERROR, "TsegBase != SmrrBase\n"));
    CpuDeadLoop ();
  }
  if (TsegLength != SmrrLength) {
    DEBUG ((EFI_D_ERROR, "TsegLength != SmrrLength\n"));
    CpuDeadLoop ();
  }
  mHostContextCommon.TsegBase   = TsegBase;
  mHostContextCommon.TsegLength = TsegLength;

  //
  // Need mark SMRAM executable again
  //
  if (ExecutionDisableOutsideSmrr)  {
    EPTSetPageAttributeRange (
      TsegBase,
      TsegLength,
      1,
      1,
      1,
      EptPageAttributeSet
      );
  }

  //
  // Protect MSEG
  //
  DEBUG ((EFI_D_INFO, "Protect MSEG\n"));
  EPTSetPageAttributeRange (
    (UINT64)(UINTN)mHostContextCommon.StmHeader,
    (UINT64)(UINTN)mHostContextCommon.StmSize,
    0,
    0,
    0,
    EptPageAttributeSet
    );
  //EptDumpPageTable (&mGuestContextCommonSmm.EptPointer);
  return ;
}
