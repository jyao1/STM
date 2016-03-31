/** @file
  Dump STM information

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Stm.h"

typedef struct {
  UINT32  Index;
  CHAR8   *Str;
} DATA_STR;

GLOBAL_REMOVE_IF_UNREFERENCED
DATA_STR mVmxCapabilityMsrStr[] = {
  {IA32_VMX_BASIC_MSR_INDEX,           "VMX_BASIC_MSR            "},
  {IA32_VMX_PINBASED_CTLS_MSR_INDEX,   "VMX_PINBASED_CTLS_MSR    "},
  {IA32_VMX_PROCBASED_CTLS_MSR_INDEX,  "VMX_PROCBASED_CTLS_MSR   "},
  {IA32_VMX_EXIT_CTLS_MSR_INDEX,       "VMX_EXIT_CTLS_MSR        "},
  {IA32_VMX_ENTRY_CTLS_MSR_INDEX,      "VMX_ENTRY_CTLS_MSR       "},
  {IA32_VMX_MISC_MSR_INDEX,            "VMX_MISC_MSR             "},
  {IA32_VMX_CR0_FIXED0_MSR_INDEX,      "VMX_CR0_FIXED0_MSR       "},
  {IA32_VMX_CR0_FIXED1_MSR_INDEX,      "VMX_CR0_FIXED1_MSR       "},
  {IA32_VMX_CR4_FIXED0_MSR_INDEX,      "VMX_CR4_FIXED0_MSR       "},
  {IA32_VMX_CR4_FIXED1_MSR_INDEX,      "VMX_CR4_FIXED1_MSR       "},
  {IA32_VMX_VMCS_ENUM_MSR_INDEX,       "VMX_VMCS_ENUM_MSR        "},
  {IA32_VMX_PROCBASED_CTLS2_MSR_INDEX, "VMX_PROCBASED_CTLS2_MSR  "},
  {IA32_VMX_EPT_VPID_CAP_MSR_INDEX,    "VMX_EPT_VPID_CAP_MSR     "},
};

/**

  This function dump VMX capability MSR.

**/
VOID
DumpVmxCapabillityMsr (
  VOID
  )
{
  UINT64  Data64;
  UINT32  Index;

  for (Index = 0; Index < sizeof(mVmxCapabilityMsrStr)/sizeof(mVmxCapabilityMsrStr[0]); Index++) {
    Data64 = AsmReadMsr64 (mVmxCapabilityMsrStr[Index].Index);
    DEBUG ((EFI_D_INFO, "%a: %016lx\n", mVmxCapabilityMsrStr[Index].Str, Data64));
  }
}

/**

  This function dump VMCS UINT16 field.

  @param DataStr VMCS data structure

**/
VOID
DumpVmcs16Filed (
  IN DATA_STR  *DataStr
  )
{
  DEBUG ((EFI_D_INFO, "%a: %04x\n", DataStr->Str, (UINTN)VmRead16 (DataStr->Index)));
}

/**

  This function dump VMCS UINT64 field.

  @param DataStr VMCS data structure

**/
VOID
DumpVmcs64Filed (
  IN DATA_STR  *DataStr
  )
{
  DEBUG ((EFI_D_INFO, "%a: %016lx\n", DataStr->Str, VmRead64 (DataStr->Index)));
}

/**

  This function dump VMCS UINT32 field.

  @param DataStr VMCS data structure

**/
VOID
DumpVmcs32Filed (
  IN DATA_STR  *DataStr
  )
{
  DEBUG ((EFI_D_INFO, "%a: %08x\n", DataStr->Str, (UINTN)VmRead32 (DataStr->Index)));
}

/**

  This function dump VMCS UINTN field.

  @param DataStr VMCS data structure

**/
VOID
DumpVmcsNFiled (
  IN DATA_STR  *DataStr
  )
{
  if (sizeof(UINTN) == sizeof(UINT64)) {
    DumpVmcs64Filed (DataStr);
  } else {
    DumpVmcs32Filed (DataStr);
  }
}

GLOBAL_REMOVE_IF_UNREFERENCED
DATA_STR mVmcs16ControlFiledStr[] = {
  {VMCS_16_CONTROL_VPID_INDEX,                             "VMCS_16_CONTROL_VPID                            "},
};

