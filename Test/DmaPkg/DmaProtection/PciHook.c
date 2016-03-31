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

typedef struct {
  UINTN                                     NumberOfBytes;
  EFI_PHYSICAL_ADDRESS                      DeviceAddress;
  VOID                                      *OrgMapping;
} MAP_INFO;

EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_MAP             mOrgMap;
EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_UNMAP           mOrgUnmap;

BOOLEAN  mVtdHookEnabled;

typedef struct {
  PCI_DESCRIPTOR  PciDescriptor;
  UINT64          BaseAddress;
  UINT64          Length;
  UINTN           AccessCount;
} ACCESS_RECORD_DATABASE;

#define  ACCESS_RECORD_DATABASE_MAX_COUNT  0x1000

UINTN                    mAccessRecordDatabaseCount;
ACCESS_RECORD_DATABASE   mAccessRecordDatabase[ACCESS_RECORD_DATABASE_MAX_COUNT];

typedef struct {
  EFI_PCI_IO_PROTOCOL              *PciIo;
  EFI_PCI_IO_PROTOCOL_MAP          OrgMap;
  EFI_PCI_IO_PROTOCOL_UNMAP        OrgUnmap;
  PCI_DESCRIPTOR                   PciDescriptor;
} PCI_IO_INFO;

UINTN        mPciIoInfoCount;
PCI_IO_INFO  mPciIoInfo[MAX_PCI_DESCRIPTORS];

VOID         *mPciIoRegistration;

/**

  This function set access attribute for PCI device.

  @param Bus           Pci bus
  @param Device        Pci device
  @param Function      Pci function
  @param BaseAddress   BaseAddress
  @param Length        Length
  @param Allow         If allow access

  @retval EFI_SUCCESS  set access attribute successfully

**/
EFI_STATUS
SetAccessAttribute (
  IN UINT8                 Bus,
  IN UINT8                 Device,
  IN UINT8                 Function,
  IN UINT64                BaseAddress,
  IN UINT64                Length,
  IN BOOLEAN               Allow
  )
{
  EFI_STATUS  Status;
  UINTN       Index;

  if (mVtdHookEnabled) {
    Status = SetPageAttributeByPciDevice (
               Bus,
               Device,
               Function,
               BaseAddress,
               Length,
               Allow
               );
    return Status;
  }
  
  DEBUG ((EFI_D_INFO,"SetAccessAttribute (B%02x D%02x F%02x) (0x%016lx - 0x%08x, %d)\n", Bus, Device, Function, BaseAddress, (UINTN)Length, Allow));

  //
  // Record Entry
  //
  for (Index = 0; Index < mAccessRecordDatabaseCount; Index++) {
    if ((mAccessRecordDatabase[Index].BaseAddress == BaseAddress) && (mAccessRecordDatabase[Index].Length == Length)) {
      //
      // Found old entry
      //
      ASSERT (mAccessRecordDatabase[Index].AccessCount != 0);
      if (Allow) {
        mAccessRecordDatabase[Index].AccessCount++;
      } else {
        mAccessRecordDatabase[Index].AccessCount--;
        if (mAccessRecordDatabase[Index].AccessCount == 0) {
          CopyMem (&mAccessRecordDatabase[Index], &mAccessRecordDatabase[mAccessRecordDatabaseCount - 1], sizeof(mAccessRecordDatabase[0]));
          mAccessRecordDatabaseCount--;
        }
      }
      return EFI_SUCCESS;
    }
  }

  //
  // Not found
  //
  ASSERT (Allow);
  ASSERT (mAccessRecordDatabaseCount < ACCESS_RECORD_DATABASE_MAX_COUNT);
  if (mAccessRecordDatabaseCount >= ACCESS_RECORD_DATABASE_MAX_COUNT) {
    DEBUG ((EFI_D_INFO, "  AccessRecordDatabase exceed - %d\n", mAccessRecordDatabaseCount));
    return EFI_OUT_OF_RESOURCES;
  }
  mAccessRecordDatabase[mAccessRecordDatabaseCount].PciDescriptor.Bus = Bus;
  mAccessRecordDatabase[mAccessRecordDatabaseCount].PciDescriptor.Device = Device;
  mAccessRecordDatabase[mAccessRecordDatabaseCount].PciDescriptor.Function = Function;
  mAccessRecordDatabase[mAccessRecordDatabaseCount].BaseAddress = BaseAddress;
  mAccessRecordDatabase[mAccessRecordDatabaseCount].Length = Length;
  mAccessRecordDatabase[mAccessRecordDatabaseCount].AccessCount = 1;
  mAccessRecordDatabaseCount++;

  return EFI_SUCCESS;
}

