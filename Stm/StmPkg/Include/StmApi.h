/** @file
  STM API definition

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _STM_API_H_
#define _STM_API_H_

// definition in STM spec

#define STM_SPEC_VERSION_MAJOR 1
#define STM_SPEC_VERSION_MINOR 0

#pragma pack (push, 1)    

#define STM_HARDWARE_FIELD_FILL_TO_2K (2048 - sizeof(UINT32) * 8)
typedef struct {
  UINT32         StmHeaderRevision;
  UINT32         MonitorFeatures;
  UINT32         GdtrLimit;
  UINT32         GdtrBaseOffset;
  UINT32         CsSelector;
  UINT32         EipOffset;
  UINT32         EspOffset;
  UINT32         Cr3Offset;
  UINT8          Reserved[STM_HARDWARE_FIELD_FILL_TO_2K];
} HARDWARE_STM_HEADER;

#define STM_FEATURES_IA32E 0x1

typedef struct {
  UINT32         Intel64ModeSupported :1;  // bitfield
  UINT32         EptSupported         :1;  // bitfield
  UINT32         Reserved             :30; // must be 0
} STM_FEAT;

typedef struct {
  UINT8          StmSpecVerMajor;
  UINT8          StmSpecVerMinor;
  UINT16         Reserved; // must be zero
  UINT32         StaticImageSize;
  UINT32         PerProcDynamicMemorySize;
  UINT32         AdditionalDynamicMemorySize;
  STM_FEAT       StmFeatures;
  UINT32         NumberOfRevIDs;
  UINT32         StmSmmRevID[1];
  //
  // The total STM_HEADER should be 4K.
  //
} SOFTWARE_STM_HEADER;

typedef struct {
  HARDWARE_STM_HEADER HwStmHdr;
  SOFTWARE_STM_HEADER SwStmHdr;
} STM_HEADER;

#define SHA1   1
#define SHA256 2
typedef struct {
  UINT64 BiosComponentBase;
  UINT32 ImageSize;
  UINT32 HashAlgorithm; // SHA1 or SHA256
  UINT8  Hash[32];
} TXT_BIOS_COMPONENT_STATUS;

#define PAGE_SIZE 4096
typedef struct {
  UINT32 ImageSize;
  UINT32 Reserved;
  UINT64 ImagePageBase[1]; //[NumberOfPages];
} TXT_BIOS_COMPONENT_UPDATE;

// If (ImageSizeInBytes % PAGE_SIZE == 0) {
//   NumberOfPages = ImageSizeInBytes / PAGE_SIZE
// } else {
//   NumberOfPages = ImageSizeInBytes / PAGE_SIZE + 1
// }


typedef struct {
  UINT64    SpeRip;
  UINT64    SpeRsp;
  UINT16    SpeSs;
  UINT16    PageViolationException:1;
  UINT16    MsrViolationException:1;
  UINT16    RegisterViolationException:1;
  UINT16    IoViolationException:1;
  UINT16    PciViolationException:1;
  UINT16    Reserved1:11;
  UINT32    Reserved2;
} STM_PROTECTION_EXCEPTION_HANDLER;

typedef struct {
  UINT8                             ExecutionDisableOutsideSmrr:1;
  UINT8                             Intel64Mode:1;
  UINT8                             Cr4Pae : 1;
  UINT8                             Cr4Pse : 1;
  UINT8                             Reserved1 : 4;
} STM_SMM_ENTRY_STATE;

typedef struct {
  UINT8                             SmramToVmcsRestoreRequired : 1; // BIOS restore hint
  UINT8                             ReinitializeVmcsRequired : 1; // BIOS request
  UINT8                             Reserved2 : 6;
} STM_SMM_RESUME_STATE;

typedef struct {
  UINT8                             DomainType : 4; // STM input to BIOS on each SMI
  UINT8                             XStatePolicy : 2; // STM input to BIOS on each SMI
  UINT8                             EptEnabled : 1;
  UINT8                             Reserved3 : 1;
} STM_SMM_STATE;

typedef struct {
  UINT64                            Signature;
  UINT16                            Size;
  UINT8                             SmmDescriptorVerMajor;
  UINT8                             SmmDescriptorVerMinor;
  UINT32                            LocalApicId;
  STM_SMM_ENTRY_STATE               SmmEntryState;
  STM_SMM_RESUME_STATE              SmmResumeState;
  STM_SMM_STATE                     StmSmmState;
  UINT8                             Reserved4;
  UINT16                            SmmCs;
  UINT16                            SmmDs;
  UINT16                            SmmSs;
  UINT16                            SmmOtherSegment;
  UINT16                            SmmTr;
  UINT16                            Reserved5;
  UINT64                            SmmCr3;
  UINT64                            SmmStmSetupRip;
  UINT64                            SmmStmTeardownRip;
  UINT64                            SmmSmiHandlerRip;
  UINT64                            SmmSmiHandlerRsp;
  UINT64                            SmmGdtPtr;
  UINT32                            SmmGdtSize;
  UINT32                            RequiredStmSmmRevId;
  STM_PROTECTION_EXCEPTION_HANDLER  StmProtectionExceptionHandler;
  UINT64                            Reserved6;
  UINT64                            BiosHwResourceRequirementsPtr;
  // extend area
  UINT64                            AcpiRsdp;
  UINT8                             PhysicalAddressBits;
} TXT_PROCESSOR_SMM_DESCRIPTOR;

#define TXT_PROCESSOR_SMM_DESCRIPTOR_SIGNATURE         SIGNATURE_64('T', 'X', 'T', 'P', 'S', 'S', 'I', 'G')
#define TXT_PROCESSOR_SMM_DESCRIPTOR_VERSION_MAJOR     1
#define TXT_PROCESSOR_SMM_DESCRIPTOR_VERSION_MINOR     0

typedef enum {
  TxtSmmPageViolation = 1,
  TxtSmmMsrViolation,
  TxtSmmRegisterViolation,
  TxtSmmIoViolation,
  TxtSmmPciViolation
} TXT_SMM_PROTECTION_EXCEPTION_TYPE;

typedef struct {
  UINT32    Rdi;
  UINT32    Rsi;
  UINT32    Rbp;
  UINT32    Rdx;
  UINT32    Rcx;
  UINT32    Rbx;
  UINT32    Rax;
  UINT32    Cr3;
  UINT32    Cr2;
  UINT32    Cr0;
  UINT32    VmcsExitInstructionInfo;
  UINT32    VmcsExitInstructionLength;
  UINT64    VmcsExitQualification;
  UINT32    ErrorCode; // TXT_SMM_PROTECTION_EXCEPTION_TYPE
  UINT32    Rip;
  UINT32    Cs;
  UINT32    Rflags;
  UINT32    Rsp;
  UINT32    Ss;
} STM_PROTECTION_EXCEPTION_STACK_FRAME_IA32;

typedef struct {
  UINT64    R15;
  UINT64    R14;
  UINT64    R13;
  UINT64    R12;
  UINT64    R11;
  UINT64    R10;
  UINT64    R9;
  UINT64    R8;
  UINT64    Rdi;
  UINT64    Rsi;
  UINT64    Rbp;
  UINT64    Rdx;
  UINT64    Rcx;
  UINT64    Rbx;
  UINT64    Rax;
  UINT64    Cr8;
  UINT64    Cr3;
  UINT64    Cr2;
  UINT64    Cr0;
  UINT64    VmcsExitInstructionInfo;
  UINT64    VmcsExitInstructionLength;
  UINT64    VmcsExitQualification;
  UINT64    ErrorCode; // TXT_SMM_PROTECTION_EXCEPTION_TYPE
  UINT64    Rip;
  UINT64    Cs;
  UINT64    Rflags;
  UINT64    Rsp;
  UINT64    Ss;
} STM_PROTECTION_EXCEPTION_STACK_FRAME_X64;

typedef union {
  STM_PROTECTION_EXCEPTION_STACK_FRAME_IA32 *Ia32StackFrame;
  STM_PROTECTION_EXCEPTION_STACK_FRAME_X64  *X64StackFrame;
} STM_PROTECTION_EXCEPTION_STACK_FRAME;

#define STM_SMM_REV_ID   0x80010100

typedef struct _STM_SMM_CPU_STATE {
  UINT8                         Reserved1[0x1d0];       // fc00h
  UINT32                        GdtBaseHiDword;         // fdd0h : NO
  UINT32                        LdtBaseHiDword;         // fdd4h : NO
  UINT32                        IdtBaseHiDword;         // fdd8h : NO
  UINT8                         Reserved2[0x4];         // fddch
  UINT64                        IoRdi;                  // fde0h : NO - restricted
  UINT64                        IoEip;                  // fde8h : YES
  UINT64                        IoRcx;                  // fdf0h : NO - restricted
  UINT64                        IoRsi;                  // fdf8h : NO - restricted
  UINT8                         Reserved3[0x40];        // fe00h
  UINT32                        Cr4;                    // fe40h : NO
  UINT8                         Reserved4[0x48];        // fe44h
  UINT32                        GdtBaseLoDword;         // fe8ch : NO
  UINT32                        GdtLimit;               // fe90h : NO - RESTRICTED
  UINT32                        IdtBaseLoDword;         // fe94h : NO
  UINT32                        IdtLimit;               // fe98h : NO - RESTRICTED
  UINT32                        LdtBaseLoDword;         // fe9ch : NO
  UINT32                        LdtLimit;               // fea0h : NO - RESTRICTED
  UINT32                        LdtInfo;                // fea4h : NO - RESTRICTED
  UINT8                         Reserved5[0x30];        // fea8h
  UINT64                        Eptp;                   // fed8h : NO
  UINT32                        EnabledEPT;             // fee0h : NO
  UINT8                         Reserved6[0x14];        // fee4h
  UINT32                        Smbase;                 // fef8h : YES - NO for STM
  UINT32                        SMMRevId;               // fefch : NO
  UINT16                        IORestart;              // ff00h : YES
  UINT16                        AutoHALTRestart;        // ff02h : YES
  UINT8                         Reserved7[0x18];        // ff04h
  UINT64                        R15;                    // ff1ch : YES
  UINT64                        R14;                    // ff24h : YES
  UINT64                        R13;                    // ff2ch : YES
  UINT64                        R12;                    // ff34h : YES
  UINT64                        R11;                    // ff3ch : YES
  UINT64                        R10;                    // ff44h : YES
  UINT64                        R9;                     // ff4ch : YES
  UINT64                        R8;                     // ff54h : YES
  UINT64                        Rax;                    // ff5ch : YES
  UINT64                        Rcx;                    // ff64h : YES
  UINT64                        Rdx;                    // ff6ch : YES
  UINT64                        Rbx;                    // ff74h : YES
  UINT64                        Rsp;                    // ff7ch : YES
  UINT64                        Rbp;                    // ff84h : YES
  UINT64                        Rsi;                    // ff8ch : YES
  UINT64                        Rdi;                    // ff94h : YES
  UINT64                        IOMemAddr;              // ff9ch : NO
  UINT32                        IOMisc;                 // ffa4h : NO
  UINT32                        Es;                     // ffa8h : NO
  UINT32                        Cs;                     // ffach : NO
  UINT32                        Ss;                     // ffb0h : NO
  UINT32                        Ds;                     // ffb4h : NO
  UINT32                        Fs;                     // ffb8h : NO
  UINT32                        Gs;                     // ffbch : NO
  UINT32                        Ldtr;                   // ffc0h : NO
  UINT32                        Tr;                     // ffc4h : NO
  UINT64                        Dr7;                    // ffc8h : NO
  UINT64                        Dr6;                    // ffd0h : NO
  UINT64                        Rip;                    // ffd8h : YES
  UINT64                        Ia32Efer;               // ffe0h : YES - NO for STM
  UINT64                        Rflags;                 // ffe8h : YES
  UINT64                        Cr3;                    // fff0h : NO
  UINT64                        Cr0;                    // fff8h : NO
} STM_SMM_CPU_STATE;

//
// STM Mapping
//

typedef struct {
  UINT64   PhysicalAddress;
  UINT64   VirtualAddress;
  UINT32   PageCount;
  UINT32   PatCacheType;
} STM_MAP_ADDRESS_RANGE_DESCRIPTOR;
#define ST_UC  0x00
#define WC     0x01
#define WT     0x04
#define WP     0x05
#define WB     0x06
#define UC     0x07
#define FOLLOW_MTRR 0xFFFFFFFF

typedef struct {
  UINT64   VirtualAddress;
  UINT32   Length;
} STM_UNMAP_ADDRESS_RANGE_DESCRIPTOR;

typedef struct {
  UINT64   InterruptedGuestVirtualAddress;
  UINT32   Length;
  UINT64   InterruptedCr3;
  UINT64   InterruptedEptp;
  UINT32   MapToSmmGuest:2;
  UINT32   InterruptedCr4Pae:1;
  UINT32   InterruptedCr4Pse:1;
  UINT32   InterruptedIa32eMode:1;
  UINT32   Reserved1:27;
  UINT32   Reserved2;
  UINT64   PhysicalAddress;
  UINT64   SmmGuestVirtualAddress;
} STM_ADDRESS_LOOKUP_DESCRIPTOR;
#define DO_NOT_MAP  0
#define ONE_TO_ONE  1
#define VIRTUAL_ADDRESS_SPECIFIED 3

//
// STM_RESOURCE_LIST
//
#define END_OF_RESOURCES        0
#define MEM_RANGE               1
#define IO_RANGE                2
#define MMIO_RANGE              3
#define MACHINE_SPECIFIC_REG    4
#define PCI_CFG_RANGE           5
#define TRAPPED_IO_RANGE        6
#define ALL_RESOURCES           7
#define REGISTER_VIOLATION      8
#define MAX_DESC_TYPE           8

typedef struct {
  UINT32 RscType;
  UINT16 Length;
  UINT16 ReturnStatus:1;
  UINT16 Reserved:14;
  UINT16 IgnoreResource:1;
} STM_RSC_DESC_HEADER;

typedef struct {
  STM_RSC_DESC_HEADER Hdr;
  UINT64              ResourceListContinuation;
} STM_RSC_END;

// byte granular Memory range support
#define STM_RSC_BGM    0x4

typedef struct {
  STM_RSC_DESC_HEADER Hdr;
  UINT64              Base;
  UINT64              Length;
  UINT32              RWXAttributes:3;
  UINT32              Reserved:29;
  UINT32              Reserved_2;
} STM_RSC_MEM_DESC;
#define STM_RSC_MEM_R    0x1
#define STM_RSC_MEM_W    0x2
#define STM_RSC_MEM_X    0x4

typedef struct {
  STM_RSC_DESC_HEADER Hdr;
  UINT16              Base;
  UINT16              Length;
  UINT32              Reserved;
} STM_RSC_IO_DESC;

// byte granular MMIO range support
#define STM_RSC_BGI    0x2

typedef struct {
  STM_RSC_DESC_HEADER Hdr;
  UINT64              Base;
  UINT64              Length;
  UINT32              RWXAttributes:3;
  UINT32              Reserved:29;
  UINT32              Reserved_2;
} STM_RSC_MMIO_DESC;
#define STM_RSC_MMIO_R    0x1
#define STM_RSC_MMIO_W    0x2
#define STM_RSC_MMIO_X    0x4

typedef struct {
  STM_RSC_DESC_HEADER Hdr;
  UINT32              MsrIndex;
  UINT32              KernelModeProcessing:1;
  UINT32              Reserved:31;
  UINT64              ReadMask;
  UINT64              WriteMask;
} STM_RSC_MSR_DESC;

// bit granular MSR resource support
#define STM_RSC_MSR    0x8

typedef struct {
  UINT8  Type; // must be 1, indicating Hardware Device Path
  UINT8  Subtype; // must be 1, indicating PCI
  UINT16 Length; // sizeof(STM_PCI_DEVICE_PATH_NODE) which is 6
  UINT8  PciFunction;
  UINT8  PciDevice;
} STM_PCI_DEVICE_PATH_NODE;
typedef struct {
  STM_RSC_DESC_HEADER       Hdr;
  UINT16                    RWAttributes:2;
  UINT16                    Reserved:14;
  UINT16                    Base;
  UINT16                    Length;
  UINT8                     OriginatingBusNumber;
  UINT8                     LastNodeIndex;
  STM_PCI_DEVICE_PATH_NODE  PciDevicePath[1];
//STM_PCI_DEVICE_PATH_NODE  PciDevicePath[LastNodeIndex + 1];
} STM_RSC_PCI_CFG_DESC;

#define STM_RSC_PCI_CFG_R    0x1
#define STM_RSC_PCI_CFG_W    0x2

typedef struct {
  STM_RSC_DESC_HEADER Hdr;
  UINT16              Base;
  UINT16              Length;
  UINT16              In:1;
  UINT16              Out:1;
  UINT16              Api:1;
  UINT16              Reserved1:13;
  UINT16              Reserved2;
} STM_RSC_TRAPPED_IO_DESC;

typedef struct {
  STM_RSC_DESC_HEADER Hdr;
} STM_RSC_ALL_RESOURCES_DESC;

typedef struct {
  STM_RSC_DESC_HEADER Hdr;
  UINT32              RegisterType;
  UINT32              Reserved;
  UINT64              ReadMask;
  UINT64              WriteMask;
} STM_REGISTER_VIOLATION_DESC;

typedef enum {
  StmRegisterCr0,
  StmRegisterCr2,
  StmRegisterCr3,
  StmRegisterCr4,
  StmRegisterCr8,
  StmRegisterMax,
} STM_REGISTER_VIOLATION_TYPE;

typedef union {
  STM_RSC_DESC_HEADER             Header;
  STM_RSC_END                     End;
  STM_RSC_MEM_DESC                Mem;
  STM_RSC_IO_DESC                 Io;
  STM_RSC_MMIO_DESC               Mmio;
  STM_RSC_MSR_DESC                Msr;
  STM_RSC_PCI_CFG_DESC            PciCfg;
  STM_RSC_TRAPPED_IO_DESC         TrappedIo;
  STM_RSC_ALL_RESOURCES_DESC      All;
  STM_REGISTER_VIOLATION_DESC     RegisterViolation;
} STM_RSC;

//
// VMCS database
//
#define STM_VMCS_DATABASE_REQUEST_ADD    1
#define STM_VMCS_DATABASE_REQUEST_REMOVE 0

// Values for DomainType
// Intepreter of DomainType
#define DOMAIN_DISALLOWED_IO_OUT (1u << 0)
#define DOMAIN_DISALLOWED_IO_IN  (1u << 1)
#define DOMAIN_INTEGRITY         (1u << 2)
#define DOMAIN_CONFIDENTIALITY   (1u << 3)

#define DOMAIN_UNPROTECTED           0x00
#define DOMAIN_INTEGRITY_PROT_OUT_IN (DOMAIN_INTEGRITY)
//#define DOMAIN_INTEGRITY_PROT_OUT    (DOMAIN_INTEGRITY | DOMAIN_DISALLOWED_IO_IN)
#define DOMAIN_FULLY_PROT_OUT_IN     (DOMAIN_CONFIDENTIALITY | DOMAIN_INTEGRITY)
//#define DOMAIN_FULLY_PROT_IN         (DOMAIN_CONFIDENTIALITY | DOMAIN_INTEGRITY | DOMAIN_DISALLOWED_IO_OUT)
//#define DOMAIN_FULLY_PROT_OUT        (DOMAIN_CONFIDENTIALITY | DOMAIN_INTEGRITY | DOMAIN_DISALLOWED_IO_IN)
#define DOMAIN_FULLY_PROT            (DOMAIN_CONFIDENTIALITY | DOMAIN_INTEGRITY | DOMAIN_DISALLOWED_IO_IN | DOMAIN_DISALLOWED_IO_OUT)

// Values for XStatePolicy
#define XSTATE_READWRITE      0x00
#define XSTATE_READONLY       0x01
#define XSTATE_SCRUB          0x03

typedef struct {
  UINT64 VmcsPhysPointer; // bits 11:0 are reserved and must be 0
  UINT32 DomainType :4;
  UINT32 XStatePolicy :2;
  UINT32 DegradationPolicy :4;
  UINT32 Reserved1 :22; // Must be 0
  UINT32 AddOrRemove;
} STM_VMCS_DATABASE_REQUEST;

//
// Event log
//
#define NEW_LOG       1
#define CONFIGURE_LOG 2
#define START_LOG     3
#define STOP_LOG      4
#define CLEAR_LOG     5
#define DELETE_LOG    6
typedef enum {
  EvtLogStarted,
  EvtLogStopped,
  EvtLogInvalidParameterDetected,
  EvtHandledProtectionException,
  // unhandled protection exceptions result in reset & cannot be logged
  EvtBiosAccessToUnclaimedResource,
  EvtMleResourceProtectionGranted,
  EvtMleResourceProtectionDenied,
  EvtMleResourceUnprotect,
  EvtMleResourceUnprotectError,
  EvtMleDomainTypeDegraded,
  // add more here
  EvtMleMax,
  // Not used
  EvtInvalid = 0xFFFFFFFF,
} EVENT_TYPE;

//#define STM_EVENT_LOG_PAGE_COUNT_MAX  62

typedef struct {
      UINT32 PageCount;
      UINT64 Pages[1]; // number of elements is PageCount
} STM_EVENT_LOG_MANAGEMENT_REQUEST_DATA_LOG_BUFFER;

typedef union {
    STM_EVENT_LOG_MANAGEMENT_REQUEST_DATA_LOG_BUFFER LogBuffer;
    UINT32                                           EventEnableBitmap; // bitmap of EVENT_TYPE
} STM_EVENT_LOG_MANAGEMENT_REQUEST_DATA;

typedef struct {
  UINT32                                SubFunctionIndex;
  STM_EVENT_LOG_MANAGEMENT_REQUEST_DATA Data;
} STM_EVENT_LOG_MANAGEMENT_REQUEST;

//
// VMCALL API Numbers
//

// API number convention: BIOS facing VMCALL interfaces have bit 16 clear
#define STM_API_MAP_ADDRESS_RANGE                  0x00000001
#define STM_API_UNMAP_ADDRESS_RANGE                0x00000002
#define STM_API_ADDRESS_LOOKUP                     0x00000003
#define STM_API_RETURN_FROM_PROTECTION_EXCEPTION   0x00000004

// API number convention: MLE facing VMCALL interfaces have bit 16 set
//
// The STM configuration lifecycle is as follows:
// 1. SENTER->SINIT->MLE: MLE begins execution with SMI disabled (masked).
// 2. MLE invokes InitializeProtectionVMCALL() to prepare STM for setup of initial protection profile. This is done on a single CPU and has global effect.
// 3. MLE invokes ProtectResourceVMCALL() to define the initial protection profile. The protection profile is global across all CPUs.
// 4. MLE invokes StartStmVMCALL() to enable the STM to begin receiving SMI events. This must be done on every logical CPU.
// 5. MLE may invoke ProtectResourceVMCALL() or UnProtectResourceVMCALL() during runtime as many times as necessary.
// 6. MLE invokes StopStmVMCALL() to disable the STM. SMI is again masked following StopStmVMCALL().
//
#define STM_API_START                              0x00010001
#define STM_API_STOP                               0x00010002
#define STM_API_PROTECT_RESOURCE                   0x00010003
#define STM_API_UNPROTECT_RESOURCE                 0x00010004
#define STM_API_GET_BIOS_RESOURCES                 0x00010005
#define STM_API_MANAGE_VMCS_DATABASE               0x00010006
#define STM_API_INITIALIZE_PROTECTION              0x00010007
#define STM_API_MANAGE_EVENT_LOG                   0x00010008

//
// Return codes
//
typedef UINT32  STM_STATUS;

#define STM_SUCCESS 0x00000000
#define SMM_SUCCESS 0x00000000
// all error codes have bit 31 set
// STM errors have bit 16 set
#define ERROR_STM_SECURITY_VIOLATION               0x80010001
#define ERROR_STM_CACHE_TYPE_NOT_SUPPORTED         0x80010002
#define ERROR_STM_PAGE_NOT_FOUND                   0x80010003
#define ERROR_STM_BAD_CR3                          0x80010004
#define ERROR_STM_PHYSICAL_OVER_4G                 0x80010005
#define ERROR_STM_VIRTUAL_SPACE_TOO_SMALL          0x80010006
#define ERROR_STM_UNPROTECTABLE_RESOURCE           0x80010007
#define ERROR_STM_ALREADY_STARTED                  0x80010008
#define ERROR_STM_WITHOUT_SMX_UNSUPPORTED          0x80010009
#define ERROR_STM_STOPPED                          0x8001000A
#define ERROR_STM_BUFFER_TOO_SMALL                 0x8001000B
#define ERROR_STM_INVALID_VMCS_DATABASE            0x8001000C
#define ERROR_STM_MALFORMED_RESOURCE_LIST          0x8001000D
#define ERROR_STM_INVALID_PAGECOUNT                0x8001000E
#define ERROR_STM_LOG_ALLOCATED                    0x8001000F
#define ERROR_STM_LOG_NOT_ALLOCATED                0x80010010
#define ERROR_STM_LOG_NOT_STOPPED                  0x80010011
#define ERROR_STM_LOG_NOT_STARTED                  0x80010012
#define ERROR_STM_RESERVED_BIT_SET                 0x80010013
#define ERROR_STM_NO_EVENTS_ENABLED                0x80010014
#define ERROR_STM_OUT_OF_RESOURCES                 0x80010015
#define ERROR_STM_FUNCTION_NOT_SUPPORTED           0x80010016
#define ERROR_STM_UNPROTECTABLE                    0x80010017
#define ERROR_STM_UNSUPPORTED_MSR_BIT              0x80010018
#define ERROR_STM_UNSPECIFIED                      0x8001FFFF

// SMM errors have bit 17 set
#define ERROR_SMM_BAD_BUFFER                       0x80020001
#define ERROR_SMM_INVALID_RSC                      0x80020004
#define ERROR_SMM_INVALID_BUFFER_SIZE              0x80020005
#define ERROR_SMM_BUFFER_TOO_SHORT                 0x80020006
#define ERROR_SMM_INVALID_LIST                     0x80020007
#define ERROR_SMM_OUT_OF_MEMORY                    0x80020008
#define ERROR_SMM_AFTER_INIT                       0x80020009
#define ERROR_SMM_UNSPECIFIED                      0x8002FFFF

// Errors that apply to both have bits 15, 16, and 17 set
#define ERROR_INVALID_API                          0x80038001
#define ERROR_INVALID_PARAMETER                    0x80038002

//
// STM TXT.ERRORCODE codes
//
#define STM_CRASH_PROTECTION_EXCEPTION             0xC000F001
#define STM_CRASH_PROTECTION_EXCEPTION_FAILURE     0xC000F002
#define STM_CRASH_DOMAIN_DEGRADATION_FAILURE       0xC000F003
#define STM_CRASH_BIOS_PANIC                       0xC000E000

typedef struct {
  UINT32 EventSerialNumber;
  UINT16 Type;
  UINT16 Lock :1;
  UINT16 Valid :1;
  UINT16 ReadByMle :1;
  UINT16 Wrapped :1;
  UINT16 Reserved :12;
} LOG_ENTRY_HEADER;

typedef struct {
  UINT32 Reserved;
} ENTRY_EVT_LOG_STARTED;

typedef struct {
  UINT32 Reserved;
} ENTRY_EVT_LOG_STOPPED;

typedef struct {
  UINT32 VmcallApiNumber;
} ENTRY_EVT_LOG_INVALID_PARAM;

typedef struct {
  STM_RSC Resource;
} ENTRY_EVT_LOG_HANDLED_PROTECTION_EXCEPTION;

typedef struct {
  STM_RSC Resource;
} ENTRY_EVT_BIOS_ACCESS_UNCLAIMED_RSC;

typedef struct {
  STM_RSC Resource;
} ENTRY_EVT_MLE_RSC_PROT_GRANTED;

typedef struct {
  STM_RSC Resource;
} ENTRY_EVT_MLE_RSC_PROT_DENIED;

typedef struct {
  STM_RSC Resource;
} ENTRY_EVT_MLE_RSC_UNPROT;

typedef struct {
  STM_RSC Resource;
} ENTRY_EVT_MLE_RSC_UNPROT_ERROR;

typedef struct {
  UINT64 VmcsPhysPointer;
  UINT8  ExpectedDomainType;
  UINT8  DegradedDomainType;
} ENTRY_EVT_MLE_DOMAIN_TYPE_DEGRADED;

typedef union {
  ENTRY_EVT_LOG_STARTED                      Started;
  ENTRY_EVT_LOG_STOPPED                      Stopped;
  ENTRY_EVT_LOG_INVALID_PARAM                InvalidParam;
  ENTRY_EVT_LOG_HANDLED_PROTECTION_EXCEPTION HandledProtectionException;
  ENTRY_EVT_BIOS_ACCESS_UNCLAIMED_RSC        BiosUnclaimedRsc;
  ENTRY_EVT_MLE_RSC_PROT_GRANTED             MleRscProtGranted;
  ENTRY_EVT_MLE_RSC_PROT_DENIED              MleRscProtDenied;
  ENTRY_EVT_MLE_RSC_UNPROT                   MleRscUnprot;
  ENTRY_EVT_MLE_RSC_UNPROT_ERROR             MleRscUnprotError;
  ENTRY_EVT_MLE_DOMAIN_TYPE_DEGRADED         MleDomainTypeDegraded;
} LOG_ENTRY_DATA;

typedef struct {
  LOG_ENTRY_HEADER Hdr;
  LOG_ENTRY_DATA   Data;
} STM_LOG_ENTRY;

#define STM_LOG_ENTRY_SIZE  256

//
//
//
#define STM_CONFIG_SMI_UNBLOCKING_BY_VMX_OFF 0x1

//
// TXT debug
//
#define SW_SMI_STM_ADD_RUNTIME_RESOURCES_SUB_FUNC   0
#define SW_SMI_STM_READ_BIOS_RESOURCES_SUB_FUNC     1
#define SW_SMI_STM_REPLACE_BIOS_RESOURCES_SUB_FUNC  2

typedef struct {
  UINT32  BufferSize;
  UINT32  Reserved;
//UINT8   Data[];
} TXT_BIOS_DEBUG;

#pragma pack (pop)

#endif
