/** @file
  SMM STM support

  Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/SmmServicesTableLib.h>

#include "PiSmmCpuDxeSmm.h"
#include "SmmStm.h"

#define TXT_EVTYPE_BASE                  0x400
#define TXT_EVTYPE_STM_HASH              (TXT_EVTYPE_BASE + 14)

#define RDWR_ACCS             3
#define FULL_ACCS             7

extern EFI_HANDLE  mSmmCpuHandle;

BOOLEAN mLockLoadMonitor = FALSE;

//
// Template of STM_RSC_END structure for copying.
//
GLOBAL_REMOVE_IF_UNREFERENCED STM_RSC_END mRscEndNode = {
  {END_OF_RESOURCES, sizeof (STM_RSC_END)},
};

GLOBAL_REMOVE_IF_UNREFERENCED UINT8  *mStmResourcesPtr         = NULL;
GLOBAL_REMOVE_IF_UNREFERENCED UINTN  mStmResourceTotalSize     = 0x0;
GLOBAL_REMOVE_IF_UNREFERENCED UINTN  mStmResourceSizeUsed      = 0x0;
GLOBAL_REMOVE_IF_UNREFERENCED UINTN  mStmResourceSizeAvailable = 0x0;

GLOBAL_REMOVE_IF_UNREFERENCED UINT32  mStmState = 0;

//
// System Configuration Table pointing to STM Configuration Table
// 
GLOBAL_REMOVE_IF_UNREFERENCED
EFI_SM_MONITOR_INIT_PROTOCOL mSmMonitorInitProtocol = {
  LoadMonitor,
  AddPiResource,
  DeletePiResource,
  GetPiResource,
  GetMonitorState,
};

/**
  This functin initialize STM configuration table.
**/
VOID
StmSmmConfigurationTableInit (
  VOID
  )
{
  EFI_STATUS    Status;

  Status = gSmst->SmmInstallProtocolInterface (
                    &mSmmCpuHandle,
                    &gEfiSmMonitorInitProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    &mSmMonitorInitProtocol
                    );
  ASSERT_EFI_ERROR (Status);

  return ;
}

/**

  Get STM state.

  @return STM state

**/
EFI_SM_MONITOR_STATE
EFIAPI
GetMonitorState (
  VOID
  )
{
  return mStmState;
}

