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
#include "Dce.h"
#include <Library\PcdLib.h>

FRM_COMMUNICATION_DATA    mCommunicationData;

FRM_HOST_CONTEXT_COMMON   mHostContextCommon;
FRM_GUEST_CONTEXT_COMMON  mGuestContextCommon;


BASE_LIBRARY_JUMP_BUFFER mGuestJumpBuffer;

volatile BOOLEAN    *mApFinished;
volatile BOOLEAN    mApLaunch = FALSE;

UINT32 mBspIndex;

extern UINT32 mExceptionHandlerLength;
extern UINTN    mApicIdList;

BASE_LIBRARY_JUMP_BUFFER           mDlmeJumpBuffer;

/**

  This is AP wakeup 32bit entrypoint.

**/
VOID
AsmApWakeup32 (
  VOID
  );

#ifdef MDE_CPU_X64
extern UINT32 mLongModeEntry;
#endif

/**

  This function is main function and it will jump back accrording to DLME argument.

  @param DlmeArgs          A pointer to the Args of DLME
**/
VOID
DlmeMain (
  IN VOID         *DlmeArgs
  )
{
  LongJump (DlmeArgs, (UINTN)-1);
  CpuDeadLoop ();
}

/**

  This function run DRTM DL_Entry.

**/
VOID
DlEntry (
  VOID
  )
{
  UINTN       Flag;
  VOID        *StackBase;
  VOID        *StackTop;
  UINT32      Ret;

  StackBase = AllocatePages (1);
  StackTop = (UINT8 *)StackBase + EFI_PAGES_TO_SIZE(1);

  Flag = SetJump (&mDlmeJumpBuffer);
  if (Flag != 0) {
    DEBUG ((EFI_D_INFO, "(TXT) !!!Great! Entre DLME!!!\n"));
    return ;
  }

  Ret = DL_Entry ((UINTN)DlmeMain, &mDlmeJumpBuffer, StackTop);

  DEBUG ((EFI_D_INFO, "(TXT) !!!DlEntry fail - 0x%x!!!\n", Ret));

  return ;
}

/**

  This function initialize FRM heap.

**/
VOID
InitHeap (
  VOID
  )
{
  //
  // Get memory from HighMemory
  // Should not use AllocatePages here, since it is chicken-egg problem
  //
  mHostContextCommon.HeapBottom = mCommunicationData.HighMemoryBase;
  mHostContextCommon.HeapTop    = mCommunicationData.HighMemoryBase + mCommunicationData.HighMemorySize;
}

/**

  This function initialize timer for FRM.

**/
VOID
InitializeTimer (
  VOID
  )
{
  mHostContextCommon.AcpiTimerIoPortBaseAddress = GetAcpiTimerPort(&mHostContextCommon.AcpiTimerWidth);
  PcdSet16(PcdAcpiTimerIoPortBaseAddress, mHostContextCommon.AcpiTimerIoPortBaseAddress);
  PcdSet8(PcdAcpiTimerWidth, mHostContextCommon.AcpiTimerWidth);
  if (mHostContextCommon.AcpiTimerIoPortBaseAddress == 0) {
    CpuDeadLoop();
  }
}

