/** @file

STM PE Header file

This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _PESTM_H_
#define _PESTM_H_

// VM/PE PeSmiControl.PeSmiState state definitions

#define PESMINULL  0      // nothing happening
#define PESMIPSMI  1      // SMI sent by VM/PE startup to get cpu state
#define PESMIHSMI  2      // normal SMI processing
#define PESMIPNMI  3      // VM/PE needs an NMI sent for it to help process the host SMI
#define PESMIHTMR  4      // smi handler has detected an SMI timer
#define PESMIPNMI2 5      // NMI has been sent to the VM/PE, now waiting for its entry

#define OFFSET_BITMASK_IA32_4K    0x00000FFF
#define OFFSET_BITMASK_IA32E_4K   0x0000000000000FFF
#define PAGE_SIZE_4K              4096ULL

typedef struct
{
	UINT64 Address;
	UINT64 Size;
} PE_REGION_LIST;

typedef struct
{
	UINT64 ModuleAddress;		// physical address of module to be loaded into PE/VM
	UINT64 ModuleLoadAddress;	// guest physical address to load module in a PE/VM 
	UINT32 ModuleSize;			// size of module in bytes
	UINT32 ModuleEntryPoint;	// entry point - relative offset to ModuleLoadAddress
	UINT64 AddressSpaceStart;	// start of guest physical address space (page aligned)
	UINT32 AddressSpaceSize;	// size of guest physical address space
	UINT32 VmConfig;			// Options to the configuration of the PE/VM
	UINT64 Cr3Load;             // CR3
	UINT64 SharedPage;			// writeable pages for sharing between the PE Module and kernel space
								// can be multible pages and is located in mail memory
	PE_REGION_LIST *Segment;    // list of read only regions (contained within a page)	
   	UINT32 SharedPageSize;		// size of]SharedPage/region
    UINT32 DoNotClearSize;      // area at beginning of memory not to be cleared
    UINT64 ModuleDataSection;   // Location of Module Data Section for VM/PE

    // data areas local to the STM go after this point

    UINT64 SharedStmPage;       // page shared between PE/VM and the STM
    UINT64 RunCount;            // count of runs starting with one (1)
   // UINTN DataRegionStart;     // data space after text region
    UINTN DataRegionSize;      // data space size
    UINTN FrontDataRegionSize; // data space size before text region
    UINTN DataRegionSmmLoc;    // start location in SMM for data region
} PE_MODULE_INFO;

// options for VmConfig only for Perm VM

#define PERM_VM_CRASH_BREAKDOWN (1<<21)  // if VM/PE crashes then breakdown
#define PERM_VM_RUN_ONCE        (1<<20)  // run once and delete
#define PERM_VM_ALLOW_TERM      (1<<19)
#define PERM_VM_RUN_PERIODIC    (1<<22)  // run using SMI Timer
#define PERM_VM_CLEAR_MEMORY    (1<<23)  // clear HEAP before run
#define PERM_VM_SET_TEXT_RW     (1<<24)  // set the text area as RW ow W
#define PERM_VM_EXEC_HEAP       (1<<25)  // Allow Heap Execution
#define PERM_VM_INJECT_INT      (1<<26)  // VM/PE will handle Internal Interrupts

typedef  struct __PE_SMI_CONTROL
{
	SPIN_LOCK PeSmiControlLock;
	UINT32 PeNmiBreak;    // when 1, a NMI has been sent to break the thread in PE_APIC_id
	UINT32 PeApicId;    // APIC id of the thread that is executing the PE V<
	UINT32 PeExec;         // when 1 PE_APIC_ID is executing a
	UINT32 PeSmiState;    // SMI is sent to get processor state
	UINT32 PeWaitTimer;   // if non-zero - waiting for timer and length of timeout
    INT32 PeCpuIndex;     // CpuIndex of PeVm
} PE_SMI_CONTROL;

typedef struct _VMX_GUEST_VMCS_STRUCT
{
	UINTN	GdtrBase;
	UINTN	Rsp;
	UINTN	Rip;
	UINTN	Rflags;
	UINTN	Cr0;
	UINTN	Cr3;
	UINTN	Cr4;
	UINTN	Dr7;

	UINT16	CsSelector;
	UINT16	DsSelector;
	UINT16	EsSelector;
	UINT16	FsSelector;
	UINT16	GsSelector;
	UINT16	SsSelector;
	UINT16	TrSelector;

	UINT32	InterruptibilityState;
	UINT32	Smbase;
	UINT32	ActivityState;

	UINT64	DebugCtlFull;
	UINT64	VmcsLinkPointerFull;
	UINT64	IA32_Efer;

	UINT32	GdtrLimit;
	UINT32	LdtrAccessRights;

	UINTN	CsBase;
	UINT32	CsAccessRights;
	UINT32	CsLimit;

	UINTN	DsBase;
	UINT32	DsAccessRights;
	UINT32	DsLimit;

	UINTN	EsBase;
	UINT32	EsAccessRights;
	UINT32	EsLimit;

	UINTN	FsBase;
	UINT32	FsAccessRights;
	UINT32	FsLimit;

	UINTN	GsBase;
	UINT32	GsAccessRights;
	UINT32	GsLimit;

	UINTN	SsBase;
	UINT32	SsAccessRights;
	UINT32	SsLimit;

	UINTN	TrBase;
	UINT32	TrAccessRights;
	UINT32	TrLimit;

} VMX_GUEST_VMCS_STRUCT;

// Guest VM Types

#define SMI_HANDLER 0
#define PE_PERM     1
#define PE_TEMP     2
#define PE_OTHER    3
#define NUM_PE_TYPE 4

typedef struct _PE_GUEST_CONTEXT_PER_CPU {
  X86_REGISTER                        Register;
  //IA32_DESCRIPTOR                     Gdtr;
  //IA32_DESCRIPTOR                     Idtr;
  //UINTN                               Cr0;
  //UINTN                               Cr3;
  //UINTN                               Cr4;
  UINTN								  Rip;
  UINTN								  Rsp;
  UINTN								  Rflags;
  //UINTN                               Stack;
  //UINT64                              Efer;
  //BOOLEAN                             UnrestrictedGuest;
  //UINTN                               XStateBuffer;
  GUEST_INTERRUPTIBILITY_STATE	      InterruptibilityState;
  UINT32							  ActivityState;
  UINT64							  VmcsLinkPointerFull;
  UINT32						      VmcsLinkPointerHigh;

  VM_EXIT_CONTROLS                             VmExitCtrls;
  VM_ENTRY_CONTROLS                            VmEntryCtrls;

  // For CPU support Save State in MSR, we need a place holder to save it in memory in advanced.
  // The reason is that when we switch to SMM guest, we lose the context in SMI guest.
  //STM_SMM_CPU_STATE                   *SmmCpuState;

  //VM_EXIT_INFO_BASIC                  InfoBasic; // hold info since we need that when return to SMI guest.
  //VM_EXIT_QUALIFICATION               Qualification; // hold info since we need that when return to SMI guest.
  //UINT32                              VmExitInstructionLength;
  //BOOLEAN                             Launched;
  //BOOLEAN                             Actived; // For SMM VMCS only, controlled by StartStmVMCALL
  //UINT64                              Vmcs;
  //UINT32                              GuestMsrEntryCount;
  //UINT64                              GuestMsrEntryAddress;

#if defined (MDE_CPU_X64)
  // Need check alignment here because we need use FXSAVE/FXRESTORE buffer
  UINT32                              Reserved;
#endif

  // Stuff we reinitialize upon every restart

   UINT16 CsSelector;
   UINTN  CsBase;
   UINT32 CsLimit;
   UINT32 CsAccessRights;      // defined by user input

   UINT16 DsSelector;
   UINTN  DsBase;
   UINT32 DsLimit;
   UINT32 DsAccessRights; 

   UINT16 EsSelector;
   UINTN  EsBase;
   UINT32 EsLimit;
   UINT32 EsAccessRights; 

   UINT16 FsSelector;
   UINTN  FsBase;
   UINT32 FsLimit;
   UINT32 FsAccessRights; 

   UINT16 GsSelector;
   UINTN  GsBase;
   UINT32 GsLimit;
   UINT32 GsAccessRights; 

   UINT16 SsSelector;
   UINTN  SsBase;
   UINT32 SsLimit;
   UINT32 SsAccessRights; 

   UINT16 TrSelector;
   UINTN  TrBase;
   UINT32 TrLimit;

   UINT32 TrAccessRights; 

   UINT16 LdtrSelector;
   UINTN  LdtrBase;
   UINT32 LdtrLimit;
   UINT32 LdtrAccessRights; 

   UINTN  GdtrBase;
   UINT32 GdtrLimit;

   UINTN IdtrBase;
   UINT32 IdtrLimit;

} PE_GUEST_CONTEXT_PER_CPU;


typedef struct _PE_GUEST_CONTEXT_COMMON {
  //EPT_POINTER                         EptPointer;
  //UINTN                               CompatiblePageTable;
  //UINTN                               CompatiblePaePageTable;
  //UINT64                              MsrBitmap;
  //UINT64                              IoBitmapA;
  //UINT64                              IoBitmapB;
  //UINT32                              Vmid;
  //UINTN                               ZeroXStateBuffer;
  //
  // BiosHwResourceRequirementsPtr: This is back up of BIOS resource - no ResourceListContinuation
  //
  //UINT64                              BiosHwResourceRequirementsPtr;
  PE_GUEST_CONTEXT_PER_CPU           GuestContextPerCpu;   // for PE we need only one
} PE_GUEST_CONTEXT_COMMON;

typedef struct _PE_HOST_CONTEXT_COMMON {
  SPIN_LOCK                           DebugLock;
  SPIN_LOCK                           MemoryLock;
  SPIN_LOCK                           SmiVmcallLock;
  UINT32                              CpuNum;
  UINT32                              JoinedCpuNum;
  UINTN                               PageTable;
  IA32_DESCRIPTOR                     Gdtr;
  IA32_DESCRIPTOR                     Idtr;
  UINT64                              HeapBottom;
  UINT64                              HeapTop;
  UINT8                               PhysicalAddressBits;
  //
  // BUGBUG: Assume only one segment for client system.
  //
  UINT64                              PciExpressBaseAddress;
  UINT64                              PciExpressLength;

  UINT64                              VmcsDatabase;
  UINT32                              TotalNumberProcessors;
  STM_HEADER                          *StmHeader;
  UINTN                               StmSize;
  UINT64                              TsegBase;
  UINT64                              TsegLength;

  //
  // Log
  //
  MLE_EVENT_LOG_STRUCTURE             EventLog;

  //
  // ProtectedResource: This is back up of MLE resource - no ResourceListContinuation
  //
  MLE_PROTECTED_RESOURCE_STRUCTURE    MleProtectedResource;
  //
  // ProtectedTrappedIoResource: This is cache for TrappedIoResource in MLE resource
  // For performance consideration only, because TrappedIoResource will be referred in each SMI.
  //
  MLE_PROTECTED_RESOURCE_STRUCTURE    MleProtectedTrappedIoResource;

  //
  // Performance measurement
  //
  STM_PERF_DATA                       PerfData;

  STM_HOST_CONTEXT_PER_CPU            HostContextPerCpu;
} PE_HOST_CONTEXT_COMMON;

#define PEVM_START_VMCALL 1
#define PEVM_START_SMI    2
#define PEVM_PRESTART_SMI 3

#define PEVM_INIT_16bit   1
#define PEVM_INIT_32bit   2
#define PEVM_INIT_64bit   3

typedef struct
{
	PE_MODULE_INFO	UserModule;
	UINT32          StartMode;  // either SMI or VMCALL
	UINT32			PeVmState;
	UINT32			PeCpuInitMode;  // VM/PE initial processor start mode
	UINTN *		    SmmBuffer;
	UINTN			SmmBufferSize;
	UINTN *			SharedPageStm;
	PE_HOST_CONTEXT_COMMON    HostState;
	PE_GUEST_CONTEXT_COMMON   GuestState;
} PE_VM_DATA;

typedef struct HEAP_HEADER
{
    UINT64        BlockLength;
    struct HEAP_HEADER*  NextBlock;
}HEAP_HEADER;

typedef struct ROOT_VMX_STATE {
	UINT64 valid;      // used by STM
	UINT64 VmcsType;   // 1 - guest-VM being serviced by VMM
					   // 2 - no current-VM active
					   // 3 - guest-VM	
    UINT64 Vmxon;      // vmxon pointer - loaded at STM startup, should never change
	UINT64 ExecutiveVMCS;
	UINT64 LinkVMCS;
	UINT64 HostRootVMCS;

    UINT64 RootHostCR0;
    UINT64 RootHostCR3;
    UINT64 RootHostCR4;
    UINT64 RootHostGDTRBase;
    UINT64 RootHostIDTRBase;
    UINT64 RootHostRSP;
    UINT64 RootHostRIP;
	UINT64 RootHostEPT;   //read from memory

    UINT64 RootGuestCR0;
    UINT64 RootGuestCR3;
    UINT64 RootGuestCR4;
    UINT64 RootGuestGDTRBase;
	UINT64 RootGuestGDTRLimit;
    UINT64 RootGuestIDTRBase;
	UINT64 RootGuestIDTRLimit;
    UINT64 RootGuestRSP;
    UINT64 RootGuestRIP;
	UINT64 RootContEPT;  // read from guest structure

    UINT32 VmxState;  // either root VMX or guest VMX
    UINT32 Padding;
} ROOT_VMX_STATE;

#define VMX_STATE_ROOT   1
#define VMX_STATE_GUEST  2

void GetRootVmxState(UINT32 CpuIndex, ROOT_VMX_STATE * RootState);

#else

#endif