/** @file

Structures to pass information about the protected execution module

This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef STMLOADVM_H
#define STMLOADVM_H

// Bit settings for VM_Configuration

// Bit settings for PE Vm setup

#define CS_L     (1 << 13)      // CS.L 64 bit mode active when set to 1
#define CS_D     (1 << 14)      // CS.D default operation (0 = 16 bit segment; 1= 32 bit segment)
                              // (NOTE: must be zero when CS.L=1 or VMX will not allow start)
#define SET_IA32E  (1 << 15)     // sets IA-32E mode upon entry

//#define SET_CR0_PE (1 << 0)
//#define SET_CR0_PG (1 << 31)    // sets the paging bit
#define SET_CR0_WP (1 << 16)
#define SET_CR0_NE (1 << 5)

#define SET_CR4_PSE (1 << 4)
#define SET_CR4_PAE (1 << 3)    // need to alias

#define PE_VM_EXCEPTION_HANDLING  (1 << 6)   // PE VM will handle execeptions - O.W. vmexit

// Return Codes

#define PE_SUCCESS                      0   //   PE setup/ran sucessfullu
#define PE_FAIL                         -1   //   catchall PE ERROR
#define PE_SPACE_TOO_LARGE              0x80040001   // requested memory space too large
#define PE_MODULE_ADDRESS_TOO_LOW       0x80040002   // module start below address space start
#define PE_MODULE_TOO_LARGE             0x80040003   // PE module too large for address space (or located such that
                                            // it overflows the end of the address space
#define PE_NO_PT_SPACE                  0x80040004   // not enough space left for PE VM page tables
#define PE_NO_RL_SPACE                  0x80040005   // not enough space left for resource list
#define PE_MEMORY_AC_SETUP_FAILURE      0x80040006   // could not setup accesses to PE space (internal error)
#define PE_SHARED_MEMORY_SETUP_ERROR    0x80040007   // could not setup shared memory 
#define PE_MODULE_MAP_FAILURE           0x80040008   // could not map in the address space
#define PE_SHARED_MAP_FAILURE           0x80040009   // could not map in the shared page
#define PE_VMCS_ALLOC_FAIL              0x8004000A  // could not allocate VMCS memory
#define PE_VMLAUNCH_ERROR               0x8004000B  // attempted to launch PE VM with bad guest state
#define PE_VM_BAD_ACCESS                0x8004000C  // PE VM attempted to access protected memory (out of bounds)
#define PE_VM_SETUP_ERROR_D_L           0x8004000D  // CS_D and CS_L cannot be set to one at the same time
#define PE_VM_SETUP_ERROR_IA32E_D       0x8004000E  // SET_IA32E must be set when CS_L is set
#define PE_VM_TRIPLE_FAULT              0x8004000F  // PE VM crashed with a triple fault
#define PE_VM_PAGE_FAULT                0x80040010  // PE VM crashed with a page fault
#define PE_VM_GP_FAULT					0x80040024  // PE VM crashed with a GP fault

#define PE_VM_ATTEMPTED_VMCALL          0x80040011  // PE VM attempted VMCALL
#define PE_VM_NMI                       0x80040012  // PE VM encountered NMI
#define PE_VM_CPUID                     0x80040013  // PE VM encountered CPUID
#define PE_VM_IO                        0x80040014  // PE VM attempted I/O
#define PE_VM_UNEXPECTED_VMEXIT         0x80040015  // PE VM attempted an unexpected VMEXIT (catchall)
#define PE_VM_READ_MSR                  0x80040016  // PE VM attempted to read an MSR
#define PE_VM_WRITE_MSR                 0x80040017  // PE VM attempted to write an MSR
#define PE_REGION_LIST_SETUP_ERROR      0x80040018  // Conflict in Permissions during setup of regions
#define PE_REGION_LIST_BAD_FORMAT       0x80040019  // Region List not properly formatted
#define PE_VM_BAD_PHYSICAL_ADDRESS      0x8004001A  // Bad physical address provide by guest
#define PE_VM_NO_PERM_VM                0x8004001B  // Attempt to execute a perm VM that does not exist
#define PE_VM_EXECUTING                 0X8004001C  // Attempt to execute a perm VM that is already executing
#define PE_VM_PERM_OPT_OUT              0x8004001D  // Perm STM/PE has been opted out
#define PE_VM_PERM_ALREADY_ESTABLISHED  0x8004001E  // Attempt setup a Perm PE VM when one has already been setup
#define PE_VM_TEMP_ACTIVE               0x8004001F  // Another Temp VM is already executing, try again later
#define PE_VM_EXCEPTION                 0x80040020  // PV VM terminated because of an exception
#define PE_VM_TEMP_OPT_OUT              0x80040021  // Temp STM/PE has been opted out
#define PE_VM_PE_SETUP_ERROR            0x80040023  

#define PE_VM_PERM_TERM                 0x80040022  // PE VM will be terminate when execution complete

// Various return codes

#define REGION_MEMSIZE_INVALID			0x80050001  // Invalid memory region request
#define INVALID_RESOURCE				0x80050002
#define ALLOC_FAILURE					0x80050003

//  PE VM States

#define PE_VM_AVAIL             0          // PE VM not in use, can be allocated
#define PE_VM_ACTIVE            1          // PE VM currently executing or being setup
#define PE_VM_IDLE              2          // PE VM (Perminate VM only) waiting to be executed
#define PE_VM_OPT_OUT           3          // PE VM not allowed by configuration
#define PE_VM_TERM              4          // PE VM is terminating
#define PE_VM_SUSPEND           5          // PE VM is suspended because of SMI

typedef unsigned char byte;
typedef unsigned short int word;
typedef unsigned long int dword;

typedef struct {
    dword   MsegHdrRevision;
    dword   SmmMonitorFeatures;
    dword   GdtrLimit;
    dword   GdtrBaseOffset;
    dword   CodeSelector;
    dword   EipOffset;
    dword   EspOffset;
    dword   Cr3Offset;
} MSEG_HEADER_STRUCT_HW;

typedef struct {
    byte    StmSpecVerMajor;
    byte    StmSpecVerMinor;
    word    ReservedSw;
    dword   StaticImageSize;
    dword   PerProcDynamicMemorySize;
    dword   AdditionalDynamicMemorySize;
    dword   StmFeatures;
    dword   NumberOfRevIDs;
    dword   StmRevisionId;
} MSEG_HEADER_STRUCT_SW;

   // MSEG_HEADER_STRUCT_HW * HwStmHdr;
   // MSEG_HEADER_STRUCT_SW * SwStmHdr;

// PE status flags found in the NonSmiHandler field of the STM_VMM_STRUCT (usually either pStmVmm or StmVmm)

#define PE_ACTIVE     1 << 0         //!<  PE active on this VM
#define PE_WAIT_SMI   1 << 1         //!<  PE waiting the completion of SMI processing

#define PRESERVE_VM    1          // keep the VM for future invocations
#define RELEASE_VM     2          // free up the resources associated with this VM
#define SUSPEND_VM     3          // keep the VM in suspended state because of SMI
#define NEW_VM         1
#define RESTART_VM     2

extern UINT32 FreePE_DataStructures(UINT32 PeType);
extern STM_STATUS AddPeVm(UINT32 ApicId, PE_MODULE_INFO * callerDataStructure, UINT32 PeVmNum, UINT32 RunVm);
extern STM_STATUS RunPermVM(UINT32 CpuIndex);
#endif