/**

  This function initialize basic context for FRM.

**/
VOID
InitBasicContext (
  VOID
  )
{
  UINT32  RegEax;

  mHostContextCommon.CpuNum = GetCpuNumFromAcpi ();

  GetPciExpressInfoFromAcpi (&mHostContextCommon.PciExpressBaseAddress, &mHostContextCommon.PciExpressLength);
  PcdSet64 (PcdPciExpressBaseAddress, mHostContextCommon.PciExpressBaseAddress);
  if (mHostContextCommon.PciExpressBaseAddress == 0) {
    CpuDeadLoop ();
  }

  mHostContextCommon.ResetIoPortBaseAddress = GetAcpiResetPort ();

  mHostContextCommon.AcpiPmControlIoPortBaseAddress = GetAcpiPmControlPort ();
  if (mHostContextCommon.AcpiPmControlIoPortBaseAddress == 0) {
    CpuDeadLoop ();
  }

  mHostContextCommon.HostContextPerCpu = AllocatePages (FRM_SIZE_TO_PAGES(sizeof(FRM_HOST_CONTEXT_PER_CPU)) * mHostContextCommon.CpuNum);
  mGuestContextCommon.GuestContextPerCpu = AllocatePages (FRM_SIZE_TO_PAGES(sizeof(FRM_GUEST_CONTEXT_PER_CPU)) * mHostContextCommon.CpuNum);

  mHostContextCommon.LowMemoryBase = mCommunicationData.LowMemoryBase;
  mHostContextCommon.LowMemorySize = mCommunicationData.LowMemorySize;
  mHostContextCommon.LowMemoryBackupBase = (UINT64)(UINTN)AllocatePages (FRM_SIZE_TO_PAGES ((UINTN)mCommunicationData.LowMemorySize));

  //
  // Save current context
  //
  mBspIndex = ApicToIndex (ReadLocalApicId ());
  mGuestContextCommon.GuestContextPerCpu[mBspIndex].Cr0 = AsmReadCr0 ();
  mGuestContextCommon.GuestContextPerCpu[mBspIndex].Cr3 = AsmReadCr3 ();
  mGuestContextCommon.GuestContextPerCpu[mBspIndex].Cr4 = AsmReadCr4 ();
  AsmReadGdtr (&mGuestContextCommon.GuestContextPerCpu[mBspIndex].Gdtr);
  AsmReadIdtr (&mGuestContextCommon.GuestContextPerCpu[mBspIndex].Idtr);

  AsmCpuid (CPUID_EXTENDED_INFORMATION, &RegEax, NULL, NULL, NULL);
  if (RegEax >= CPUID_EXTENDED_ADDRESS_SIZE) {
    AsmCpuid (CPUID_EXTENDED_ADDRESS_SIZE, &RegEax, NULL, NULL, NULL);
    mHostContextCommon.PhysicalAddressBits = (UINT8)RegEax;
  } else {
    mHostContextCommon.PhysicalAddressBits = 36;
  }
}

//
// Host related init
//

/**

  This function initialize host VMCS.

**/
VOID
InitHostVmcs (
  UINTN Index
  )
{
  UINT64  Data64;
  UINTN   Size;

  //
  // VMCS size
  //
  Data64 = AsmReadMsr64 (IA32_VMX_BASIC_MSR_INDEX);
  Size = (UINTN)(RShiftU64 (Data64, 32) & 0xFFFF);

  //
  // Allocate
  //
  mHostContextCommon.HostContextPerCpu[Index].Vmcs = (UINT64)(UINTN)AllocatePages (FRM_SIZE_TO_PAGES(Size));

  //
  // Set RevisionIdentifier
  //
  *(UINT32 *)(UINTN)mHostContextCommon.HostContextPerCpu[Index].Vmcs = (UINT32)Data64;

  return ;
}