GLOBAL_REMOVE_IF_UNREFERENCED
DATA_STR mVmcs64ControlFiledStr[] = {
  {VMCS_64_CONTROL_IO_BITMAP_A_INDEX,                      "VMCS_64_CONTROL_IO_BITMAP_A                     "},
  {VMCS_64_CONTROL_IO_BITMAP_B_INDEX,                      "VMCS_64_CONTROL_IO_BITMAP_B                     "},
  {VMCS_64_CONTROL_MSR_BITMAP_INDEX,                       "VMCS_64_CONTROL_MSR_BITMAP                      "},
  {VMCS_64_CONTROL_VMEXIT_MSR_STORE_INDEX,                 "VMCS_64_CONTROL_VMEXIT_MSR_STORE                "},
  {VMCS_64_CONTROL_VMEXIT_MSR_LOAD_INDEX,                  "VMCS_64_CONTROL_VMEXIT_MSR_LOAD                 "},
  {VMCS_64_CONTROL_VMENTRY_MSR_LOAD_INDEX,                 "VMCS_64_CONTROL_VMENTRY_MSR_LOAD                "},
  {VMCS_64_CONTROL_EXECUTIVE_VMCS_PTR_INDEX,               "VMCS_64_CONTROL_EXECUTIVE_VMCS_PTR              "},
  {VMCS_64_CONTROL_TSC_OFFSET_INDEX,                       "VMCS_64_CONTROL_TSC_OFFSET                      "},
  {VMCS_64_CONTROL_VIRTUAL_APIC_ADDR_INDEX,                "VMCS_64_CONTROL_VIRTUAL_APIC_ADDR               "},
  {VMCS_64_CONTROL_APIC_ACCESS_ADDR_INDEX,                 "VMCS_64_CONTROL_APIC_ACCESS_ADDR                "},
  {VMCS_64_CONTROL_EPT_PTR_INDEX,                          "VMCS_64_CONTROL_EPT_PTR                         "},
};

GLOBAL_REMOVE_IF_UNREFERENCED
DATA_STR mVmcs32ControlFiledStr[] = {
  {VMCS_32_CONTROL_PIN_BASED_VM_EXECUTION_INDEX,           "VMCS_32_CONTROL_PIN_BASED_VM_EXECUTION          "},
  {VMCS_32_CONTROL_PROCESSOR_BASED_VM_EXECUTION_INDEX,     "VMCS_32_CONTROL_PROCESSOR_BASED_VM_EXECUTION    "},
  {VMCS_32_CONTROL_EXCEPTION_BITMAP_INDEX,                 "VMCS_32_CONTROL_EXCEPTION_BITMAP                "},
  {VMCS_32_CONTROL_PAGE_FAULT_ERROR_CODE_MASK_INDEX,       "VMCS_32_CONTROL_PAGE_FAULT_ERROR_CODE_MASK      "},
  {VMCS_32_CONTROL_PAGE_FAULT_ERROR_CODE_MATCH_INDEX,      "VMCS_32_CONTROL_PAGE_FAULT_ERROR_CODE_MATCH     "},
  {VMCS_32_CONTROL_CR3_TARGET_COUNT_INDEX,                 "VMCS_32_CONTROL_CR3_TARGET_COUNT                "},
  {VMCS_32_CONTROL_VMEXIT_CONTROLS_INDEX,                  "VMCS_32_CONTROL_VMEXIT_CONTROLS                 "},
  {VMCS_32_CONTROL_VMEXIT_MSR_STORE_COUNT_INDEX,           "VMCS_32_CONTROL_VMEXIT_MSR_STORE_COUNT          "},
  {VMCS_32_CONTROL_VMEXIT_MSR_LOAD_COUNT_INDEX,            "VMCS_32_CONTROL_VMEXIT_MSR_LOAD_COUNT           "},
  {VMCS_32_CONTROL_VMENTRY_CONTROLS_INDEX,                 "VMCS_32_CONTROL_VMENTRY_CONTROLS                "},
  {VMCS_32_CONTROL_VMENTRY_MSR_LOAD_COUNT_INDEX,           "VMCS_32_CONTROL_VMENTRY_MSR_LOAD_COUNT          "},
  {VMCS_32_CONTROL_VMENTRY_INTERRUPTION_INFO_INDEX,        "VMCS_32_CONTROL_VMENTRY_INTERRUPTION_INFO       "},
  {VMCS_32_CONTROL_VMENTRY_EXCEPTION_ERROR_CODE_INDEX,     "VMCS_32_CONTROL_VMENTRY_EXCEPTION_ERROR_CODE    "},
  {VMCS_32_CONTROL_VMENTRY_INSTRUCTION_LENGTH_INDEX,       "VMCS_32_CONTROL_VMENTRY_INSTRUCTION_LENGTH      "},
  {VMCS_32_CONTROL_TPR_THRESHOLD_INDEX,                    "VMCS_32_CONTROL_TPR_THRESHOLD                   "},
  {VMCS_32_CONTROL_2ND_PROCESSOR_BASED_VM_EXECUTION_INDEX, "VMCS_32_CONTROL_2ND_PROCESSOR_BASED_VM_EXECUTION"},
  {VMCS_32_CONTROL_PLE_GAP_INDEX,                          "VMCS_32_CONTROL_PLE_GAP                         "},
  {VMCS_32_CONTROL_PLE_WINDOW_INDEX,                       "VMCS_32_CONTROL_PLE_WINDOW                      "},
};

