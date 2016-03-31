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
#include "FrmInit.h"

extern UINT32 mApWakeupSegmentOffset;
extern UINT32 mApProtectedModeEntryOffset;
extern UINT32 mApGdtBaseOffset;
extern UINT32 mApGdtBase;

#ifdef MDE_CPU_X64
extern UINT32 mLongModeEntryOffset;
extern UINT32 mLongModeEntry;
extern UINT32 mPageTableOffset;
#endif

UINT32   mCpuNum;
UINTN    mApStack;
UINTN    mApicIdList;

/**

  This is AP wakeup entrypoint.

**/
VOID
AsmApWakeup (
  VOID
  );

/**

  This is AP wakeup 32bit entrypoint.

**/
VOID
AsmApWakeup32 (
  VOID
  );

/**

  This function is guest AP entrypoint.

**/
VOID
AsmGuestApEntrypoint (
  VOID
  );


/**

  This function initialize all APs.

**/
VOID
InitAllAps (
  VOID
  )
{
  mCpuNum = mHostContextCommon.CpuNum;
  mApStack = (UINTN)AllocatePages (1 * mHostContextCommon.CpuNum);
//mApicIdList = (UINTN)AllocatePages (FRM_SIZE_TO_PAGES (sizeof(UINT32) * mHostContextCommon.CpuNum));
//GetApicIdListFromAcpi ((UINT32 *)mApicIdList);
}

/**

  This function wakeup all APs.

**/
VOID
WakeupAllAps (
  VOID
  )
{
  //
  // Prepare handler
  //
  CopyMem ((VOID *)(UINTN)mHostContextCommon.LowMemoryBase, (VOID *)(UINTN)AsmApWakeup, 0x1000);

  //
  // Fix up
  //
  *(UINT16 *)(UINTN)(mHostContextCommon.LowMemoryBase + mApWakeupSegmentOffset) = (UINT16)(((UINT32)mHostContextCommon.LowMemoryBase) >> 4);
  *(UINT32 *)(UINTN)(mHostContextCommon.LowMemoryBase + mApProtectedModeEntryOffset) = (UINT32)(UINTN)AsmApWakeup32;
  *(UINT32 *)(UINTN)(mHostContextCommon.LowMemoryBase + mApGdtBaseOffset) = (UINT32)(mHostContextCommon.LowMemoryBase + mApGdtBase);

#ifdef MDE_CPU_X64
  *(UINT32 *)((UINTN)AsmApWakeup32 + mLongModeEntryOffset) = (UINT32)((UINTN)AsmApWakeup32 + mLongModeEntry);
  *(UINT32 *)((UINTN)AsmApWakeup32 + mPageTableOffset) = (UINT32)(AsmReadCr3 ());
#endif

  //
  // Send SIPI
  //
  SendSipi ((UINT8)((UINT32)mHostContextCommon.LowMemoryBase >> 12));
}

/**

  This function is AP wakeup C function.

**/
VOID
EFIAPI
ApWakeupC (
  IN UINT32  Index
  )
{
  UINTN  Rflags;

  AsmWriteGdtr (&mHostContextCommon.Gdtr);
  AsmWriteIdtr (&mHostContextCommon.Idtr);

  InitHostContextPerCpu (Index);

  // Tell BSP OK
  mApFinished[Index] = TRUE;

  // Wait for BSP
  while (!mApLaunch) {
    ; // WAIT
  }

  InitGuestContextPerCpu (Index);

  //
  // Load VMCS
  //
  AsmVmPtrLoad (&mGuestContextCommon.GuestContextPerCpu[Index].Vmcs);

  //
  // Launch STM
  //
  LaunchStm (Index);

  //
  // Load VMCS
  //
  AsmVmPtrLoad (&mGuestContextCommon.GuestContextPerCpu[Index].Vmcs);

  //
  // set EIP to jump $
  //
  VmWriteN (VMCS_N_GUEST_RIP_INDEX, (UINTN)AsmGuestApEntrypoint);

  AsmWbinvd ();

  //
  // Launch Guest
  //
  Rflags = AsmVmLaunch (&mGuestContextCommon.GuestContextPerCpu[Index].Register);
  if (Index == 1) {
    DEBUG ((EFI_D_ERROR, "(FRM) !!!LaunchGuestAp FAIL!!!\n"));
    DEBUG ((EFI_D_ERROR, "(FRM) Rflags: %08x\n", Rflags));
    DEBUG ((EFI_D_ERROR, "(FRM) VMCS_32_RO_VM_INSTRUCTION_ERROR: %08x\n", (UINTN)VmRead32 (VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX)));
  }

  CpuDeadLoop ();

  return ;
}