/**

  This function initialize host common context.

**/
VOID
InitHostContextCommon (
  VOID
  )
{
  UINT32                      Index;
  INTERRUPT_GATE_DESCRIPTOR   *IdtGate;

  //
  // PageTable
  //
  mHostContextCommon.PageTable = CreatePageTable ();

  mHostContextCommon.Gdtr.Limit = mGuestContextCommon.GuestContextPerCpu[mBspIndex].Gdtr.Limit;
  mHostContextCommon.Gdtr.Base = (UINTN)AllocatePages (FRM_SIZE_TO_PAGES (mHostContextCommon.Gdtr.Limit + 1));
  CopyMem ((VOID *)mHostContextCommon.Gdtr.Base, (VOID *)mGuestContextCommon.GuestContextPerCpu[mBspIndex].Gdtr.Base, mHostContextCommon.Gdtr.Limit + 1);
  AsmWriteGdtr (&mHostContextCommon.Gdtr);

  mHostContextCommon.Idtr.Limit = 0x100 * sizeof (INTERRUPT_GATE_DESCRIPTOR) - 1;
  mHostContextCommon.Idtr.Base = (UINTN)AllocatePages (FRM_SIZE_TO_PAGES (mHostContextCommon.Idtr.Limit + 1));
  CopyMem ((VOID *)mHostContextCommon.Idtr.Base, (VOID *)mGuestContextCommon.GuestContextPerCpu[mBspIndex].Idtr.Base, mHostContextCommon.Idtr.Limit + 1);
  IdtGate = (INTERRUPT_GATE_DESCRIPTOR *)mHostContextCommon.Idtr.Base;
  for (Index = 0; Index < 0x100; Index++) {
    IdtGate[Index].Offset15To0 = (UINT16)((UINTN)AsmExceptionHandlers + Index * mExceptionHandlerLength);
    IdtGate[Index].Offset31To16 = (UINT16)(((UINTN)AsmExceptionHandlers + Index * mExceptionHandlerLength) >> 16);
#ifdef MDE_CPU_X64
    IdtGate[Index].Offset63To32 = (UINT32)(((UINTN)AsmExceptionHandlers + Index * mExceptionHandlerLength) >> 32);
#endif
  }
  //
  // Special for NMI
  //
  IdtGate[2].Offset15To0 = (UINT16)((UINTN)AsmNmiExceptionHandler);
  IdtGate[2].Offset31To16 = (UINT16)(((UINTN)AsmNmiExceptionHandler) >> 16);
#ifdef MDE_CPU_X64
  IdtGate[2].Offset63To32 = (UINT32)(((UINTN)AsmNmiExceptionHandler) >> 32);
#endif

  //
  // VmExitHandler
  //
  InitFrmHandler ();
  mTeardownFinished = AllocatePages (FRM_SIZE_TO_PAGES (mHostContextCommon.CpuNum));

  //
  // Init HostContextPerCpu
  //
  mApicIdList = (UINTN)AllocatePages (FRM_SIZE_TO_PAGES (sizeof(UINT32) * mHostContextCommon.CpuNum));
  GetApicIdListFromAcpi ((UINT32 *)mApicIdList);

  for (Index = 0; Index < mHostContextCommon.CpuNum; Index++) {
    mHostContextCommon.HostContextPerCpu[Index].Index = Index;
    mHostContextCommon.HostContextPerCpu[Index].ApicId = *((UINT32 *)mApicIdList + Index);

    mHostContextCommon.HostContextPerCpu[Index].Stack = (UINTN)AllocatePages (32);
    mHostContextCommon.HostContextPerCpu[Index].Stack += FRM_PAGES_TO_SIZE (32);

    InitHostVmcs (Index);
  }
}

/**

  This function initialize host context per CPU.

  @param Index   CPU Index

**/
VOID
InitHostContextPerCpu (
  IN UINT32 Index
  )
{
  //
  // VmxOn for this CPU
  //
  AsmWbinvd ();
  AsmWriteCr3 (mHostContextCommon.PageTable);
  AsmWriteCr4 (AsmReadCr4 () | CR4_PAE | ((UINTN)AsmReadMsr64 (IA32_VMX_CR4_FIXED0_MSR_INDEX) & (UINTN)AsmReadMsr64 (IA32_VMX_CR4_FIXED1_MSR_INDEX)));
  AsmWriteCr0 (AsmReadCr0 () | ((UINTN)AsmReadMsr64 (IA32_VMX_CR0_FIXED0_MSR_INDEX) & (UINTN)AsmReadMsr64 (IA32_VMX_CR0_FIXED1_MSR_INDEX)));
  AsmVmxOn (&mHostContextCommon.HostContextPerCpu[Index].Vmcs);
}

/**

  This function initialize host context.

**/
VOID
InitHostContext (
  VOID
  )
{
  UINT32  Index;

  //
  // Set up common data structure
  //
  InitHostContextCommon ();

  //
  // Init BSP
  //
  InitHostContextPerCpu (mBspIndex);

  // Backup
  CopyMem ((VOID *)(UINTN)mHostContextCommon.LowMemoryBackupBase, (VOID *)(UINTN)mHostContextCommon.LowMemoryBase, (UINTN)mHostContextCommon.LowMemorySize);

  //
  // Wakeup AP for init, because we need AP run VMXON instruction
  //
  mApFinished = AllocatePages (FRM_SIZE_TO_PAGES (mHostContextCommon.CpuNum));
  mApFinished[mBspIndex] = TRUE;
  InitAllAps ();

  if (IsMleLaunched()) {
    DEBUG((EFI_D_INFO, "(TXT) TxtWakeUpRlps ...\n"));
#ifdef MDE_CPU_X64
    TxtWakeUpRlps((UINT32)((UINTN)AsmApWakeup32 + mLongModeEntry));
#else
    TxtWakeUpRlps((UINT32)((UINTN)AsmApWakeup32));
#endif
  } else {
    DEBUG((EFI_D_INFO, "(FRM) WakeupAllAps ...\n"));
    WakeupAllAps();
  }

  // Wait
  for (Index = 0; Index < mHostContextCommon.CpuNum; Index++) {
    while (!mApFinished[Index]) {
      ; // WAIT
    }
  }
  DEBUG((EFI_D_INFO, "(FRM) mApFinished\n"));

  // OK All AP finished

  // Restore
  CopyMem ((VOID *)(UINTN)mHostContextCommon.LowMemoryBase, (VOID *)(UINTN)mHostContextCommon.LowMemoryBackupBase, (UINTN)mHostContextCommon.LowMemorySize);
}