GLOBAL_REMOVE_IF_UNREFERENCED
DATA_STR mVmcsNControlFiledStr[] = {
  {VMCS_N_CONTROL_CR0_GUEST_HOST_MASK_INDEX,               "VMCS_N_CONTROL_CR0_GUEST_HOST_MASK              "},
  {VMCS_N_CONTROL_CR4_GUEST_HOST_MASK_INDEX,               "VMCS_N_CONTROL_CR4_GUEST_HOST_MASK              "},
  {VMCS_N_CONTROL_CR0_READ_SHADOW_INDEX,                   "VMCS_N_CONTROL_CR0_READ_SHADOW                  "},
  {VMCS_N_CONTROL_CR4_READ_SHADOW_INDEX,                   "VMCS_N_CONTROL_CR4_READ_SHADOW                  "},
  {VMCS_N_CONTROL_CR3_TARGET_VALUE0_INDEX,                 "VMCS_N_CONTROL_CR3_TARGET_VALUE0                "},
  {VMCS_N_CONTROL_CR3_TARGET_VALUE1_INDEX,                 "VMCS_N_CONTROL_CR3_TARGET_VALUE1                "},
  {VMCS_N_CONTROL_CR3_TARGET_VALUE2_INDEX,                 "VMCS_N_CONTROL_CR3_TARGET_VALUE2                "},
  {VMCS_N_CONTROL_CR3_TARGET_VALUE3_INDEX,                 "VMCS_N_CONTROL_CR3_TARGET_VALUE3                "},
};

/**

  This function dump VMCS control field.

**/
VOID
DumpVmcsControlField (
  VOID
  )
{
  UINT32  Index;

  for (Index = 0; Index < sizeof(mVmcs16ControlFiledStr)/sizeof(mVmcs16ControlFiledStr[0]); Index++) {
    DumpVmcs16Filed (&mVmcs16ControlFiledStr[Index]);
  }

  for (Index = 0; Index < sizeof(mVmcs64ControlFiledStr)/sizeof(mVmcs64ControlFiledStr[0]); Index++) {
    DumpVmcs64Filed (&mVmcs64ControlFiledStr[Index]);
  }

  for (Index = 0; Index < sizeof(mVmcs32ControlFiledStr)/sizeof(mVmcs32ControlFiledStr[0]); Index++) {
    DumpVmcs32Filed (&mVmcs32ControlFiledStr[Index]);
  }

  for (Index = 0; Index < sizeof(mVmcsNControlFiledStr)/sizeof(mVmcsNControlFiledStr[0]); Index++) {
    DumpVmcsNFiled (&mVmcsNControlFiledStr[Index]);
  }
}

