/** @file
  DlEntry Cache initialization

  Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Dce.h"

#define MTRR_MASK  0xFFFFFF000ll
#define MTRR_VALID 0x800

/**

  This function prepare MTRR update environment.

  @return interrupt state

**/
BOOLEAN
MtrrUpdateEntry (
  VOID
  )
{
  BOOLEAN                   InterruptState;
  UINTN                     Index;
  UINTN                     Count;

  InterruptState = GetInterruptState ();
  DisableInterrupts ();
  DEBUG((EFI_D_INFO, "(TXT) InterruptState - 0x%x\n", InterruptState));

  AsmWriteCr0 ((AsmReadCr0 () & ~CR0_NW) | CR0_CD);
  AsmWbinvd ();

  AsmWriteCr4 (AsmReadCr4() & ~CR4_PGE);
  AsmWriteCr3 (AsmReadCr3 ());

  AsmWriteMsr64(IA32_MTRR_DEF_TYPE_MSR_INDEX, 0);

  Count = (UINTN)AsmReadMsr64(IA32_MTRRCAP_MSR_INDEX) & 0xFF;
  for (Index = 0; Index < Count; Index++) {
    AsmWriteMsr64 ((UINT32)(IA32_MTRR_PHYSBASE0_MSR_INDEX + Index*2 + 1), AsmReadMsr64 ((UINT32)(IA32_MTRR_PHYSBASE0_MSR_INDEX + Index*2 + 1)) & ~0x800);
  }

  return InterruptState;
}

/**

  This function setup environment after MTRR update.

  @param InterruptState interrupt state

**/
VOID
MtrrUpdateExit (
  BOOLEAN          InterruptState
  )
{
  AsmWbinvd ();

  AsmWriteCr3 (AsmReadCr3 ());

  AsmWriteMsr64 (IA32_MTRR_DEF_TYPE_MSR_INDEX, AsmReadMsr64(IA32_MTRR_DEF_TYPE_MSR_INDEX) | IA32_MTRR_DEF_TYPE_E);

  AsmWriteCr0 (AsmReadCr0 () & ~CR0_NW & ~CR0_CD);
  AsmWriteCr4 (AsmReadCr4 () | CR4_PGE);

  if (InterruptState) {
    //EnableInterrupts ();
  }

  return ;
}

/**

  This function save MTRR informatin into TXT heap.

  @param MlePrivateData TXT MlePrivateData

**/
VOID
TxtSaveMtrr (
  OUT MLE_PRIVATE_DATA    *MlePrivateData
  )
{
  UINTN            Index;
  UINTN            Count;

  Count = (UINTN)AsmReadMsr64(IA32_MTRRCAP_MSR_INDEX) & 0xFF;
  ASSERT (Count <= VARIABLE_MTRR_NUM_MAX);
  for (Index = 0; Index < Count; Index++) {
    MlePrivateData->VariableMtrr[Index * 2]     = AsmReadMsr64 ((UINT32)(IA32_MTRR_PHYSBASE0_MSR_INDEX + Index * 2));
    MlePrivateData->VariableMtrr[Index * 2 + 1] = AsmReadMsr64 ((UINT32)(IA32_MTRR_PHYSBASE0_MSR_INDEX + Index * 2 + 1));
  }
  MlePrivateData->DefaultTypeMsr = AsmReadMsr64 (IA32_MTRR_DEF_TYPE_MSR_INDEX);
  MlePrivateData->MiscEnableMsr  = AsmReadMsr64 (IA32_MISC_ENABLE_MSR_INDEX);

  return ;
}

