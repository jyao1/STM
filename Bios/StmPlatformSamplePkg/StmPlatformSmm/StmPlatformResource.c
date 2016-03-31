/** @file
  STM platform SMM resource

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmPlatformSmm.h"

#define RDWR_ACCS             3
#define FULL_ACCS             7

UINT32                          mMaxBus;
UINT32                          mTsegBase;
UINT32                          mTsegSize;
UINT16                          mPmBase;

//
// Fixed memory ranges
//

//
// TSEG memory!
// 
STM_RSC_MEM_DESC  RscTsegMemory = {
  {MEM_RANGE, sizeof (STM_RSC_MEM_DESC)},
  0,
  0,
  FULL_ACCS
};
//
// Flash part
//
STM_RSC_MEM_DESC  RscSpiMemory = {
  {MEM_RANGE, sizeof (STM_RSC_MEM_DESC)},
  0xFE000000,
  0x01000000,
  FULL_ACCS
};
//
// ACPI
//
STM_RSC_IO_DESC RscPmIo = {
  {IO_RANGE, sizeof (STM_RSC_IO_DESC)},
  0,
  128
};

//
// PCIE MMIO
//
STM_RSC_MMIO_DESC  RscPcieMmio = {
  {MMIO_RANGE, sizeof (STM_RSC_MMIO_DESC)},
  0,
  0, // Length
  RDWR_ACCS
};
//
// Local APIC
//
STM_RSC_MMIO_DESC  RscApicMmio = {
  {MMIO_RANGE, sizeof (STM_RSC_MMIO_DESC)},
  0,
  0x400,
  RDWR_ACCS
};
//
// Software SMI
//
STM_RSC_TRAPPED_IO_DESC RscSwSmiTrapIo = {
  {TRAPPED_IO_RANGE, sizeof (STM_RSC_TRAPPED_IO_DESC)},
  0xB2,
  2
};
//
// End of list
//
STM_RSC_END       RscListEnd = {
  {END_OF_RESOURCES, sizeof (STM_RSC_END)},
  0
};

//
// Common PCI devices
//
//
// LPC bridge
//
STM_RSC_PCI_CFG_DESC RscLpcBridgePci = {
  {PCI_CFG_RANGE, sizeof (STM_RSC_PCI_CFG_DESC)},
  RDWR_ACCS, 0,
  0,
  0x1000,
  0,
  0,
  {
    {1, 1, sizeof(STM_PCI_DEVICE_PATH_NODE), LPC_FUNCTION, LPC_DEVICE},
  },
};

//
// Template for MSR resources.
//
STM_RSC_MSR_DESC RscMsrTpl = {
  {MACHINE_SPECIFIC_REG, sizeof (STM_RSC_MSR_DESC)},
};

//
// MSR indices to register
//
typedef struct {
  UINT32      MsrIndex;
  UINT64      ReadMask;
  UINT64      WriteMask;
} MSR_TABLE_ENTRY;

MSR_TABLE_ENTRY MsrTable[] = {
  // Index                                 Read    Write   // MASK64 means need access, MASK0 means no need access.
  {SMRR_PHYSBASE_MSR,                      MASK64, MASK0},
  {SMRR_PHYSMASK_MSR,                      MASK64, MASK0},
};

/**
  Reads a 64-bit PCI configuration register.

  @param  Address Address that encodes the PCI Bus, Device, Function and
                  Register.

  @return The read value from the PCI configuration register.

**/
UINT64
PciRead64 (
  IN      UINTN                     Address
  )
{ 
  UINT64  RegLow;
  UINT64  RegHigh;

  RegLow = (UINT64) PciRead32(Address);
  RegHigh = (UINT64) PciRead32(Address + 4);

  return (LShiftU64 (RegHigh, 32) + RegLow);
}

/**

  BIOS resources initialization.

**/
VOID
ResourceInit (
  VOID
  )
{
  mMaxBus = 255;

  mPmBase = (UINT16) PciRead16 (
                        PCI_LIB_ADDRESS (
                          LPC_BUS,
                          LPC_DEVICE,
                          LPC_FUNCTION,
                          R_ACPI_PM_BASE)
                        ) & ACPI_PM_BASE_MASK;

  mTsegBase = (UINT32)AsmReadMsr64 (SMRR_PHYSBASE_MSR) & 0xFFFFF000;
  mTsegSize = (UINT32)(~((UINT32)AsmReadMsr64 (SMRR_PHYSMASK_MSR) & 0xFFFFF000) + 1);
}

/**

  Fix up PCIE resource.

**/
VOID
FixupPciexResource (
  VOID
  )
{
  //
  // Find max bus number and PCIEX length
  // 
  RscPcieMmio.Length = 0x10000000;    // 256 MB
  RscPcieMmio.Base = PcdGet64 (PcdPciExpressBaseAddress);
}

/**

  Add basic resources to BIOS resource database.

**/
VOID
AddSimpleResources (
  VOID
  )
{
  EFI_STATUS  Status;

  //
  // Fix-up values
  //
  RscTsegMemory.Base   = mTsegBase;
  RscTsegMemory.Length = mTsegSize;

  RscPmIo.Base = (UINT16) mPmBase;
  
  //
  // Local APIC. We assume that all thteads are programmed identically
  // despite that it is possible to have individual APIC address for
  // each of the threads. If this is the case this programming should
  // be corrected. 
  //
  RscApicMmio.Base = AsmReadMsr64 (IA32_APIC_BASE_MSR_INDEX) & 0xFFFFFF000ull;

  //
  // PCIEX BAR
  //
  FixupPciexResource ();

  Status = mSmMonitorInitProtocol->AddPiResource((VOID *) &RscTsegMemory, 0);
  ASSERT_EFI_ERROR (Status);
  Status = mSmMonitorInitProtocol->AddPiResource((VOID *) &RscLpcBridgePci, 1);
  ASSERT_EFI_ERROR (Status);
}

/**

  Add MSR resources to BIOS resource database.

**/
VOID
AddMsrResources (
  VOID
  )
{
  EFI_STATUS Status;
  UINT32     Index;

  for (Index = 0; Index < sizeof(MsrTable)/sizeof(MsrTable[0]); Index ++) {
    
    RscMsrTpl.MsrIndex = (UINT32) MsrTable[Index].MsrIndex;
    RscMsrTpl.ReadMask = (UINT64) MsrTable[Index].ReadMask;
    RscMsrTpl.WriteMask = (UINT64) MsrTable[Index].WriteMask;

    Status = mSmMonitorInitProtocol->AddPiResource ((VOID *) &RscMsrTpl, 1);
    ASSERT_EFI_ERROR (Status);
  }
}

/**

  Add resources to BIOS resource database.

**/
VOID
AddResourcesCmd (
  VOID
  )
{
  ResourceInit ();

  AddSimpleResources();

  AddMsrResources ();
}