/**

  Handle single Resource to see if it can be merged into Record.

  @param Resource  A pointer to resource node to be added
  @param Record    A pointer to record node to be merged

  @retval TRUE  resource handled
  @retval FALSE resource is not handled

**/
BOOLEAN
HandleSingleResource (
  IN  STM_RSC      *Resource,
  IN  STM_RSC      *Record
  )
{             
  UINT64      ResourceLo;
  UINT64      ResourceHi;
  UINT64      RecordLo;
  UINT64      RecordHi;

  ResourceLo = 0;
  ResourceHi = 0;
  RecordLo = 0;
  RecordHi = 0;

  //
  // Calling code is responsible for making sure that
  // Resource->Header.RscType == (*Record)->Header.RscType
  // thus we use just one of them as switch variable.
  // 
  switch (Resource->Header.RscType) {
  case MEM_RANGE:
  case MMIO_RANGE:
    ResourceLo = Resource->Mem.Base;
    ResourceHi = Resource->Mem.Base + Resource->Mem.Length;
    RecordLo = Record->Mem.Base;
    RecordHi = Record->Mem.Base + Record->Mem.Length;
    if (Resource->Mem.RWXAttributes != Record->Mem.RWXAttributes) {
      if ((ResourceLo == RecordLo) && (ResourceHi == RecordHi)) {
        Record->Mem.RWXAttributes = Resource->Mem.RWXAttributes | Record->Mem.RWXAttributes;
        return TRUE;
      } else {
        return FALSE;
      }
    }
    break;
  case IO_RANGE:
  case TRAPPED_IO_RANGE:
    ResourceLo = (UINT64) Resource->Io.Base;
    ResourceHi = (UINT64) Resource->Io.Base + (UINT64) Resource->Io.Length;
    RecordLo = (UINT64) Record->Io.Base;
    RecordHi = (UINT64) Record->Io.Base + (UINT64) Record->Io.Length;
    break;
  case PCI_CFG_RANGE:
    if ((Resource->PciCfg.OriginatingBusNumber != Record->PciCfg.OriginatingBusNumber) ||
        (Resource->PciCfg.LastNodeIndex != Record->PciCfg.LastNodeIndex)) {
      return FALSE;
    }
    if (CompareMem (Resource->PciCfg.PciDevicePath, Record->PciCfg.PciDevicePath, sizeof(STM_PCI_DEVICE_PATH_NODE) * (Resource->PciCfg.LastNodeIndex + 1)) != 0) {
      return FALSE;
    }
    ResourceLo = (UINT64) Resource->PciCfg.Base;
    ResourceHi = (UINT64) Resource->PciCfg.Base + (UINT64) Resource->PciCfg.Length;
    RecordLo = (UINT64) Record->PciCfg.Base;
    RecordHi = (UINT64) Record->PciCfg.Base + (UINT64) Record->PciCfg.Length;
    if (Resource->PciCfg.RWAttributes != Record->PciCfg.RWAttributes) {
      if ((ResourceLo == RecordLo) && (ResourceHi == RecordHi)) {
        Record->PciCfg.RWAttributes = Resource->PciCfg.RWAttributes | Record->PciCfg.RWAttributes;
        return TRUE;
      } else {
        return FALSE;
      }
    }
    break;
  case MACHINE_SPECIFIC_REG:
    //
    // Special case - merge MSR masks in place.
    // 
    if (Resource->Msr.MsrIndex != Record->Msr.MsrIndex) {
      return FALSE;
    }
    Record->Msr.ReadMask |= Resource->Msr.ReadMask; 
    Record->Msr.WriteMask |= Resource->Msr.WriteMask; 
    return TRUE;
  default:
    return FALSE;
  }
  //
  // If resources are disjoint
  // 
  if ((ResourceHi < RecordLo) || (ResourceLo > RecordHi)) {
    return FALSE;
  }

  //
  // If resource is consumed by record.
  // 
  if ((ResourceLo >= RecordLo) && (ResourceHi <= RecordHi)) {
    return TRUE;
  }
  //
  // Resources are overlapping.
  // Resource and record are merged.
  // 
  ResourceLo = (ResourceLo < RecordLo) ? ResourceLo : RecordLo;
  ResourceHi = (ResourceHi > RecordHi) ? ResourceHi : RecordHi;

  switch (Resource->Header.RscType) {
  case MEM_RANGE:
  case MMIO_RANGE:
    Record->Mem.Base = ResourceLo;
    Record->Mem.Length = ResourceHi - ResourceLo;
    break;
  case IO_RANGE:
  case TRAPPED_IO_RANGE:
    Record->Io.Base = (UINT16) ResourceLo;
    Record->Io.Length = (UINT16) (ResourceHi - ResourceLo);
    break;
  case PCI_CFG_RANGE:
    Record->PciCfg.Base = (UINT16) ResourceLo;
    Record->PciCfg.Length = (UINT16) (ResourceHi - ResourceLo);
    break; 
  default:
    return FALSE;
  }

  return TRUE;
}

/**

  Add resource node.

  @param Resource  A pointer to resource node to be added

**/
VOID
AddSingleResource (
  IN  STM_RSC    *Resource
  )
{
  STM_RSC    *Record;

  Record = (STM_RSC *)mStmResourcesPtr;

  while (TRUE) {
    if (Record->Header.RscType == END_OF_RESOURCES) {
      break;
    }
    //
    // Go to next record if resource and record types don't match.
    //
    if (Resource->Header.RscType != Record->Header.RscType) {
      Record = (STM_RSC *)((UINTN)Record + Record->Header.Length);
      continue;
    }
    //
    // Record is handled inside of procedure - don't adjust.
    //
    if (HandleSingleResource (Resource, Record)) {
      return ;
    }
    Record = (STM_RSC *)((UINTN)Record + Record->Header.Length);
  }

  //
  // Add resource to the end of area.
  //
  CopyMem (
    mStmResourcesPtr + mStmResourceSizeUsed - sizeof(mRscEndNode),
    Resource,
    Resource->Header.Length
    );
  CopyMem (
    mStmResourcesPtr + mStmResourceSizeUsed - sizeof(mRscEndNode) + Resource->Header.Length,
    &mRscEndNode,
    sizeof(mRscEndNode)
    );
  mStmResourceSizeUsed += Resource->Header.Length;
  mStmResourceSizeAvailable = mStmResourceTotalSize - mStmResourceSizeUsed;

  return ;
}