GLOBAL_REMOVE_IF_UNREFERENCED
DATA_STR mVmcs64ReadOnlyFiledStr[] = {
  {VMCS_64_RO_GUEST_PHYSICAL_ADDR_INDEX,                   "VMCS_64_RO_GUEST_PHYSICAL_ADDR                  "},
};

GLOBAL_REMOVE_IF_UNREFERENCED
DATA_STR mVmcs32ReadOnlyFiledStr[] = {
  {VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX,                  "VMCS_32_RO_VM_INSTRUCTION_ERROR                 "},
  {VMCS_32_RO_EXIT_REASON_INDEX,                           "VMCS_32_RO_EXIT_REASON                          "},
  {VMCS_32_RO_VMEXIT_INTERRUPTION_INFO_INDEX,              "VMCS_32_RO_VMEXIT_INTERRUPTION_INFO             "},
  {VMCS_32_RO_VMEXIT_INTERRUPTION_ERROR_CODE_INDEX,        "VMCS_32_RO_VMEXIT_INTERRUPTION_ERROR_CODE       "},
  {VMCS_32_RO_IDT_VECTORING_INFO_INDEX,                    "VMCS_32_RO_IDT_VECTORING_INFO                   "},
  {VMCS_32_RO_IDT_VECTORING_ERROR_CODE_INDEX,              "VMCS_32_RO_IDT_VECTORING_ERROR_CODE             "},
  {VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX,             "VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH            "},
  {VMCS_32_RO_VMEXIT_INSTRUCTION_INFO_INDEX,               "VMCS_32_RO_VMEXIT_INSTRUCTION_INFO              "},
};

GLOBAL_REMOVE_IF_UNREFERENCED
DATA_STR mVmcsNReadOnlyFiledStr[] = {
  {VMCS_N_RO_EXIT_QUALIFICATION_INDEX,                     "VMCS_N_RO_EXIT_QUALIFICATION                    "},
  {VMCS_N_RO_IO_RCX_INDEX,                                 "VMCS_N_RO_IO_RCX                                "},
  {VMCS_N_RO_IO_RSI_INDEX,                                 "VMCS_N_RO_IO_RSI                                "},
  {VMCS_N_RO_IO_RDI_INDEX,                                 "VMCS_N_RO_IO_RDI                                "},
  {VMCS_N_RO_IO_RIP_INDEX,                                 "VMCS_N_RO_IO_RIP                                "},
  {VMCS_N_RO_GUEST_LINEAR_ADDR_INDEX,                      "VMCS_N_RO_GUEST_LINEAR_ADDR                     "},
};

/**

  This function dump VMCS read-only field.

**/
VOID
DumpVmcsReadOnlyField (
  VOID
  )
{
  UINT32  Index;

  for (Index = 0; Index < sizeof(mVmcs64ReadOnlyFiledStr)/sizeof(mVmcs64ReadOnlyFiledStr[0]); Index++) {
    DumpVmcs64Filed (&mVmcs64ReadOnlyFiledStr[Index]);
  }

  for (Index = 0; Index < sizeof(mVmcs32ReadOnlyFiledStr)/sizeof(mVmcs32ReadOnlyFiledStr[0]); Index++) {
    DumpVmcs32Filed (&mVmcs32ReadOnlyFiledStr[Index]);
  }

  for (Index = 0; Index < sizeof(mVmcsNReadOnlyFiledStr)/sizeof(mVmcsNReadOnlyFiledStr[0]); Index++) {
    DumpVmcsNFiled (&mVmcsNReadOnlyFiledStr[Index]);
  }
}

GLOBAL_REMOVE_IF_UNREFERENCED
DATA_STR mVmcs16GuestFiledStr[] = {
  {VMCS_16_GUEST_ES_INDEX,                                 "VMCS_16_GUEST_ES                                "},
  {VMCS_16_GUEST_CS_INDEX,                                 "VMCS_16_GUEST_CS                                "},
  {VMCS_16_GUEST_SS_INDEX,                                 "VMCS_16_GUEST_SS                                "},
  {VMCS_16_GUEST_DS_INDEX,                                 "VMCS_16_GUEST_DS                                "},
  {VMCS_16_GUEST_FS_INDEX,                                 "VMCS_16_GUEST_FS                                "},
  {VMCS_16_GUEST_GS_INDEX,                                 "VMCS_16_GUEST_GS                                "},
  {VMCS_16_GUEST_LDTR_INDEX,                               "VMCS_16_GUEST_LDTR                              "},
  {VMCS_16_GUEST_TR_INDEX,                                 "VMCS_16_GUEST_TR                                "},
};