/**

  This function restore MTRR informatin from TXT heap.

  @param MlePrivateData TXT MlePrivateData

**/
VOID
TxtRestoreMtrr (
  IN MLE_PRIVATE_DATA    *MlePrivateData
  )
{
  UINTN            Index;
  UINTN            Count;
  BOOLEAN          InterruptState;

  DEBUG((EFI_D_INFO, "(TXT) MtrrUpdateEntry ...\n"));
  InterruptState = MtrrUpdateEntry ();
  DEBUG((EFI_D_INFO, "(TXT) MtrrUpdateEntry Done\n"));

  Count = (UINTN)AsmReadMsr64(IA32_MTRRCAP_MSR_INDEX) & 0xFF;
  ASSERT (Count <= VARIABLE_MTRR_NUM_MAX);
  for (Index = 0; Index < Count; Index++) {
    DEBUG((EFI_D_INFO, "(TXT) MtrrUpdate 0x%016x - 0x%016lx\n", MlePrivateData->VariableMtrr[Index * 2], MlePrivateData->VariableMtrr[Index * 2 + 1]));
    AsmWriteMsr64 ((UINT32)(IA32_MTRR_PHYSBASE0_MSR_INDEX + Index * 2),     MlePrivateData->VariableMtrr[Index * 2]);
    AsmWriteMsr64 ((UINT32)(IA32_MTRR_PHYSBASE0_MSR_INDEX + Index * 2 + 1), MlePrivateData->VariableMtrr[Index * 2 + 1]);
  }

  DEBUG((EFI_D_INFO, "(TXT) MtrrUpdate Default 0x%016lx\n", MlePrivateData->DefaultTypeMsr));
  AsmWriteMsr64 (IA32_MTRR_DEF_TYPE_MSR_INDEX, MlePrivateData->DefaultTypeMsr);
  DEBUG((EFI_D_INFO, "(TXT) MtrrUpdate MisEnable 0x%016lx\n", MlePrivateData->MiscEnableMsr));
  AsmWriteMsr64 (IA32_MISC_ENABLE_MSR_INDEX,   MlePrivateData->MiscEnableMsr);

  DEBUG((EFI_D_INFO, "(TXT) MtrrUpdateExit ...\n"));
  MtrrUpdateExit (InterruptState);
  DEBUG((EFI_D_INFO, "(TXT) MtrrUpdateExit Done\n"));

  return ;
}

/**

  This function configure MTRR environment for SENTER.

  @param MemBase ACM memory base
  @param MemSize ACM memory size
  @param MemType ACM memory type

**/
VOID
TxtConfigMtrr (
  IN UINT64                MemBase,
  IN UINT64                MemSize,
  IN UINT8                 MemType
  )
{
  BOOLEAN                 InterruptState;
  UINT64                  MtrrLength;
  UINTN                   Index;
  UINTN                   Count;
  
  InterruptState = MtrrUpdateEntry ();

  MemSize = EFI_PAGES_TO_SIZE (EFI_SIZE_TO_PAGES((UINTN)MemSize));

  Count = (UINTN)AsmReadMsr64(IA32_MTRRCAP_MSR_INDEX) & 0xFF;
  Index = 0;
  DEBUG((EFI_D_INFO, "(TXT) MTRR Count - 0x%x\n", Count));
  DEBUG((EFI_D_INFO, "(TXT) MemBase - 0x%lx\n", MemBase));
  DEBUG((EFI_D_INFO, "(TXT) MemSize - 0x%lx\n", MemSize));

  while ((MemSize != 0) && (Index < Count)) {
    MtrrLength = (UINT64)GetPowerOfTwo32 ((UINT32)MemSize);
    while ((MemBase & (MtrrLength - 1)) != 0) {
      MtrrLength = (UINTN)MtrrLength / 2;
    }
    DEBUG((EFI_D_INFO, "(TXT) MtrrBase - 0x%lx\n", MemBase));
    DEBUG((EFI_D_INFO, "(TXT) MtrrLength - 0x%lx\n", MtrrLength));

    AsmWriteMsr64 ((UINT32)(IA32_MTRR_PHYSBASE0_MSR_INDEX + Index * 2), MemBase | MemType);
    DEBUG((EFI_D_INFO, "(TXT) MtrrBase updated\n"));
    AsmWriteMsr64 ((UINT32)(IA32_MTRR_PHYSBASE0_MSR_INDEX + Index * 2 + 1), (~(MtrrLength - 1) & MTRR_MASK) | MTRR_VALID);
    DEBUG((EFI_D_INFO, "(TXT) MtrrMask updated\n"));

    MemSize -= MtrrLength;
    MemBase += MtrrLength;
    Index ++;
  }

  DEBUG((EFI_D_INFO, "(TXT) MtrrUpdateExit ...\n"));
  MtrrUpdateExit (InterruptState);
  DEBUG((EFI_D_INFO, "(TXT) MtrrUpdateExit Done\n"));

  return ;
}