/**

  Add resource list.

  @param ResourceList  A pointer to resource list to be added
  @param NumEntries    Optional number of entries.
                       If 0, list must be terminated by END_OF_RESOURCES.

**/
VOID
AddResource (
  IN  STM_RSC    *ResourceList,
  IN  UINT32      NumEntries OPTIONAL
  )
{
  UINT32      Count;
  UINTN       Index;
  STM_RSC    *Resource;

  if (NumEntries == 0) {
    Count = 0xFFFFFFFF;
  } else {
    Count = NumEntries;
  }

  Resource = ResourceList;
  
  for (Index = 0; Index < Count; Index++) {
    if (Resource->Header.RscType == END_OF_RESOURCES) {
      return ;
    }
    AddSingleResource (Resource);
    Resource = (STM_RSC *)((UINTN)Resource + Resource->Header.Length);
  }
  return ;
}

/**

  Validate resource list.

  @param ResourceList  A pointer to resource list to be added
  @param NumEntries    Optional number of entries.
                       If 0, list must be terminated by END_OF_RESOURCES.

  @retval TRUE  resource valid
  @retval FALSE resource invalid

**/
BOOLEAN
ValidateResource (
  IN  STM_RSC    *ResourceList,
  IN  UINT32      NumEntries OPTIONAL
  )
{
  UINT32      Count;
  UINTN       Index;
  STM_RSC    *Resource;
  UINTN       SubIndex;

  //
  // If NumEntries == 0 make it very big. Scan will be terminated by
  // END_OF_RESOURCES. 
  // 
  if (NumEntries == 0) {
    Count = 0xFFFFFFFF;
  } else {
    Count = NumEntries;
  }

  //
  // Start from beginning of resource list.
  // 
  Resource = ResourceList;
  
  for (Index = 0; Index < Count; Index++) {
    DEBUG ((EFI_D_ERROR, "ValidateResource (%d) - RscType(%x)\n", Index, Resource->Header.RscType));
    //
    // Validate resource. 
    //                  
    switch (Resource->Header.RscType) {
      case END_OF_RESOURCES:
        if (Resource->Header.Length != sizeof (STM_RSC_END)) {
          return  FALSE;
        }
        //
        // If we are passed actual number of resources to add,
        // END_OF_RESOURCES structure between them is considered an
        // error. If NumEntries == 0 END_OF_RESOURCES is a termination. 
        //
        if (NumEntries != 0) {
          return  FALSE;
        } else {
          //
          // If NumEntries == 0 and list reached end - return success.
          // 
          return TRUE;
        }
        break;

      case MEM_RANGE:
      case MMIO_RANGE:
        if (Resource->Header.Length != sizeof (STM_RSC_MEM_DESC)) {
          return FALSE;
        }

        if (Resource->Mem.RWXAttributes > FULL_ACCS) {
          return FALSE;
        }
        break;
        
      case IO_RANGE:
      case TRAPPED_IO_RANGE: 
        if (Resource->Header.Length != sizeof (STM_RSC_IO_DESC)) {
          return FALSE;
        }

        if ((Resource->Io.Base + Resource->Io.Length) > 0xFFFF) {
          return FALSE;
        }
        break;

      case PCI_CFG_RANGE:
        DEBUG ((EFI_D_ERROR, "ValidateResource - PCI (0x%02x, 0x%08x, 0x%02x, 0x%02x)\n", Resource->PciCfg.OriginatingBusNumber, Resource->PciCfg.LastNodeIndex, Resource->PciCfg.PciDevicePath[0].PciDevice, Resource->PciCfg.PciDevicePath[0].PciFunction));
        if (Resource->Header.Length != sizeof (STM_RSC_PCI_CFG_DESC) + (sizeof(STM_PCI_DEVICE_PATH_NODE) * Resource->PciCfg.LastNodeIndex)) {
          return FALSE;
        }
        for (SubIndex = 0; SubIndex <= Resource->PciCfg.LastNodeIndex; SubIndex++) {
          if ((Resource->PciCfg.PciDevicePath[SubIndex].PciDevice > 0x1F) || (Resource->PciCfg.PciDevicePath[SubIndex].PciFunction > 7)) {
            return FALSE;
          }
        }
        if ((Resource->PciCfg.Base + Resource->PciCfg.Length) > 0x1000) {
          return FALSE;
        }
        break;

      case MACHINE_SPECIFIC_REG:
        if (Resource->Header.Length != sizeof (STM_RSC_MSR_DESC)) {
          return FALSE;
        }
        break;
        
      default : 
        DEBUG ((EFI_D_ERROR, "ValidateResource - Unknown RscType(%x)\n", Resource->Header.RscType));
        return FALSE;
    }
    Resource = (STM_RSC *)((UINTN)Resource + Resource->Header.Length);
  }
  return TRUE;
}

