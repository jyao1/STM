/** @file
  STM resource management

  Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Stm.h"
#include "StmRuntime.h"

#define DEFAULT_PROTECTED_DEFAULT_PAGES  4

/**

  This function dump STM resource node header.

  @param Resource STM resource node

**/
VOID
DumpStmResourceHeader (
  IN STM_RSC   *Resource
  )
{
  DEBUG ((EFI_D_INFO, "  RscType       : %08x\n", Resource->Header.RscType));
  DEBUG ((EFI_D_INFO, "  RscLength     : %04x\n", Resource->Header.Length));
  DEBUG ((EFI_D_INFO, "  ReturnStatus  : %04x\n", Resource->Header.ReturnStatus));
  DEBUG ((EFI_D_INFO, "  IgnoreResource: %04x\n", Resource->Header.IgnoreResource));
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
    DEBUG ((EFI_D_INFO, "END_OF_RESOURCES:\n"));
    DumpStmResourceHeader (Resource);
    DEBUG ((EFI_D_INFO, "  ResourceListContinuation : %016lx\n", Resource->End.ResourceListContinuation));
    break;
  case MEM_RANGE:
    DEBUG ((EFI_D_INFO, "MEM_RANGE:\n"));
    DumpStmResourceHeader (Resource);
    DEBUG ((EFI_D_INFO, "  Base          : %016lx\n", Resource->Mem.Base));
    DEBUG ((EFI_D_INFO, "  Length        : %016lx\n", Resource->Mem.Length));
    DEBUG ((EFI_D_INFO, "  RWXAttributes : %08x\n",   (UINTN)Resource->Mem.RWXAttributes));
    break;
  case IO_RANGE:
    DEBUG ((EFI_D_INFO, "IO_RANGE:\n"));
    DumpStmResourceHeader (Resource);
    DEBUG ((EFI_D_INFO, "  Base          : %04x\n",   (UINTN)Resource->Io.Base));
    DEBUG ((EFI_D_INFO, "  Length        : %04x\n",   (UINTN)Resource->Io.Length));
    break;
  case MMIO_RANGE:
    DEBUG ((EFI_D_INFO, "MMIO_RANGE:\n"));
    DumpStmResourceHeader (Resource);
    DEBUG ((EFI_D_INFO, "  Base          : %016lx\n", Resource->Mmio.Base));
    DEBUG ((EFI_D_INFO, "  Length        : %016lx\n", Resource->Mmio.Length));
    DEBUG ((EFI_D_INFO, "  RWXAttributes : %08x\n",   (UINTN)Resource->Mmio.RWXAttributes));
    break;
  case MACHINE_SPECIFIC_REG:
    DEBUG ((EFI_D_INFO, "MSR_RANGE:\n"));
    DumpStmResourceHeader (Resource);
    DEBUG ((EFI_D_INFO, "  MsrIndex      : %08x\n",   (UINTN)Resource->Msr.MsrIndex));
    DEBUG ((EFI_D_INFO, "  KernelModeProc: %08x\n",   (UINTN)Resource->Msr.KernelModeProcessing));
    DEBUG ((EFI_D_INFO, "  ReadMask      : %016lx\n", Resource->Msr.ReadMask));
    DEBUG ((EFI_D_INFO, "  WriteMask     : %016lx\n", Resource->Msr.WriteMask));
    break;
  case PCI_CFG_RANGE:
    DEBUG ((EFI_D_INFO, "PCI_CFG_RANGE:\n"));
    DumpStmResourceHeader (Resource);
    DEBUG ((EFI_D_INFO, "  RWAttributes  : %04x\n",   (UINTN)Resource->PciCfg.RWAttributes));
    DEBUG ((EFI_D_INFO, "  Base          : %04x\n",   (UINTN)Resource->PciCfg.Base));
    DEBUG ((EFI_D_INFO, "  Length        : %04x\n",   (UINTN)Resource->PciCfg.Length));
    DEBUG ((EFI_D_INFO, "  OriginatingBus: %02x\n",   (UINTN)Resource->PciCfg.OriginatingBusNumber));
    DEBUG ((EFI_D_INFO, "  LastNodeIndex : %02x\n",   (UINTN)Resource->PciCfg.LastNodeIndex));
    for (PciIndex = 0; PciIndex < Resource->PciCfg.LastNodeIndex + 1; PciIndex++) {
      DEBUG ((EFI_D_INFO, "  Type          : %02x\n",   (UINTN)Resource->PciCfg.PciDevicePath[PciIndex].Type));
      DEBUG ((EFI_D_INFO, "  Subtype       : %02x\n",   (UINTN)Resource->PciCfg.PciDevicePath[PciIndex].Subtype));
      DEBUG ((EFI_D_INFO, "  Length        : %04x\n",   (UINTN)Resource->PciCfg.PciDevicePath[PciIndex].Length));
      DEBUG ((EFI_D_INFO, "  PciDevice     : %02x\n",   (UINTN)Resource->PciCfg.PciDevicePath[PciIndex].PciDevice));
      DEBUG ((EFI_D_INFO, "  PciFunction   : %02x\n",   (UINTN)Resource->PciCfg.PciDevicePath[PciIndex].PciFunction));
    }
    break;
  case TRAPPED_IO_RANGE:
    DEBUG ((EFI_D_INFO, "TRAPPED_IO_RANGE:\n"));
    DumpStmResourceHeader (Resource);
    DEBUG ((EFI_D_INFO, "  Base          : %04x\n",   (UINTN)Resource->TrappedIo.Base));
    DEBUG ((EFI_D_INFO, "  Length        : %04x\n",   (UINTN)Resource->TrappedIo.Length));
    DEBUG ((EFI_D_INFO, "  In            : %04x\n",   (UINTN)Resource->TrappedIo.In));
    DEBUG ((EFI_D_INFO, "  Out           : %04x\n",   (UINTN)Resource->TrappedIo.Out));
    DEBUG ((EFI_D_INFO, "  Api           : %04x\n",   (UINTN)Resource->TrappedIo.Api));
    break;
  case ALL_RESOURCES:
    DEBUG ((EFI_D_INFO, "ALL_RESOURCES:\n"));
    DumpStmResourceHeader (Resource);
    break;
  case REGISTER_VIOLATION:
    DEBUG ((EFI_D_INFO, "REGISTER_VIOLATION:\n"));
    DumpStmResourceHeader (Resource);
    DEBUG ((EFI_D_INFO, "  RegisterType  : %08x\n",   (UINTN)Resource->RegisterViolation.RegisterType));
    DEBUG ((EFI_D_INFO, "  ReadMask      : %016lx\n", Resource->RegisterViolation.ReadMask));
    DEBUG ((EFI_D_INFO, "  WriteMask     : %016lx\n", Resource->RegisterViolation.WriteMask));
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

  This function return last node bus number of STM PCI resource node.

  @param ResourceNode   STM PCI resource node

  @return last node bus number

**/
UINT8
GetLastNodeBus (
  IN STM_RSC   *ResourceNode
  )
{
  UINT8   PciIndex;
  UINT8   Bus;
  UINT8   Device;
  UINT8   Function;

  Bus = ResourceNode->PciCfg.OriginatingBusNumber;
  for (PciIndex = 0; PciIndex < ResourceNode->PciCfg.LastNodeIndex + 1; PciIndex++) {
    Device = ResourceNode->PciCfg.PciDevicePath[PciIndex].PciDevice;
    Function = ResourceNode->PciCfg.PciDevicePath[PciIndex].PciFunction;

    if (PciIndex == ResourceNode->PciCfg.LastNodeIndex) {
      return Bus;
    }

    //
    // Next node
    //
    Bus = PciRead8 (PCI_LIB_ADDRESS(Bus, Device, Function, PCI_BRIDGE_SECONDARY_BUS_REGISTER_OFFSET));
    if (Bus == 0) {
      return 0;
    }
  }
  return Bus;
}

/**

  This function validate STM PCI resource node.

  @param ResourceNode   STM PCI resource node

  @retval TRUE  pass validation
  @retval FALSE fail validation

**/
BOOLEAN
IsPciResourceNodeValid (
  IN STM_RSC   *ResourceNode
  )
{
  UINT8   PciIndex;
  UINT8   Bus;
  UINT8   Device;
  UINT8   Function;
  UINT16  Data16;
  UINT8   Data8;
  UINT64  PciExpressDeviceBase;

  Bus = ResourceNode->PciCfg.OriginatingBusNumber;
  for (PciIndex = 0; PciIndex < ResourceNode->PciCfg.LastNodeIndex + 1; PciIndex++) {
    Device = ResourceNode->PciCfg.PciDevicePath[PciIndex].PciDevice;
    Function = ResourceNode->PciCfg.PciDevicePath[PciIndex].PciFunction;
//    DEBUG ((EFI_D_INFO, "IsPciResourceNodeValid - B(%02x)D(%02x)F(%02x)...\n", (UINTN)Bus, (UINTN)Device, (UINTN)Function));
    if (ResourceNode->PciCfg.PciDevicePath[PciIndex].Type != 1) {
      return FALSE;
    }
    if (ResourceNode->PciCfg.PciDevicePath[PciIndex].Subtype != 1) {
      return FALSE;
    }
    if (ResourceNode->PciCfg.PciDevicePath[PciIndex].Length != sizeof(STM_PCI_DEVICE_PATH_NODE)) {
      return FALSE;
    }
    if (Device > 0x1F) {
      return FALSE;
    }
    if (Function > 0x7) {
      return FALSE;
    }
    PciExpressDeviceBase = PCI_EXPRESS_ADDRESS(Bus, Device, Function, 0);
    if (PciExpressDeviceBase >= mHostContextCommon.PciExpressLength) {
      return FALSE;
    }

    if (PciIndex == ResourceNode->PciCfg.LastNodeIndex) {
      return TRUE;
    }
    Data16 = PciRead16 (PCI_LIB_ADDRESS(Bus, Device, Function, PCI_VENDOR_ID_OFFSET));
    if (Data16 == 0xFFFF) {
      DEBUG ((EFI_D_ERROR, "IsPciResourceNodeValid - PCI_VENDOR_ID == 0xFFFF\n"));
      return FALSE;
    }
    Data16 = PciRead16 (PCI_LIB_ADDRESS(Bus, Device, Function, PCI_DEVICE_ID_OFFSET));
    if (Data16 == 0xFFFF) {
      DEBUG ((EFI_D_ERROR, "IsPciResourceNodeValid - PCI_DEVICE_ID == 0xFFFF\n"));
      return FALSE;
    }

    //
    // Next node
    //
    Data8 = PciRead8 (PCI_LIB_ADDRESS(Bus, Device, Function, PCI_HEADER_TYPE_OFFSET));
    if ((Data8 & HEADER_LAYOUT_CODE) != HEADER_TYPE_PCI_TO_PCI_BRIDGE) {
      DEBUG ((EFI_D_ERROR, "IsPciResourceNodeValid - HEADER_LAYOUT_CODE != HEADER_TYPE_PCI_TO_PCI_BRIDGE\n"));
      return FALSE;
    }
    Bus = PciRead8 (PCI_LIB_ADDRESS(Bus, Device, Function, PCI_BRIDGE_SECONDARY_BUS_REGISTER_OFFSET));
    if (Bus == 0) {
      DEBUG ((EFI_D_ERROR, "IsPciResourceNodeValid - BRIDGE_SECONDARY_BUS == 0\n"));
      return FALSE;
    }
  }
  return TRUE;
}

/**

  This function validate STM resource node.

  @param ResourceNode   STM resource node
  @param FromMle        TRUE means request from MLE, FALSE means request from BIOS
  @param ForLogging     TRUE means request for logging, FALSE means request for protection

  @retval TRUE  pass validation
  @retval FALSE fail validation

**/
BOOLEAN
IsResourceNodeValid (
  IN STM_RSC   *ResourceNode,
  IN BOOLEAN   FromMle,
  IN BOOLEAN   ForLogging
  )
{
  if (ResourceNode->Header.IgnoreResource != 0) {
    return TRUE;
  }
  if ((UINTN)ResourceNode > mHostContextCommon.MaximumSupportAddress - ResourceNode->Header.Length) {
    goto CheckFail;
  }
  switch (ResourceNode->Header.RscType) {
  case END_OF_RESOURCES:
    if (ResourceNode->Header.Length != sizeof(STM_RSC_END)) {
      goto CheckFail;
    }
    break;
  case MEM_RANGE:
    if (ResourceNode->Header.Length != sizeof(STM_RSC_MEM_DESC)) {
      goto CheckFail;
    }
    if ((ResourceNode->Mem.RWXAttributes & ~(STM_RSC_MEM_R | STM_RSC_MEM_W | STM_RSC_MEM_X)) != 0) {
      goto CheckFail;
    }
    if (ResourceNode->Mem.Reserved != 0) {
      goto CheckFail;
    }
    if (ResourceNode->Mem.Reserved_2 != 0) {
      goto CheckFail;
    }
    if (ResourceNode->Mem.Length == 0) {
      goto CheckFail;
    }
    if (ResourceNode->Mem.Base > mHostContextCommon.MaximumSupportAddress) {
      goto CheckFail;
    }
    if (ResourceNode->Mem.Length > mHostContextCommon.MaximumSupportAddress - ResourceNode->Mem.Base) {
      goto CheckFail;
    }
    // STM_RSC_BGM is NOT supported in this version
    if (FromMle) {
      if (((UINT32)ResourceNode->Mem.Base & (SIZE_4KB - 1)) != 0) {
        goto CheckFail;
      }
      if (((UINT32)ResourceNode->Mem.Length & (SIZE_4KB - 1)) != 0) {
        goto CheckFail;
      }
    }
    break;
  case IO_RANGE:
    if (ResourceNode->Header.Length != sizeof(STM_RSC_IO_DESC)) {
      goto CheckFail;
    }
    if (ResourceNode->Io.Reserved != 0) {
      goto CheckFail;
    }
    if (ResourceNode->Io.Length == 0) {
      goto CheckFail;
    }
    break;
  case MMIO_RANGE:
    if (ResourceNode->Header.Length != sizeof(STM_RSC_MMIO_DESC)) {
      goto CheckFail;
    }
    if ((ResourceNode->Mmio.RWXAttributes & ~(STM_RSC_MMIO_R | STM_RSC_MMIO_W | STM_RSC_MMIO_X)) != 0) {
      goto CheckFail;
    }
    if (ResourceNode->Mmio.Reserved != 0) {
      goto CheckFail;
    }
    if (ResourceNode->Mmio.Reserved_2 != 0) {
      goto CheckFail;
    }
    if (ResourceNode->Mmio.Length == 0) {
      goto CheckFail;
    }
    if (ResourceNode->Mmio.Base > mHostContextCommon.MaximumSupportAddress) {
      goto CheckFail;
    }
    if (ResourceNode->Mmio.Length > mHostContextCommon.MaximumSupportAddress - ResourceNode->Mmio.Base) {
      goto CheckFail;
    }
    // STM_RSC_BGI is NOT supported in this version
    if (FromMle) {
      if (((UINT32)ResourceNode->Mmio.Base & (SIZE_4KB - 1)) != 0) {
        goto CheckFail;
      }
      if (((UINT32)ResourceNode->Mmio.Length & (SIZE_4KB - 1)) != 0) {
        goto CheckFail;
      }
    }
    break;
  case MACHINE_SPECIFIC_REG:
    if (ResourceNode->Header.Length != sizeof(STM_RSC_MSR_DESC)) {
      goto CheckFail;
    }
    if (ResourceNode->Msr.Reserved != 0) {
      goto CheckFail;
    }
    // STM_RSC_MSR is NOT supported in this version
    if (FromMle) {
      if ((ResourceNode->Msr.ReadMask != 0) && (ResourceNode->Msr.ReadMask != (UINT64)-1)) {
        goto CheckFail;
      }
      if ((ResourceNode->Msr.WriteMask != 0) && (ResourceNode->Msr.WriteMask != (UINT64)-1)) {
        goto CheckFail;
      }
    }
    break;
  case PCI_CFG_RANGE:
    if (ResourceNode->Header.Length != sizeof(STM_RSC_PCI_CFG_DESC) + (sizeof(STM_PCI_DEVICE_PATH_NODE) * ResourceNode->PciCfg.LastNodeIndex)) {
      goto CheckFail;
    }
    if ((ResourceNode->PciCfg.RWAttributes & ~(STM_RSC_PCI_CFG_R | STM_RSC_PCI_CFG_W)) != 0) {
      goto CheckFail;
    }
    if (ResourceNode->PciCfg.Reserved != 0) {
      goto CheckFail;
    }
    if (ResourceNode->PciCfg.Length == 0) {
      goto CheckFail;
    }
    if (ResourceNode->PciCfg.Base >= 0x1000) {
      goto CheckFail;
    }
    if (!IsPciResourceNodeValid (ResourceNode)) {
      DEBUG ((EFI_D_ERROR, "IsPciResourceNodeValid - fail!!!\n"));
      goto CheckFail;
    }
    // STM_RSC_BGI is NOT supported in this version
    if (FromMle) {
      // So we have to mark PciCfg MMIO space are protected, all PCI registers for this device.
      if (ResourceNode->PciCfg.Base != 0) {
        goto CheckFail;
      }
      if (ResourceNode->PciCfg.Length != 0x1000) {
        goto CheckFail;
      }
    }
    break;
  case TRAPPED_IO_RANGE:
    if (FromMle) {
      goto CheckFail;
    }
    if (ResourceNode->Header.Length != sizeof(STM_RSC_TRAPPED_IO_DESC)) {
      goto CheckFail;
    }
    if (ResourceNode->TrappedIo.Reserved1 != 0) {
      goto CheckFail;
    }
    if (ResourceNode->TrappedIo.Reserved2 != 0) {
      goto CheckFail;
    }
    if (ResourceNode->TrappedIo.Length == 0) {
      goto CheckFail;
    }
    break;
  case ALL_RESOURCES:
    if (ResourceNode->Header.Length != sizeof(STM_RSC_ALL_RESOURCES_DESC)) {
      goto CheckFail;
    }
    break;
  case REGISTER_VIOLATION:
    if (!ForLogging) {
      goto CheckFail;
    }
    if (ResourceNode->Header.Length != sizeof(STM_REGISTER_VIOLATION_DESC)) {
      goto CheckFail;
    }
    if (ResourceNode->RegisterViolation.RegisterType >= StmRegisterMax) {
      goto CheckFail;
    }
    if (ResourceNode->RegisterViolation.Reserved != 0) {
      goto CheckFail;
    }
    break;
  default:
    goto CheckFail;
    break;
  }

  //
  // All check pass
  //
  return TRUE;

CheckFail:
  DEBUG ((EFI_D_ERROR, "Resource invalid (FromMle - %x, ForLogging - %x):\n", FromMle, ForLogging));
  DEBUG ((EFI_D_INFO, "ResourceNode:\n"));
  DumpStmResourceNode (ResourceNode);
  return FALSE;
}

/**

  This function return if 2 resource overlap.

  @param Address1   Address of 1st resource
  @param Length1    Length of 1st resource
  @param Address2   Address of 2nd resource
  @param Length2    Length of 2nd resource

  @retval TRUE  overlap
  @retval FALSE no overlap

**/
BOOLEAN
IsOverlap (
  IN UINT64  Address1,
  IN UINT64  Length1,
  IN UINT64  Address2,
  IN UINT64  Length2
  )
{
  if ((Address1 + Length1 > Address2) && (Address1 < Address2 + Length2)) {
    // Overlap
    return TRUE;
  } else {
    return FALSE;
  }
}

/**

  This function validate STM resource list.

  @param ResourceList   STM resource list
  @param FromMle        TRUE means request from MLE, FALSE means request from BIOS

  @retval TRUE  pass validation
  @retval FALSE fail validation

**/
BOOLEAN
IsResourceListValid (
  IN STM_RSC   *ResourceList,
  IN BOOLEAN   FromMle
  )
{
  while (ResourceList->Header.RscType != END_OF_RESOURCES) {
    if (!IsResourceNodeValid (ResourceList, FromMle, FALSE)) {
      return FALSE;
    }
    ResourceList = (STM_RSC *)((UINTN)ResourceList + ResourceList->Header.Length);
  }
  if (ResourceList->End.ResourceListContinuation != 0) {
    return IsResourceListValid ((STM_RSC *)(UINTN)ResourceList->End.ResourceListContinuation, FromMle);
  }

  //
  // All check pass
  //
  return TRUE;
}

/**

  This function check STM resource node overlap.

  @param ResourceNode1 STM resource node1
  @param ResourceNode2 STM resource node2

  @retval TRUE  overlap
  @retval FALSE not overlap

**/
BOOLEAN
IsResourceNodeOverlap (
  IN STM_RSC   *ResourceNode1,
  IN STM_RSC   *ResourceNode2
  )
{
  UINT8   PciIndex;

  if (ResourceNode1->Header.IgnoreResource != 0 || ResourceNode2->Header.IgnoreResource != 0) {
    return FALSE;
  }
  if (ResourceNode1->Header.RscType != ResourceNode2->Header.RscType) {
    return FALSE;
  }

  //
  // RscType same, compare them by type
  //
  switch (ResourceNode1->Header.RscType) {
  case END_OF_RESOURCES:
    return FALSE;
    break;
  case MEM_RANGE:
    if (!IsOverlap (ResourceNode1->Mem.Base, ResourceNode1->Mem.Length, ResourceNode2->Mem.Base, ResourceNode2->Mem.Length)) {
      return FALSE;
    }
    if ((ResourceNode1->Mem.RWXAttributes & ResourceNode2->Mem.RWXAttributes) == 0) {
      return FALSE;
    }
    goto OverlapHappen;
    break;
  case IO_RANGE:
    if (!IsOverlap(ResourceNode1->Io.Base, ResourceNode1->Io.Length, ResourceNode2->Io.Base, ResourceNode2->Io.Length)) {
      return FALSE;
    }
    goto OverlapHappen;
    break;
  case MMIO_RANGE:
    if (!IsOverlap(ResourceNode1->Mmio.Base, ResourceNode1->Mmio.Length, ResourceNode2->Mmio.Base, ResourceNode2->Mmio.Length)) {
      return FALSE;
    }
    if ((ResourceNode1->Mmio.RWXAttributes & ResourceNode2->Mmio.RWXAttributes) == 0) {
      return FALSE;
    }
    goto OverlapHappen;
    break;
  case MACHINE_SPECIFIC_REG:
    if (ResourceNode1->Msr.MsrIndex != ResourceNode2->Msr.MsrIndex) {
      return FALSE;
    }
    if (((ResourceNode1->Msr.ReadMask  & ResourceNode2->Msr.ReadMask) == 0) &&
        ((ResourceNode1->Msr.WriteMask & ResourceNode2->Msr.WriteMask) == 0)) {
      return FALSE;
    }
    goto OverlapHappen;
    break;
  case PCI_CFG_RANGE:
    if (ResourceNode1->PciCfg.OriginatingBusNumber != ResourceNode2->PciCfg.OriginatingBusNumber) {
      return FALSE;
    }
    if (ResourceNode1->PciCfg.LastNodeIndex != ResourceNode2->PciCfg.LastNodeIndex) {
      return FALSE;
    }
    for (PciIndex = 0; PciIndex < ResourceNode1->PciCfg.LastNodeIndex + 1; PciIndex++) {
      if (ResourceNode1->PciCfg.PciDevicePath[PciIndex].PciDevice != ResourceNode2->PciCfg.PciDevicePath[PciIndex].PciDevice) {
        return FALSE;
      }
      if (ResourceNode1->PciCfg.PciDevicePath[PciIndex].PciFunction != ResourceNode2->PciCfg.PciDevicePath[PciIndex].PciFunction) {
        return FALSE;
      }
    }
    if (!IsOverlap(ResourceNode1->PciCfg.Base, ResourceNode1->PciCfg.Length, ResourceNode2->PciCfg.Base, ResourceNode2->PciCfg.Length)) {
      return FALSE;
    }
    if ((ResourceNode1->PciCfg.RWAttributes & ResourceNode2->PciCfg.RWAttributes) == 0) {
      return FALSE;
    }
    goto OverlapHappen;
    break;
  case TRAPPED_IO_RANGE:
    if (!IsOverlap(ResourceNode1->TrappedIo.Base, ResourceNode1->TrappedIo.Length, ResourceNode2->TrappedIo.Base, ResourceNode2->TrappedIo.Length)) {
      return FALSE;
    }
    goto OverlapHappen;
    break;
  case ALL_RESOURCES:
    goto OverlapHappen;
    break;
  case REGISTER_VIOLATION:
    if (ResourceNode1->RegisterViolation.RegisterType != ResourceNode2->RegisterViolation.RegisterType) {
      return FALSE;
    }
    if (((ResourceNode1->RegisterViolation.ReadMask  & ResourceNode2->RegisterViolation.ReadMask) == 0) &&
        ((ResourceNode1->RegisterViolation.WriteMask & ResourceNode2->RegisterViolation.WriteMask) == 0)) {
      return FALSE;
    }
    goto OverlapHappen;
    break;
  }

  //
  // No overlap
  //
  return FALSE;

OverlapHappen:
  DEBUG ((EFI_D_ERROR, "Resource overlap:\n"));
  DEBUG ((EFI_D_INFO, "ResourceNode1:\n"));
  DumpStmResourceNode (ResourceNode1);
  DEBUG ((EFI_D_INFO, "ResourceNode2:\n"));
  DumpStmResourceNode (ResourceNode2);
  return TRUE;
}

/**

  This function check STM resource list overlap with node.

  @param ResourceNode1 STM resource node1
  @param ResourceList2 STM resource list2

  @retval TRUE  overlap
  @retval FALSE not overlap

**/
BOOLEAN
IsResourceListOverlapWithNode (
  IN STM_RSC   *ResourceNode1,
  IN STM_RSC   *ResourceList2
  )
{
  while (ResourceList2->Header.RscType != END_OF_RESOURCES) {
    if ((UINTN)ResourceList2 > mHostContextCommon.MaximumSupportAddress - ResourceList2->Header.Length) {
      return TRUE;
    }
    if (IsResourceNodeOverlap (ResourceNode1, ResourceList2)) {
      return TRUE;
    }
    ResourceList2 = (STM_RSC *)((UINTN)ResourceList2 + ResourceList2->Header.Length);
  }
  if (ResourceList2->End.ResourceListContinuation != 0) {
    return IsResourceListOverlapWithNode (ResourceNode1, (STM_RSC *)(UINTN)ResourceList2->End.ResourceListContinuation);
  }

  //
  // No overlap
  //
  return FALSE;
}

/**

  This function check STM resource list overlap.

  @param ResourceList1 STM resource list1
  @param ResourceList2 STM resource list2

  @retval TRUE  overlap
  @retval FALSE not overlap

**/
BOOLEAN
IsResourceListOverlap (
  IN STM_RSC   *ResourceList1,
  IN STM_RSC   *ResourceList2
  )
{
  while (ResourceList1->Header.RscType != END_OF_RESOURCES) {
    if ((UINTN)ResourceList1 > mHostContextCommon.MaximumSupportAddress - ResourceList1->Header.Length) {
      return TRUE;
    }
    if (IsResourceListOverlapWithNode (ResourceList1, ResourceList2)) {
      return TRUE;
    }
    ResourceList1 = (STM_RSC *)((UINTN)ResourceList1 + ResourceList1->Header.Length);
  }
  if (ResourceList1->End.ResourceListContinuation != 0) {
    return IsResourceListOverlap ((STM_RSC *)(UINTN)ResourceList1->End.ResourceListContinuation, ResourceList2);
  }

  //
  // No overlap
  //
  return FALSE;
}

/**

  This function return STM resource list size without parsing END node.

  @param Resource STM resource list

  @return STM resource list size (excluding last node)
  @retval 0 This list is invalid

**/
UINTN
GetSizeFromThisResourceList (
  IN STM_RSC   *Resource
  )
{
  STM_RSC   *Header;

  Header = Resource;
  while (Resource->Header.RscType != END_OF_RESOURCES) {
    if ((UINTN)Resource > mHostContextCommon.MaximumSupportAddress - Resource->Header.Length) {
      return 0;
    }
    Resource = (STM_RSC *)((UINTN)Resource + Resource->Header.Length);
  }

  //
  // Not parse ResourceListContinuation
  // Exclude last END node
  //
  return (UINTN)Resource - (UINTN)Header;
}

/**

  This function return STM resource list size without parsing END node.

  @param Resource          STM resource list
  @param ResourceType      Only resource with this type will be calculated
  @param ThisResourceSize  The resource size of this type (excluding last node)

  @return Whole STM resource list size (excluding last node)
  @retval 0 This list is invalid

**/
UINTN
GetSizeFromThisResourceListWithType (
  IN STM_RSC   *Resource,
  IN UINT32    ResourceType,
  OUT UINTN    *ThisResourceSize
  )
{
  STM_RSC   *Header;
  UINTN     Size;

  Header = Resource;
  Size = 0;
  while (Resource->Header.RscType != END_OF_RESOURCES) {
    if (Resource->Header.RscType == ResourceType) {
      if (Size > mHostContextCommon.MaximumSupportAddress - Resource->Header.Length) {
        return 0;
      }
      if ((UINTN)Resource > mHostContextCommon.MaximumSupportAddress - Resource->Header.Length) {
        return 0;
      }
      Size += Resource->Header.Length;
    }
    Resource = (STM_RSC *)((UINTN)Resource + Resource->Header.Length);
  }

  //
  // Not parse ResourceListContinuation
  // Exclude last END node
  //
  *ThisResourceSize = Size;
  return (UINTN)Resource - (UINTN)Header;
}

/**

  This function return STM resource list size.

  @param Resource STM resource list

  @return STM resource list size (including last node)
  @retval 0 This list is invalid

**/
UINTN
GetSizeFromResource (
  IN STM_RSC   *Resource
  )
{
  STM_RSC   *Header;
  UINTN     Length;
  UINTN     ThisLength;

  Header = Resource;
  Length = 0;

  do {
    ThisLength = GetSizeFromThisResourceList (Resource);
    if (ThisLength == 0) {
      return 0;
    }
    //
    // Overflow check
    //
    if (Length > mHostContextCommon.MaximumSupportAddress - ThisLength) {
      return 0;
    }
    Length += ThisLength;
    if ((UINTN)Header > mHostContextCommon.MaximumSupportAddress - Length) {
      return 0;
    }
    Resource = (STM_RSC *)((UINTN)Header + Length);
    ASSERT (Resource->Header.RscType == END_OF_RESOURCES);
    if (Resource->End.ResourceListContinuation != 0) {
      Resource = (STM_RSC *)(UINTN)Resource->End.ResourceListContinuation;
    } else {
      break;
    }
  } while (TRUE);

  //
  // Include last END node
  //
  if (Length > mHostContextCommon.MaximumSupportAddress - sizeof(STM_RSC_END)) {
    return 0;
  }
  return Length + sizeof(STM_RSC_END);
}

/**

  This function return STM resource list size.

  @param Resource      STM resource list
  @param ResourceType  Only resource with this type will be calculated

  @return STM resource list size (including last node)
  @retval 0 This list is invalid

**/
UINTN
GetSizeFromResourceWithType (
  IN STM_RSC   *Resource,
  IN UINT32    ResourceType
  )
{
  STM_RSC   *Header;
  UINTN     Length;
  UINTN     ThisLength;
  UINTN     ThisSize;
  UINTN     ThisTypeSize;

  Header = Resource;
  Length = 0;
  ThisTypeSize = 0;

  do {
    ThisLength = GetSizeFromThisResourceListWithType (Resource, ResourceType, &ThisSize);
    if (ThisLength == 0) {
      return 0;
    }
    //
    // Overflow check
    //
    if (Length > mHostContextCommon.MaximumSupportAddress - ThisLength) {
      return 0;
    }
    Length += ThisLength;
    ThisTypeSize += ThisSize;
    if ((UINTN)Header > mHostContextCommon.MaximumSupportAddress - Length) {
      return 0;
    }
    Resource = (STM_RSC *)((UINTN)Header + Length);
    ASSERT (Resource->Header.RscType == END_OF_RESOURCES);
    if (Resource->End.ResourceListContinuation != 0) {
      Resource = (STM_RSC *)(UINTN)Resource->End.ResourceListContinuation;
    } else {
      break;
    }
  } while (TRUE);

  //
  // Include last END node
  //
  if (ThisTypeSize > mHostContextCommon.MaximumSupportAddress - sizeof(STM_RSC_END)) {
    return 0;
  }
  return ThisTypeSize + sizeof(STM_RSC_END);
}

/**

  This function copy STM resource list from source to destination.

  @param Destination  Destination buffer, which always has ResourceListContinuation as 0.
  @param Source       Source STM resource list, which may include ResourceListContinuation.

**/
VOID
CopyStmResourceMem (
  IN STM_RSC   *Destination,
  IN STM_RSC   *Source
  )
{
  UINTN  Length;

  do {
    Length = GetSizeFromThisResourceList (Source);
    CopyMem (Destination, Source, Length);
    Destination = (STM_RSC *)((UINTN)Destination + Length);
    Source = (STM_RSC *)((UINTN)Source + Length);
    if (Source->End.ResourceListContinuation != 0) {
      Source = (STM_RSC *)(UINTN)Source->End.ResourceListContinuation;
    } else {
      break;
    }
  } while (TRUE);

  ZeroMem (Destination, sizeof(STM_RSC_END));
  Destination->Header.RscType = END_OF_RESOURCES;
  Destination->Header.Length = sizeof(STM_RSC_END);

  return ;
}

/**

  This function return STM MEM/MMIO resource according to information.

  @param Resource      STM resource list
  @param Address       MEM/MMIO address
  @param RWXAttributes RWXAttributes mask

  @return STM MEM/MMIO resource

**/
STM_RSC_MEM_DESC *
GetStmResourceMem (
  IN STM_RSC   *Resource,
  IN UINT64    Address,
  IN UINT32    RWXAttributes
  )
{
  if (Resource == NULL) {
    return NULL;
  }
  while (Resource->Header.RscType != END_OF_RESOURCES) {
    if ((Resource->Header.IgnoreResource == 0) &&
        ((Resource->Header.RscType == MEM_RANGE) || (Resource->Header.RscType == MMIO_RANGE)) &&
        ((Resource->Mem.Base <= Address) && (Resource->Mem.Base + Resource->Mem.Length > Address)) &&
        ((Resource->Mem.RWXAttributes & RWXAttributes) != 0)) {
      return &Resource->Mem;
    }
    Resource = (STM_RSC *)((UINTN)Resource + Resource->Header.Length);
  }
  if (Resource->End.ResourceListContinuation != 0) {
    return GetStmResourceMem ((STM_RSC *)(UINTN)Resource->End.ResourceListContinuation, Address, RWXAttributes);
  }

  return NULL;
}

/**

  This function return STM IO resource according to information.

  @param Resource STM resource list
  @param IoPort   IO port

  @return STM IO resource

**/
STM_RSC_IO_DESC *
GetStmResourceIo (
  IN STM_RSC   *Resource,
  IN UINT16    IoPort
  )
{
  if (Resource == NULL) {
    return NULL;
  }
  while (Resource->Header.RscType != END_OF_RESOURCES) {
    if ((Resource->Header.IgnoreResource == 0) &&
        (Resource->Header.RscType == IO_RANGE) &&
        ((Resource->Io.Base <= IoPort) && (Resource->Io.Base + Resource->Io.Length > IoPort))) {
      return &Resource->Io;
    }
    Resource = (STM_RSC *)((UINTN)Resource + Resource->Header.Length);
  }
  if (Resource->End.ResourceListContinuation != 0) {
    return GetStmResourceIo ((STM_RSC *)(UINTN)Resource->End.ResourceListContinuation, IoPort);
  }

  return NULL;
}

/**

  This function return STM TrappedIo resource according to information.

  @param Resource STM resource list
  @param IoPort   TrappedIo port

  @return STM TrappedIo resource

**/
STM_RSC_TRAPPED_IO_DESC *
GetStmResourceTrappedIo (
  IN STM_RSC   *Resource,
  IN UINT16    IoPort
  )
{
  if (Resource == NULL) {
    return NULL;
  }
  while (Resource->Header.RscType != END_OF_RESOURCES) {
    if ((Resource->Header.IgnoreResource == 0) &&
        (Resource->Header.RscType == TRAPPED_IO_RANGE) &&
        ((Resource->TrappedIo.Base <= IoPort) && (Resource->TrappedIo.Base + Resource->TrappedIo.Length > IoPort))) {
      return &Resource->TrappedIo;
    }
    Resource = (STM_RSC *)((UINTN)Resource + Resource->Header.Length);
  }
  if (Resource->End.ResourceListContinuation != 0) {
    return GetStmResourceTrappedIo ((STM_RSC *)(UINTN)Resource->End.ResourceListContinuation, IoPort);
  }

  return NULL;
}

/**

  This function return STM PCI resource according to information.

  @param Resource     STM resource list
  @param Bus          Pci bus
  @param Device       Pci device
  @param Function     Pci function
  @param Register     Pci register
  @param RWAttributes RWAttributes mask

  @return STM PCI resource

**/
STM_RSC_PCI_CFG_DESC *
GetStmResourcePci (
  IN STM_RSC   *Resource,
  IN UINT8     Bus,
  IN UINT8     Device,
  IN UINT8     Function,
  IN UINT16    Register,
  IN UINT16    RWAttributes
  )
{
  UINT8   LastNodeBus;

  if (Resource == NULL) {
    return NULL;
  }
  while (Resource->Header.RscType != END_OF_RESOURCES) {
    if ((Resource->Header.IgnoreResource == 0) &&
        (Resource->Header.RscType == PCI_CFG_RANGE) &&
        ((Resource->PciCfg.Base <= Register) && (Resource->PciCfg.Base + Resource->PciCfg.Length > Register)) &&
        ((Resource->PciCfg.RWAttributes & RWAttributes) != 0)) {
      LastNodeBus = GetLastNodeBus (Resource);
      if ((LastNodeBus == Bus) &&
          (Resource->PciCfg.PciDevicePath[Resource->PciCfg.LastNodeIndex].PciDevice == Device) &&
          (Resource->PciCfg.PciDevicePath[Resource->PciCfg.LastNodeIndex].PciFunction == Function) ) {
        return &Resource->PciCfg;
      }
    }
    Resource = (STM_RSC *)((UINTN)Resource + Resource->Header.Length);
  }
  if (Resource->End.ResourceListContinuation != 0) {
    return GetStmResourcePci ((STM_RSC *)(UINTN)Resource->End.ResourceListContinuation, Bus, Device, Function, Register, RWAttributes);
  }

  return NULL;
}

/**

  This function return STM MSR resource according to information.

  @param Resource STM resource list
  @param MsrIndex Msr index

  @return STM MSR resource

**/
STM_RSC_MSR_DESC *
GetStmResourceMsr (
  IN STM_RSC   *Resource,
  IN UINT32    MsrIndex
  )
{
  if (Resource == NULL) {
    return NULL;
  }

  while (Resource->Header.RscType != END_OF_RESOURCES) {
    if ((Resource->Header.IgnoreResource == 0) &&
        (Resource->Header.RscType == MACHINE_SPECIFIC_REG) &&
        (Resource->Msr.MsrIndex == MsrIndex)) {
      return &Resource->Msr;
    }
    Resource = (STM_RSC *)((UINTN)Resource + Resource->Header.Length);
  }
  if (Resource->End.ResourceListContinuation != 0) {
    return GetStmResourceMsr ((STM_RSC *)(UINTN)Resource->End.ResourceListContinuation, MsrIndex);
  }

  return NULL;
}

/**

  This function add resource list to protected resource structure.

  @param ProtectedResource Protected resource structure
  @param Resource          Resource list to be added

  @retval STM_SUCCESS                resource added successfully
  @retval ERROR_STM_OUT_OF_RESOURCES no enough resource

**/
STM_STATUS
AddProtectedResource (
  IN MLE_PROTECTED_RESOURCE_STRUCTURE  *ProtectedResource,
  IN STM_RSC                           *Resource
  )
{
  UINTN                             ResourceSize;
  UINTN                             Pages;
  STM_RSC                           *Base;
  STM_RSC                           *Last;
  
  ResourceSize = GetSizeFromResource (Resource);
  if (ResourceSize == 0) {
    return ERROR_STM_SECURITY_VIOLATION;
  }

  //
  // Allocate default if new
  //
  if (ProtectedResource->Base == NULL) {
    Pages = STM_SIZE_TO_PAGES (ResourceSize);
    Pages = ((Pages / DEFAULT_PROTECTED_DEFAULT_PAGES)  * DEFAULT_PROTECTED_DEFAULT_PAGES) + 1; 
    Base = AllocatePages (Pages);
    ProtectedResource->Pages = Pages;
    ProtectedResource->Base = Base;
    ProtectedResource->UsedSize = 0;
  }

  //
  // Reallocate if not enough left size
  //
  else if (ResourceSize > STM_PAGES_TO_SIZE (ProtectedResource->Pages) - ProtectedResource->UsedSize) {
    Pages = STM_SIZE_TO_PAGES (ResourceSize);
    Pages = ((Pages / DEFAULT_PROTECTED_DEFAULT_PAGES)  * DEFAULT_PROTECTED_DEFAULT_PAGES) + 1;
    Pages += ProtectedResource->Pages;
    Base = AllocatePages (Pages);
    if (Base == NULL) {
      return ERROR_STM_OUT_OF_RESOURCES;
    }
    CopyMem (Base, ProtectedResource->Base, ProtectedResource->UsedSize);
    FreePages (ProtectedResource->Base, ProtectedResource->Pages);
    ProtectedResource->Pages = Pages;
    ProtectedResource->Base = Base;
    // Keep UsedSize unchanged
  }

  //
  // Adjust Pointer
  //
  if (ProtectedResource->UsedSize != 0) {
    ProtectedResource->UsedSize -= sizeof(STM_RSC_END);
  }
  Last = (STM_RSC *)((UINTN)ProtectedResource->Base + ProtectedResource->UsedSize);

  CopyStmResourceMem (Last, Resource);
  ProtectedResource->UsedSize += ResourceSize;

  return STM_SUCCESS;
}

/**

  This function add resource list to protected resource structure.

  @param ProtectedResource Protected resource structure
  @param Resource          Resource list to be added
  @param ResourceType      Only resource with this type will be added

  @retval STM_SUCCESS                resource added successfully
  @retval ERROR_STM_OUT_OF_RESOURCES no enough resource

**/
STM_STATUS
AddProtectedResourceWithType (
  IN MLE_PROTECTED_RESOURCE_STRUCTURE  *ProtectedResource,
  IN STM_RSC                           *Resource,
  IN UINT32                            ResourceType
  )
{
  UINTN                             ResourceSize;
  UINTN                             Pages;
  STM_RSC                           *Base;
  STM_RSC                           *Last;
  
  ResourceSize = GetSizeFromResourceWithType (Resource, ResourceType);
  if (ResourceSize == 0) {
    return ERROR_STM_SECURITY_VIOLATION;
  }

  //
  // Allocate default if new
  //
  if (ProtectedResource->Base == NULL) {
    Pages = STM_SIZE_TO_PAGES (ResourceSize);
    Pages = ((Pages / DEFAULT_PROTECTED_DEFAULT_PAGES)  * DEFAULT_PROTECTED_DEFAULT_PAGES) + 1; 
    Base = AllocatePages (Pages);
    ProtectedResource->Pages = Pages;
    ProtectedResource->Base = Base;
    ProtectedResource->UsedSize = 0;
  }

  //
  // Reallocate if not enough left size
  //
  else if (ResourceSize > STM_PAGES_TO_SIZE (ProtectedResource->Pages) - ProtectedResource->UsedSize) {
    Pages = STM_SIZE_TO_PAGES (ResourceSize);
    Pages = ((Pages / DEFAULT_PROTECTED_DEFAULT_PAGES)  * DEFAULT_PROTECTED_DEFAULT_PAGES) + 1;
    Pages += ProtectedResource->Pages;
    Base = AllocatePages (Pages);
    if (Base == NULL) {
      return ERROR_STM_OUT_OF_RESOURCES;
    }
    CopyMem (Base, ProtectedResource->Base, ProtectedResource->UsedSize);
    FreePages (ProtectedResource->Base, ProtectedResource->Pages);
    ProtectedResource->Pages = Pages;
    ProtectedResource->Base = Base;
    // Keep UsedSize unchanged
  }

  //
  // Adjust Pointer
  //
  if (ProtectedResource->UsedSize != 0) {
    ProtectedResource->UsedSize -= sizeof(STM_RSC_END);
  }
  Last = (STM_RSC *)((UINTN)ProtectedResource->Base + ProtectedResource->UsedSize);

  //
  // CopyNode one by one
  //
  do {
    while (Resource->Header.RscType != END_OF_RESOURCES) {
      if (Resource->Header.RscType == ResourceType) {
        CopyMem ((VOID *)Last, Resource, Resource->Header.Length);
        Last = (STM_RSC *)((UINTN)Last + Resource->Header.Length);
      }
      Resource = (STM_RSC *)((UINTN)Resource + Resource->Header.Length);
    }
    if (Resource->End.ResourceListContinuation != 0) {
      Resource = (STM_RSC *)(UINTN)Resource->End.ResourceListContinuation;
    } else {
      break;
    }
  } while (TRUE);

  Last->Header.RscType = END_OF_RESOURCES;
  Last->Header.Length = sizeof(STM_RSC_END);
  Last->Header.ReturnStatus = 0;
  Last->Header.IgnoreResource = 0;
  Last->End.ResourceListContinuation = 0;

  ProtectedResource->UsedSize += ResourceSize;

  return STM_SUCCESS;
}

/**

  This function delete resource node to protected resource structure.

  @param ProtectedResource Protected resource structure
  @param ResourceNode      Resource node to be deleted

**/
VOID
DeleteProtectedResourceNode (
  IN MLE_PROTECTED_RESOURCE_STRUCTURE  *ProtectedResource,
  IN STM_RSC                           *ResourceNode
  )
{
  STM_RSC   *Resource;
  UINTN     CopyLength;

  if (ResourceNode->Header.IgnoreResource != 0) {
    return ;
  }

  Resource = ProtectedResource->Base;
  while (Resource->Header.RscType != END_OF_RESOURCES) {
    if ((Resource->Header.IgnoreResource == 0) &&
        (Resource->Header.RscType == ResourceNode->Header.RscType) &&
        (Resource->Header.Length == ResourceNode->Header.Length) &&
        (CompareMem (
           (VOID *)((UINTN)Resource + sizeof(STM_RSC_DESC_HEADER)),
           (VOID *)((UINTN)ResourceNode + sizeof(STM_RSC_DESC_HEADER)),
           ResourceNode->Header.Length - sizeof(STM_RSC_DESC_HEADER)
           ) == 0)) {
      //
      // Find it
      //
      CopyLength = ProtectedResource->UsedSize - ((UINTN)Resource + Resource->Header.Length - (UINTN)ProtectedResource->Base);
      //
      // Override memory to delete it
      //
      CopyMem (Resource, (VOID *)((UINTN)Resource + Resource->Header.Length), CopyLength);
      //
      // Do not use Resource any more
      //
      ProtectedResource->UsedSize -= ResourceNode->Header.Length;
      return ;
    }
    Resource = (STM_RSC *)((UINTN)Resource + Resource->Header.Length);
  }
}

/**

  This function delete resource list to protected resource structure.

  @param ProtectedResource Protected resource structure
  @param Resource          Resource list to be deleted

**/
VOID
DeleteProtectedResource (
  IN MLE_PROTECTED_RESOURCE_STRUCTURE  *ProtectedResource,
  IN STM_RSC                           *Resource
  )
{
  while (Resource->Header.RscType != END_OF_RESOURCES) {
    DeleteProtectedResourceNode (ProtectedResource, Resource);
    Resource = (STM_RSC *)((UINTN)Resource + Resource->Header.Length);
  }
  if (Resource->End.ResourceListContinuation != 0) {
    DeleteProtectedResource (ProtectedResource, (STM_RSC *)(UINTN)Resource->End.ResourceListContinuation);
  }
}

/**

  This function delete resource list to protected resource structure.

  @param ProtectedResource Protected resource structure
  @param Resource          Resource list to be deleted
  @param ResourceType      Only resource with this type will be added

**/
VOID
DeleteProtectedResourceWithType (
  IN MLE_PROTECTED_RESOURCE_STRUCTURE  *ProtectedResource,
  IN STM_RSC                           *Resource,
  IN UINT32                            ResourceType
  )
{
  //
  // Just use same API as DeleteProtectedResource(), because it will delete node one by one.
  //
  DeleteProtectedResource (ProtectedResource, Resource);
}

/**

  This function register resource node to VMCS.

  @param Resource          Resource node to be registered

**/
VOID
RegisterProtectedResourceNode (
  IN STM_RSC   *Resource
  )
{
  UINT8   LastNodeBus;
  UINT64  PciExpressDeviceBase;

  if (Resource->Header.IgnoreResource != 0) {
    return ;
  }
  switch (Resource->Header.RscType) {
  case MEM_RANGE:
  case MMIO_RANGE:
    EPTSetPageAttributeRange (
      Resource->Mem.Base,
      Resource->Mem.Length,
      ((Resource->Mem.RWXAttributes & STM_RSC_MEM_R) != 0) ? 0 : 1,
      ((Resource->Mem.RWXAttributes & STM_RSC_MEM_W) != 0) ? 0 : 1,
      ((Resource->Mem.RWXAttributes & STM_RSC_MEM_X) != 0) ? 0 : 1,
      EptPageAttributeAnd
      );
    Resource->Header.ReturnStatus = 1;
    break;
  case IO_RANGE:
    SetIoBitmapRange (Resource->Io.Base, Resource->Io.Length);
    Resource->Header.ReturnStatus = 1;
    break;
  case MACHINE_SPECIFIC_REG:
    if (Resource->Msr.ReadMask != 0) {
      SetMsrBitmap (Resource->Msr.MsrIndex, FALSE);
    }
    if (Resource->Msr.WriteMask != 0) {
      SetMsrBitmap (Resource->Msr.MsrIndex, TRUE);
    }
    Resource->Header.ReturnStatus = 1;
    break;
  case PCI_CFG_RANGE:
    SetIoBitmapRange (0xCF8, 1);
    SetIoBitmapRange (0xCFC, 4);
    // STM_RSC_BGI is NOT supported in this version
    if (mHostContextCommon.PciExpressBaseAddress != 0) {
      LastNodeBus = GetLastNodeBus (Resource);
      PciExpressDeviceBase = PCI_EXPRESS_ADDRESS(LastNodeBus, Resource->PciCfg.PciDevicePath[Resource->PciCfg.LastNodeIndex].PciDevice, Resource->PciCfg.PciDevicePath[Resource->PciCfg.LastNodeIndex].PciFunction, 0);
      EPTSetPageAttributeRange (
        PciExpressDeviceBase + mHostContextCommon.PciExpressBaseAddress,
        SIZE_4KB,
        ((Resource->PciCfg.RWAttributes & STM_RSC_PCI_CFG_R) != 0) ? 0 : 1,
        ((Resource->PciCfg.RWAttributes & STM_RSC_PCI_CFG_W) != 0) ? 0 : 1,
        0,
        EptPageAttributeAnd
        );
    }
    Resource->Header.ReturnStatus = 1;
    break;
  case TRAPPED_IO_RANGE:
    // Not supported
    break;
  case ALL_RESOURCES:
    // Seems imposible
    break;
  case REGISTER_VIOLATION:
    // Not supported
    break;
  default:
    // Unknown should not be here
    ASSERT (FALSE);
    break;
  }
}

/**

  This function register resource list to VMCS.

  @param Resource          Resource list to be registered

**/
VOID
RegisterProtectedResource (
  IN STM_RSC   *Resource
  )
{
  while (Resource->Header.RscType != END_OF_RESOURCES) {
    RegisterProtectedResourceNode (Resource);
    AddEventLogForResource (EvtMleResourceProtectionGranted, Resource);
    Resource = (STM_RSC *)((UINTN)Resource + Resource->Header.Length);
  }
  if (Resource->End.ResourceListContinuation != 0) {
    RegisterProtectedResource ((STM_RSC *)(UINTN)Resource->End.ResourceListContinuation);
  }
}

/**

  This function unregister resource node to VMCS.

  @param Resource          Resource node to be unregistered

**/
VOID
UnRegisterProtectedResourceNode (
  IN STM_RSC   *Resource
  )
{
  UINT8   LastNodeBus;
  UINT64  PciExpressDeviceBase;

  if (Resource->Header.IgnoreResource != 0) {
    return ;
  }
  switch (Resource->Header.RscType) {
  case MEM_RANGE:
  case MMIO_RANGE:
    EPTSetPageAttributeRange (
      Resource->Mem.Base,
      Resource->Mem.Length,
      ((Resource->Mem.RWXAttributes & STM_RSC_MEM_R) != 0) ? 1 : 0,
      ((Resource->Mem.RWXAttributes & STM_RSC_MEM_W) != 0) ? 1 : 0,
      ((Resource->Mem.RWXAttributes & STM_RSC_MEM_X) != 0) ? 1 : 0,
      EptPageAttributeOr
      );
    Resource->Header.ReturnStatus = 1;
    break;
  case IO_RANGE:
    UnSetIoBitmapRange (Resource->Io.Base, Resource->Io.Length);
    Resource->Header.ReturnStatus = 1;
    break;
  case MACHINE_SPECIFIC_REG:
    if (Resource->Msr.ReadMask != 0) {
      UnSetMsrBitmap (Resource->Msr.MsrIndex, FALSE);
    }
    if (Resource->Msr.WriteMask != 0) {
      UnSetMsrBitmap (Resource->Msr.MsrIndex, TRUE);
    }
    Resource->Header.ReturnStatus = 1;
    break;
  case PCI_CFG_RANGE:
    // Do nothing for CF8/CFC access, we can not UnSetIoBitmapRange (0xCFC, 4)
    // STM_RSC_BGI is NOT supported in this version
    if (mHostContextCommon.PciExpressBaseAddress != 0) {
      LastNodeBus = GetLastNodeBus (Resource);
      PciExpressDeviceBase = PCI_EXPRESS_ADDRESS(LastNodeBus, Resource->PciCfg.PciDevicePath[Resource->PciCfg.LastNodeIndex].PciDevice, Resource->PciCfg.PciDevicePath[Resource->PciCfg.LastNodeIndex].PciFunction, 0);
      EPTSetPageAttributeRange (
        PciExpressDeviceBase + mHostContextCommon.PciExpressBaseAddress,
        SIZE_4KB,
        ((Resource->PciCfg.RWAttributes & STM_RSC_PCI_CFG_R) != 0) ? 1 : 0,
        ((Resource->PciCfg.RWAttributes & STM_RSC_PCI_CFG_W) != 0) ? 1 : 0,
        0,
        EptPageAttributeOr
        );
    }
    Resource->Header.ReturnStatus = 1;
    break;
  case TRAPPED_IO_RANGE:
    // Not supported
    break;
  case ALL_RESOURCES:
    // Seems imposible
    break;
  case REGISTER_VIOLATION:
    // Not supported
    break;
  default:
    // Unknown should not be here
    ASSERT (FALSE);
    break;
  }
}

/**

  This function unregister resource list to VMCS.

  @param Resource          Resource list to be unregistered

**/
VOID
UnRegisterProtectedResource (
  IN STM_RSC   *Resource
  )
{
  while (Resource->Header.RscType != END_OF_RESOURCES) {
    UnRegisterProtectedResourceNode (Resource);
    AddEventLogForResource (EvtMleResourceUnprotect, Resource);
    Resource = (STM_RSC *)((UINTN)Resource + Resource->Header.Length);
  }
  if (Resource->End.ResourceListContinuation != 0) {
    UnRegisterProtectedResource ((STM_RSC *)(UINTN)Resource->End.ResourceListContinuation);
  }
}

/**

  This function return end node of resource list in 4K page.

  @param Resource STM resource list to be parsed

  @return end node of resource list in 4K page.
          NULL means no end node found in 4K page, or list is mal-format.

**/
STM_RSC *
GetEndNodeOfThisList (
  IN STM_RSC   *Resource
  )
{
  STM_RSC   *Node;

  Node = Resource;
  do {
    if (Node->Header.RscType == END_OF_RESOURCES) {
      return Node;
    }
    if (Node->Header.RscType > MAX_DESC_TYPE) {
      return NULL;
    }
    Node = (STM_RSC *)((UINTN)Node + Node->Header.Length);
  } while ((UINTN)Node < (UINTN)Resource + STM_PAGES_TO_SIZE (1) - sizeof(STM_RSC_END));

  return NULL;
}

/**

  This function will allocate page one by one, to duplicate STM resource list.

  This function also validates input StmResource address region.
  Address region is not allowed to overlap with MSEG.
  Address region is not allowed to exceed STM accessable region.

  @param Resource STM resource list to be duplicated

  @return STM resource list duplicated

**/
STM_RSC *
RawDuplicateResource (
  IN STM_RSC   *Resource
  )
{
  STM_RSC   *Base;
  STM_RSC   *NextDestination;
  STM_RSC   *NextSource;
  STM_RSC   *ThisEndNode;

  Base = NULL;
  ThisEndNode = NULL;
  NextSource = Resource;
  while (NextSource != NULL) {
    if (!IsGuestAddressValid ((UINTN)NextSource, 1, TRUE)) {
      return NULL;
    }

    NextDestination = AllocatePages (1);
    if (NextDestination == NULL) {
      return NULL;
    }
    if (Base == NULL) {
      Base = NextDestination;
    } else {
      ThisEndNode->End.ResourceListContinuation = (UINT64)(UINTN)NextDestination;
    }

    CopyMem (NextDestination, NextSource, STM_PAGES_TO_SIZE (1));
    ThisEndNode = GetEndNodeOfThisList (NextDestination);
    if (ThisEndNode == NULL) {
      return NULL;
    }
    NextSource = (STM_RSC *)(UINTN)ThisEndNode->End.ResourceListContinuation;
  }

  return Base;
}

/**

  This function return end node which pointer to last resource list.
  Each resource list is in 4K page.

  @param Resource STM resource list to be parsed

  @return end node which pointer to last resource list.
          NULL means this is already last resource list, or list is mal-format.

**/
STM_RSC *
GetEndNodeToLastList (
  IN STM_RSC   *Resource
  )
{
  STM_RSC   *NextSource;
  STM_RSC   *ReturnedEndNode;
  STM_RSC   *ThisEndNode;

  ReturnedEndNode = NULL;
  NextSource = Resource;
  while (NextSource != NULL) {
    ThisEndNode = GetEndNodeOfThisList (NextSource);
    if (ThisEndNode == NULL) {
      break;
    }
    if (ThisEndNode->End.ResourceListContinuation == 0) {
      break;
    }
    ReturnedEndNode = ThisEndNode;
    NextSource = (STM_RSC *)(UINTN)ThisEndNode->End.ResourceListContinuation;
  }
  return ReturnedEndNode;
}

/**

  This function will free STM resource list (one by one page).

  NOTE: This function will find and free last list, and then last but one, ...
  and finally first list.
  The algo is O(n^2), instead of O(n) as normal free list function.
  It is purposely designed, because FreePages function only support *recollect*
  page from last AllocatePages(). In order to avoid complicated heap management algo.

  @param Resource STM resource list to be freed

**/
VOID
RawFreeResource (
  IN STM_RSC   *Resource
  )
{
  STM_RSC   *Node;

  while (TRUE) {
    Node = GetEndNodeToLastList (Resource);
    if (Node == NULL) {
      break;
    }
    FreePages ((VOID *)(UINTN)Node->End.ResourceListContinuation, 1);
    Node->End.ResourceListContinuation = 0;
  }

  FreePages (Resource, 1);
}

/**

  This functin duplicate STM resource list.

  @param Resource STM resource list to be duplicated

  @return STM resource list duplicated

**/
STM_RSC *
DuplicateResource (
  IN STM_RSC   *Resource
  )
{
  UINTN     ResourceSize;
  STM_RSC   *Base;

  ResourceSize = GetSizeFromResource (Resource);
  if (ResourceSize == 0) {
    return NULL;
  }
  Base = AllocatePages (STM_SIZE_TO_PAGES (ResourceSize));
  if (Base == NULL) {
    return NULL;
  }

  CopyStmResourceMem (Base, Resource);

  return Base;
}

/**

  This function register BIOS resource node to VMCS.

  @param Resource          Resource node to be registered

**/
VOID
RegisterBiosResourceNode (
  IN STM_RSC   *Resource
  )
{
  if (Resource->Header.IgnoreResource != 0) {
    return ;
  }
  switch (Resource->Header.RscType) {
  case MACHINE_SPECIFIC_REG:
    if (Resource->Msr.KernelModeProcessing != 0) {
      if (Resource->Msr.ReadMask != 0) {
        SetMsrBitmap (Resource->Msr.MsrIndex, FALSE);
      }
      if (Resource->Msr.WriteMask != 0) {
        SetMsrBitmap (Resource->Msr.MsrIndex, TRUE);
      }
    }
    break;
  case MEM_RANGE:
  case MMIO_RANGE:
  case IO_RANGE:
  case PCI_CFG_RANGE:
  case TRAPPED_IO_RANGE:
    // Not supported
    break;
  case ALL_RESOURCES:
    // Seems imposible
    break;
  case REGISTER_VIOLATION:
    // Not supported
    break;
  default:
    // Unknown should not be here
    ASSERT (FALSE);
    break;
  }
}

/**

  This function register BIOS resource list to VMCS.

  @param Resource          Resource list to be registered

**/
VOID
RegisterBiosResource (
  IN STM_RSC   *Resource
  )
{
  //
  // TBD: Need adjust resource protection according to BiosHwResourceRequirementsPtr (for EvtBiosAccessToUnclaimedResource).
  //
  while (Resource->Header.RscType != END_OF_RESOURCES) {
    RegisterBiosResourceNode (Resource);
    Resource = (STM_RSC *)((UINTN)Resource + Resource->Header.Length);
  }
  if (Resource->End.ResourceListContinuation != 0) {
    RegisterBiosResource ((STM_RSC *)(UINTN)Resource->End.ResourceListContinuation);
  }
}