GLOBAL_REMOVE_IF_UNREFERENCED
DATA_STR mVmcs64GuestFiledStr[] = {
  {VMCS_64_GUEST_VMCS_LINK_PTR_INDEX,                      "VMCS_64_GUEST_VMCS_LINK_PTR                     "},
  {VMCS_64_GUEST_IA32_DEBUGCTL_INDEX,                      "VMCS_64_GUEST_IA32_DEBUGCTL                     "},
  {VMCS_64_GUEST_IA32_PAT_INDEX,                           "VMCS_64_GUEST_IA32_PAT                          "},
  {VMCS_64_GUEST_IA32_EFER_INDEX,                          "VMCS_64_GUEST_IA32_EFER                         "},
  {VMCS_64_GUEST_IA32_PERF_GLOBAL_CTRL_INDEX,              "VMCS_64_GUEST_IA32_PERF_GLOBAL_CTRL             "},
  {VMCS_64_GUEST_PDPTE0_INDEX,                             "VMCS_64_GUEST_PDPTE0                            "},
  {VMCS_64_GUEST_PDPTE1_INDEX,                             "VMCS_64_GUEST_PDPTE1                            "},
  {VMCS_64_GUEST_PDPTE2_INDEX,                             "VMCS_64_GUEST_PDPTE2                            "},
  {VMCS_64_GUEST_PDPTE3_INDEX,                             "VMCS_64_GUEST_PDPTE3                            "},
};

GLOBAL_REMOVE_IF_UNREFERENCED
DATA_STR mVmcs32GuestFiledStr[] = {
  {VMCS_32_GUEST_ES_LIMIT_INDEX,                           "VMCS_32_GUEST_ES_LIMIT                          "},
  {VMCS_32_GUEST_CS_LIMIT_INDEX,                           "VMCS_32_GUEST_CS_LIMIT                          "},
  {VMCS_32_GUEST_SS_LIMIT_INDEX,                           "VMCS_32_GUEST_SS_LIMIT                          "},
  {VMCS_32_GUEST_DS_LIMIT_INDEX,                           "VMCS_32_GUEST_DS_LIMIT                          "},
  {VMCS_32_GUEST_FS_LIMIT_INDEX,                           "VMCS_32_GUEST_FS_LIMIT                          "},
  {VMCS_32_GUEST_GS_LIMIT_INDEX,                           "VMCS_32_GUEST_GS_LIMIT                          "},
  {VMCS_32_GUEST_LDTR_LIMIT_INDEX,                         "VMCS_32_GUEST_LDTR_LIMIT                        "},
  {VMCS_32_GUEST_TR_LIMIT_INDEX,                           "VMCS_32_GUEST_TR_LIMIT                          "},
  {VMCS_32_GUEST_GDTR_LIMIT_INDEX,                         "VMCS_32_GUEST_GDTR_LIMIT                        "},
  {VMCS_32_GUEST_IDTR_LIMIT_INDEX,                         "VMCS_32_GUEST_IDTR_LIMIT                        "},
  {VMCS_32_GUEST_ES_ACCESS_RIGHT_INDEX,                    "VMCS_32_GUEST_ES_ACCESS_RIGHT                   "},
  {VMCS_32_GUEST_CS_ACCESS_RIGHT_INDEX,                    "VMCS_32_GUEST_CS_ACCESS_RIGHT                   "},
  {VMCS_32_GUEST_SS_ACCESS_RIGHT_INDEX,                    "VMCS_32_GUEST_SS_ACCESS_RIGHT                   "},
  {VMCS_32_GUEST_DS_ACCESS_RIGHT_INDEX,                    "VMCS_32_GUEST_DS_ACCESS_RIGHT                   "},
  {VMCS_32_GUEST_FS_ACCESS_RIGHT_INDEX,                    "VMCS_32_GUEST_FS_ACCESS_RIGHT                   "},
  {VMCS_32_GUEST_GS_ACCESS_RIGHT_INDEX,                    "VMCS_32_GUEST_GS_ACCESS_RIGHT                   "},
  {VMCS_32_GUEST_LDTR_ACCESS_RIGHT_INDEX,                  "VMCS_32_GUEST_LDTR_ACCESS_RIGHT                 "},
  {VMCS_32_GUEST_TR_ACCESS_RIGHT_INDEX,                    "VMCS_32_GUEST_TR_ACCESS_RIGHT                   "},
  {VMCS_32_GUEST_INTERRUPTIBILITY_STATE_INDEX,             "VMCS_32_GUEST_INTERRUPTIBILITY_STATE            "},
  {VMCS_32_GUEST_ACTIVITY_STATE_INDEX,                     "VMCS_32_GUEST_ACTIVITY_STATE                    "},
  {VMCS_32_GUEST_SMBASE_INDEX,                             "VMCS_32_GUEST_SMBASE                            "},
  {VMCS_32_GUEST_IA32_SYSENTER_CS_INDEX,                   "VMCS_32_GUEST_IA32_SYSENTER_CS                  "},
  {VMCS_32_GUEST_VMX_PREEMPTION_TIMER_VALUE_INDEX,         "VMCS_32_GUEST_VMX_PREEMPTION_TIMER_VALUE        "},
};