/**

  Get resource list.
  EndResource is excluded.

  @param ResourceList  A pointer to resource list to be added
  @param NumEntries    Optional number of entries.
                       If 0, list must be terminated by END_OF_RESOURCES.

  @retval TRUE  resource valid
  @retval FALSE resource invalid

**/
UINTN
GetResourceSize (
  IN  STM_RSC    *ResourceList,
  IN  UINT32      NumEntries OPTIONAL
  )
{
  UINT32      Count;
  UINTN       Index;
  STM_RSC    *Resource;

  Resource = ResourceList;

  //
  // If NumEntries == 0 make it very big. Scan will be terminated by
  // END_OF_RESOURCES. 
  // 
  if (NumEntries == 0) {
    Count = 0xFFFFFFFF;
  } else {
    Count = NumEntries;
  }

  //
  // Start from beginning of resource list.
  // 
  Resource = ResourceList;
  
  for (Index = 0; Index < Count; Index++) {
    if (Resource->Header.RscType == END_OF_RESOURCES) {
      break;
    }
    Resource = (STM_RSC *)((UINTN)Resource + Resource->Header.Length);
  }

  return (UINTN)Resource - (UINTN)ResourceList;
}

/**

  Add resources in list to database. Allocate new memory areas as needed.

  @param ResourceList  A pointer to resource list to be added
  @param NumEntries    Optional number of entries.
                       If 0, list must be terminated by END_OF_RESOURCES.

  @retval EFI_SUCCESS            If resources are added
  @retval EFI_INVALID_PARAMETER  If nested procedure detected resource failer
  @retval EFI_OUT_OF_RESOURCES   If nested procedure returned it and we cannot allocate more areas.

**/
EFI_STATUS
EFIAPI
AddPiResource (
  IN  STM_RSC    *ResourceList,
  IN  UINT32      NumEntries OPTIONAL
  )
{
  EFI_STATUS            Status;
  UINTN                 ResourceSize;
  EFI_PHYSICAL_ADDRESS  NewResource;
  UINTN                 NewResourceSize;

  DEBUG ((EFI_D_INFO, "AddPiResource - Enter\n"));

  if (!ValidateResource (ResourceList, NumEntries)) {
    return EFI_INVALID_PARAMETER;
  }

  ResourceSize = GetResourceSize (ResourceList, NumEntries);
  DEBUG ((EFI_D_INFO, "ResourceSize - 0x%08x\n", ResourceSize));
  if (ResourceSize == 0) {
    return EFI_INVALID_PARAMETER;
  }

  if (mStmResourcesPtr == NULL) {
    //
    // First time allocation
    //
    NewResourceSize = EFI_PAGES_TO_SIZE (EFI_SIZE_TO_PAGES (ResourceSize + sizeof(mRscEndNode)));
    DEBUG ((EFI_D_INFO, "Allocate - 0x%08x\n", NewResourceSize));
    Status = gSmst->SmmAllocatePages (
                      AllocateAnyPages,
                      EfiRuntimeServicesData,
                      EFI_SIZE_TO_PAGES (NewResourceSize),
                      &NewResource
                      );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    //
    // Copy EndResource for intialization
    //
    mStmResourcesPtr = (UINT8 *)(UINTN)NewResource;
    mStmResourceTotalSize = NewResourceSize;
    CopyMem (mStmResourcesPtr, &mRscEndNode, sizeof(mRscEndNode));
    mStmResourceSizeUsed      = sizeof(mRscEndNode);
    mStmResourceSizeAvailable = mStmResourceTotalSize - sizeof(mRscEndNode);

    //
    // Let SmmCore change resource ptr
    //
    NotifyStmResourceChange (mStmResourcesPtr);
  } else if (mStmResourceSizeAvailable < ResourceSize) {
    //
    // Need enlarge
    //
    NewResourceSize = mStmResourceTotalSize + (ResourceSize - mStmResourceSizeAvailable);
    NewResourceSize = EFI_PAGES_TO_SIZE (EFI_SIZE_TO_PAGES (NewResourceSize));
    DEBUG ((EFI_D_INFO, "ReAllocate - 0x%08x\n", NewResourceSize));
    Status = gSmst->SmmAllocatePages (
                      AllocateAnyPages,
                      EfiRuntimeServicesData,
                      EFI_SIZE_TO_PAGES (NewResourceSize),
                      &NewResource
                      );
    if (EFI_ERROR (Status)) {
      return Status;
    }
    CopyMem ((VOID *)(UINTN)NewResource, mStmResourcesPtr, mStmResourceSizeUsed);
    mStmResourceSizeAvailable = NewResourceSize - mStmResourceSizeUsed;

    gSmst->SmmFreePages (
             (EFI_PHYSICAL_ADDRESS)(UINTN)mStmResourcesPtr,
             EFI_SIZE_TO_PAGES (mStmResourceTotalSize)
             );

    mStmResourceTotalSize = NewResourceSize;
    mStmResourcesPtr = (UINT8 *)(UINTN)NewResource;

    //
    // Let SmmCore change resource ptr
    //
    NotifyStmResourceChange (mStmResourcesPtr);
  }

#if 0
  //
  // Copy directly
  //
  CopyMem (
    mStmResourcesPtr + mStmResourceSizeUsed - sizeof(mRscEndNode),
    ResourceList,
    ResourceSize
    );
  CopyMem (
    mStmResourcesPtr + mStmResourceSizeUsed - sizeof(mRscEndNode) + ResourceSize,
    &mRscEndNode,
    sizeof(mRscEndNode)
    );
  mStmResourceSizeUsed += ResourceSize;
  mStmResourceSizeAvailable = mStmResourceTotalSize - mStmResourceSizeUsed;
#else
  //
  // Check duplication
  //
  AddResource (ResourceList, NumEntries);
#endif

  return EFI_SUCCESS;
}