//
// Guest related init
//

/**

  This function is guest entrypoint during initialization.

**/
VOID
GuestEntrypoint (
  VOID
  )
{
  DEBUG ((EFI_D_INFO, "(FRM) !!!Entry Guest!!!\n"));
  LongJump (&mGuestJumpBuffer, (UINTN)-1);
}

/**

  This function initialize guest VMCS.

**/
VOID
InitGuestVmcs (
  IN UINT32 Index
  )
{
  IA32_VMX_BASIC_MSR  VmxBasicMsr;

  //
  // VMCS size
  //
  VmxBasicMsr.Uint64 = AsmReadMsr64 (IA32_VMX_BASIC_MSR_INDEX);

  //
  // Allocate
  //
  mGuestContextCommon.GuestContextPerCpu[Index].Vmcs = (UINT64)(UINTN)AllocatePages (FRM_SIZE_TO_PAGES((UINTN)VmxBasicMsr.Bits.VmcsRegionSize));

  //
  // Set RevisionIdentifier
  //
  *(UINT32 *)(UINTN)mGuestContextCommon.GuestContextPerCpu[Index].Vmcs = (UINT32)VmxBasicMsr.Bits.RevisionIdentifier;

  return ;
}

/**

  This function initialize guest common context.

**/
VOID
InitGuestContextCommon (
  VOID
  )
{
  UINT32  Index;

  //
  // CompatiblePageTable for IA32 flat mode only
  //
  mGuestContextCommon.CompatiblePageTable = CreateCompatiblePageTable ();
  mGuestContextCommon.CompatiblePageTablePae = CreateCompatiblePageTablePae ();

  mGuestContextCommon.MsrBitmap = (UINT64)(UINTN)AllocatePages (1);

  EptInit ();

  IoInit ();

  VmxTimerInit ();

  //
  // Init GuestContextPerCpu
  //
  for (Index = 0; Index < mHostContextCommon.CpuNum; Index++) {
    mGuestContextCommon.GuestContextPerCpu[Index].Stack = (UINTN)AllocatePages (1);
    mGuestContextCommon.GuestContextPerCpu[Index].Stack += FRM_PAGES_TO_SIZE (1);

    mGuestContextCommon.GuestContextPerCpu[Index].Cr0 = mGuestContextCommon.GuestContextPerCpu[mBspIndex].Cr0;
    mGuestContextCommon.GuestContextPerCpu[Index].Cr3 = mGuestContextCommon.GuestContextPerCpu[mBspIndex].Cr3;
    mGuestContextCommon.GuestContextPerCpu[Index].Cr4 = mGuestContextCommon.GuestContextPerCpu[mBspIndex].Cr4;
    CopyMem (&mGuestContextCommon.GuestContextPerCpu[Index].Gdtr, &mGuestContextCommon.GuestContextPerCpu[mBspIndex].Gdtr, sizeof(IA32_DESCRIPTOR));
    CopyMem (&mGuestContextCommon.GuestContextPerCpu[Index].Idtr, &mGuestContextCommon.GuestContextPerCpu[mBspIndex].Idtr, sizeof(IA32_DESCRIPTOR));

    mGuestContextCommon.GuestContextPerCpu[Index].VmExitMsrStore = (UINT64)(UINTN)AllocatePages (1);
    mGuestContextCommon.GuestContextPerCpu[Index].VmExitMsrLoad  = (UINT64)(UINTN)AllocatePages (1);
    mGuestContextCommon.GuestContextPerCpu[Index].VmEnterMsrLoad = (UINT64)(UINTN)AllocatePages (1);

    //
    // Allocate GuestVmcs
    //
    InitGuestVmcs (Index);
  }

}

