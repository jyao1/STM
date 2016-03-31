/** @file
  SMM runtime header file

  Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _STM_RUNTIME_H_
#define _STM_RUNTIME_H_

#include "Stm.h"

/**

  This is STM VmExit handler.

  @param Index CPU index

**/
typedef
VOID
(* STM_HANDLER) (
  IN UINT32  Index
  );

/**

  This is STM VMCALL handler.

  @param Index             CPU index
  @param AddressParameter  Addresss parameter

  @return VMCALL status

**/
typedef
STM_STATUS
(* STM_VMCALL_HANDLER) (
  IN UINT32  Index,
  IN UINT64  AddressParameter
  );

typedef struct {
  UINT32                 FuncIndex;
  STM_VMCALL_HANDLER     StmVmcallHandler;
} STM_VMCALL_HANDLER_STRUCT;

/**

  This function is unknown handler for SMI.

  @param Index CPU index

**/
VOID
UnknownHandlerSmi (
  IN UINT32 Index
  );

/**

  This function is SMI event handler for SMI.

  @param Index CPU index

**/
VOID
SmiEventHandler (
  IN UINT32 Index
  );

/**

  This function is VMCALL handler for SMI.

  @param Index CPU index

**/
VOID
SmiVmcallHandler (
  IN UINT32 Index
  );

/**

  This function is unknown handler for SMM.

  @param Index CPU index

**/
VOID
UnknownHandlerSmm (
  IN UINT32 Index
  );

/**

  This function is RSM handler for SMM.

  @param Index CPU index

**/
VOID
RsmHandler (
  IN UINT32 Index
  );

/**

  This function is VMCALL handler for SMM.

  @param Index CPU index

**/
VOID
SmmVmcallHandler (
  IN UINT32 Index
  );

/**

  This function is exception handler for SMM.

  @param Index CPU index

**/
VOID
SmmExceptionHandler (
  IN UINT32 Index
  );

/**

  This function is IO instruction handler for SMM.

  @param Index CPU index

**/
VOID
SmmIoHandler (
  IN UINT32 Index
  );

/**

  This function is INVD handler for SMM.

  @param Index CPU index

**/
VOID
SmmInvdHandler (
  IN UINT32 Index
  );

/**

  This function is WBINVD handler for SMM.

  @param Index CPU index

**/
VOID
SmmWbinvdHandler (
  IN UINT32 Index
  );

/**

  This function is TaskSwitch handler for SMM.

  @param Index CPU index

**/
VOID
SmmTaskSwitchHandler (
  IN UINT32 Index
  );

/**

  This function is CPUID handler for SMM.

  @param Index CPU index

**/
VOID
SmmCpuidHandler (
  IN UINT32 Index
  );

/**

  This function is CR access handler for SMM.

  @param Index CPU index

**/
VOID
SmmCrHandler (
  IN UINT32 Index
  );

/**

  This function is EPT violation handler for SMM.

  @param Index CPU index

**/
VOID
SmmEPTViolationHandler (
  IN UINT32 Index
  );

/**

  This function is EPT misconfiguration handler for SMM.

  @param Index CPU index

**/
VOID
SmmEPTMisconfigurationHandler (
  IN UINT32  Index
  );

/**

  This function is INVEPT handler for SMM.

  @param Index CPU index

**/
VOID
SmmInvEPTHandler (
  IN UINT32  Index
  );

/**

  This function is RDMSR handler for SMM.

  @param Index CPU index

**/
VOID
SmmReadMsrHandler (
  IN UINT32 Index
  );

/**

  This function is WRMSR handler for SMM.

  @param Index CPU index

**/
VOID
SmmWriteMsrHandler (
  IN UINT32 Index
  );

/**

  This function return CPU index according to APICID.

  @param ApicId APIC ID

  @return CPU index

**/
UINT32
ApicToIndex (
  IN UINT32  ApicId
  );

/**

  This function wait all processor rendez-vous.

  @param CurrentIndex Current CPU index

**/
VOID
WaitAllProcessorRendezVous (
  IN UINT32   CurrentIndex
  );

/**

  This function write SMM state save area accroding to VMCS.

  @param Index CPU index

**/
VOID
WriteSyncSmmStateSaveArea (
  IN UINT32 Index
  );

/**

  This function read SMM state save area sync to VMCS.

  @param Index CPU index

**/
VOID
ReadSyncSmmStateSaveArea (
  IN UINT32 Index
  );

/**

  This function resume to BIOS exception handler.

  @param Index CPU index

**/
VOID
ResumeToBiosExceptionHandler (
  IN UINT32  Index
  );

/**

  This function return from BIOS exception handler.

  @param Index CPU index

**/
VOID
ReturnFromBiosExceptionHandler (
  IN UINT32  Index
  );

/**

  This function teardown STM.

  @param Index CPU index

**/
VOID
StmTeardown (
  IN UINT32  Index
  );

/**

  This function run BIOS SMM provided SetupRip.

  @param Index CPU index

**/
VOID
SmmSetup (
  IN UINT32  Index
  );

/**

  This function run BIOS SMM provided TeardownRip.

  @param Index CPU index

**/
VOID
SmmTeardown (
  IN UINT32  Index
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

/**

  This function delete resource list to protected resource structure.

  @param ProtectedResource Protected resource structure
  @param Resource          Resource list to be deleted

**/
VOID
DeleteProtectedResource (
  IN MLE_PROTECTED_RESOURCE_STRUCTURE  *ProtectedResource,
  IN STM_RSC                           *Resource
  );

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
  );

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
  );