/**

  Delete resources in list to database.

  @param ResourceList  A pointer to resource list to be deleted
                       NULL means delete all resources.
  @param NumEntries    Optional number of entries.
                       If 0, list must be terminated by END_OF_RESOURCES.

  @retval EFI_SUCCESS            If resources are deleted
  @retval EFI_INVALID_PARAMETER  If nested procedure detected resource failer

**/
EFI_STATUS
EFIAPI
DeletePiResource (
  IN  STM_RSC    *ResourceList,
  IN  UINT32      NumEntries OPTIONAL
  )
{
  if (ResourceList != NULL) {
    // TBD
    ASSERT (FALSE);
    return EFI_UNSUPPORTED;
  }
  //
  // Delete all
  //
  CopyMem (mStmResourcesPtr, &mRscEndNode, sizeof(mRscEndNode));
  mStmResourceSizeUsed      = sizeof(mRscEndNode);
  mStmResourceSizeAvailable = mStmResourceTotalSize - sizeof(mRscEndNode);
  return EFI_SUCCESS;
}

/**

  Get BIOS resources.

  @param ResourceList  A pointer to resource list to be filled
  @param ResourceSize  On input it means size of resource list input.
                       On output it means size of resource list filled,
                       or the size of resource list to be filled if size of too small.

  @retval EFI_SUCCESS            If resources are returned.
  @retval EFI_BUFFER_TOO_SMALL   If resource list buffer is too small to hold the whole resources.

**/
EFI_STATUS
EFIAPI
GetPiResource (
  OUT    STM_RSC *ResourceList,
  IN OUT UINT32  *ResourceSize
  )
{
  if (*ResourceSize < mStmResourceSizeUsed) {
    *ResourceSize = (UINT32)mStmResourceSizeUsed;
    return EFI_BUFFER_TOO_SMALL;
  }

  CopyMem (ResourceList, mStmResourcesPtr, mStmResourceSizeUsed);
  *ResourceSize = (UINT32)mStmResourceSizeUsed;
  return EFI_SUCCESS;
}