GLOBAL_REMOVE_IF_UNREFERENCED
DATA_STR mVmcsNGuestFiledStr[] = {
  {VMCS_N_GUEST_CR0_INDEX,                                 "VMCS_N_GUEST_CR0                                "},
  {VMCS_N_GUEST_CR3_INDEX,                                 "VMCS_N_GUEST_CR3                                "},
  {VMCS_N_GUEST_CR4_INDEX,                                 "VMCS_N_GUEST_CR4                                "},
  {VMCS_N_GUEST_ES_BASE_INDEX,                             "VMCS_N_GUEST_ES_BASE                            "},
  {VMCS_N_GUEST_CS_BASE_INDEX,                             "VMCS_N_GUEST_CS_BASE                            "},
  {VMCS_N_GUEST_SS_BASE_INDEX,                             "VMCS_N_GUEST_SS_BASE                            "},
  {VMCS_N_GUEST_DS_BASE_INDEX,                             "VMCS_N_GUEST_DS_BASE                            "},
  {VMCS_N_GUEST_FS_BASE_INDEX,                             "VMCS_N_GUEST_FS_BASE                            "},
  {VMCS_N_GUEST_GS_BASE_INDEX,                             "VMCS_N_GUEST_GS_BASE                            "},
  {VMCS_N_GUEST_LDTR_BASE_INDEX,                           "VMCS_N_GUEST_LDTR_BASE                          "},
  {VMCS_N_GUEST_TR_BASE_INDEX,                             "VMCS_N_GUEST_TR_BASE                            "},
  {VMCS_N_GUEST_GDTR_BASE_INDEX,                           "VMCS_N_GUEST_GDTR_BASE                          "},
  {VMCS_N_GUEST_IDTR_BASE_INDEX,                           "VMCS_N_GUEST_IDTR_BASE                          "},
  {VMCS_N_GUEST_DR7_INDEX,                                 "VMCS_N_GUEST_DR7                                "},
  {VMCS_N_GUEST_RSP_INDEX,                                 "VMCS_N_GUEST_RSP                                "},
  {VMCS_N_GUEST_RIP_INDEX,                                 "VMCS_N_GUEST_RIP                                "},
  {VMCS_N_GUEST_RFLAGS_INDEX,                              "VMCS_N_GUEST_RFLAGS                             "},
  {VMCS_N_GUEST_PENDING_DEBUG_EXCEPTIONS_INDEX,            "VMCS_N_GUEST_PENDING_DEBUG_EXCEPTIONS           "},
  {VMCS_N_GUEST_IA32_SYSENTER_ESP_INDEX,                   "VMCS_N_GUEST_IA32_SYSENTER_ESP                  "},
  {VMCS_N_GUEST_IA32_SYSENTER_EIP_INDEX,                   "VMCS_N_GUEST_IA32_SYSENTER_EIP                  "},
};

