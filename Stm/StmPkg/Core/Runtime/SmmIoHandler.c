/** @file
  SMM IO handler

  Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmRuntime.h"

#define BUS_FROM_CF8_ADDRESS(PciAddress)       (UINT8)(((UINTN)(PciAddress) & 0x00FF0000) >> 16)
#define DEVICE_FROM_CF8_ADDRESS(PciAddress)    (UINT8)(((UINTN)(PciAddress) & 0x0000F800) >> 13)
#define FUNCTION_FROM_CF8_ADDRESS(PciAddress)  (UINT8)(((UINTN)(PciAddress) & 0x00000700) >> 8)
#define REGISTER_FROM_CF8_ADDRESS(PciAddress)  (UINT16)((UINTN)(PciAddress) & 0x000000FC)

/**

  This function translate guest linear address to host address.

  @param CpuIndex           CPU index
  @param GuestLinearAddress Guest linear address

  @return Host physical address
**/
UINTN
GuestLinearToHostPhysical (
  UINT32  CpuIndex,
  UINTN   GuestLinearAddress
  );

/**

  This function is IO instruction handler for SMM.

  @param Index CPU index

**/
VOID
SmmIoHandler (
  IN UINT32 Index
  )
{
  VM_EXIT_QUALIFICATION   Qualification;
  UINT16                  Port;
  UINTN                   *DataPtr;
  UINTN                   LinearAddr;
  X86_REGISTER            *Reg;
  STM_RSC_IO_DESC         *IoDesc;
  STM_RSC_PCI_CFG_DESC    *PciCfgDesc;
  UINT32                  PciAddress;
  STM_RSC_IO_DESC         LocalIoDesc;
  STM_RSC_PCI_CFG_DESC    *LocalPciCfgDescPtr;
  UINT8                   LocalPciCfgDescBuf[STM_LOG_ENTRY_SIZE];

  Reg = &mGuestContextCommonSmm.GuestContextPerCpu[Index].Register;

  Qualification.UintN = VmReadN (VMCS_N_RO_EXIT_QUALIFICATION_INDEX);

  if (Qualification.IoInstruction.Operand != 0) {
    Port = (UINT16)Qualification.IoInstruction.PortNum;
  } else {
    Port = (UINT16)Reg->Rdx;
  }
  DataPtr = (UINTN *)&Reg->Rax;

  //
  // We need handle case that CF9 is protected, but CF8, CFC need to be pass-through.
  // But DWORD CF8 programming will be caught here.
  // So add check here. CF8 will be bypassed, because it does not in CF9 scope.
  //
  IoDesc = GetStmResourceIo (mHostContextCommon.MleProtectedResource.Base, Port);
  if (IoDesc != NULL) {
    DEBUG ((EFI_D_ERROR, "IO violation!\n"));
    AddEventLogForResource (EvtHandledProtectionException, (STM_RSC *)IoDesc);
    SmmExceptionHandler (Index);
    CpuDeadLoop ();
  }

  IoDesc = GetStmResourceIo ((STM_RSC *)(UINTN)mGuestContextCommonSmm.BiosHwResourceRequirementsPtr, Port);
  if (IoDesc == NULL) {
    ZeroMem (&LocalIoDesc, sizeof(LocalIoDesc));
    LocalIoDesc.Hdr.RscType = IO_RANGE;
    LocalIoDesc.Hdr.Length = sizeof(LocalIoDesc);
    LocalIoDesc.Base = Port;
    LocalIoDesc.Length = (UINT16)(Qualification.IoInstruction.Size + 1);
    AddEventLogForResource (EvtBiosAccessToUnclaimedResource, (STM_RSC *)&LocalIoDesc);
  }

  //
  // Check PCI - 0xCF8, 0xCFC~0xCFF access
  //
  if (Port == 0xCF8) {
    // Access PciAddress

    //
    // We need make sure PciAddress access and PciData access is atomic.
    //
    AcquireSpinLock (&mHostContextCommon.PciLock);
  }
  if ((Port >= 0xCFC) && (Port <= 0xCFF)) {
    // Access PciData

    //
    // AcquireLock to prevent 0xCF8 access
    //
    AcquireSpinLock (&mHostContextCommon.PciLock);
    PciAddress = IoRead32 (0xCF8);
    PciCfgDesc = GetStmResourcePci (
                   mHostContextCommon.MleProtectedResource.Base,
                   BUS_FROM_CF8_ADDRESS(PciAddress),
                   DEVICE_FROM_CF8_ADDRESS(PciAddress),
                   FUNCTION_FROM_CF8_ADDRESS(PciAddress),
                   REGISTER_FROM_CF8_ADDRESS(PciAddress) + (Port & 0x3),
                   (Qualification.IoInstruction.Direction != 0) ? STM_RSC_PCI_CFG_R : STM_RSC_PCI_CFG_W
                   );
    if (PciCfgDesc != NULL) {
      DEBUG ((EFI_D_ERROR, "IO (PCI) violation!\n"));
      AddEventLogForResource (EvtHandledProtectionException, (STM_RSC *)PciCfgDesc);
      ReleaseSpinLock (&mHostContextCommon.PciLock);
      SmmExceptionHandler (Index);
      CpuDeadLoop ();
    }

    PciCfgDesc = GetStmResourcePci (
                   (STM_RSC *)(UINTN)mGuestContextCommonSmm.BiosHwResourceRequirementsPtr,
                   BUS_FROM_CF8_ADDRESS(PciAddress),
                   DEVICE_FROM_CF8_ADDRESS(PciAddress),
                   FUNCTION_FROM_CF8_ADDRESS(PciAddress),
                   REGISTER_FROM_CF8_ADDRESS(PciAddress) + (Port & 0x3),
                   (Qualification.IoInstruction.Direction != 0) ? STM_RSC_PCI_CFG_R : STM_RSC_PCI_CFG_W
                   );
    if (PciCfgDesc == NULL) {
      DEBUG((EFI_D_ERROR, "Add unclaimed PCI_RSC!\n"));
      LocalPciCfgDescPtr = (STM_RSC_PCI_CFG_DESC *)LocalPciCfgDescBuf;
      ZeroMem (LocalPciCfgDescBuf, sizeof(LocalPciCfgDescBuf));
      LocalPciCfgDescPtr->Hdr.RscType = PCI_CFG_RANGE;
      LocalPciCfgDescPtr->Hdr.Length = sizeof(STM_RSC_PCI_CFG_DESC); // BUGBUG: Just report this PCI device, it is hard to create PCI hierachy here.
      LocalPciCfgDescPtr->RWAttributes = (Qualification.IoInstruction.Direction != 0) ? STM_RSC_PCI_CFG_R : STM_RSC_PCI_CFG_W;
      LocalPciCfgDescPtr->Base = REGISTER_FROM_CF8_ADDRESS(PciAddress) + (Port & 0x3);
      LocalPciCfgDescPtr->Length = (UINT16)(Qualification.IoInstruction.Size + 1);
      LocalPciCfgDescPtr->OriginatingBusNumber = BUS_FROM_CF8_ADDRESS(PciAddress);
      LocalPciCfgDescPtr->LastNodeIndex = 0;
      LocalPciCfgDescPtr->PciDevicePath[0].Type = 1;
      LocalPciCfgDescPtr->PciDevicePath[0].Subtype = 1;
      LocalPciCfgDescPtr->PciDevicePath[0].Length = sizeof(STM_PCI_DEVICE_PATH_NODE);
      LocalPciCfgDescPtr->PciDevicePath[0].PciFunction = FUNCTION_FROM_CF8_ADDRESS(PciAddress);
      LocalPciCfgDescPtr->PciDevicePath[0].PciDevice = DEVICE_FROM_CF8_ADDRESS(PciAddress);
      AddEventLogForResource (EvtBiosAccessToUnclaimedResource, (STM_RSC *)LocalPciCfgDescPtr);
    }
  }

  if (Qualification.IoInstruction.Rep != 0) {
    UINT64 RcxMask;

    RcxMask = 0xFFFFFFFFFFFFFFFFull;
    if ((mGuestContextCommonSmm.GuestContextPerCpu[Index].Efer & IA32_EFER_MSR_MLA) == 0) {
      RcxMask = 0xFFFFFFFFull;
    }
    if ((Reg->Rcx & RcxMask) == 0) {
      // Skip
      if ((Port == 0xCF8) || ((Port >= 0xCFC) && (Port <= 0xCFF))) {
        ReleaseSpinLock (&mHostContextCommon.PciLock);
      }
      VmWriteN (VMCS_N_GUEST_RIP_INDEX, VmReadN(VMCS_N_GUEST_RIP_INDEX) + VmRead32(VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX));
      return ;
    }
  }

  if (Qualification.IoInstruction.String != 0) {
    LinearAddr = VmReadN (VMCS_N_RO_GUEST_LINEAR_ADDR_INDEX);
    if (VmReadN (VMCS_N_GUEST_CR0_INDEX) & CR0_PG) {
      DataPtr = (UINTN *)(UINTN)GuestLinearToHostPhysical (Index, LinearAddr);
    } else {
      DataPtr = (UINTN *)LinearAddr;
    }

    if ((VmReadN (VMCS_N_GUEST_RFLAGS_INDEX) & RFLAGS_DF) != 0) {
      if (Qualification.IoInstruction.Direction != 0) {
        Reg->Rdi -= Qualification.IoInstruction.Size + 1;
      } else {
        Reg->Rsi -= Qualification.IoInstruction.Size + 1;
      }
    } else {
      if (Qualification.IoInstruction.Direction != 0) {
        Reg->Rdi += Qualification.IoInstruction.Size + 1;
      } else {
        Reg->Rsi += Qualification.IoInstruction.Size + 1;
      }
    }
  }

  if (Qualification.IoInstruction.Direction != 0) { // IN
    switch (Qualification.IoInstruction.Size) {
    case 0:
      *(UINT8 *)DataPtr = IoRead8 (Port);
      goto Ret;
      break;
    case 1:
      *(UINT16 *)DataPtr = IoRead16 (Port);
      goto Ret;
      break;
    case 3:
      *(UINT32 *)DataPtr = IoRead32 (Port);
      goto Ret;
      break;
    default:
      break;
    }
  } else { // OUT
    switch (Qualification.IoInstruction.Size) {
    case 0:
      IoWrite8 (Port, (UINT8)*DataPtr);
      goto Ret;
      break;
    case 1:
      IoWrite16 (Port, (UINT16)*DataPtr);
      goto Ret;
      break;
    case 3:
      IoWrite32 (Port, (UINT32)*DataPtr);
      goto Ret;
      break;
    default:
      break;
    }
  }

  if ((Port == 0xCF8) || ((Port >= 0xCFC) && (Port <= 0xCFF))) {
    ReleaseSpinLock (&mHostContextCommon.PciLock);
  }
  DEBUG ((EFI_D_INFO, "!!!IoHandler!!!\n"));
  DumpVmcsAllField ();

  CpuDeadLoop ();

Ret:
  if ((Port == 0xCF8) || ((Port >= 0xCFC) && (Port <= 0xCFF))) {
    ReleaseSpinLock (&mHostContextCommon.PciLock);
  }
  if (Qualification.IoInstruction.Rep != 0) {
    // replay
    Reg->Rcx --;
    return ;
  }

  VmWriteN (VMCS_N_GUEST_RIP_INDEX, VmReadN(VMCS_N_GUEST_RIP_INDEX) + VmRead32(VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX));
  return ;
}