/**

  Set valid bit for MSEG MSR.

  @param Buffer Ap function buffer. (not used)

**/
VOID
EFIAPI
EnableMsegMsr (
  IN VOID  *Buffer
  )
{
  AsmWriteMsr64 (
    IA32_SMM_MONITOR_CTL_MSR_INDEX,
    AsmReadMsr64(IA32_SMM_MONITOR_CTL_MSR_INDEX) | IA32_SMM_MONITOR_VALID
    );

  return ;
}

/**

  Write MSEG CPU register.

**/
VOID
WriteCpuMsegInfo (
  VOID
  )
{
  UINTN    CpuIndex;

  for (CpuIndex = 0; CpuIndex < gSmmCpuPrivate->SmmCoreEntryContext.NumberOfCpus; CpuIndex++) {
    if (CpuIndex == gSmmCpuPrivate->SmmCoreEntryContext.CurrentlyExecutingCpu) {
      EnableMsegMsr (NULL);
    } else {
      SmmStartupThisAp (
        EnableMsegMsr,
        CpuIndex,
        NULL
        );
    }
  }

  return ;
}

/**

  Get 4K page aligned VMCS size.
  
  @return 4K page aligned VMCS size

**/
UINT32
GetVmcsSize (
  VOID
  )
{
  UINT32 ThisVmcsSize;

  ThisVmcsSize = (UINT32)(RShiftU64 (AsmReadMsr64 (IA32_VMX_BASIC_MSR_INDEX), 32) & 0xFFFF);

  //
  // VMCS require 0x1000 alignment
  //
  ThisVmcsSize = EFI_PAGES_TO_SIZE (EFI_SIZE_TO_PAGES (ThisVmcsSize));

  return ThisVmcsSize;
}

/**

  Create 4G page table for STM.
  4M Non-PAE page table in IA32 version.

  @param PageTableBase        The page table base in MSEG

**/
VOID
StmGen4GPageTableIa32 (
  IN UINTN              PageTableBase
  )
{
  UINTN                             Index;
  UINT32                            *Pte;
  UINT32                            Address;

  Pte = (UINT32*)(UINTN)PageTableBase;

  Address = 0;
  for (Index = 0; Index < SIZE_4KB / sizeof (*Pte); Index++) {
    *Pte = Address | IA32_PG_PS | IA32_PG_RW | IA32_PG_P;
    Pte++;
    Address += SIZE_4MB;
  }

  return ;
}

/**

  Create 4G page table for STM.
  2M PAE page table in X64 version.

  @param PageTableBase        The page table base in MSEG

**/
VOID
StmGen4GPageTableX64 (
  IN UINTN              PageTableBase
  )
{
  UINTN                             Index;
  UINTN                             SubIndex;
  UINT64                            *Pde;
  UINT64                            *Pte;
  UINT64                            *Pml4;

  Pml4 = (UINT64*)(UINTN)PageTableBase;
  PageTableBase += SIZE_4KB;
  *Pml4 = PageTableBase | IA32_PG_RW | IA32_PG_P;

  Pde = (UINT64*)(UINTN)PageTableBase;
  PageTableBase += SIZE_4KB;
  Pte = (UINT64 *)(UINTN)PageTableBase;

  for (Index = 0; Index < 4; Index++) {
    *Pde = PageTableBase | IA32_PG_RW | IA32_PG_P;
    Pde++;
    PageTableBase += SIZE_4KB;

    for (SubIndex = 0; SubIndex < SIZE_4KB / sizeof (*Pte); SubIndex++) {
      *Pte = (((Index << 9) + SubIndex) << 21) | IA32_PG_PS | IA32_PG_RW | IA32_PG_P;
      Pte++;
    }
  }

  return ;
}