/**
  Provides the PCI controller-specific address needed to access
  system memory for DMA.

  @param This           A pointer to the EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL.
  @param Operation      Indicate if the bus master is going to read or write
                        to system memory.
  @param HostAddress    The system memory address to map on the PCI controller.
  @param NumberOfBytes  On input the number of bytes to map.
                        On output the number of bytes that were mapped.
  @param DeviceAddress  The resulting map address for the bus master PCI
                        controller to use to access the system memory's HostAddress.
  @param Mapping        The value to pass to Unmap() when the bus master DMA
                        operation is complete.

  @retval EFI_SUCCESS            Success.
  @retval EFI_INVALID_PARAMETER  Invalid parameters found.
  @retval EFI_UNSUPPORTED        The HostAddress cannot be mapped as a common buffer.
  @retval EFI_DEVICE_ERROR       The System hardware could not map the requested address.
  @retval EFI_OUT_OF_RESOURCES   The request could not be completed due to lack of resources.
**/
EFI_STATUS
EFIAPI
VtdRootBridgeIoMap (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL            *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_OPERATION  Operation,
  IN     VOID                                       *HostAddress,
  IN OUT UINTN                                      *NumberOfBytes,
  OUT    EFI_PHYSICAL_ADDRESS                       *DeviceAddress,
  OUT    VOID                                       **Mapping
  )
{
  EFI_STATUS                       Status;
  MAP_INFO                         *MyMapping;
  VOID                             *OrgMapping;

  Status = mOrgMap (This, Operation, HostAddress, NumberOfBytes, DeviceAddress, &OrgMapping);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  MyMapping = AllocatePool (sizeof(MAP_INFO));
  if (MyMapping == NULL) {
    mOrgUnmap (This, OrgMapping);
    return EFI_OUT_OF_RESOURCES;
  }

  MyMapping->NumberOfBytes = *NumberOfBytes;
  MyMapping->DeviceAddress = *DeviceAddress;
  MyMapping->OrgMapping    = OrgMapping;
  *Mapping = MyMapping;

  //
  // Enable
  //
  Status = SetAccessAttribute (
             0,
             0,
             0,
             *DeviceAddress,
             *NumberOfBytes,
             TRUE
             );
  if (EFI_ERROR(Status)) {
    mOrgUnmap (This, OrgMapping);
    return Status;
  }

  return EFI_SUCCESS;
}

/**
  Completes the Map() operation and releases any corresponding resources.

  The Unmap() function completes the Map() operation and releases any
  corresponding resources.
  If the operation was an EfiPciOperationBusMasterWrite or
  EfiPciOperationBusMasterWrite64, the data is committed to the target system
  memory.
  Any resources used for the mapping are freed.

  @param[in] This      A pointer to the EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL.
  @param[in] Mapping   The mapping value returned from Map().

  @retval EFI_SUCCESS            The range was unmapped.
  @retval EFI_INVALID_PARAMETER  Mapping is not a value that was returned by Map().
  @retval EFI_DEVICE_ERROR       The data was not committed to the target system memory.
**/
EFI_STATUS
EFIAPI
VtdRootBridgeIoUnmap (
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  IN VOID                             *Mapping
  )
{
  EFI_STATUS                       Status;
  MAP_INFO                         *MyMapping;

  MyMapping = Mapping;
  Status = mOrgUnmap (This, MyMapping->OrgMapping);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Disable
  //
  SetAccessAttribute (
    0,
    0,
    0,
    MyMapping->DeviceAddress,
    MyMapping->NumberOfBytes,
    FALSE
    );

  FreePool (MyMapping);

  return EFI_SUCCESS;
}

/**

  This function return PCI_IO_INFO structure based on PCI_IO protocol.

  @param PciIo  PCI_IO protocol

  @return PCI_IO_INFO structure

**/
PCI_IO_INFO *
GetPciIoInfo (
  IN     EFI_PCI_IO_PROTOCOL            *PciIo
  )
{
  UINTN  Index;

  for (Index = 0; Index < mPciIoInfoCount; Index++) {
    if (mPciIoInfo[Index].PciIo == PciIo) {
      return &mPciIoInfo[Index];
    }
  }

  return NULL;
}