/**

  This function initialize guest context per CPU.

  @param Index   CPU Index

**/
VOID
InitGuestContextPerCpu (
  IN UINT32 Index
  )
{
  mGuestContextCommon.GuestContextPerCpu[Index].EFER = AsmReadMsr64 (IA32_EFER_MSR_INDEX);

  //
  // Load current VMCS, after that we can access VMCS region
  //
  AsmVmClear (&mGuestContextCommon.GuestContextPerCpu[Index].Vmcs);
  AsmVmPtrLoad (&mGuestContextCommon.GuestContextPerCpu[Index].Vmcs);

  SetVmcsHostField (Index);
  SetVmcsGuestField (Index);
  SetVmcsControlField (Index);
}

/**

  This function initialize guest context.

**/
VOID
InitGuestContext (
  VOID
  )
{
  //
  // Setup common data structure
  //
  InitGuestContextCommon ();

  InitGuestContextPerCpu (mBspIndex);
}

/**

  This function launch guest BSP.

**/
VOID
LaunchGuestBsp (
  VOID
  )
{
  UINTN             Flag;
  UINTN             Rflags;

  Flag = SetJump (&mGuestJumpBuffer);
  if (Flag != 0) {
    DEBUG ((EFI_D_INFO, "(FRM) !!!Guest JumpBack to EFI!!!\n"));
    // Run successfully
    return ;
  }

  AsmVmPtrLoad (&mGuestContextCommon.GuestContextPerCpu[mBspIndex].Vmcs);

  //
  // Launch STM
  //
  LaunchStm (mBspIndex);

  AsmVmPtrLoad (&mGuestContextCommon.GuestContextPerCpu[mBspIndex].Vmcs);

  AcquireSpinLock (&mHostContextCommon.DebugLock);
  DumpVmcsAllField ();
  ReleaseSpinLock (&mHostContextCommon.DebugLock);

  AsmWbinvd ();

  Rflags = AsmVmLaunch (&mGuestContextCommon.GuestContextPerCpu[mBspIndex].Register);
  DEBUG ((EFI_D_ERROR, "(FRM) !!!LaunchGuestBsp FAIL!!!\n"));
  DEBUG ((EFI_D_ERROR, "(FRM) Rflags: %08x\n", Rflags));
  DEBUG ((EFI_D_ERROR, "(FRM) VMCS_32_RO_VM_INSTRUCTION_ERROR: %08x\n", (UINTN)VmRead32 (VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX)));
 
  CpuDeadLoop ();
}

/**

  This function launch guest AP.

**/
VOID
LaunchGuestAp (
  VOID
  )
{
  mApLaunch = TRUE;
}

/**

  This function launch guest.

**/
VOID
LauchGuest (
  VOID
  )
{
  //
  // For AP, deadloop
  //
  LaunchGuestAp ();

  //
  // For BSP, long jump back
  //
  LaunchGuestBsp ();
}

/**

  This function initialize guest BSP in S3.

**/
VOID
BspS3Init (
  VOID
  )
{
  UINT32  Index;

  mGuestContextCommon.GuestContextPerCpu[mBspIndex].Cr0 = AsmReadCr0 ();
  mGuestContextCommon.GuestContextPerCpu[mBspIndex].Cr3 = AsmReadCr3 ();
  mGuestContextCommon.GuestContextPerCpu[mBspIndex].Cr4 = AsmReadCr4 ();
  AsmReadGdtr (&mGuestContextCommon.GuestContextPerCpu[mBspIndex].Gdtr);
  AsmReadIdtr (&mGuestContextCommon.GuestContextPerCpu[mBspIndex].Idtr);

  InitHostContextPerCpu (mBspIndex);

  for (Index = 0; Index < mHostContextCommon.CpuNum; Index++) {
    mGuestContextCommon.GuestContextPerCpu[Index].Cr0 = mGuestContextCommon.GuestContextPerCpu[mBspIndex].Cr0;
    mGuestContextCommon.GuestContextPerCpu[Index].Cr3 = mGuestContextCommon.GuestContextPerCpu[mBspIndex].Cr3;
    mGuestContextCommon.GuestContextPerCpu[Index].Cr4 = mGuestContextCommon.GuestContextPerCpu[mBspIndex].Cr4;
    CopyMem (&mGuestContextCommon.GuestContextPerCpu[Index].Gdtr, &mGuestContextCommon.GuestContextPerCpu[mBspIndex].Gdtr, sizeof(IA32_DESCRIPTOR));
  }
  InitGuestContextPerCpu (mBspIndex);
}