/**

  This function dump VMCS guest field.

**/
VOID
DumpVmcsGuestField (
  VOID
  )
{
  UINT32  Index;

  for (Index = 0; Index < sizeof(mVmcs16GuestFiledStr)/sizeof(mVmcs16GuestFiledStr[0]); Index++) {
    DumpVmcs16Filed (&mVmcs16GuestFiledStr[Index]);
  }

  for (Index = 0; Index < sizeof(mVmcs64GuestFiledStr)/sizeof(mVmcs64GuestFiledStr[0]); Index++) {
    DumpVmcs64Filed (&mVmcs64GuestFiledStr[Index]);
  }

  for (Index = 0; Index < sizeof(mVmcs32GuestFiledStr)/sizeof(mVmcs32GuestFiledStr[0]); Index++) {
    DumpVmcs32Filed (&mVmcs32GuestFiledStr[Index]);
  }

  for (Index = 0; Index < sizeof(mVmcsNGuestFiledStr)/sizeof(mVmcsNGuestFiledStr[0]); Index++) {
    DumpVmcsNFiled (&mVmcsNGuestFiledStr[Index]);
  }
}

GLOBAL_REMOVE_IF_UNREFERENCED
DATA_STR mVmcs16HostFiledStr[] = {
  {VMCS_16_HOST_ES_INDEX,                                  "VMCS_16_HOST_ES                                 "},
  {VMCS_16_HOST_CS_INDEX,                                  "VMCS_16_HOST_CS                                 "},
  {VMCS_16_HOST_SS_INDEX,                                  "VMCS_16_HOST_SS                                 "},
  {VMCS_16_HOST_DS_INDEX,                                  "VMCS_16_HOST_DS                                 "},
  {VMCS_16_HOST_FS_INDEX,                                  "VMCS_16_HOST_FS                                 "},
  {VMCS_16_HOST_GS_INDEX,                                  "VMCS_16_HOST_GS                                 "},
  {VMCS_16_HOST_TR_INDEX,                                  "VMCS_16_HOST_TR                                 "},
};

GLOBAL_REMOVE_IF_UNREFERENCED
DATA_STR mVmcs64HostFiledStr[] = {
  {VMCS_64_HOST_IA32_PAT_INDEX,                            "VMCS_64_HOST_IA32_PAT                           "},
  {VMCS_64_HOST_IA32_EFER_INDEX,                           "VMCS_64_HOST_IA32_EFER                          "},
  {VMCS_64_HOST_IA32_PERF_GLOBAL_CTRL_INDEX,               "VMCS_64_HOST_IA32_PERF_GLOBAL_CTRL              "},
};

GLOBAL_REMOVE_IF_UNREFERENCED
DATA_STR mVmcs32HostFiledStr[] = {
  {VMCS_32_HOST_IA32_SYSENTER_CS_INDEX,                    "VMCS_32_HOST_IA32_SYSENTER_CS                   "},
};

GLOBAL_REMOVE_IF_UNREFERENCED
DATA_STR mVmcsNHostFiledStr[] = {
  {VMCS_N_HOST_CR0_INDEX,                                  "VMCS_N_HOST_CR0                                 "},
  {VMCS_N_HOST_CR3_INDEX,                                  "VMCS_N_HOST_CR3                                 "},
  {VMCS_N_HOST_CR4_INDEX,                                  "VMCS_N_HOST_CR4                                 "},
  {VMCS_N_HOST_FS_BASE_INDEX,                              "VMCS_N_HOST_FS_BASE                             "},
  {VMCS_N_HOST_GS_BASE_INDEX,                              "VMCS_N_HOST_GS_BASE                             "},
  {VMCS_N_HOST_TR_BASE_INDEX,                              "VMCS_N_HOST_TR_BASE                             "},
  {VMCS_N_HOST_GDTR_BASE_INDEX,                            "VMCS_N_HOST_GDTR_BASE                           "},
  {VMCS_N_HOST_IDTR_BASE_INDEX,                            "VMCS_N_HOST_IDTR_BASE                           "},
  {VMCS_N_HOST_IA32_SYSENTER_ESP_INDEX,                    "VMCS_N_HOST_IA32_SYSENTER_ESP                   "},
  {VMCS_N_HOST_IA32_SYSENTER_EIP_INDEX,                    "VMCS_N_HOST_IA32_SYSENTER_EIP                   "},
  {VMCS_N_HOST_RSP_INDEX,                                  "VMCS_N_HOST_RSP                                 "},
  {VMCS_N_HOST_RIP_INDEX,                                  "VMCS_N_HOST_RIP                                 "},
};