/**

  Check STM image size.

  @param StmImage      STM image
  @param StmImageSize  STM image size

  @retval TRUE  check pass
  @retval FALSE check fail
**/
BOOLEAN
StmCheckStmImage (
  IN EFI_PHYSICAL_ADDRESS StmImage,
  IN UINTN                StmImageSize
  )
{
  UINTN                     MinMsegSize;
  STM_HEADER                *StmHeader;

  StmHeader = (STM_HEADER *)(UINTN)StmImage;

  //
  // Get Minimal required Mseg size
  //
  MinMsegSize = (EFI_PAGES_TO_SIZE (EFI_SIZE_TO_PAGES (StmHeader->SwStmHdr.StaticImageSize)) +
                 StmHeader->SwStmHdr.AdditionalDynamicMemorySize +
                 (StmHeader->SwStmHdr.PerProcDynamicMemorySize + GetVmcsSize () * 2) * gSmmCpuPrivate->SmmCoreEntryContext.NumberOfCpus);
  if (MinMsegSize < StmImageSize) {
    MinMsegSize = StmImageSize;
  }

  if (StmHeader->HwStmHdr.Cr3Offset >= StmHeader->SwStmHdr.StaticImageSize) {
    //
    // We will create page table, just in case that SINIT does not create it.
    //
    if (MinMsegSize < StmHeader->HwStmHdr.Cr3Offset + EFI_PAGES_TO_SIZE(6)) {
      MinMsegSize = StmHeader->HwStmHdr.Cr3Offset + EFI_PAGES_TO_SIZE(6);
    }
  }

  //
  // Check if it exceeds MSEG size
  //
  if (MinMsegSize > PcdGet32 (PcdCpuMsegSize)) {
    return FALSE;
  }

  return TRUE;
}

/**

  Load STM image to MSEG.

  @param StmImage      STM image
  @param StmImageSize  STM image size

**/
VOID
StmLoadStmImage (
  IN EFI_PHYSICAL_ADDRESS StmImage,
  IN UINTN                StmImageSize
  )
{
  UINT32                    MsegBase;
  STM_HEADER                *StmHeader;

  StmHeader = (STM_HEADER *)(UINTN)StmImage;

  MsegBase = (UINT32)AsmReadMsr64(IA32_SMM_MONITOR_CTL_MSR_INDEX) & 0xFFFFF000;

  ZeroMem ((VOID *)(UINTN)MsegBase, (UINTN)PcdGet32 (PcdCpuMsegSize));
  CopyMem ((VOID *)(UINTN)MsegBase, (VOID *)(UINTN)StmImage, StmImageSize);

  if (sizeof(UINTN) == sizeof(UINT64)) {
    StmGen4GPageTableX64 ((UINTN)MsegBase + StmHeader->HwStmHdr.Cr3Offset);
  } else {
    StmGen4GPageTableIa32 ((UINTN)MsegBase + StmHeader->HwStmHdr.Cr3Offset);
  }

  // BUGBUG: SNB can not write 0x9B twice
//  WriteCpuMsegInfo ();
}