/**

  This function initialize guest AP in S3.

**/
VOID
ApS3Init (
  VOID
  )
{
  UINT32  Index;

  // Backup
  CopyMem ((VOID *)(UINTN)mHostContextCommon.LowMemoryBackupBase, (VOID *)(UINTN)mHostContextCommon.LowMemoryBase, (UINTN)mHostContextCommon.LowMemorySize);

  //
  // Init env
  //
  mApLaunch = FALSE;
  for (Index = 0; Index < mHostContextCommon.CpuNum; Index++) {
    mApFinished[Index] = FALSE;
  }

  mApFinished[mBspIndex] = TRUE;
  WakeupAllAps ();

  // Wait
  for (Index = 0; Index < mHostContextCommon.CpuNum; Index++) {
    while (!mApFinished[Index]) {
      ; // WAIT
    }
  }

  // Restore
  CopyMem ((VOID *)(UINTN)mHostContextCommon.LowMemoryBase, (VOID *)(UINTN)mHostContextCommon.LowMemoryBackupBase, (UINTN)mHostContextCommon.LowMemorySize);
}


BOOLEAN  mAlreadyEntered = FALSE;

/**

  This is FRM entrypoint in S3.

**/
RETURN_STATUS
FrmS3Entrypoint (
  VOID
  )
{
  DEBUG ((EFI_D_INFO, "(FRM) !!!FrmS3Entrypoint!!!\n"));
  DEBUG ((EFI_D_INFO, "(FRM) !!!FRM build time - %a %a!!!\n", (CHAR8 *)__DATE__, (CHAR8 *)__TIME__));

  BspS3Init ();
  ApS3Init (); 

  LauchGuest ();
  return RETURN_SUCCESS;
}

