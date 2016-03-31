/** @file

  Copyright (c) 2014, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "DmaProtection.h"

/**

  This function return PCI descriptor.

  @param VtdIndex      VTd engine index
  @param Bus           Pci bus
  @param Device        Pci device
  @param Function      Pci function

  @return PCI descriptor index

**/
UINT8
GetPciDescriptor (
  IN UINTN VtdIndex,
  IN UINT8 Bus,
  IN UINT8 Device,
  IN UINT8 Function
  )
{
  UINT8  Index;
  
  for (Index = 1; Index <= mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDescriptorNumber; Index++) {
    if ((mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDescriptors[Index].Bus == Bus) &&
        (mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDescriptors[Index].Device == Device) &&
        (mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDescriptors[Index].Function == Function) ) {
      return Index;
    }
  }

  return 0;
}

/**

  This function register PCI device.

  @param VtdIndex      VTd engine index
  @param Bus           Pci bus
  @param Device        Pci device
  @param Function      Pci function
  @param CheckExist    If check PCI device exist

**/
EFI_STATUS
RegisterPciDevice (
  IN UINTN          VtdIndex,
  IN UINT8          Bus,
  IN UINT8          Device,
  IN UINT8          Function,
  IN BOOLEAN        CheckExist
  )
{
  PCI_DESCRIPTOR          *PciDescriptor;
  UINT32                  PciDescriptorIndex;
  UINTN                   Index;

  if (mVtdUnitInformation[VtdIndex].PciDeviceInfo.IncludeAllFlag) {
    //
    // Do not register device in other VTD Unit
    //
    for (Index = 0; Index < VtdIndex; Index++) {
      PciDescriptorIndex = GetPciDescriptor (Index, Bus, Device, Function);
      if (PciDescriptorIndex != 0) {
        DEBUG ((EFI_D_INFO, "  RegisterPciDevice: PCI B%02x D%02x F%02x already registered by Other Vtd(%d)\n", Bus, Device, Function, Index));
        return EFI_SUCCESS;
      }
    }
  }

  PciDescriptorIndex = GetPciDescriptor (VtdIndex, Bus, Device, Function);
  if (PciDescriptorIndex == 0) {
    mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDescriptorNumber++;
    ASSERT (mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDescriptorNumber < MAX_PCI_DESCRIPTORS);
    if (mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDescriptorNumber >= MAX_PCI_DESCRIPTORS) {
      mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDescriptorNumber--;
      DEBUG ((EFI_D_INFO, "  PciDescriptorNumber exceed - %d\n", mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDescriptorNumber));
      return EFI_OUT_OF_RESOURCES;
    }

    PciDescriptor = &mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDescriptors[mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDescriptorNumber];
    PciDescriptor->Bus = Bus;
    PciDescriptor->Device = Device;
    PciDescriptor->Function = Function;

    DEBUG ((EFI_D_INFO, "  RegisterPciDevice: PCI B%02x D%02x F%02x\n", Bus, Device, Function));
  } else {
    if (CheckExist) {
      DEBUG ((EFI_D_INFO, "  RegisterPciDevice: PCI B%02x D%02x F%02x already registered\n", Bus, Device, Function));
      return EFI_DEVICE_ERROR;
    }
  }

  return EFI_SUCCESS;
}

/**

  This function scan PCI bus.

  @param VtdIndex      VTd engine index
  @param Bus           Pci bus

  @retval EFI_SUCCESS  scan PCI bus successfully.

**/
EFI_STATUS
ScanPciBus (
  IN UINTN          VtdIndex,
  IN UINT8          Bus
  )
{
  UINT8                   Device;
  UINT8                   Function;
  UINT8                   SecondaryBusNumber;
  UINT8                   HeaderType;
  UINT8                   BaseClass;
  UINT8                   SubClass;
  UINT32                  MaxFunction;
  UINT16                  VendorID;
  UINT16                  DeviceID;
  EFI_STATUS              Status;

  // Scan the PCI bus for devices
  for (Device = 0; Device < PCI_MAX_DEVICE + 1; Device++) {
    HeaderType = PciRead8 (PCI_LIB_ADDRESS(Bus, Device, 0, PCI_HEADER_TYPE_OFFSET));
    MaxFunction = PCI_MAX_FUNC + 1;
    if ((HeaderType & HEADER_TYPE_MULTI_FUNCTION) == 0x00) {
      MaxFunction = 1;
    }
    for (Function = 0; Function < MaxFunction; Function++) {
      VendorID  = PciRead16 (PCI_LIB_ADDRESS(Bus, Device, Function, PCI_VENDOR_ID_OFFSET));
      DeviceID  = PciRead16 (PCI_LIB_ADDRESS(Bus, Device, Function, PCI_DEVICE_ID_OFFSET));
      if (VendorID == 0xFFFF && DeviceID == 0xFFFF) {
        continue;
      }

      Status = RegisterPciDevice (VtdIndex, Bus, Device, Function, FALSE);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      BaseClass = PciRead8 (PCI_LIB_ADDRESS(Bus, Device, Function, PCI_CLASSCODE_OFFSET + 2));
      if (BaseClass == PCI_CLASS_BRIDGE) {
        SubClass = PciRead8 (PCI_LIB_ADDRESS(Bus, Device, Function, PCI_CLASSCODE_OFFSET + 1));
        if (SubClass == PCI_CLASS_BRIDGE_P2P) {
          SecondaryBusNumber = PciRead8 (PCI_LIB_ADDRESS(Bus, Device, Function, PCI_BRIDGE_SECONDARY_BUS_REGISTER_OFFSET));
          DEBUG ((EFI_D_INFO,"  ScanPciBus: PCI bridge B%02x D%02x F%02x (SecondBus:%02x)\n", Bus, Device, Function, SecondaryBusNumber));
          if (SecondaryBusNumber != 0) {
            ScanPciBus (VtdIndex, SecondaryBusNumber);
          }
        }
      }
    }
  }

  return EFI_SUCCESS;
}