/**

  Load STM image to MSEG.

  @param StmImage      STM image
  @param StmImageSize  STM image size

  @retval EFI_SUCCESS            Load STM to MSEG successfully
  @retval EFI_ALREADY_STARTED    STM image is already loaded to MSEG
  @retval EFI_BUFFER_TOO_SMALL   MSEG is smaller than minimal requirement of STM image
  @retval EFI_UNSUPPORTED        MSEG is not enabled

**/
EFI_STATUS
EFIAPI
LoadMonitor (
  IN EFI_PHYSICAL_ADDRESS StmImage,
  IN UINTN                StmImageSize
  )
{
  if (mLockLoadMonitor) {
    return EFI_ACCESS_DENIED;
  }
  if ((AsmReadMsr64 (IA32_SMM_MONITOR_CTL_MSR_INDEX) & 0xFFFFF000) == 0) {
    return EFI_UNSUPPORTED;
  }
  if ((AsmReadMsr64 (IA32_SMM_MONITOR_CTL_MSR_INDEX) & IA32_SMM_MONITOR_VALID) != 0) {
    // BUGBUG: SNB can not write 0x9B twice
//    return EFI_ALREADY_STARTED;
  }

  if (!StmCheckStmImage (StmImage, StmImageSize)) {
    return EFI_BUFFER_TOO_SMALL;
  }

  // Record STM_HASH to PCR 0, just in case it is NOT TXT launch, we still need provide the evidence.
  TpmMeasureAndLogData(
    0,                        // PcrIndex
    TXT_EVTYPE_STM_HASH,      // EventType
    NULL,                     // EventLog
    0,                        // LogLen
    (VOID *)(UINTN)StmImage,  // HashData
    StmImageSize              // HashDataLen
    );

  StmLoadStmImage (StmImage, StmImageSize);

  mStmState |= EFI_SM_MONITOR_STATE_ENABLED;

  return EFI_SUCCESS;
}

/**
  This function return BIOS STM resource.
  Produced by SmmStm.
  Comsumed by SmmMpService when Init.

  @return BIOS STM resource

**/
VOID *
GetStmResource(
  VOID
  )
{
  return mStmResourcesPtr;
}

/**
  This function notify STM resource change.

  @param StmResource BIOS STM resource

**/
VOID
NotifyStmResourceChange (
  VOID *StmResource
  )
{
  UINTN                          Index;
  TXT_PROCESSOR_SMM_DESCRIPTOR       *Psd;

  for (Index = 0; Index < gSmmCpuPrivate->SmmCoreEntryContext.NumberOfCpus; Index++) {
    Psd = (TXT_PROCESSOR_SMM_DESCRIPTOR*)(VOID*)(UINTN)(mCpuHotPlugData.SmBase[Index] + SMM_PSD_OFFSET);
    Psd->BiosHwResourceRequirementsPtr = (UINT64)(UINTN)StmResource;
  }
  return ;
}

/**
  This is STM setup BIOS callback.
**/
VOID
SmmStmSetup (
  VOID
  )
{
  if (FeaturePcdGet (PcdCpuStmSupport)) {
    mStmState |= EFI_SM_MONITOR_STATE_ACTIVATED;
  }
}

/**
  This is STM teardown BIOS callback.
**/
VOID
SmmStmTeardown (
  VOID
  )
{
  if (FeaturePcdGet (PcdCpuStmSupport)) {
    mStmState &= ~EFI_SM_MONITOR_STATE_ACTIVATED;
  }
}

/**
  This is SMM exception handle.
  Consumed by STM when exception happen.

  @param Context  STM protection exception stack frame

  @return the EBX value for STM reference.
          EBX = 0: resume SMM guest using register state found on exception stack.
          EBX = 1 to 0x0F: EBX contains a BIOS error code which the STM must record in the
                           TXT.ERRORCODE register and subsequently reset the system via
                           TXT.CMD.SYS_RESET. The value of the TXT.ERRORCODE register is calculated as
                           follows: TXT.ERRORCODE = (EBX & 0x0F) | STM_CRASH_BIOS_PANIC
          EBX = 0x10 to 0xFFFFFFFF - reserved, do not use.

**/
UINT32
SmmStmExceptionHandler (
  IN OUT STM_PROTECTION_EXCEPTION_STACK_FRAME Context
  )
{
  if (FeaturePcdGet (PcdCpuStmSupport)) {
    // TBD - SmmStmExceptionHandler, record information
    DEBUG ((EFI_D_ERROR, "SmmStmExceptionHandler ...\n"));
    //
    // Skip this instruction and continue;
    //
    if (sizeof(UINTN) == sizeof(UINT64)) {
      Context.X64StackFrame->Rip += Context.X64StackFrame->VmcsExitInstructionLength;
    } else {
      Context.Ia32StackFrame->Rip += Context.Ia32StackFrame->VmcsExitInstructionLength;
    }
  }

  return 0;
}
