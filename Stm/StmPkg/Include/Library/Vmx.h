/** @file
  VMX header file

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _VMX_H_
#define _VMX_H_

#include "CpuArchSpecific.h"

#define IA32_SMM_MONITOR_CTL_MSR_INDEX      0x9B
#define   IA32_SMM_MONITOR_VALID            1u
#define   IA32_SMM_MONITOR_SMI_UNBLOCKING_BY_VMX_OFF (1u << 2)  // If IA32_VMX_MISC[bit 28])
#define IA32_SMBASE_INDEX                   0x9E // if IA32_VMX_MISC[bit 15] = 1
#define IA32_VMX_BASIC_MSR_INDEX           0x480
#define IA32_VMX_PINBASED_CTLS_MSR_INDEX   0x481
#define IA32_VMX_PROCBASED_CTLS_MSR_INDEX  0x482
#define IA32_VMX_EXIT_CTLS_MSR_INDEX       0x483
#define IA32_VMX_ENTRY_CTLS_MSR_INDEX      0x484
#define IA32_VMX_MISC_MSR_INDEX            0x485
#define IA32_VMX_CR0_FIXED0_MSR_INDEX      0x486
#define IA32_VMX_CR0_FIXED1_MSR_INDEX      0x487
#define IA32_VMX_CR4_FIXED0_MSR_INDEX      0x488
#define IA32_VMX_CR4_FIXED1_MSR_INDEX      0x489
#define IA32_VMX_VMCS_ENUM_MSR_INDEX       0x48A
#define IA32_VMX_PROCBASED_CTLS2_MSR_INDEX 0x48B
#define IA32_VMX_EPT_VPID_CAP_MSR_INDEX    0x48C

#define VMCS_16_CONTROL_VPID_INDEX                             0x0000
#define VMCS_16_GUEST_ES_INDEX                                 0x0800
#define VMCS_16_GUEST_CS_INDEX                                 0x0802
#define VMCS_16_GUEST_SS_INDEX                                 0x0804
#define VMCS_16_GUEST_DS_INDEX                                 0x0806
#define VMCS_16_GUEST_FS_INDEX                                 0x0808
#define VMCS_16_GUEST_GS_INDEX                                 0x080A
#define VMCS_16_GUEST_LDTR_INDEX                               0x080C
#define VMCS_16_GUEST_TR_INDEX                                 0x080E
#define VMCS_16_HOST_ES_INDEX                                  0x0C00
#define VMCS_16_HOST_CS_INDEX                                  0x0C02
#define VMCS_16_HOST_SS_INDEX                                  0x0C04
#define VMCS_16_HOST_DS_INDEX                                  0x0C06
#define VMCS_16_HOST_FS_INDEX                                  0x0C08
#define VMCS_16_HOST_GS_INDEX                                  0x0C0A
#define VMCS_16_HOST_TR_INDEX                                  0x0C0C
#define VMCS_64_CONTROL_IO_BITMAP_A_INDEX                      0x2000
#define VMCS_64_CONTROL_IO_BITMAP_B_INDEX                      0x2002
#define VMCS_64_CONTROL_MSR_BITMAP_INDEX                       0x2004
#define VMCS_64_CONTROL_VMEXIT_MSR_STORE_INDEX                 0x2006
#define VMCS_64_CONTROL_VMEXIT_MSR_LOAD_INDEX                  0x2008
#define VMCS_64_CONTROL_VMENTRY_MSR_LOAD_INDEX                 0x200A
#define VMCS_64_CONTROL_EXECUTIVE_VMCS_PTR_INDEX               0x200C
#define VMCS_64_CONTROL_TSC_OFFSET_INDEX                       0x2010
#define VMCS_64_CONTROL_VIRTUAL_APIC_ADDR_INDEX                0x2012
#define VMCS_64_CONTROL_APIC_ACCESS_ADDR_INDEX                 0x2014
#define VMCS_64_CONTROL_VM_FUNCTION_CONTROLS_INDEX             0x2018
#define VMCS_64_CONTROL_EPT_PTR_INDEX                          0x201A
#define VMCS_64_CONTROL_EPTP_LIST_ADDRESS_INDEX                0x2024
#define VMCS_64_RO_GUEST_PHYSICAL_ADDR_INDEX                   0x2400
#define VMCS_64_GUEST_VMCS_LINK_PTR_INDEX                      0x2800
#define VMCS_64_GUEST_IA32_DEBUGCTL_INDEX                      0x2802
#define VMCS_64_GUEST_IA32_PAT_INDEX                           0x2804
#define VMCS_64_GUEST_IA32_EFER_INDEX                          0x2806
#define VMCS_64_GUEST_IA32_PERF_GLOBAL_CTRL_INDEX              0x2808
#define VMCS_64_GUEST_PDPTE0_INDEX                             0x280A
#define VMCS_64_GUEST_PDPTE1_INDEX                             0x280C
#define VMCS_64_GUEST_PDPTE2_INDEX                             0x280E
#define VMCS_64_GUEST_PDPTE3_INDEX                             0x2810
#define VMCS_64_HOST_IA32_PAT_INDEX                            0x2C00
#define VMCS_64_HOST_IA32_EFER_INDEX                           0x2C02
#define VMCS_64_HOST_IA32_PERF_GLOBAL_CTRL_INDEX               0x2C04
#define VMCS_32_CONTROL_PIN_BASED_VM_EXECUTION_INDEX           0x4000
#define VMCS_32_CONTROL_PROCESSOR_BASED_VM_EXECUTION_INDEX     0x4002
#define VMCS_32_CONTROL_EXCEPTION_BITMAP_INDEX                 0x4004
#define VMCS_32_CONTROL_PAGE_FAULT_ERROR_CODE_MASK_INDEX       0x4006
#define VMCS_32_CONTROL_PAGE_FAULT_ERROR_CODE_MATCH_INDEX      0x4008
#define VMCS_32_CONTROL_CR3_TARGET_COUNT_INDEX                 0x400A
#define VMCS_32_CONTROL_VMEXIT_CONTROLS_INDEX                  0x400C
#define VMCS_32_CONTROL_VMEXIT_MSR_STORE_COUNT_INDEX           0x400E
#define VMCS_32_CONTROL_VMEXIT_MSR_LOAD_COUNT_INDEX            0x4010
#define VMCS_32_CONTROL_VMENTRY_CONTROLS_INDEX                 0x4012
#define VMCS_32_CONTROL_VMENTRY_MSR_LOAD_COUNT_INDEX           0x4014
#define VMCS_32_CONTROL_VMENTRY_INTERRUPTION_INFO_INDEX        0x4016
#define VMCS_32_CONTROL_VMENTRY_EXCEPTION_ERROR_CODE_INDEX     0x4018
#define VMCS_32_CONTROL_VMENTRY_INSTRUCTION_LENGTH_INDEX       0x401A
#define VMCS_32_CONTROL_TPR_THRESHOLD_INDEX                    0x401C
#define VMCS_32_CONTROL_2ND_PROCESSOR_BASED_VM_EXECUTION_INDEX 0x401E
#define VMCS_32_CONTROL_PLE_GAP_INDEX                          0x4020
#define VMCS_32_CONTROL_PLE_WINDOW_INDEX                       0x4022
#define VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX                  0x4400
#define VMCS_32_RO_EXIT_REASON_INDEX                           0x4402
#define VMCS_32_RO_VMEXIT_INTERRUPTION_INFO_INDEX              0x4404
#define VMCS_32_RO_VMEXIT_INTERRUPTION_ERROR_CODE_INDEX        0x4406
#define VMCS_32_RO_IDT_VECTORING_INFO_INDEX                    0x4408
#define VMCS_32_RO_IDT_VECTORING_ERROR_CODE_INDEX              0x440A
#define VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX             0x440C
#define VMCS_32_RO_VMEXIT_INSTRUCTION_INFO_INDEX               0x440E
#define VMCS_32_GUEST_ES_LIMIT_INDEX                           0x4800
#define VMCS_32_GUEST_CS_LIMIT_INDEX                           0x4802
#define VMCS_32_GUEST_SS_LIMIT_INDEX                           0x4804
#define VMCS_32_GUEST_DS_LIMIT_INDEX                           0x4806
#define VMCS_32_GUEST_FS_LIMIT_INDEX                           0x4808
#define VMCS_32_GUEST_GS_LIMIT_INDEX                           0x480A
#define VMCS_32_GUEST_LDTR_LIMIT_INDEX                         0x480C
#define VMCS_32_GUEST_TR_LIMIT_INDEX                           0x480E
#define VMCS_32_GUEST_GDTR_LIMIT_INDEX                         0x4810
#define VMCS_32_GUEST_IDTR_LIMIT_INDEX                         0x4812
#define VMCS_32_GUEST_ES_ACCESS_RIGHT_INDEX                    0x4814
#define VMCS_32_GUEST_CS_ACCESS_RIGHT_INDEX                    0x4816
#define VMCS_32_GUEST_SS_ACCESS_RIGHT_INDEX                    0x4818
#define VMCS_32_GUEST_DS_ACCESS_RIGHT_INDEX                    0x481A
#define VMCS_32_GUEST_FS_ACCESS_RIGHT_INDEX                    0x481C
#define VMCS_32_GUEST_GS_ACCESS_RIGHT_INDEX                    0x481E
#define VMCS_32_GUEST_LDTR_ACCESS_RIGHT_INDEX                  0x4820
#define VMCS_32_GUEST_TR_ACCESS_RIGHT_INDEX                    0x4822
#define VMCS_32_GUEST_INTERRUPTIBILITY_STATE_INDEX             0x4824
#define VMCS_32_GUEST_ACTIVITY_STATE_INDEX                     0x4826
#define VMCS_32_GUEST_SMBASE_INDEX                             0x4828
#define VMCS_32_GUEST_IA32_SYSENTER_CS_INDEX                   0x482A
#define VMCS_32_GUEST_VMX_PREEMPTION_TIMER_VALUE_INDEX         0x482E
#define VMCS_32_HOST_IA32_SYSENTER_CS_INDEX                    0x4C00
#define VMCS_N_CONTROL_CR0_GUEST_HOST_MASK_INDEX               0x6000
#define VMCS_N_CONTROL_CR4_GUEST_HOST_MASK_INDEX               0x6002
#define VMCS_N_CONTROL_CR0_READ_SHADOW_INDEX                   0x6004
#define VMCS_N_CONTROL_CR4_READ_SHADOW_INDEX                   0x6006
#define VMCS_N_CONTROL_CR3_TARGET_VALUE0_INDEX                 0x6008
#define VMCS_N_CONTROL_CR3_TARGET_VALUE1_INDEX                 0x600A
#define VMCS_N_CONTROL_CR3_TARGET_VALUE2_INDEX                 0x600C
#define VMCS_N_CONTROL_CR3_TARGET_VALUE3_INDEX                 0x600E
#define VMCS_N_RO_EXIT_QUALIFICATION_INDEX                     0x6400
#define VMCS_N_RO_IO_RCX_INDEX                                 0x6402
#define VMCS_N_RO_IO_RSI_INDEX                                 0x6404
#define VMCS_N_RO_IO_RDI_INDEX                                 0x6406
#define VMCS_N_RO_IO_RIP_INDEX                                 0x6408
#define VMCS_N_RO_GUEST_LINEAR_ADDR_INDEX                      0x640A
#define VMCS_N_GUEST_CR0_INDEX                                 0x6800
#define VMCS_N_GUEST_CR3_INDEX                                 0x6802
#define VMCS_N_GUEST_CR4_INDEX                                 0x6804
#define VMCS_N_GUEST_ES_BASE_INDEX                             0x6806
#define VMCS_N_GUEST_CS_BASE_INDEX                             0x6808
#define VMCS_N_GUEST_SS_BASE_INDEX                             0x680A
#define VMCS_N_GUEST_DS_BASE_INDEX                             0x680C
#define VMCS_N_GUEST_FS_BASE_INDEX                             0x680E
#define VMCS_N_GUEST_GS_BASE_INDEX                             0x6810
#define VMCS_N_GUEST_LDTR_BASE_INDEX                           0x6812
#define VMCS_N_GUEST_TR_BASE_INDEX                             0x6814
#define VMCS_N_GUEST_GDTR_BASE_INDEX                           0x6816
#define VMCS_N_GUEST_IDTR_BASE_INDEX                           0x6818
#define VMCS_N_GUEST_DR7_INDEX                                 0x681A
#define VMCS_N_GUEST_RSP_INDEX                                 0x681C
#define VMCS_N_GUEST_RIP_INDEX                                 0x681E
#define VMCS_N_GUEST_RFLAGS_INDEX                              0x6820
#define VMCS_N_GUEST_PENDING_DEBUG_EXCEPTIONS_INDEX            0x6822
#define VMCS_N_GUEST_IA32_SYSENTER_ESP_INDEX                   0x6824
#define VMCS_N_GUEST_IA32_SYSENTER_EIP_INDEX                   0x6826
#define VMCS_N_HOST_CR0_INDEX                                  0x6C00
#define VMCS_N_HOST_CR3_INDEX                                  0x6C02
#define VMCS_N_HOST_CR4_INDEX                                  0x6C04
#define VMCS_N_HOST_FS_BASE_INDEX                              0x6C06
#define VMCS_N_HOST_GS_BASE_INDEX                              0x6C08
#define VMCS_N_HOST_TR_BASE_INDEX                              0x6C0A
#define VMCS_N_HOST_GDTR_BASE_INDEX                            0x6C0C
#define VMCS_N_HOST_IDTR_BASE_INDEX                            0x6C0E
#define VMCS_N_HOST_IA32_SYSENTER_ESP_INDEX                    0x6C10
#define VMCS_N_HOST_IA32_SYSENTER_EIP_INDEX                    0x6C12
#define VMCS_N_HOST_RSP_INDEX                                  0x6C14
#define VMCS_N_HOST_RIP_INDEX                                  0x6C16

typedef enum {
  VmExitReasonExceptionNmi,
  VmExitReasonExternalInterrupt,
  VmExitReasonTripleFault,
  VmExitReasonInit,
  VmExitReasonSipi,
  VmExitReasonIoSmi,
  VmExitReasonOtherSmi,
  VmExitReasonInterruptWindow,
  VmExitReasonNmiWindow,
  VmExitReasonTaskSwitch,
  VmExitReasonCpuid,
  VmExitReasonGetsec,
  VmExitReasonHlt,
  VmExitReasonInvd,
  VmExitReasonInvlpg,
  VmExitReasonRdpmc,
  VmExitReasonRdtsc,
  VmExitReasonRsm,
  VmExitReasonVmCall,
  VmExitReasonVmClear,
  VmExitReasonVmLaunch,
  VmExitReasonVmPtrld,
  VmExitReasonVmptrst,
  VmExitReasonVmRead,
  VmExitReasonVmResume,
  VmExitReasonVmWrite,
  VmExitReasonVmxOff,
  VmExitReasonVmxOn,
  VmExitReasonCrAccess,
  VmExitReasonMovDr,
  VmExitReasonIoInstruction,
  VmExitReasonRdmsr,
  VmExitReasonWrmsr,
  VmExitReasonVmEntryFailureDueToInvalidGuestState,
  VmExitReasonVmEntryFailureDueToMsrLoading,
  VmExitReason35,
  VmExitReasonMwait,
  VmExitReasonMonitorTrapFlag,
  VmExitReason38,
  VmExitReasonMonitor,
  VmExitReasonPause,
  VmExitReasonVmEntryFailureDueToMachineCheck,
  VmExitReason42,
  VmExitReasonTprBelowThreshold,
  VmExitReasonApicAccess,
  VmExitReason45,
  VmExitReasonAccessGdtrIdtr,
  VmExitReasonAccessLdtrTr,
  VmExitReasonEptViolation,
  VmExitReasonEptMisConfiguration,
  VmExitReasonInvEpt,
  VmExitReasonRdtscp,
  VmExitReasonVmxPreEmptionTimerExpired,
  VmExitReasonInvVpid,
  VmExitReasonWbinvd,
  VmExitReasonXsetbv,
  VmExitReason56,
  VmExitReasonRdrand,
  VmExitReasonInvpcid,
  VmExitReasonVmfunc,
  VmExitReasonMax,
} VMX_BASIC_EXIT_REASON;

typedef enum {
  VmxFailError0,
  VmxFailErrorVmCallExecutedInVmxRootOperation,
  VmxFailErrorVmClearWithInvalidPhysicalAddress,
  VmxFailErrorVmClearWithVmxOnPointer,
  VmxFailErrorVmLaunchWithNonClearVmcs,
  VmxFailErrorVmResumeWithNonLaunchedVmcs,
  VmxFailErrorVmResumeWithCorruptedVmcs,
  VmxFailErrorVmEntryWithInvalidControlField,
  VmxFailErrorVmEntryWithInvalidHostField,
  VmxFailErrorVmPtrldWithInvalidPhysicalAddress,
  VmxFailErrorVmPtrldWithVmxOnPointer,
  VmxFailErrorVmPtrldWithIncorrectVmcsRevisionIdentifier,
  VmxFailErrorVmReadVmWriteFromToUnsupportedVmcsComponent,
  VmxFailErrorVmWriteToReadOnlyVmcsComponent,
  VmxFailError14,
  VmxFailErrorVmxOnExecutedInVmxRootOperation,
  VmxFailErrorVmEntryWithInvalidExecutiveVmcsPointer,
  VmxFailErrorVmEntryWithNonLaunchedExecutive_VMCS,
  VmxFailErrorVmEntryWithExecutiveVmcsPointerNotVmxOnPointer,
  VmxFailErrorVmCallWithNonClearVmcs,
  VmxFailErrorVmCallWithInvalidVmExitControlFields,
  VmxFailError21,
  VmxFailErrorVmCallWithIncorrectMsegRevisionIdentifier,
  VmxFailErrorVmxOffUnderDualMonitorTreatmentOfSmisAndSmm,
  VmxFailErrorVmCallWithInvalidSmmMonitorFeatures,
  VmxFailErrorVmEntryWithInvalidVmExectionControlFieldsInExecutiveVmcs,
  VmxFailErrorVmEntryWithEventsBlockedByMovSs,
  VmxFailError27,
  VmxFailErrorInvalidOperandToInvEptInvVpid,
  VmxFailError29,
  VmxFailErrorMax,
} VMX_FAIL_ERROR_NUMBER;

#pragma pack (push, 1)

#define GUEST_ACTIVITY_STATE_ACTIVE         0
#define GUEST_ACTIVITY_STATE_HLT            1
#define GUEST_ACTIVITY_STATE_SHUTDOWN       2
#define GUEST_ACTIVITY_STATE_WAIT_FOR_SIPI  3

typedef struct {
    UINT32  BlockingBySti:1;
    UINT32  BlockingByMovSs:1;
    UINT32  BlockingBySmi:1;
    UINT32  BlockingByNmi:1;
    UINT32  Rsvd4To31:28;
} GUEST_INTERRUPTIBILITY_STATE_BITS;

typedef union {
  GUEST_INTERRUPTIBILITY_STATE_BITS Bits;
  UINT32                            Uint32;
} GUEST_INTERRUPTIBILITY_STATE;

typedef struct {
    UINT64  Bp0:1;
    UINT64  Bp1:1;
    UINT64  Bp2:1;
    UINT64  Bp3:1;
    UINT64  Rsvd4To11:8;
    UINT64  EnableBreakpoint:1;
    UINT64  Rsvd13:1;
    UINT64  Bs:1;
    UINT64  Rsvd15To63:49;
} PENDING_DEBUG_EXCEPTION_BITS;

typedef union {
  PENDING_DEBUG_EXCEPTION_BITS Bits;
  UINT64                       Uint64;
} PENDING_DEBUG_EXCEPTION;

typedef struct {
    UINT32 ExternalInterrupt:1;
    UINT32 Rsvd1To2:2;
    UINT32 Nmi:1;
    UINT32 Rsvd4:1;
    UINT32 VirtualNmi:1;
    UINT32 VmxPreemptionTimer:1;
    UINT32 Rsvd7To31:25;
} VM_EXEC_PIN_BASES_VMEXIT_CONTROLS_BITS;

typedef union {
  VM_EXEC_PIN_BASES_VMEXIT_CONTROLS_BITS Bits;
  UINT32                                 Uint32; 
} VM_EXEC_PIN_BASES_VMEXIT_CONTROLS;

typedef struct {
    UINT32 Rsvd0To1:2;
    UINT32 InterruptWindow:1;
    UINT32 TscOffsetting:1;
    UINT32 Rsvd4To6:3;
    UINT32 Hlt:1;
    UINT32 Rsvd8:1;
    UINT32 Invlpg:1;
    UINT32 Mwait:1;
    UINT32 Rdpmc:1;
    UINT32 Rdtsc:1;
    UINT32 Rsvd13To14:2;
    UINT32 Cr3Load:1;
    UINT32 Cr3Store:1;
    UINT32 Rsvd17To18:2;
    UINT32 Cr8Load:1;
    UINT32 Cr8Store:1;
    UINT32 TprShadow:1;
    UINT32 NmiWindow:1;
    UINT32 MovDr:1;
    UINT32 UnconditionalIo:1;
    UINT32 IoBitmap:1;
    UINT32 Rsvd26:1;
    UINT32 MonitorTrapFlag:1;
    UINT32 MsrBitmap:1;
    UINT32 Monitor:1;
    UINT32 Pause:1;
    UINT32 SecondaryControl:1;
} VM_EXEC_PROCESSOR_BASES_VMEXIT_CONTROLS_BITS;

typedef union {
  VM_EXEC_PROCESSOR_BASES_VMEXIT_CONTROLS_BITS Bits;
  UINT32                                       Uint32; 
} VM_EXEC_PROCESSOR_BASES_VMEXIT_CONTROLS;

typedef struct {
    UINT32 VirtualApic:1;
    UINT32 Ept:1;
    UINT32 DescriptorTable:1;
    UINT32 Rdtscp:1;
    UINT32 VirtualizeX2Apic:1;
    UINT32 Vpid:1;
    UINT32 Wbinvd:1;
    UINT32 UnrestrictedGuest:1;
    UINT32 Rsvd8To9:2;
    UINT32 PauseLoop:1;
    UINT32 RdrandExit:1;
    UINT32 EnableInvpcid:1;
    UINT32 EnableVmFunctions:1;
    UINT32 Rsvd14To31:18;
} VM_EXEC_2ND_PROCESSOR_BASES_VMEXIT_CONTROLS_BITS;

typedef union {
  VM_EXEC_2ND_PROCESSOR_BASES_VMEXIT_CONTROLS_BITS Bits;
  UINT32                                           Uint32; 
} VM_EXEC_2ND_PROCESSOR_BASES_VMEXIT_CONTROLS;

#define EPT_GAW_21BIT  0
#define EPT_GAW_30BIT  1
#define EPT_GAW_39BIT  2
#define EPT_GAW_48BIT  3
#define EPT_GAW_57BIT  4

typedef struct {
    UINT64  Etmt:3;      // EPT Table Memory Type
    UINT64  Gaw:3;       // Guest Address Width
    UINT64  Rsvd6To11:6;
    UINT64  Asr:52;      // Address space root
} EPT_POINTER_BITS;

typedef struct {
    UINT32  Etmt:3;      // EPT Table Memory Type
    UINT32  Gaw:3;       // Guest Address Width
    UINT32  Rsvd6To11:6;
    UINT32  AsrLo:20;
    UINT32  AsrHi;
} EPT_POINTER_BITS32;

typedef union {
  EPT_POINTER_BITS   Bits;
  UINT64             Uint64;
  EPT_POINTER_BITS32 Bits32;
} EPT_POINTER;

typedef struct {
    UINT64  Ra:1;
    UINT64  Wa:1;
    UINT64  Xa:1;
    UINT64  Emt:3;       // EPT Memory Type
    UINT64  Igmt:1;      // Ignore PAT Memory Type
    UINT64  Sp:1;        // Super page
    UINT64  Avail8To11:4;
    UINT64  Addr:40;
    UINT64  Avail52To63:12;
} EPT_ENTRY_BITS;

typedef struct {
    UINT32  Ra:1;
    UINT32  Wa:1;
    UINT32  Xa:1;
    UINT32  Emt:3;       // EPT Memory Type
    UINT32  Igmt:1;      // Ignore PAT Memory Type
    UINT32  Sp:1;        // Super page
    UINT32  Avail8To11:4;
    UINT32  AddrLo:20;
    UINT32  AddrHi:20;
    UINT32  Avail52To63:12;
} EPT_ENTRY_BITS32;

typedef union {
  EPT_ENTRY_BITS   Bits;
  UINT64           Uint64;
  EPT_ENTRY_BITS32 Bits32;
} EPT_ENTRY;

typedef struct {
    UINT32 Rsvd0To1:2;
    UINT32 SaveDebugControls:1;
    UINT32 Rsvd3To8:6;
    UINT32 Ia32eHost:1;
    UINT32 Rsvd10To11:2;
    UINT32 LoadIA32_PERF_GLOBAL_CTRL:1;
    UINT32 Rsvd13To14:2;
    UINT32 AcknowledgeInterrupt:1;
    UINT32 Rsvd16To17:2;
    UINT32 SaveIA32_PAT:1;
    UINT32 LoadIA32_PAT:1;
    UINT32 SaveIA32_EFER:1;
    UINT32 LoadIA32_EFER:1;
    UINT32 SaveVmxPreemptionTimerValue:1;
    UINT32 Rsvd23To31:9;
} VM_EXIT_CONTROLS_BITS;

typedef union {
  VM_EXIT_CONTROLS_BITS Bits;
  UINT32                Uint32; 
} VM_EXIT_CONTROLS;

typedef struct {
  UINT32  MsrIndex;
  UINT32  Reserved;
  UINT64  MsrData;
} VM_EXIT_MSR_ENTRY;

typedef struct {
    UINT32 Rsvd0To1:2;
    UINT32 LoadDebugControls:1;
    UINT32 Rsvd3To8:6;
    UINT32 Ia32eGuest:1;
    UINT32 EntryToSmm:1;
    UINT32 DeactivateDualMonitor:1;
    UINT32 Rsvd12:1;
    UINT32 LoadIA32_PERF_GLOBAL_CTRL:1;
    UINT32 LoadIA32_PAT:1;
    UINT32 LoadIA32_EFER:1;
    UINT32 Rsvd16To31:16;
} VM_ENTRY_CONTROLS_BITS;

typedef union {
  VM_ENTRY_CONTROLS_BITS Bits;
  UINT32                 Uint32; 
} VM_ENTRY_CONTROLS;

typedef struct {
    UINT32 Vector:8;
    UINT32 InterruptType:3;
    UINT32 DeliverErrorCode:1;
    UINT32 Rsvd12To30:19;
    UINT32 Valid:1;
} VM_ENTRY_CONTROL_INTERRUPT_BITS;

typedef union {
  VM_ENTRY_CONTROL_INTERRUPT_BITS Bits;
  UINT32                          Uint32; 
} VM_ENTRY_CONTROL_INTERRUPT;

typedef struct {
    UINT32 Reason:16;
    UINT32 Rsvd16To27:12;
    UINT32 PendingMtf:1;
    UINT32 FromVmxRootOperation:1;
    UINT32 Rsvd30:1;
    UINT32 VmEntryFail:1;
} VM_EXIT_INFO_BASIC_BITS;

typedef union {
  VM_EXIT_INFO_BASIC_BITS Bits;
  UINT32                  Uint32; 
} VM_EXIT_INFO_BASIC;

typedef struct {
    UINT32  Bp0:1;
    UINT32  Bp1:1;
    UINT32  Bp2:1;
    UINT32  Bp3:1;
    UINT32  Rsvd4To12:9;
    UINT32  Bd:1;
    UINT32  Bs:1;
    UINT32  Rsvd15To31:17;
} VM_EXIT_QUALIFICATION_DEBUG_EXCEPTION;

#define TASK_SWITCH_SOURCE_CALL      0
#define TASK_SWITCH_SOURCE_IRET      1
#define TASK_SWITCH_SOURCE_JMP       2
#define TASK_SWITCH_SOURCE_TASK_GATE 3
typedef struct {
    UINT32  Tss:16;
    UINT32  Rsvd16To29:14;
    UINT32  TaskType:2;
} VM_EXIT_QUALIFICATION_TASK_SWITCH;

typedef struct {
    UINT32  CrNum:4;
    UINT32  AccessType:2;
    UINT32  LmswType:1;
    UINT32  Rsvd7:1;
    UINT32  GpReg:4;
    UINT32  Rsvd12To15:4;
    UINT32  LmswSource:16;
} VM_EXIT_QUALIFICATION_CR_ACCESS;

typedef struct {
    UINT32  DrNum:3;
    UINT32  Rsvd3:1;
    UINT32  Direction:1;
    UINT32  Rsvd5To7:3;
    UINT32  GpReg:4;
    UINT32  Rsvd12To31:20;
} VM_EXIT_QUALIFICATION_MOV_DR;

typedef struct {
    UINT32  Size:3;
    UINT32  Direction:1;
    UINT32  String:1;
    UINT32  Rep:1;
    UINT32  Operand:1;
    UINT32  Rsvd7To15:9;
    UINT32  PortNum:16;
} VM_EXIT_QUALIFICATION_IO_INSTRUCTION;

typedef struct {
    UINT32  Offset:12;
    UINT32  AccessType:4;
    UINT32  Rsvd15To31:16;
} VM_EXIT_QUALIFICATION_APIC_ACCESS;

typedef struct {
    UINT32  Ra:1;
    UINT32  Wa:1;
    UINT32  Xa:1;
    UINT32  EptR:1;
    UINT32  EptW:1;
    UINT32  EptX:1;
    UINT32  Rsvd6:1;
    UINT32  GlaValid:1;
    UINT32  Gpa:1;
    UINT32  Rsvd9To11:3;
    UINT32  NmiUnblockingDueToIret:1;
    UINT32  Rsvd13To31:19;
} VM_EXIT_QUALIFICATION_EPT_VIOLATION;

typedef union {
  VM_EXIT_QUALIFICATION_DEBUG_EXCEPTION DebugExceptions;
  VM_EXIT_QUALIFICATION_TASK_SWITCH     TaskSwitch;
  VM_EXIT_QUALIFICATION_CR_ACCESS       CrAccess;
  VM_EXIT_QUALIFICATION_MOV_DR          MovDr;
  VM_EXIT_QUALIFICATION_IO_INSTRUCTION  IoInstruction;
  VM_EXIT_QUALIFICATION_APIC_ACCESS     ApicAccess;
  VM_EXIT_QUALIFICATION_EPT_VIOLATION   EptViolation;
  UINTN                                 UintN;
} VM_EXIT_QUALIFICATION;

#define VM_EXIT_INSTRUCTION_ADDRESS_SIZE_16 0
#define VM_EXIT_INSTRUCTION_ADDRESS_SIZE_32 1
#define VM_EXIT_INSTRUCTION_ADDRESS_SIZE_64 2
#define VM_EXIT_INSTRUCTION_SEGMENT_ES      0
#define VM_EXIT_INSTRUCTION_SEGMENT_CS      1
#define VM_EXIT_INSTRUCTION_SEGMENT_SS      2
#define VM_EXIT_INSTRUCTION_SEGMENT_DS      3
#define VM_EXIT_INSTRUCTION_SEGMENT_FS      4
#define VM_EXIT_INSTRUCTION_SEGMENT_GS      5

typedef struct {
    UINT32  Rsvd0To6:7;
    UINT32  Size:3;
    UINT32  Rsvd10To14:5;
    UINT32  Segment:3;
    UINT32  Rsvd18To31:14;
} VM_EXIT_INSTRUCTION_INFORMATION_BITS;

typedef union {
  VM_EXIT_INSTRUCTION_INFORMATION_BITS InOuts;
  UINT32                               Uint32;
} VM_EXIT_INSTRUCTION_INFORMATION;

#define INTERRUPT_TYPE_EXTERNAL_EXTERNAL_INTERRUPT              0
#define INTERRUPT_TYPE_EXTERNAL_NMI                             2
#define INTERRUPT_TYPE_EXTERNAL_HARDWARE_EXCEPTION              3
#define INTERRUPT_TYPE_EXTERNAL_SOFTWARE_INTERRUPT              4
#define INTERRUPT_TYPE_EXTERNAL_PRIVILEDGED_SOFTWARE_EXCEPTION  5
#define INTERRUPT_TYPE_EXTERNAL_SOFTWARE_EXCEPTIONT             6
#define INTERRUPT_TYPE_OTHER_EVENT                              7

typedef struct {
    UINT32 Vector:8;
    UINT32 InterruptType:3;
    UINT32 ErrorCodeValid:1;
    UINT32 NmiUnblockingDueToIret:1;
    UINT32 Rsvd13To30:18;
    UINT32 Valid:1;
} VM_EXIT_INFO_INTERRUPTION_BITS;

typedef union {
  VM_EXIT_INFO_INTERRUPTION_BITS Bits;
  UINT32                         Uint32; 
} VM_EXIT_INFO_INTERRUPTION;

typedef struct {
    UINT32 Vector:8;
    UINT32 InterruptType:3;
    UINT32 ErrorCodeValid:1;
    UINT32 Rsvd12To30:19;
    UINT32 Valid:1;
} VM_EXIT_INFO_IDT_VECTORING_BITS;

typedef union {
  VM_EXIT_INFO_IDT_VECTORING_BITS Bits;
  UINT32                          Uint32; 
} VM_EXIT_INFO_IDT_VECTORING;

typedef struct {
    UINT32  RevisionIdentifier:32;
    UINT32  VmcsRegionSize:13;
    UINT32  Rsvd45To47:3;
    UINT32  VmcsAddrWidth:1;
    UINT32  DualMonitorTreatment:1;
    UINT32  VmcsMemoryType:4;
    UINT32  VmExitInstructionInformation:1;
    UINT32  VmxControlDefaultClear:1;
    UINT32  Rsvd56To63:8;
} IA32_VMX_BASIC_MSR_BITS;

typedef union {
  IA32_VMX_BASIC_MSR_BITS Bits;
  UINT64                  Uint64;
} IA32_VMX_BASIC_MSR;

typedef struct {
    UINT32  VmxPreemptionTimerRate:5;
    UINT32  Ia32eToEFER:1;
    UINT32  ActivityStateSupportHlt:1;
    UINT32  ActivityStateSupportShutdown:1;
    UINT32  ActivityStateSupportWaitForSipi:1;
    UINT32  Rsvd9To15:7;
    UINT32  NumberOfCr3TargetValue:9;
    UINT32  MaxNumberOfMsrInStoreList:3;
    UINT32  VmxOffUnblockSmiSupport:1;
    UINT32  Rsvd29To31:3;
    UINT32  MsegRevisionIdentifier:32;
} IA32_VMX_MISC_MSR_BITS;

typedef union {
  IA32_VMX_MISC_MSR_BITS Bits;
  UINT64                 Uint64;
} IA32_VMX_MISC_MSR;

#pragma pack (pop)

/**

  This function read UINT16 data from VMCS region.

  @param Index VMCS region index

  @return VMCS region value

**/
UINT16
VmRead16 (
  IN UINT32  Index
  );