/**

  This function register resource list to VMCS.

  @param Resource          Resource list to be registered

**/
VOID
RegisterProtectedResource (
  IN STM_RSC   *Resource
  );

/**

  This function unregister resource list to VMCS.

  @param Resource          Resource list to be unregistered

**/
VOID
UnRegisterProtectedResource (
  IN STM_RSC   *Resource
  );

/**

  This function register BIOS resource list to VMCS.

  @param Resource          Resource list to be registered

**/
VOID
RegisterBiosResource (
  IN STM_RSC   *Resource
  );

/**

  This function translate guest linear address to guest physical address.

  @param CpuIndex           CPU index
  @param GuestLinearAddress Guest linear address

  @return Guest physical address
**/
UINT64
GuestLinearToGuestPhysical (
  IN UINT32  CpuIndex,
  IN UINTN   GuestLinearAddress
  );

/**

  This function lookup SMI guest virtual address to guest physical address.

  @param SmiGuestCr3         SmiGuest Cr3
  @param SmiGuestCr4Pae      SmiGuest Cr4Pae
  @param SmiGuestCr4Pse      SmiGuest Cr4Pse
  @param SmiGuestIa32e       SmiGuest Ia32e
  @param GuestVirtualAddress Guest virtual address

  @return Guest physical address
**/
UINT64
LookupSmiGuestVirtualToGuestPhysical (
  IN UINTN   SmiGuestCr3,
  IN BOOLEAN SmiGuestCr4Pae,
  IN BOOLEAN SmiGuestCr4Pse,
  IN BOOLEAN SmiGuestIa32e,
  IN UINTN   GuestVirtualAddress
  );

/**

  This function translate guest physical address to host address.

  @param EptPointer           EPT pointer
  @param GuestPhysicalAddress Guest physical address
  @param HostPhysicalAddress  Host physical address

  @retval TRUE  HostPhysicalAddress is found
  @retval FALSE HostPhysicalAddress is not found
**/
BOOLEAN
LookupSmiGuestPhysicalToHostPhysical (
  IN  UINT64  EptPointer,
  IN  UINTN   GuestPhysicalAddress,
  OUT UINTN   *HostPhysicalAddress
  );

/**

  This function map virtual address to physical address.

  @param CpuIndex           CPU index
  @param PhysicalAddress    Guest physical address
  @param VirtualAddress     Guest virtual address
  @param PageCount          Guest page count

**/
VOID
MapVirtualAddressToPhysicalAddress (
  IN UINT32   CpuIndex,
  IN UINT64   PhysicalAddress,
  IN UINTN    VirtualAddress,
  IN UINTN    PageCount
  );

/**

  This function unmap virtual address.

  @param CpuIndex           CPU index
  @param VirtualAddress     Guest virtual address
  @param PageCount          Guest page count

**/
VOID
UnmapVirtualAddressToPhysicalAddress (
  IN UINT32   CpuIndex,
  IN UINTN    VirtualAddress,
  IN UINTN    PageCount
  );

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
  );

/**

  This functin duplicate STM resource list.

  @param Resource STM resource list to be duplicated

  @return STM resource list duplicated

**/
STM_RSC *
DuplicateResource (
  IN STM_RSC   *Resource
  );

/**

  This functin duplicate STM resource list.
  This function will allocate page one by one, to duplicate resource

  @param Resource STM resource list to be duplicated

  @return STM resource list duplicated

**/
STM_RSC *
RawDuplicateResource (
  IN STM_RSC   *Resource
  );

/**

  This function will free page one by one for each resource node.

  @param Resource STM resource list to be freed

**/
VOID
RawFreeResource (
  IN STM_RSC   *Resource
  );

/**

  This function write SMM state save SSE2 accroding to VMCS.

  @param Index    CPU index
  @param Scrub    Nee Scrub

**/
VOID
WriteSyncSmmStateSaveAreaSse2 (
  IN UINT32                             Index,
  IN BOOLEAN                            Scrub
  );

/**

  This function read SMM state save SSE2 sync to VMCS.

  @param Index CPU index

**/
VOID
ReadSyncSmmStateSaveAreaSse2 (
  IN UINT32                             Index
  );

/**

  This function validate input address region.
  Address region is not allowed to overlap with MSEG.
  Address region is not allowed to exceed STM accessable region.

  @param Address              Address to be validated
  @param Length               Address length to be validated
  @param FromProtectedDomain  If this request is from protected domain
                              TRUE means this address region is allowed to overlap with MLE protected region.
                              FALSE means this address region is not allowed to overlap with MLE protected region.

  @retval TRUE  Validation pass
  @retval FALSE Validation fail

**/
BOOLEAN
IsGuestAddressValid (
  IN UINTN    Address,
  IN UINTN    Length,
  IN BOOLEAN  FromProtectedDomain
  );

/**

  This function checks Pending Mtf before resume.

  @param Index CPU index

**/
VOID
CheckPendingMtf (
  IN UINT32  Index
  );

/**

  This function issue TXT reset.

  @param ErrorCode            TXT reset error code

**/
VOID
StmTxtReset (
  IN UINT32  ErrorCode
  );

#endif