/**

  This function dump PCI device information.

  @param VtdIndex      VTd engine index

**/
VOID
DumpPciDeviceInfo (
  IN UINTN  VtdIndex
  )
{
  UINTN  Index;

  DEBUG ((EFI_D_INFO,"PCI Device Information (Number 0x%x, IncludeAll - %d):\n",
    mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDescriptorNumber,
    mVtdUnitInformation[VtdIndex].PciDeviceInfo.IncludeAllFlag
    ));
  for (Index = 1; Index <= mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDescriptorNumber; Index++) {
    DEBUG ((EFI_D_INFO,"  B%02x D%02x F%02x\n",
      mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDescriptors[Index].Bus,
      mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDescriptors[Index].Device,
      mVtdUnitInformation[VtdIndex].PciDeviceInfo.PciDescriptors[Index].Function
      ));
  }
}

/**

  This function return VTd engine index by PCI bus/device/function.

  @param Bus           Pci bus
  @param Device        Pci device
  @param Function      Pci function

  @return VTd engine index

**/
UINTN
FindVtdIndexByPciDevice (
  IN UINT8          Bus,
  IN UINT8          Device,
  IN UINT8          Function
  )
{
  UINTN                   VtdIndex;
  VTD_ROOT_ENTRY          *RootEntry;
  VTD_CONTEXT_ENTRY       *ContextEntryTable;
  VTD_CONTEXT_ENTRY       *ContextEntry;
  VTD_EXT_ROOT_ENTRY      *ExtRootEntry;
  VTD_EXT_CONTEXT_ENTRY   *ExtContextEntryTable;
  VTD_EXT_CONTEXT_ENTRY   *ExtContextEntry;
  UINT32                  PciDescriptorIndex;

  for (VtdIndex = 0; VtdIndex < mVtdUnitNumber; VtdIndex++) {
    PciDescriptorIndex = GetPciDescriptor (VtdIndex, Bus, Device, Function);
    if (PciDescriptorIndex == 0) {
      continue;
    }

//    DEBUG ((EFI_D_INFO,"FindVtdIndex(0x%x) for B%02x D%02x F%02x\n", VtdIndex, Bus, Device, Function));

    if (mVtdUnitInformation[VtdIndex].ExtRootEntryTable != 0) {
      ExtRootEntry = &mVtdUnitInformation[VtdIndex].ExtRootEntryTable[Bus];
      ExtContextEntryTable = (VTD_EXT_CONTEXT_ENTRY *)(UINTN)LShiftU64 (ExtRootEntry->Bits.LowerContextTablePointer, 12) ;
      ExtContextEntry      = &ExtContextEntryTable[(Device << 3) | Function];
      if (ExtContextEntry->Bits.DomainIdentifier != UEFI_DOMAIN_ID) {
        continue;
      }
    } else {
      RootEntry = &mVtdUnitInformation[VtdIndex].RootEntryTable[Bus];
      ContextEntryTable = (VTD_CONTEXT_ENTRY *)(UINTN)LShiftU64 (RootEntry->Bits.ContextTablePointer, 12) ;
      ContextEntry      = &ContextEntryTable[(Device << 3) | Function];
      if (ContextEntry->Bits.DomainIdentifier != UEFI_DOMAIN_ID) {
        continue;
      }
    }

    return VtdIndex;
  }

  return (UINTN)-1;
}