/**
  Provides the PCI controller-specific addresses needed to access system memory.

  @param  This                  A pointer to the EFI_PCI_IO_PROTOCOL instance.
  @param  Operation             Indicates if the bus master is going to read or write to system memory.
  @param  HostAddress           The system memory address to map to the PCI controller.
  @param  NumberOfBytes         On input the number of bytes to map. On output the number of bytes
                                that were mapped.
  @param  DeviceAddress         The resulting map address for the bus master PCI controller to use to
                                access the hosts HostAddress.
  @param  Mapping               A resulting value to pass to Unmap().

  @retval EFI_SUCCESS           The range was mapped for the returned NumberOfBytes.
  @retval EFI_UNSUPPORTED       The HostAddress cannot be mapped as a common buffer.
  @retval EFI_INVALID_PARAMETER One or more parameters are invalid.
  @retval EFI_OUT_OF_RESOURCES  The request could not be completed due to a lack of resources.
  @retval EFI_DEVICE_ERROR      The system hardware could not map the requested address.

**/
EFI_STATUS
EFIAPI
VtdPciIoMap (
  IN     EFI_PCI_IO_PROTOCOL            *This,
  IN     EFI_PCI_IO_PROTOCOL_OPERATION  Operation,
  IN     VOID                           *HostAddress,
  IN OUT UINTN                          *NumberOfBytes,
  OUT    EFI_PHYSICAL_ADDRESS           *DeviceAddress,
  OUT    VOID                           **Mapping
  )
{
  EFI_STATUS                       Status;
  PCI_IO_INFO                      *PciIoInfo;
  MAP_INFO                         *MyMapping;
  VOID                             *OrgMapping;

  PciIoInfo = GetPciIoInfo (This);
  ASSERT (PciIoInfo != NULL);

  Status = PciIoInfo->OrgMap (This, Operation, HostAddress, NumberOfBytes, DeviceAddress, &OrgMapping);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  MyMapping = AllocatePool (sizeof(MAP_INFO));
  if (MyMapping == NULL) {
    PciIoInfo->OrgUnmap (This, OrgMapping);
    return EFI_OUT_OF_RESOURCES;
  }

  MyMapping->NumberOfBytes = *NumberOfBytes;
  MyMapping->DeviceAddress = *DeviceAddress;
  MyMapping->OrgMapping    = OrgMapping;
  *Mapping = MyMapping;

  //
  // Enable
  //
  Status = SetAccessAttribute (
             PciIoInfo->PciDescriptor.Bus,
             PciIoInfo->PciDescriptor.Device,
             PciIoInfo->PciDescriptor.Function,
             *DeviceAddress,
             *NumberOfBytes,
             TRUE
             );
  if (EFI_ERROR(Status)) {
    PciIoInfo->OrgUnmap (This, OrgMapping);
    return Status;
  }

  return EFI_SUCCESS;
}

/**
  Completes the Map() operation and releases any corresponding resources.

  @param  This                  A pointer to the EFI_PCI_IO_PROTOCOL instance.
  @param  Mapping               The mapping value returned from Map().

  @retval EFI_SUCCESS           The range was unmapped.
  @retval EFI_DEVICE_ERROR      The data was not committed to the target system memory.

**/
EFI_STATUS
EFIAPI
VtdPciIoUnmap (
  IN EFI_PCI_IO_PROTOCOL  *This,
  IN VOID                 *Mapping
  )
{
  EFI_STATUS                       Status;
  MAP_INFO                         *MyMapping;
  PCI_IO_INFO                      *PciIoInfo;
  
  PciIoInfo = GetPciIoInfo (This);
  ASSERT (PciIoInfo != NULL);

  MyMapping = Mapping;
  Status = PciIoInfo->OrgUnmap (This, MyMapping->OrgMapping);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Disable
  //
  SetAccessAttribute (
    PciIoInfo->PciDescriptor.Bus,
    PciIoInfo->PciDescriptor.Device,
    PciIoInfo->PciDescriptor.Function,
    MyMapping->DeviceAddress,
    MyMapping->NumberOfBytes,
    FALSE
    );

  FreePool (MyMapping);

  return EFI_SUCCESS;
}

