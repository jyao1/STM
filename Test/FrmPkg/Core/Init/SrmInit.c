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

#define MAX_RESOURCE_PAGES 4

extern UINT32   mCpuNum;
extern FRM_COMMUNICATION_DATA    mCommunicationData;

volatile BOOLEAN   mStmBspDone = FALSE;
volatile UINT32    mApFinishCount = 0;

UINT32  mSrmGuestId;

/**

  This function dump STM resource node header.

  @param Resource STM resource node

**/
VOID
DumpStmResourceHeader (
  IN STM_RSC   *Resource
  )
{
  DEBUG ((EFI_D_INFO, "(FRM)   RscType       : %08x\n", Resource->Header.RscType));
  DEBUG ((EFI_D_INFO, "(FRM)   RscLength     : %04x\n", Resource->Header.Length));
  DEBUG ((EFI_D_INFO, "(FRM)   ReturnStatus  : %04x\n", Resource->Header.ReturnStatus));
  DEBUG ((EFI_D_INFO, "(FRM)   IgnoreResource: %04x\n", Resource->Header.IgnoreResource));
}

/**

  This function dump STM resource node.

  @param Resource STM resource node

**/
VOID
DumpStmResourceNode (
  IN STM_RSC   *Resource
  )
{
  UINT8   PciIndex;

  switch (Resource->Header.RscType) {
  case END_OF_RESOURCES:
    DEBUG ((EFI_D_INFO, "(FRM) END_OF_RESOURCES:\n"));
    DumpStmResourceHeader (Resource);
    DEBUG ((EFI_D_INFO, "(FRM)   ResourceListContinuation : %016lx\n", Resource->End.ResourceListContinuation));
    break;
  case MEM_RANGE:
    DEBUG ((EFI_D_INFO, "(FRM) MEM_RANGE:\n"));
    DumpStmResourceHeader (Resource);
    DEBUG ((EFI_D_INFO, "(FRM)   Base          : %016lx\n", Resource->Mem.Base));
    DEBUG ((EFI_D_INFO, "(FRM)   Length        : %016lx\n", Resource->Mem.Length));
    DEBUG ((EFI_D_INFO, "(FRM)   RWXAttributes : %08x\n",   (UINTN)Resource->Mem.RWXAttributes));
    break;
  case IO_RANGE:
    DEBUG ((EFI_D_INFO, "(FRM) IO_RANGE:\n"));
    DumpStmResourceHeader (Resource);
    DEBUG ((EFI_D_INFO, "(FRM)   Base          : %04x\n",   (UINTN)Resource->Io.Base));
    DEBUG ((EFI_D_INFO, "(FRM)   Length        : %04x\n",   (UINTN)Resource->Io.Length));
    break;
  case MMIO_RANGE:
    DEBUG ((EFI_D_INFO, "(FRM) MMIO_RANGE:\n"));
    DumpStmResourceHeader (Resource);
    DEBUG ((EFI_D_INFO, "(FRM)   Base          : %016lx\n", Resource->Mmio.Base));
    DEBUG ((EFI_D_INFO, "(FRM)   Length        : %016lx\n", Resource->Mmio.Length));
    DEBUG ((EFI_D_INFO, "(FRM)   RWXAttributes : %08x\n",   (UINTN)Resource->Mmio.RWXAttributes));
    break;
  case MACHINE_SPECIFIC_REG:
    DEBUG ((EFI_D_INFO, "(FRM) MSR_RANGE:\n"));
    DumpStmResourceHeader (Resource);
    DEBUG ((EFI_D_INFO, "(FRM)   MsrIndex      : %08x\n",   (UINTN)Resource->Msr.MsrIndex));
    DEBUG ((EFI_D_INFO, "(FRM)   KernelModeProc: %08x\n",   (UINTN)Resource->Msr.KernelModeProcessing));
    DEBUG ((EFI_D_INFO, "(FRM)   ReadMask      : %016lx\n", Resource->Msr.ReadMask));
    DEBUG ((EFI_D_INFO, "(FRM)   WriteMask     : %016lx\n", Resource->Msr.WriteMask));
    break;
  case PCI_CFG_RANGE:
    DEBUG ((EFI_D_INFO, "(FRM) PCI_CFG_RANGE:\n"));
    DumpStmResourceHeader (Resource);
    DEBUG ((EFI_D_INFO, "(FRM)   RWAttributes  : %04x\n",   (UINTN)Resource->PciCfg.RWAttributes));
    DEBUG ((EFI_D_INFO, "(FRM)   Base          : %04x\n",   (UINTN)Resource->PciCfg.Base));
    DEBUG ((EFI_D_INFO, "(FRM)   Length        : %04x\n",   (UINTN)Resource->PciCfg.Length));
    DEBUG ((EFI_D_INFO, "(FRM)   OriginatingBus: %02x\n",   (UINTN)Resource->PciCfg.OriginatingBusNumber));
    DEBUG ((EFI_D_INFO, "(FRM)   LastNodeIndex : %02x\n",   (UINTN)Resource->PciCfg.LastNodeIndex));
    for (PciIndex = 0; PciIndex < Resource->PciCfg.LastNodeIndex + 1; PciIndex++) {
      DEBUG ((EFI_D_INFO, "(FRM)   Type          : %02x\n",   (UINTN)Resource->PciCfg.PciDevicePath[PciIndex].Type));
      DEBUG ((EFI_D_INFO, "(FRM)   Subtype       : %02x\n",   (UINTN)Resource->PciCfg.PciDevicePath[PciIndex].Subtype));
      DEBUG ((EFI_D_INFO, "(FRM)   Length        : %04x\n",   (UINTN)Resource->PciCfg.PciDevicePath[PciIndex].Length));
      DEBUG ((EFI_D_INFO, "(FRM)   PciDevice     : %02x\n",   (UINTN)Resource->PciCfg.PciDevicePath[PciIndex].PciDevice));
      DEBUG ((EFI_D_INFO, "(FRM)   PciFunction   : %02x\n",   (UINTN)Resource->PciCfg.PciDevicePath[PciIndex].PciFunction));
    }
    break;
  case TRAPPED_IO_RANGE:
    DEBUG ((EFI_D_INFO, "(FRM) TRAPPED_IO_RANGE:\n"));
    DumpStmResourceHeader (Resource);
    DEBUG ((EFI_D_INFO, "(FRM)   Base          : %04x\n",   (UINTN)Resource->TrappedIo.Base));
    DEBUG ((EFI_D_INFO, "(FRM)   Length        : %04x\n",   (UINTN)Resource->TrappedIo.Length));
    DEBUG ((EFI_D_INFO, "(FRM)   In            : %04x\n",   (UINTN)Resource->TrappedIo.In));
    DEBUG ((EFI_D_INFO, "(FRM)   Out           : %04x\n",   (UINTN)Resource->TrappedIo.Out));
    DEBUG ((EFI_D_INFO, "(FRM)   Api           : %04x\n",   (UINTN)Resource->TrappedIo.Api));
    break;
  case ALL_RESOURCES:
    DEBUG ((EFI_D_INFO, "(FRM) ALL_RESOURCES:\n"));
    DumpStmResourceHeader (Resource);
    break;
  case REGISTER_VIOLATION:
    DEBUG ((EFI_D_INFO, "(FRM) REGISTER_VIOLATION:\n"));
    DumpStmResourceHeader (Resource);
    DEBUG ((EFI_D_INFO, "(FRM)   RegisterType  : %08x\n",   (UINTN)Resource->RegisterViolation.RegisterType));
    DEBUG ((EFI_D_INFO, "(FRM)   ReadMask      : %016lx\n", Resource->RegisterViolation.ReadMask));
    DEBUG ((EFI_D_INFO, "(FRM)   WriteMask     : %016lx\n", Resource->RegisterViolation.WriteMask));
    break;
  default:
    DumpStmResourceHeader (Resource);
    break;
  }
}