/**

  This function read UINT32 data from VMCS region.

  @param Index VMCS region index

  @return VMCS region value

**/
UINT32
VmRead32 (
  IN UINT32  Index
  );

/**

  This function read UINT64 data from VMCS region.

  @param Index VMCS region index

  @return VMCS region value

**/
UINT64
VmRead64 (
  IN UINT32  Index
  );

/**

  This function read UINTN data from VMCS region.

  @param Index VMCS region index

  @return VMCS region value

**/
UINTN
VmReadN (
  IN UINT32  Index
  );

/**

  This function write UINN16 data to VMCS region.

  @param Index VMCS region index
  @param Data  VMCS region value

**/
VOID
VmWrite16 (
  IN UINT32  Index,
  IN UINT16  Data
  );

/**

  This function write UINN32 data to VMCS region.

  @param Index VMCS region index
  @param Data  VMCS region value

**/
VOID
VmWrite32 (
  IN UINT32  Index,
  IN UINT32  Data
  );

/**

  This function write UINN64 data to VMCS region.

  @param Index VMCS region index
  @param Data  VMCS region value

**/
VOID
VmWrite64 (
  IN UINT32  Index,
  IN UINT64  Data
  );

/**

  This function write UINNN data to VMCS region.

  @param Index VMCS region index
  @param Data  VMCS region value

**/
VOID
VmWriteN (
  IN UINT32  Index,
  IN UINTN   Data
  );