/**

  This function hook PCI_IO protocol.

**/
EFI_STATUS
HookPciIo (
  IN EFI_PCI_IO_PROTOCOL   *PciIo
  )
{
  EFI_STATUS            Status;
  UINTN                 Seg;
  UINTN                 Bus;
  UINTN                 Device;
  UINTN                 Function;
  PCI_IO_INFO           *PciIoInfo;

  PciIoInfo = GetPciIoInfo (PciIo);
  if (PciIoInfo != NULL) {
    return EFI_SUCCESS;
  }

  Status = PciIo->GetLocation (PciIo, &Seg, &Bus, &Device, &Function);
  ASSERT_EFI_ERROR (Status);
  
  DEBUG ((EFI_D_INFO,"HookPciIo - Pci device (B%02x D%02x F%02x)\n", Bus, Device, Function));

  ASSERT (mPciIoInfoCount < MAX_PCI_DESCRIPTORS);

  mPciIoInfo[mPciIoInfoCount].PciIo = PciIo;
  mPciIoInfo[mPciIoInfoCount].OrgMap = PciIo->Map;
  mPciIoInfo[mPciIoInfoCount].OrgUnmap = PciIo->Unmap;
  mPciIoInfo[mPciIoInfoCount].PciDescriptor.Bus = (UINT8)Bus;
  mPciIoInfo[mPciIoInfoCount].PciDescriptor.Device = (UINT8)Device;
  mPciIoInfo[mPciIoInfoCount].PciDescriptor.Function = (UINT8)Function;
  mPciIoInfoCount++;

  PciIo->Map = VtdPciIoMap;
  PciIo->Unmap = VtdPciIoUnmap;

  return EFI_SUCCESS;
}

/**

  This function is PCI_IO protocol notification.

  @param Event   event
  @param Context event context

**/
VOID
EFIAPI
PciIoNotification (
  IN  EFI_EVENT                Event,
  IN  VOID                     *Context
  )
{
  EFI_PCI_IO_PROTOCOL   *PciIo;
  EFI_HANDLE            Handle;
  UINTN                 BufferSize;
  EFI_STATUS            Status;

  Status = gBS->LocateProtocol (
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  (VOID **)&PciIo
                  );
  if (EFI_ERROR (Status)) {
    return ;
  }

  DEBUG((EFI_D_INFO, "Vtd PciIoNotification\n"));
  while (TRUE) {
    BufferSize = sizeof (Handle);
    Status = gBS->LocateHandle (
                    ByRegisterNotify,
                    NULL,
                    mPciIoRegistration,
                    &BufferSize,
                    &Handle
                    );
    if (EFI_ERROR(Status)) {
      break;
    }
  
    Status = gBS->HandleProtocol (
                    Handle,
                    &gEfiPciIoProtocolGuid,
                    (VOID **)&PciIo
                    );
    ASSERT_EFI_ERROR (Status);
    HookPciIo (PciIo);
  }

  return ;
}

/**

  This function hook PCI related protocol.

  @return EFI_SUCCESS hook PCI related protocol successfully.

**/
EFI_STATUS
HookPciProtocol (
  VOID
  )
{
  EFI_STATUS                       Status;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *PciRootBridgeIo;
  EFI_EVENT                        NotifyEvent;

  Status = gBS->LocateProtocol (
                  &gEfiPciRootBridgeIoProtocolGuid,
                  NULL,
                  (VOID **)&PciRootBridgeIo
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (0) {
    mOrgMap = PciRootBridgeIo->Map;
    mOrgUnmap = PciRootBridgeIo->Unmap;
    PciRootBridgeIo->Map = VtdRootBridgeIoMap;
    PciRootBridgeIo->Unmap = VtdRootBridgeIoUnmap;
  }
  
  //
  // Do not use PciEnumerationComplet, because that is triggered before PciIo
  //
  NotifyEvent = EfiCreateProtocolNotifyEvent (
                  &gEfiPciIoProtocolGuid,
                  TPL_CALLBACK,
                  PciIoNotification,
                  NULL,
                  &mPciIoRegistration
                  );
  ASSERT (NotifyEvent != NULL);

  return EFI_SUCCESS;
}

/**

  This function activate hook PCI protocol.

  @return EFI_SUCCESS activate hook PCI protocol successfully.

**/
EFI_STATUS
ActivateHookPciProtocol (
  VOID
  )
{
  UINTN       Index;
  UINTN       SubIndex;
  EFI_STATUS  Status;

  mVtdHookEnabled = TRUE;
  
  DEBUG ((EFI_D_INFO, "ActivateHookPciProtocol - %d\n", mAccessRecordDatabaseCount));

  for (Index = 0; Index < mAccessRecordDatabaseCount; Index++) {
    for (SubIndex = 0; SubIndex < mAccessRecordDatabase[Index].AccessCount; SubIndex++) {
      Status = SetPageAttributeByPciDevice (
                 mAccessRecordDatabase[Index].PciDescriptor.Bus,
                 mAccessRecordDatabase[Index].PciDescriptor.Device,
                 mAccessRecordDatabase[Index].PciDescriptor.Function,
                 mAccessRecordDatabase[Index].BaseAddress,
                 mAccessRecordDatabase[Index].Length,
                 TRUE
                 );
      if (EFI_ERROR (Status)) {
        // Do not check error because some device might not in list
      }
    }
  }
  return EFI_SUCCESS;
}