/**

  This function dump STM resource list.

  @param Resource STM resource list

**/
VOID
DumpStmResource (
  IN STM_RSC   *Resource
  )
{
  while (Resource->Header.RscType != END_OF_RESOURCES) {
    DumpStmResourceNode (Resource);
    Resource = (STM_RSC *)((UINTN)Resource + Resource->Header.Length);
  }
  // Dump End Node
  DumpStmResourceNode (Resource);

  if (Resource->End.ResourceListContinuation != 0) {
    DumpStmResource ((STM_RSC *)(UINTN)Resource->End.ResourceListContinuation);
  }
}

/**

  This function launch STM.

  @param CpuIndex   CPU Index

**/
VOID
LaunchStm (
  IN UINTN                           CpuIndex
  )
{
  UINT64                             TempVmcs;
  UINT64                             CurrentVmcs;
  UINT64                             Data64;
  UINTN                              VmcsSize;
  EFI_SM_MONITOR_SERVICE_PROTOCOL    *SmMonitorServiceProtocol;

  Data64 = AsmReadMsr64 (IA32_SMM_MONITOR_CTL_MSR_INDEX);
  if ((Data64 & IA32_SMM_MONITOR_VALID) == 0) {
    return ;
  }

  SmMonitorServiceProtocol = (EFI_SM_MONITOR_SERVICE_PROTOCOL *)(UINTN)mCommunicationData.SmMonitorServiceProtocol;
  if (SmMonitorServiceProtocol == NULL) {
    return ;
  }

  //
  // VMCS size
  //
  Data64 = AsmReadMsr64 (IA32_VMX_BASIC_MSR_INDEX);
  VmcsSize = (UINTN)(RShiftU64 (Data64, 32) & 0xFFFF);

  //
  // Setup temp VMCS
  //
  TempVmcs = (UINT64)(UINTN)AllocatePages (FRM_SIZE_TO_PAGES (VmcsSize));

  AsmVmPtrStore (&CurrentVmcs);

  AsmVmClear (&CurrentVmcs);
  CopyMem ((VOID *)(UINTN)TempVmcs, (VOID *)(UINTN)CurrentVmcs, VmcsSize);

  //
  // Load temp VMCS
  //
  AsmVmPtrLoad (&TempVmcs);

  AsmWbinvd ();

  // Check BIOS resource (assume same on every CPU)
  if (CpuIndex == 0) {
    // Init protection
    DEBUG ((EFI_D_INFO, "(FRM) InitializeProtection(%d)\n", CpuIndex));
    SmMonitorServiceProtocol->InitializeProtection ();
    
    DEBUG ((EFI_D_INFO, "(FRM) ReturnPiResource(%d)\n", CpuIndex));
    {
      VOID   *Resource;
      UINT32 ResourceSize;

      Resource = AllocatePages (MAX_RESOURCE_PAGES);
      ZeroMem (Resource, FRM_PAGES_TO_SIZE(MAX_RESOURCE_PAGES));
      ResourceSize = FRM_PAGES_TO_SIZE(MAX_RESOURCE_PAGES);
      SmMonitorServiceProtocol->ReturnPiResource (Resource, &ResourceSize);
      DumpStmResource (Resource);
      FreePages (Resource, MAX_RESOURCE_PAGES);
    }
    
    DEBUG ((EFI_D_INFO, "(FRM) ProtectOsResource(%d)\n", CpuIndex));
    {
      VOID             *Resource;
      STM_RSC_MEM_DESC *StmMem;

      Resource = AllocatePages (MAX_RESOURCE_PAGES);
      ZeroMem (Resource, FRM_PAGES_TO_SIZE(MAX_RESOURCE_PAGES));
      StmMem = (STM_RSC_MEM_DESC *)Resource;
      StmMem->Hdr.RscType = MEM_RANGE;
      StmMem->Hdr.Length = sizeof(STM_RSC_MEM_DESC);
      StmMem->Base = mHostContextCommon.HeapBottom;
      StmMem->Length = mHostContextCommon.HeapTop - mHostContextCommon.HeapBottom;
      StmMem->RWXAttributes = STM_RSC_MEM_R|STM_RSC_MEM_W|STM_RSC_MEM_X;

      StmMem++;
      StmMem->Hdr.RscType = MEM_RANGE;
      StmMem->Hdr.Length = sizeof(STM_RSC_MEM_DESC);
      StmMem->Base = mHostContextCommon.ImageBase;
      StmMem->Length = FRM_PAGES_TO_SIZE(FRM_SIZE_TO_PAGES((UINTN)mHostContextCommon.ImageSize));
      StmMem->RWXAttributes = STM_RSC_MEM_R|STM_RSC_MEM_W|STM_RSC_MEM_X;

      StmMem++;
      StmMem->Hdr.RscType = MEM_RANGE;
      StmMem->Hdr.Length = sizeof(STM_RSC_MEM_DESC);
      StmMem->Base = mCommunicationData.SmMonitorServiceImageBase;
      StmMem->Length = FRM_PAGES_TO_SIZE(FRM_SIZE_TO_PAGES((UINTN)mCommunicationData.SmMonitorServiceImageSize));
      StmMem->RWXAttributes = STM_RSC_MEM_R|STM_RSC_MEM_W|STM_RSC_MEM_X;

      SmMonitorServiceProtocol->ProtectOsResource (Resource, 0);
      FreePages (Resource, MAX_RESOURCE_PAGES);
    }

    DEBUG ((EFI_D_INFO, "(FRM) UnprotectOsResource(%d)\n", CpuIndex));
    {
      SmMonitorServiceProtocol->UnprotectOsResource (0, 0);
    }

    // Start STM
    DEBUG ((EFI_D_INFO, "(FRM) Start(%d)\n", CpuIndex));
    SmMonitorServiceProtocol->Start ();
    mStmBspDone = TRUE;

    while ((mApFinishCount + 1) != mCpuNum) {
      ;
    }
  } else {
    //
    // Wait until done by BSP
    //
    while (!mStmBspDone) {
      ;
    }

    AcquireSpinLock (&mHostContextCommon.DebugLock);
    // Start STM
    DEBUG ((EFI_D_INFO, "(FRM) Start(%d)\n", CpuIndex));
    SmMonitorServiceProtocol->Start ();
    InterlockedIncrement ((UINT32 *)&mApFinishCount);
    ReleaseSpinLock (&mHostContextCommon.DebugLock);
  }

  AsmWbinvd ();

  //
  // Load Current VMCS
  //
  AsmVmPtrLoad (&CurrentVmcs);

  return ;
}