/**

  This function enter VMX.

  @param Vmcs  VMCS pointer

  @return RFLAGS if VmxOn fail

**/
UINTN
AsmVmxOn (
  IN UINT64 *Vmcs
  );

/**

  This function leave VMX.

  @return RFLAGS if VmxOff fail

**/
UINTN
AsmVmxOff (
  VOID
  );

/**

  This function clear VMCS.

  @param Vmcs  VMCS pointer

  @return RFLAGS if VmClear fail

**/
UINTN
AsmVmClear (
  IN UINT64 *Vmcs
  );

/**

  This function store VMCS.

  @param Vmcs  VMCS pointer

  @return RFLAGS if VmPtrStore fail

**/
UINTN
AsmVmPtrStore (
  IN UINT64 *Vmcs
  );

/**

  This function load VMCS.

  @param Vmcs  VMCS pointer

  @return RFLAGS if VmPtrLoad fail

**/
UINTN
AsmVmPtrLoad (
  IN UINT64 *Vmcs
  );

/**

  This function launch VM.

  @param Register  General purpose register set

  @return RFLAGS if VmLaunch fail

**/
UINTN
AsmVmLaunch (
  IN X86_REGISTER *Register
  );

/**

  This function resume to VM.

  @param Register  General purpose register set

  @return RFLAGS if VmResume fail

**/
UINTN
AsmVmResume (
  IN X86_REGISTER *Register
  );