/**

  This is FRM module entrypoint.

  @param CommunicationData  FRM communication data.

  @retval RETURN_SUCCESS     FRM is launched.
  @retval RETURN_UNSUPPORTED FRM is unsupproted.

**/
RETURN_STATUS
_ModuleEntryPoint (
  IN FRM_COMMUNICATION_DATA    *CommunicationData
  )
{
  BOOLEAN        InterruptEnabled;
  RETURN_STATUS  Status;

  if ((AsmReadMsr64 (IA32_FEATURE_CONTROL_MSR_INDEX) & IA32_FEATURE_CONTROL_VMX) == 0) {
    DEBUG ((EFI_D_ERROR, "(FRM) !!!VMX not enabled!\n"));
    return RETURN_UNSUPPORTED;
  }

  if (mAlreadyEntered) {
    return FrmS3Entrypoint ();
  }

  InterruptEnabled = SaveAndDisableInterrupts ();

  if (sizeof(UINTN) == sizeof(UINT32)) {
    DEBUG ((EFI_D_INFO, "(FRM) !!!FrmEntrypoint32!!!\n"));
  } else {
    DEBUG ((EFI_D_INFO, "(FRM) !!!FrmEntrypoint64!!!\n"));
  }
  DEBUG ((EFI_D_INFO, "(FRM) !!!FRM build time - %a %a!!!\n", (CHAR8 *)__DATE__, (CHAR8 *)__TIME__));

  CopyMem (&mCommunicationData, CommunicationData, sizeof(mCommunicationData));

  DumpVmxCapabillityMsr ();

  DEBUG ((EFI_D_INFO, "(FRM) HighMemoryBase    - %016lx\n", mCommunicationData.HighMemoryBase));
  DEBUG ((EFI_D_INFO, "(FRM) HighMemorySize    - %016lx\n", mCommunicationData.HighMemorySize));
  DEBUG ((EFI_D_INFO, "(FRM) LowMemoryBase     - %016lx\n", mCommunicationData.LowMemoryBase));
  DEBUG ((EFI_D_INFO, "(FRM) LowMemorySize     - %016lx\n", mCommunicationData.LowMemorySize));
  DEBUG ((EFI_D_INFO, "(FRM) ImageBase         - %016lx\n", mCommunicationData.ImageBase));
  DEBUG ((EFI_D_INFO, "(FRM) ImageSize         - %016lx\n", mCommunicationData.ImageSize));
  DEBUG ((EFI_D_INFO, "(FRM) TimerPeriod       - %016lx\n", mCommunicationData.TimerPeriod));
  DEBUG ((EFI_D_INFO, "(FRM) AcpiRsdp          - %016lx\n", mCommunicationData.AcpiRsdp));
  DEBUG ((EFI_D_INFO, "(FRM) SmMonitorService  - %016lx\n", mCommunicationData.SmMonitorServiceProtocol));
  DEBUG ((EFI_D_INFO, "(FRM) SmMonitorBase     - %016lx\n", mCommunicationData.SmMonitorServiceImageBase));
  DEBUG ((EFI_D_INFO, "(FRM) SmMonitorSize     - %016lx\n", mCommunicationData.SmMonitorServiceImageSize));
  DEBUG ((EFI_D_INFO, "(TXT) MeasuredImageSize - %016lx\n", mCommunicationData.MeasuredImageSize));
  DEBUG ((EFI_D_INFO, "(TXT) SinitAcmBase      - %016lx\n", mCommunicationData.SinitAcmBase));
  DEBUG ((EFI_D_INFO, "(TXT) SinitAcmSize      - %016lx\n", mCommunicationData.SinitAcmSize));
  DEBUG ((EFI_D_INFO, "(TXT) LcpPoBase         - %016lx\n", mCommunicationData.LcpPoBase));
  DEBUG ((EFI_D_INFO, "(TXT) LcpPoSize         - %016lx\n", mCommunicationData.LcpPoSize));
  DEBUG ((EFI_D_INFO, "(TXT) EventLogBase      - %016lx\n", mCommunicationData.EventLogBase));
  DEBUG ((EFI_D_INFO, "(TXT) EventLogSize      - %016lx\n", mCommunicationData.EventLogSize));
  DEBUG ((EFI_D_INFO, "(TXT) DprBase           - %016lx\n", mCommunicationData.DprBase));
  DEBUG ((EFI_D_INFO, "(TXT) DprSize           - %016lx\n", mCommunicationData.DprSize));
  DEBUG ((EFI_D_INFO, "(TXT) PmrLowBase        - %016lx\n", mCommunicationData.PmrLowBase));
  DEBUG ((EFI_D_INFO, "(TXT) PmrLowSize        - %016lx\n", mCommunicationData.PmrLowSize));
  DEBUG ((EFI_D_INFO, "(TXT) PmrHighBase       - %016lx\n", mCommunicationData.PmrLowBase));
  DEBUG ((EFI_D_INFO, "(TXT) PmrHighSize       - %016lx\n", mCommunicationData.PmrLowSize));
  DEBUG ((EFI_D_INFO, "(TXT) TpmType           - %08x\n", mCommunicationData.TpmType));
  DEBUG ((EFI_D_INFO, "(TXT) ActivePcrBanks    - %08x\n", mCommunicationData.TpmType));
  
  mHostContextCommon.ImageBase = mCommunicationData.ImageBase;
  mHostContextCommon.ImageSize = mCommunicationData.ImageSize;

  //
  // Prepare heap, then we can use memory service
  //
  InitHeap ();
  // after that we can use mHostContextCommon

  InitializeSpinLock (&mHostContextCommon.DebugLock);
  // after that we can use AcquireSpinLock/ReleaseSpinLock (&mHostContextCommon.DebugLock) to control block level debug.

  InitializeSpinLock (&mHostContextCommon.MemoryLock);
  // after that we can use MemoryServices

  InitializeTimer();
  // after that we can use MicroSecondDelay()

  Status = DceLoaderEntrypoint();
  if (!EFI_ERROR(Status)) {
    DlEntry();
  }

  InitBasicContext ();

  InitHostContext ();

  InitGuestContext ();

  LauchGuest ();

  mAlreadyEntered = TRUE;

  if (InterruptEnabled) {
    EnableInterrupts();
  }
  return RETURN_SUCCESS;
}