/**

  This function dump VMCS host field.

**/
VOID
DumpVmcsHostField (
  VOID
  )
{
  UINT32  Index;

  for (Index = 0; Index < sizeof(mVmcs16HostFiledStr)/sizeof(mVmcs16HostFiledStr[0]); Index++) {
    DumpVmcs16Filed (&mVmcs16HostFiledStr[Index]);
  }

  for (Index = 0; Index < sizeof(mVmcs64HostFiledStr)/sizeof(mVmcs64HostFiledStr[0]); Index++) {
    DumpVmcs64Filed (&mVmcs64HostFiledStr[Index]);
  }

  for (Index = 0; Index < sizeof(mVmcs32HostFiledStr)/sizeof(mVmcs32HostFiledStr[0]); Index++) {
    DumpVmcs32Filed (&mVmcs32HostFiledStr[Index]);
  }

  for (Index = 0; Index < sizeof(mVmcsNHostFiledStr)/sizeof(mVmcsNHostFiledStr[0]); Index++) {
    DumpVmcsNFiled (&mVmcsNHostFiledStr[Index]);
  }
}

/**

  This function dump VMCS all field.

**/
VOID
DumpVmcsAllField (
  VOID
  )
{
  DumpVmcsControlField ();
  DumpVmcsReadOnlyField ();
  DumpVmcsGuestField ();
  DumpVmcsHostField ();
}

GLOBAL_REMOVE_IF_UNREFERENCED
DATA_STR mX86RegisterStr[] = {
  {0 * sizeof(UINTN),     "RAX"},
  {1 * sizeof(UINTN),     "RCX"},
  {2 * sizeof(UINTN),     "RDX"},
  {3 * sizeof(UINTN),     "RBX"},
  {4 * sizeof(UINTN),     "RSP"},
  {5 * sizeof(UINTN),     "RBP"},
  {6 * sizeof(UINTN),     "RSI"},
  {7 * sizeof(UINTN),     "RDI"},
  {8 * sizeof(UINTN),     "R8 "},
  {9 * sizeof(UINTN),     "R9 "},
  {10* sizeof(UINTN),     "R10"},
  {11* sizeof(UINTN),     "R11"},
  {12* sizeof(UINTN),     "R12"},
  {13* sizeof(UINTN),     "R13"},
  {14* sizeof(UINTN),     "R14"},
  {15* sizeof(UINTN),     "R15"},
};

/**

  This function dump X86 register field.

  @param Reg     X86 register context
  @param DataStr X86 register structure

**/
VOID
DumpRegFiled (
  IN X86_REGISTER *Reg,
  IN DATA_STR     *DataStr
  )
{
  UINTN  *RegPtr;

  RegPtr = (UINTN *)Reg;
  if (sizeof(UINTN) == sizeof(UINT64)) {
    DEBUG ((EFI_D_INFO, "%a: %016lx\n", DataStr->Str, RegPtr[DataStr->Index]));
  } else {
    DEBUG ((EFI_D_INFO, "%a: %08x\n", DataStr->Str, RegPtr[DataStr->Index]));
  }
}

/**

  This function dump X86 register context.

  @param Reg X86 register context

**/
VOID
DumpRegContext (
  IN X86_REGISTER *Reg
  )
{
  UINT32  Index;
  for (Index = 0; Index < sizeof(mX86RegisterStr)/sizeof(mX86RegisterStr[0]) / (sizeof(UINT64)/sizeof(UINTN)); Index++) {
    DumpRegFiled (Reg, &mX86RegisterStr[Index]);
  }
}