/**

  This function read VMCS region.

  @param Index VMCS region index
  @param Data  VMCS region value

  @return RFLAGS if VmRead fail

**/
UINTN
AsmVmRead (
  IN UINT32  Index,
  OUT UINTN  *Data
  );

/**

  This function write VMCS region.

  @param Index VMCS region index
  @param Data  VMCS region value

  @return RFLAGS if VmWrite fail

**/
UINTN
AsmVmWrite (
  IN UINT32  Index,
  IN UINTN   Data
  );

typedef struct {
  UINT64 Lo;
  UINT64 Hi;
} UINT_128;

#define  INVEPT_TYPE_SINGLE_CONTEXT_INVALIDATION   1
#define  INVEPT_TYPE_GLOBAL_INVALIDATION           2

/**

  This function invalidate EPT TLB.

  @param Type  INVEPT type
  @param Addr  INVEPT desciptor

  @return RFLAGS if InvEpt fail

**/
UINTN
AsmInvEpt (
  IN UINTN    Type,
  IN UINT_128 *Addr
  );

#define  INVVPID_TYPE_INDIVIDUAL_ADDRESS_INVALIDATION                           1
#define  INVVPID_TYPE_SINGLE_CONTEXT_INVALIDATION                               2
#define  INVVPID_TYPE_ALL_CONTEXTS_INVALIDATION                                 3
#define  INVVPID_TYPE_SINGLE_CONTEXT_INVALIDATION_RETAINING_GLOBAL_TRANSLATION  4

/**

  This function invalidate VPID.

  @param Type  INVVPID type
  @param Addr  INVVPID desciptor

  @return RFLAGS if InvVpid fail

**/
UINTN
AsmInvVpid (
  IN UINTN    Type,
  IN UINT_128 *Addr
  );

/**

  This function invoke VMCALL with context.

  @param Eax   EAX register
  @param Ebx   EBX register
  @param Ecx   ECX register
  @param Edx   EDX register

  @return EAX register

**/
UINT32
AsmVmCall (
  IN UINT32  Eax,
  IN UINT32  Ebx,
  IN UINT32  Ecx,
  IN UINT32  Edx
  );

#endif
