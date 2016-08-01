/** @file
  MSR initialization

  Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmInit.h"

/**

  This function set/unset MSR bitmap.

  @param MsrIndex MSR index
  @param MsrWrite TRUE means MsrWrite, FALSE means MsrRead
  @param Set      TRUE means set MSR bitmap, FALSE means unset MSR bitmap

**/
VOID
SetMsrBitmapEx (
  IN UINT32  MsrIndex,
  IN BOOLEAN MsrWrite,
  IN BOOLEAN Set
  )
{
  UINT8 *MsrBitmap;
  UINTN Index;
  UINTN Offset;

  if (!MsrWrite) {
    if (MsrIndex < 0x2000) {
      MsrBitmap = (UINT8 *)(UINTN)mGuestContextCommonSmm.MsrBitmap;
    } else if (MsrIndex >= 0xC0000000 && MsrIndex < 0xC0002000){
      MsrBitmap = (UINT8 *)(UINTN)(mGuestContextCommonSmm.MsrBitmap + 0x400);
      MsrIndex -= 0xC0000000;
    } else {
      return ;
    }
  } else {
    if (MsrIndex < 0x2000) {
      MsrBitmap = (UINT8 *)(UINTN)(mGuestContextCommonSmm.MsrBitmap + 0x800);
    } else if (MsrIndex >= 0xC0000000 && MsrIndex < 0xC0002000){
      MsrBitmap = (UINT8 *)(UINTN)(mGuestContextCommonSmm.MsrBitmap + 0xC00);
      MsrIndex -= 0xC0000000;
    } else {
      return ;
    }
  }

  Index = MsrIndex / 8;
  Offset = MsrIndex % 8;

  if (Set) {
    MsrBitmap[Index] |= (UINT8)(1 << Offset);
  } else {
    MsrBitmap[Index] &= (UINT8)~(1 << Offset);
  }

  return ;
}

/**

  This function set MSR bitmap.

  @param MsrIndex MSR index
  @param MsrWrite TRUE means MsrWrite, FALSE means MsrRead

**/
VOID
EFIAPI
SetMsrBitmap (
  IN UINT32  MsrIndex,
  IN BOOLEAN MsrWrite
  )
{
  SetMsrBitmapEx (MsrIndex, MsrWrite, TRUE);
}

/**

  This function unset MSR bitmap.

  @param MsrIndex MSR index
  @param MsrWrite TRUE means MsrWrite, FALSE means MsrRead

**/
VOID
UnSetMsrBitmap (
  IN UINT32  MsrIndex,
  IN BOOLEAN MsrWrite
  )
{
  SetMsrBitmapEx (MsrIndex, MsrWrite, FALSE);
}

typedef struct {
  UINT32    Index;
  UINT32    Support;
  UINT32    Enable;
} MSR_BITMAP_MASK;

#define NEED_MICROCODE_UPDATE         BIT0

MSR_BITMAP_MASK mMsrReadWriteExit[] = {
  {IA32_EFER_MSR_INDEX,           0, 0},
  {IA32_SYSENTER_CS_MSR_INDEX,    0, 0},
  {IA32_SYSENTER_ESP_MSR_INDEX,   0, 0},
  {IA32_SYSENTER_EIP_MSR_INDEX,   0, 0},
  {IA32_FS_BASE_MSR_INDEX,        0, 0},
  {IA32_GS_BASE_MSR_INDEX,        0, 0},
};

MSR_BITMAP_MASK mMsrWriteExit[] = {
  {IA32_SMM_MONITOR_CTL_MSR_INDEX,  0, 0},
  {EFI_MSR_NEHALEM_SMRR_PHYS_BASE,  0, 0},
  {EFI_MSR_NEHALEM_SMRR_PHYS_MASK,  0, 0},
  {IA32_BIOS_UPDT_TRIG_MSR_INDEX,   0, 0},
};

MSR_BITMAP_MASK mMsrReadExit[] = {
  {IA32_EFER_MSR_INDEX,           0, 0}, // BUGBUG: Placeholder
};

/**

  This function update MSR table.

**/
VOID
UpdateMsrTable (
  IN UINT32  BitMask
  )
{
  UINTN  Index;

  for (Index = 0; Index < sizeof(mMsrReadWriteExit)/sizeof(mMsrReadWriteExit[0]); Index++) {
    if ((mMsrReadWriteExit[Index].Support & BitMask) != 0) {
      mMsrReadWriteExit[Index].Enable |= BitMask;
    }
  }
  for (Index = 0; Index < sizeof(mMsrWriteExit)/sizeof(mMsrWriteExit[0]); Index++) {
    if ((mMsrWriteExit[Index].Support & BitMask) != 0) {
      mMsrWriteExit[Index].Enable |= BitMask;
    }
  }
  for (Index = 0; Index < sizeof(mMsrReadExit)/sizeof(mMsrReadExit[0]); Index++) {
    if ((mMsrReadExit[Index].Support & BitMask) != 0) {
      mMsrReadExit[Index].Enable |= BitMask;
    }
  }
}

/**

  This function set MSR bitmap.

**/
VOID
SetAllMsrBitmaps (
  VOID
  )
{
  UINTN  Index;

  for (Index = 0; Index < sizeof(mMsrReadWriteExit)/sizeof(mMsrReadWriteExit[0]); Index++) {
    if (mMsrReadWriteExit[Index].Support == mMsrReadWriteExit[Index].Enable) {
      SetMsrBitmap (mMsrReadWriteExit[Index].Index, FALSE);
      SetMsrBitmap (mMsrReadWriteExit[Index].Index, TRUE);
    }
  }
  for (Index = 0; Index < sizeof(mMsrWriteExit)/sizeof(mMsrWriteExit[0]); Index++) {
    if (mMsrWriteExit[Index].Support == mMsrWriteExit[Index].Enable) {
      SetMsrBitmap (mMsrWriteExit[Index].Index, TRUE);
    }
  }
  for (Index = 0; Index < sizeof(mMsrReadExit)/sizeof(mMsrReadExit[0]); Index++) {
    if (mMsrReadExit[Index].Support == mMsrReadExit[Index].Enable) {
      SetMsrBitmap (mMsrReadExit[Index].Index, FALSE);
    }
  }
}

/**

  This function initialize MSR bitmap.

**/
VOID
MsrInit (
  VOID
  )
{
  mGuestContextCommonSmm.MsrBitmap = (UINT64)(UINTN)AllocatePages (1);

  SetAllMsrBitmaps ();

  StmPlatformLibSetMsrBitmaps ();
}
