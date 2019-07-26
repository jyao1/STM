/** @file
VMCS Memory Mapper

Gov't Copyright stuff

**/

#include "StmRuntime.h"
#include "VmcsOffsets.h"

#define VmcsSizeInPages 1    // current VMCS size in pages
#define memcpy CopyMem


VMCSFIELDOFFSET VmcsFieldOffsetTable[] =
{
	{ VMCS_16_CONTROL_VPID_INDEX, 0 }, //                             0x0000
	{ VMCS_16_GUEST_ES_INDEX, 0 },     //                             0x0800
	{ VMCS_16_GUEST_CS_INDEX, 0 },     //                             0x0802
	{ VMCS_16_GUEST_SS_INDEX, 0 },     //                             0x0804
	{ VMCS_16_GUEST_DS_INDEX, 0 },     //                             0x0806
	{ VMCS_16_GUEST_FS_INDEX, 0 },     //                             0x0808
	{ VMCS_16_GUEST_GS_INDEX, 0 },     //                             0x080A
	{ VMCS_16_GUEST_LDTR_INDEX, 0 },   //                             0x080C
	{ VMCS_16_GUEST_TR_INDEX, 0 },     //                             0x080E
	{ VMCS_16_HOST_ES_INDEX, 0 },      //                             0x0C00
	{ VMCS_16_HOST_CS_INDEX, 0 },      //                             0x0C02
	{ VMCS_16_HOST_SS_INDEX, 0 },      //                             0x0C04
	{ VMCS_16_HOST_DS_INDEX, 0 },      //                             0x0C06
	{ VMCS_16_HOST_FS_INDEX, 0 },      //                             0x0C08
	{ VMCS_16_HOST_GS_INDEX, 0 },      //                             0x0C0A
	{ VMCS_16_HOST_TR_INDEX, 0 },      //                             0x0C0C
	{ VMCS_64_CONTROL_IO_BITMAP_A_INDEX, 0 }, //                      0x2000
	{ VMCS_64_CONTROL_IO_BITMAP_B_INDEX, 0 }, //                      0x2002
	{ VMCS_64_CONTROL_MSR_BITMAP_INDEX, 0 },  //                      0x2004
	{ VMCS_64_CONTROL_VMEXIT_MSR_STORE_INDEX, 0 },  //                0x2006
	{ VMCS_64_CONTROL_VMEXIT_MSR_LOAD_INDEX, 0 },   //                0x2008
	{ VMCS_64_CONTROL_VMENTRY_MSR_LOAD_INDEX, 0 },  //                0x200A
	{ VMCS_64_CONTROL_EXECUTIVE_VMCS_PTR_INDEX, 0 }, //               0x200C
	{ VMCS_64_CONTROL_TSC_OFFSET_INDEX, 0 }, //                       0x2010
	{ VMCS_64_CONTROL_VIRTUAL_APIC_ADDR_INDEX, 0 }, //                0x2012
	{ VMCS_64_CONTROL_APIC_ACCESS_ADDR_INDEX, 0 }, //                 0x2014
	{ VMCS_64_CONTROL_VM_FUNCTION_CONTROLS_INDEX, 0 }, //            0x2018
	{ VMCS_64_CONTROL_EPT_PTR_INDEX, 0 },              //            0x201A
	{ VMCS_64_CONTROL_EPTP_LIST_ADDRESS_INDEX, 0 },    //            0x2024
	{ VMCS_64_RO_GUEST_PHYSICAL_ADDR_INDEX, 0 },       //            0x2400
	{ VMCS_64_GUEST_VMCS_LINK_PTR_INDEX, 0 },          //            0x2800	
	{ VMCS_64_GUEST_IA32_DEBUGCTL_INDEX, 0 },              //        0x2802
	{ VMCS_64_GUEST_IA32_PAT_INDEX, 0 },                   //        0x2804
	{ VMCS_64_GUEST_IA32_EFER_INDEX, 0 }, //                          0x2806
	{ VMCS_64_GUEST_IA32_PERF_GLOBAL_CTRL_INDEX, 0 }, //              0x2808
	{ VMCS_64_GUEST_PDPTE0_INDEX, 0 }, //                             0x280A
	{ VMCS_64_GUEST_PDPTE1_INDEX, 0 }, //                             0x280C
	{ VMCS_64_GUEST_PDPTE2_INDEX, 0 }, //                             0x280E
	{ VMCS_64_GUEST_PDPTE3_INDEX, 0 }, //                             0x2810
	{ VMCS_64_HOST_IA32_PAT_INDEX, 0 }, //                            0x2C00
	{ VMCS_64_HOST_IA32_EFER_INDEX, 0 }, //                           0x2C02
	{ VMCS_64_HOST_IA32_PERF_GLOBAL_CTRL_INDEX, 0 }, //               0x2C04
	{ VMCS_32_CONTROL_PIN_BASED_VM_EXECUTION_INDEX, 0 }, //           0x4000
	{ VMCS_32_CONTROL_PROCESSOR_BASED_VM_EXECUTION_INDEX, 0 }, //     0x4002
	{ VMCS_32_CONTROL_EXCEPTION_BITMAP_INDEX, 0 }, //                 0x4004
	{ VMCS_32_CONTROL_PAGE_FAULT_ERROR_CODE_MASK_INDEX, 0 }, //       0x4006
	{ VMCS_32_CONTROL_PAGE_FAULT_ERROR_CODE_MATCH_INDEX, 0 }, //      0x4008
	{ VMCS_32_CONTROL_CR3_TARGET_COUNT_INDEX, 0 }, //                 0x400A
	{ VMCS_32_CONTROL_VMEXIT_CONTROLS_INDEX, 0 }, //                  0x400C
	{ VMCS_32_CONTROL_VMEXIT_MSR_STORE_COUNT_INDEX, 0 }, //           0x400E
	{ VMCS_32_CONTROL_VMEXIT_MSR_LOAD_COUNT_INDEX, 0 }, //            0x4010
	{ VMCS_32_CONTROL_VMENTRY_CONTROLS_INDEX, 0 }, //                 0x4012
	{ VMCS_32_CONTROL_VMENTRY_MSR_LOAD_COUNT_INDEX, 0 }, //           0x4014
	{ VMCS_32_CONTROL_VMENTRY_INTERRUPTION_INFO_INDEX, 0 }, //        0x4016
	{ VMCS_32_CONTROL_VMENTRY_EXCEPTION_ERROR_CODE_INDEX, 0 }, //     0x4018
	{ VMCS_32_CONTROL_VMENTRY_INSTRUCTION_LENGTH_INDEX, 0 }, //       0x401A
	{ VMCS_32_CONTROL_TPR_THRESHOLD_INDEX, 0 }, //                    0x401C
	{ VMCS_32_CONTROL_2ND_PROCESSOR_BASED_VM_EXECUTION_INDEX, 0 }, // 0x401E
	{ VMCS_32_CONTROL_PLE_GAP_INDEX, 0 }, //                          0x4020
	{ VMCS_32_CONTROL_PLE_WINDOW_INDEX, 0 }, //                       0x4022
	{ VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX, 0 }, //                  0x4400
	{ VMCS_32_RO_EXIT_REASON_INDEX, 0 }, //                           0x4402
	{ VMCS_32_RO_VMEXIT_INTERRUPTION_INFO_INDEX, 0 }, //              0x4404
	{ VMCS_32_RO_VMEXIT_INTERRUPTION_ERROR_CODE_INDEX, 0 }, //        0x4406
	{ VMCS_32_RO_IDT_VECTORING_INFO_INDEX, 0 }, //                    0x4408
	{ VMCS_32_RO_IDT_VECTORING_ERROR_CODE_INDEX, 0 }, //              0x440A
	{ VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX, 0 }, //             0x440C
	{ VMCS_32_RO_VMEXIT_INSTRUCTION_INFO_INDEX, 0 }, //               0x440E
	{ VMCS_32_GUEST_ES_LIMIT_INDEX, 0 }, //                           0x4800
	{ VMCS_32_GUEST_CS_LIMIT_INDEX, 0 }, //                           0x4802
	{ VMCS_32_GUEST_SS_LIMIT_INDEX, 0 }, //                           0x4804
	{ VMCS_32_GUEST_DS_LIMIT_INDEX, 0 }, //                           0x4806
	{ VMCS_32_GUEST_FS_LIMIT_INDEX, 0 }, //                           0x4808
	{ VMCS_32_GUEST_GS_LIMIT_INDEX, 0 }, //                           0x480A
	{ VMCS_32_GUEST_LDTR_LIMIT_INDEX, 0 }, //                         0x480C
	{ VMCS_32_GUEST_TR_LIMIT_INDEX, 0 }, //                           0x480E
	{ VMCS_32_GUEST_GDTR_LIMIT_INDEX, 0 }, //                         0x4810
	{ VMCS_32_GUEST_IDTR_LIMIT_INDEX, 0 }, //                         0x4812
	{ VMCS_32_GUEST_ES_ACCESS_RIGHT_INDEX, 0 }, //                    0x4814
	{ VMCS_32_GUEST_CS_ACCESS_RIGHT_INDEX, 0 }, //                    0x4816
	{ VMCS_32_GUEST_SS_ACCESS_RIGHT_INDEX, 0 }, //                    0x4818
	{ VMCS_32_GUEST_DS_ACCESS_RIGHT_INDEX, 0 }, //                    0x481A
	{ VMCS_32_GUEST_FS_ACCESS_RIGHT_INDEX, 0 }, //                    0x481C
	{ VMCS_32_GUEST_GS_ACCESS_RIGHT_INDEX, 0 }, //                    0x481E
	{ VMCS_32_GUEST_LDTR_ACCESS_RIGHT_INDEX, 0 }, //                  0x4820
	{ VMCS_32_GUEST_TR_ACCESS_RIGHT_INDEX, 0 }, //                    0x4822
	{ VMCS_32_GUEST_INTERRUPTIBILITY_STATE_INDEX, 0 }, //             0x4824
	{ VMCS_32_GUEST_ACTIVITY_STATE_INDEX, 0 }, //                     0x4826
	{ VMCS_32_GUEST_SMBASE_INDEX, 0 }, //                             0x4828
	{ VMCS_32_GUEST_IA32_SYSENTER_CS_INDEX, 0 }, //                   0x482A
	{ VMCS_32_GUEST_VMX_PREEMPTION_TIMER_VALUE_INDEX, 0 }, //         0x482E
	{ VMCS_32_HOST_IA32_SYSENTER_CS_INDEX, 0 }, //                    0x4C00
	{ VMCS_N_CONTROL_CR0_GUEST_HOST_MASK_INDEX, 0 }, //               0x6000
	{ VMCS_N_CONTROL_CR4_GUEST_HOST_MASK_INDEX, 0 }, //               0x6002
	{ VMCS_N_CONTROL_CR0_READ_SHADOW_INDEX, 0 }, //                   0x6004
	{ VMCS_N_CONTROL_CR4_READ_SHADOW_INDEX, 0 }, //                   0x6006
	{ VMCS_N_CONTROL_CR3_TARGET_VALUE0_INDEX, 0 }, //                 0x6008
	{ VMCS_N_CONTROL_CR3_TARGET_VALUE1_INDEX, 0 }, //                 0x600A
	{ VMCS_N_CONTROL_CR3_TARGET_VALUE2_INDEX, 0 }, //                 0x600C
	{ VMCS_N_CONTROL_CR3_TARGET_VALUE3_INDEX, 0 }, //                 0x600E
	{ VMCS_N_RO_EXIT_QUALIFICATION_INDEX, 0 }, //                     0x6400
	{ VMCS_N_RO_IO_RCX_INDEX, 0 }, //                                 0x6402
	{ VMCS_N_RO_IO_RSI_INDEX, 0 }, //                                 0x6404
	{ VMCS_N_RO_IO_RDI_INDEX, 0 }, //                                 0x6406
	{ VMCS_N_RO_IO_RIP_INDEX, 0 }, //                                 0x6408
	{ VMCS_N_RO_GUEST_LINEAR_ADDR_INDEX, 0 }, //                      0x640A
	{ VMCS_N_GUEST_CR0_INDEX, 0 }, //                                 0x6800
	{ VMCS_N_GUEST_CR3_INDEX, 0 }, //                                 0x6802
	{ VMCS_N_GUEST_CR4_INDEX, 0 }, //                                 0x6804
	{ VMCS_N_GUEST_ES_BASE_INDEX, 0 }, //                             0x6806
	{ VMCS_N_GUEST_CS_BASE_INDEX, 0 }, //                             0x6808
	{ VMCS_N_GUEST_SS_BASE_INDEX, 0 }, //                             0x680A
	{ VMCS_N_GUEST_DS_BASE_INDEX, 0 }, //                             0x680C
	{ VMCS_N_GUEST_FS_BASE_INDEX, 0 }, //                             0x680E
	{ VMCS_N_GUEST_GS_BASE_INDEX, 0 }, //                             0x6810
	{ VMCS_N_GUEST_LDTR_BASE_INDEX, 0 }, //                           0x6812
	{ VMCS_N_GUEST_TR_BASE_INDEX, 0 }, //                             0x6814
	{ VMCS_N_GUEST_GDTR_BASE_INDEX, 0 }, //                           0x6816
	{ VMCS_N_GUEST_IDTR_BASE_INDEX, 0 }, //                           0x6818
	{ VMCS_N_GUEST_DR7_INDEX, 0 }, //                                 0x681A
	{ VMCS_N_GUEST_RSP_INDEX, 0 }, //                                 0x681C
	{ VMCS_N_GUEST_RIP_INDEX, 0 }, //                                 0x681E
	{ VMCS_N_GUEST_RFLAGS_INDEX, 0 }, //                              0x6820
	{ VMCS_N_GUEST_PENDING_DEBUG_EXCEPTIONS_INDEX, 0 }, //            0x6822
	{ VMCS_N_GUEST_IA32_SYSENTER_ESP_INDEX, 0 }, //                   0x6824
	{ VMCS_N_GUEST_IA32_SYSENTER_EIP_INDEX, 0 }, //                   0x6826
	{ VMCS_N_HOST_CR0_INDEX, 0 }, //                                  0x6C00
	{ VMCS_N_HOST_CR3_INDEX, 0 }, //                                  0x6C02
	{ VMCS_N_HOST_CR4_INDEX, 0 }, //                                  0x6C04
	{ VMCS_N_HOST_FS_BASE_INDEX, 0 }, //                              0x6C06
	{ VMCS_N_HOST_GS_BASE_INDEX, 0 }, //                              0x6C08
	{ VMCS_N_HOST_TR_BASE_INDEX, 0 }, //                              0x6C0A
	{ VMCS_N_HOST_GDTR_BASE_INDEX, 0 }, //                            0x6C0C
	{ VMCS_N_HOST_IDTR_BASE_INDEX, 0 }, //                            0x6C0E
	{ VMCS_N_HOST_IA32_SYSENTER_ESP_INDEX, 0 }, //                    0x6C10
	{ VMCS_N_HOST_IA32_SYSENTER_EIP_INDEX, 0 }, //                    0x6C12
	{ VMCS_N_HOST_RSP_INDEX, 0 }, //                                  0x6C14
	{ VMCS_N_HOST_RIP_INDEX, 0 }, //                                  0x6C16
	{ 0xFFFF, 0 }
};


VMCSFIELDPRINT VmcsFieldPrintTable[] =
{
	{ VMCS_16_CONTROL_VPID_INDEX, "VMCS_16_CONTROL_VPID" },  //       0x0000
	{ VMCS_16_GUEST_ES_INDEX, "VMCS_16_GUEST_ES" },          //       0x0800
	{ VMCS_16_GUEST_CS_INDEX, "VMCS_16_GUEST_ES" },          //                             0x0802
	{ VMCS_16_GUEST_SS_INDEX, "VMCS_16_GUEST_SS" },          //                             0x0804
	{ VMCS_16_GUEST_DS_INDEX, "VMCS_16_GUEST_DS" },          //                             0x0806
	{ VMCS_16_GUEST_FS_INDEX, "VMCS_16_GUEST_FS" },          //                             0x0808
	{ VMCS_16_GUEST_GS_INDEX, "VMCS_16_GUEST_GS" },          //                             0x080A
	{ VMCS_16_GUEST_LDTR_INDEX, "VMCS_16_GUEST_LDTR" },      //                             0x080C
	{ VMCS_16_GUEST_TR_INDEX, "VMCS_16_GUEST_TR" },          //                             0x080E
	{ VMCS_16_HOST_ES_INDEX, "VMCS_16_HOST_ES_INDEX" },      //                             0x0C00
	{ VMCS_16_HOST_CS_INDEX, "VMCS_16_HOST_CS_INDEX" },      //                             0x0C02
	{ VMCS_16_HOST_SS_INDEX, "VMCS_16_HOST_SS_INDEX" },      //                             0x0C04
	{ VMCS_16_HOST_DS_INDEX, "VMCS_16_HOST_DS_INDEX" },      //                             0x0C06
	{ VMCS_16_HOST_FS_INDEX, "VMCS_16_HOST_FS_INDEX" },      //                             0x0C08
	{ VMCS_16_HOST_GS_INDEX, "VMCS_16_HOST_GS_INDEX" },      //                             0x0C0A
	{ VMCS_16_HOST_TR_INDEX, "VMCS_16_HOST_TR_INDEX" },      //                             0x0C0C
	{ VMCS_64_CONTROL_IO_BITMAP_A_INDEX, "VMCS_64_CONTROL_IO_BITMAP_A_INDEX" }, //                      0x2000
	{ VMCS_64_CONTROL_IO_BITMAP_B_INDEX, "VMCS_64_CONTROL_IO_BITMAP_B_INDEX" }, //                      0x2002
	{ VMCS_64_CONTROL_MSR_BITMAP_INDEX, "VMCS_64_CONTROL_MSR_BITMAP_INDEX" },  //                      0x2004
	{ VMCS_64_CONTROL_VMEXIT_MSR_STORE_INDEX, "VMCS_64_CONTROL_VMEXIT_MSR_STORE_INDEX" },  //                0x2006
	{ VMCS_64_CONTROL_VMEXIT_MSR_LOAD_INDEX, "VMCS_64_CONTROL_VMEXIT_MSR_LOAD_INDEX" },   //                0x2008
	{ VMCS_64_CONTROL_VMENTRY_MSR_LOAD_INDEX, "VMCS_64_CONTROL_VMENTRY_MSR_LOAD_INDEX" },  //                0x200A
	{ VMCS_64_CONTROL_EXECUTIVE_VMCS_PTR_INDEX, "VMCS_64_CONTROL_EXECUTIVE_VMCS_PTR_INDEX" }, //               0x200C
	{ VMCS_64_CONTROL_TSC_OFFSET_INDEX, "VMCS_64_CONTROL_TSC_OFFSET_INDEX" }, //                       0x2010
	{ VMCS_64_CONTROL_VIRTUAL_APIC_ADDR_INDEX, "VMCS_64_CONTROL_VIRTUAL_APIC_ADDR_INDEX" }, //                0x2012
	{ VMCS_64_CONTROL_APIC_ACCESS_ADDR_INDEX, "VMCS_64_CONTROL_APIC_ACCESS_ADDR_INDEX" }, //                 0x2014
	{ VMCS_64_CONTROL_VM_FUNCTION_CONTROLS_INDEX, "VMCS_64_CONTROL_VM_FUNCTION_CONTROLS_INDEX" }, //            0x2018
	{ VMCS_64_CONTROL_EPT_PTR_INDEX, "VMCS_64_CONTROL_EPT_PTR_INDEX" },              //            0x201A
	{ VMCS_64_CONTROL_EPTP_LIST_ADDRESS_INDEX, "VMCS_64_CONTROL_EPTP_LIST_ADDRESS_INDEX" },    //            0x2024
	{ VMCS_64_RO_GUEST_PHYSICAL_ADDR_INDEX, "VMCS_64_RO_GUEST_PHYSICAL_ADDR_INDEX" },       //            0x2400
	{ VMCS_64_GUEST_VMCS_LINK_PTR_INDEX, "VMCS_64_GUEST_VMCS_LINK_PTR_INDEX" },          //            0x2800	
	{ VMCS_64_GUEST_IA32_DEBUGCTL_INDEX, "VMCS_64_GUEST_IA32_DEBUGCTL_INDEX" },              //        0x2802
	{ VMCS_64_GUEST_IA32_PAT_INDEX, "VMCS_64_GUEST_IA32_PAT_INDEX" },                   //        0x2804
	{ VMCS_64_GUEST_IA32_EFER_INDEX, "VMCS_64_GUEST_IA32_EFER_INDEX" }, //                          0x2806
	{ VMCS_64_GUEST_IA32_PERF_GLOBAL_CTRL_INDEX, "VMCS_64_GUEST_IA32_PERF_GLOBAL_CTRL_INDEX" }, //              0x2808
	{ VMCS_64_GUEST_PDPTE0_INDEX, "VMCS_64_GUEST_PDPTE0_INDEX" }, //                             0x280A
	{ VMCS_64_GUEST_PDPTE1_INDEX, "VMCS_64_GUEST_PDPTE1_INDEX" }, //                             0x280C
	{ VMCS_64_GUEST_PDPTE2_INDEX, "VMCS_64_GUEST_PDPTE2_INDEX" }, //                             0x280E
	{ VMCS_64_GUEST_PDPTE3_INDEX, "VMCS_64_GUEST_PDPTE3_INDEX" }, //                             0x2810
	{ VMCS_64_HOST_IA32_PAT_INDEX, "VMCS_64_HOST_IA32_PAT_INDEX" }, //                            0x2C00
	{ VMCS_64_HOST_IA32_EFER_INDEX, "VMCS_64_HOST_IA32_EFER_INDEX" }, //                           0x2C02
	{ VMCS_64_HOST_IA32_PERF_GLOBAL_CTRL_INDEX, "VMCS_64_HOST_IA32_PERF_GLOBAL_CTRL_INDEX" }, //               0x2C04
	{ VMCS_32_CONTROL_PIN_BASED_VM_EXECUTION_INDEX, "VMCS_32_CONTROL_PIN_BASED_VM_EXECUTION_INDEX" }, //           0x4000
	{ VMCS_32_CONTROL_PROCESSOR_BASED_VM_EXECUTION_INDEX, "VMCS_32_CONTROL_PROCESSOR_BASED_VM_EXECUTION_INDEX" }, //     0x4002
	{ VMCS_32_CONTROL_EXCEPTION_BITMAP_INDEX, "VMCS_32_CONTROL_EXCEPTION_BITMAP_INDEX" }, //                 0x4004
	{ VMCS_32_CONTROL_PAGE_FAULT_ERROR_CODE_MASK_INDEX, "VMCS_32_CONTROL_PAGE_FAULT_ERROR_CODE_MASK_INDEX" }, //       0x4006
	{ VMCS_32_CONTROL_PAGE_FAULT_ERROR_CODE_MATCH_INDEX, "VMCS_32_CONTROL_PAGE_FAULT_ERROR_CODE_MATCH_INDEX" }, //      0x4008
	{ VMCS_32_CONTROL_CR3_TARGET_COUNT_INDEX, "VMCS_32_CONTROL_CR3_TARGET_COUNT_INDEX" }, //                 0x400A
	{ VMCS_32_CONTROL_VMEXIT_CONTROLS_INDEX, "VMCS_32_CONTROL_VMEXIT_CONTROLS_INDEX" }, //                  0x400C
	{ VMCS_32_CONTROL_VMEXIT_MSR_STORE_COUNT_INDEX, "VMCS_32_CONTROL_VMEXIT_MSR_STORE_COUNT_INDEX" }, //           0x400E
	{ VMCS_32_CONTROL_VMEXIT_MSR_LOAD_COUNT_INDEX, "VMCS_32_CONTROL_VMEXIT_MSR_LOAD_COUNT_INDEX" }, //            0x4010
	{ VMCS_32_CONTROL_VMENTRY_CONTROLS_INDEX, "VMCS_32_CONTROL_VMENTRY_CONTROLS_INDEX" }, //                 0x4012
	{ VMCS_32_CONTROL_VMENTRY_MSR_LOAD_COUNT_INDEX, "VMCS_32_CONTROL_VMENTRY_MSR_LOAD_COUNT_INDEX" }, //           0x4014
	{ VMCS_32_CONTROL_VMENTRY_INTERRUPTION_INFO_INDEX, "VMCS_32_CONTROL_VMENTRY_INTERRUPTION_INFO_INDEX" }, //        0x4016
	{ VMCS_32_CONTROL_VMENTRY_EXCEPTION_ERROR_CODE_INDEX, "VMCS_32_CONTROL_VMENTRY_EXCEPTION_ERROR_CODE_INDEX" }, //     0x4018
	{ VMCS_32_CONTROL_VMENTRY_INSTRUCTION_LENGTH_INDEX, "VMCS_32_CONTROL_VMENTRY_INSTRUCTION_LENGTH_INDEX" }, //       0x401A
	{ VMCS_32_CONTROL_TPR_THRESHOLD_INDEX, "VMCS_32_CONTROL_TPR_THRESHOLD_INDEX" }, //                    0x401C
	{ VMCS_32_CONTROL_2ND_PROCESSOR_BASED_VM_EXECUTION_INDEX, "VMCS_32_CONTROL_2ND_PROCESSOR_BASED_VM_EXECUTION_INDEX" }, // 0x401E
	{ VMCS_32_CONTROL_PLE_GAP_INDEX, "VMCS_32_CONTROL_PLE_GAP_INDEX" }, //                          0x4020
	{ VMCS_32_CONTROL_PLE_WINDOW_INDEX, "VMCS_32_CONTROL_PLE_WINDOW_INDEX" }, //                       0x4022
	{ VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX, "VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX" }, //                  0x4400
	{ VMCS_32_RO_EXIT_REASON_INDEX, "VMCS_32_RO_EXIT_REASON_INDEX" }, //                           0x4402
	{ VMCS_32_RO_VMEXIT_INTERRUPTION_INFO_INDEX, "VMCS_32_RO_VMEXIT_INTERRUPTION_INFO_INDEX" }, //              0x4404
	{ VMCS_32_RO_VMEXIT_INTERRUPTION_ERROR_CODE_INDEX, "VMCS_32_RO_VMEXIT_INTERRUPTION_ERROR_CODE_INDEX" }, //        0x4406
	{ VMCS_32_RO_IDT_VECTORING_INFO_INDEX, "VMCS_32_RO_IDT_VECTORING_INFO_INDEX" }, //                    0x4408
	{ VMCS_32_RO_IDT_VECTORING_ERROR_CODE_INDEX, "VMCS_32_RO_IDT_VECTORING_ERROR_CODE_INDEX" }, //              0x440A
	{ VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX, "VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX" }, //             0x440C
	{ VMCS_32_RO_VMEXIT_INSTRUCTION_INFO_INDEX, "VMCS_32_RO_VMEXIT_INSTRUCTION_INFO_INDEX" }, //               0x440E
	{ VMCS_32_GUEST_ES_LIMIT_INDEX, "VMCS_32_GUEST_ES_LIMIT_INDEX" }, //                           0x4800
	{ VMCS_32_GUEST_CS_LIMIT_INDEX, "VMCS_32_GUEST_CS_LIMIT_INDEX" }, //                           0x4802
	{ VMCS_32_GUEST_SS_LIMIT_INDEX, "VMCS_32_GUEST_SS_LIMIT_INDEX" }, //                           0x4804
	{ VMCS_32_GUEST_DS_LIMIT_INDEX, "VMCS_32_GUEST_DS_LIMIT_INDEX" }, //                           0x4806
	{ VMCS_32_GUEST_FS_LIMIT_INDEX, "VMCS_32_GUEST_FS_LIMIT_INDEX" }, //                           0x4808
	{ VMCS_32_GUEST_GS_LIMIT_INDEX, "VMCS_32_GUEST_GS_LIMIT_INDEX" }, //                           0x480A
	{ VMCS_32_GUEST_LDTR_LIMIT_INDEX, "VMCS_32_GUEST_LDTR_LIMIT_INDEX" }, //                         0x480C
	{ VMCS_32_GUEST_TR_LIMIT_INDEX, "VMCS_32_GUEST_TR_LIMIT_INDEX" }, //                           0x480E
	{ VMCS_32_GUEST_GDTR_LIMIT_INDEX, "VMCS_32_GUEST_GDTR_LIMIT_INDEX" }, //                         0x4810
	{ VMCS_32_GUEST_IDTR_LIMIT_INDEX, "VMCS_32_GUEST_IDTR_LIMIT_INDEX" }, //                         0x4812
	{ VMCS_32_GUEST_ES_ACCESS_RIGHT_INDEX, "VMCS_32_GUEST_ES_ACCESS_RIGHT_INDEX" }, //                    0x4814
	{ VMCS_32_GUEST_CS_ACCESS_RIGHT_INDEX, "VMCS_32_GUEST_CS_ACCESS_RIGHT_INDEX" }, //                    0x4816
	{ VMCS_32_GUEST_SS_ACCESS_RIGHT_INDEX, "VMCS_32_GUEST_SS_ACCESS_RIGHT_INDEX" }, //                    0x4818
	{ VMCS_32_GUEST_DS_ACCESS_RIGHT_INDEX, "VMCS_32_GUEST_DS_ACCESS_RIGHT_INDEX" }, //                    0x481A
	{ VMCS_32_GUEST_FS_ACCESS_RIGHT_INDEX, "VMCS_32_GUEST_FS_ACCESS_RIGHT_INDEX" }, //                    0x481C
	{ VMCS_32_GUEST_GS_ACCESS_RIGHT_INDEX, "VMCS_32_GUEST_GS_ACCESS_RIGHT_INDEX" }, //                    0x481E
	{ VMCS_32_GUEST_LDTR_ACCESS_RIGHT_INDEX, "VMCS_32_GUEST_LDTR_ACCESS_RIGHT_INDEX" }, //                  0x4820
	{ VMCS_32_GUEST_TR_ACCESS_RIGHT_INDEX, "VMCS_32_GUEST_TR_ACCESS_RIGHT_INDEX" }, //                    0x4822
	{ VMCS_32_GUEST_INTERRUPTIBILITY_STATE_INDEX, "VMCS_32_GUEST_INTERRUPTIBILITY_STATE_INDEX" }, //             0x4824
	{ VMCS_32_GUEST_ACTIVITY_STATE_INDEX, "VMCS_32_GUEST_ACTIVITY_STATE_INDEX" }, //                     0x4826
	{ VMCS_32_GUEST_SMBASE_INDEX, "VMCS_32_GUEST_SMBASE_INDEX" }, //                             0x4828
	{ VMCS_32_GUEST_IA32_SYSENTER_CS_INDEX, "VMCS_32_GUEST_IA32_SYSENTER_CS_INDEX" }, //                   0x482A
	{ VMCS_32_GUEST_VMX_PREEMPTION_TIMER_VALUE_INDEX, "VMCS_32_GUEST_VMX_PREEMPTION_TIMER_VALUE_INDEX" }, //         0x482E
	{ VMCS_32_HOST_IA32_SYSENTER_CS_INDEX, "VMCS_32_HOST_IA32_SYSENTER_CS_INDEX" }, //                    0x4C00
	{ VMCS_N_CONTROL_CR0_GUEST_HOST_MASK_INDEX, "VMCS_N_CONTROL_CR0_GUEST_HOST_MASK_INDEX" }, //               0x6000
	{ VMCS_N_CONTROL_CR4_GUEST_HOST_MASK_INDEX, "VMCS_N_CONTROL_CR4_GUEST_HOST_MASK_INDEX" }, //               0x6002
	{ VMCS_N_CONTROL_CR0_READ_SHADOW_INDEX, "VMCS_N_CONTROL_CR0_READ_SHADOW_INDEX" }, //                   0x6004
	{ VMCS_N_CONTROL_CR4_READ_SHADOW_INDEX, "VMCS_N_CONTROL_CR4_READ_SHADOW_INDEX" }, //                   0x6006
	{ VMCS_N_CONTROL_CR3_TARGET_VALUE0_INDEX, "VMCS_N_CONTROL_CR3_TARGET_VALUE0_INDEX" }, //                 0x6008
	{ VMCS_N_CONTROL_CR3_TARGET_VALUE1_INDEX, "VMCS_N_CONTROL_CR3_TARGET_VALUE1_INDEX" }, //                 0x600A
	{ VMCS_N_CONTROL_CR3_TARGET_VALUE2_INDEX, "VMCS_N_CONTROL_CR3_TARGET_VALUE2_INDEX" }, //                 0x600C
	{ VMCS_N_CONTROL_CR3_TARGET_VALUE3_INDEX, "VMCS_N_CONTROL_CR3_TARGET_VALUE3_INDEX" }, //                 0x600E
	{ VMCS_N_RO_EXIT_QUALIFICATION_INDEX, "VMCS_N_RO_EXIT_QUALIFICATION_INDEX" }, //                     0x6400
	{ VMCS_N_RO_IO_RCX_INDEX, "VMCS_N_RO_IO_RCX_INDEX" }, //                                 0x6402
	{ VMCS_N_RO_IO_RSI_INDEX, "VMCS_N_RO_IO_RSI_INDEX" }, //                                 0x6404
	{ VMCS_N_RO_IO_RDI_INDEX, "VMCS_N_RO_IO_RDI_INDEX" }, //                                 0x6406
	{ VMCS_N_RO_IO_RIP_INDEX, "VMCS_N_RO_IO_RIP_INDEX" }, //                                 0x6408
	{ VMCS_N_RO_GUEST_LINEAR_ADDR_INDEX, "VMCS_N_RO_GUEST_LINEAR_ADDR_INDEX" }, //                      0x640A
	{ VMCS_N_GUEST_CR0_INDEX, "VMCS_N_GUEST_CR0_INDEX" }, //                                 0x6800
	{ VMCS_N_GUEST_CR3_INDEX, "VMCS_N_GUEST_CR3_INDEX" }, //                                 0x6802
	{ VMCS_N_GUEST_CR4_INDEX, "VMCS_N_GUEST_CR4_INDEX" }, //                                 0x6804
	{ VMCS_N_GUEST_ES_BASE_INDEX, "VMCS_N_GUEST_ES_BASE_INDEX" }, //                             0x6806
	{ VMCS_N_GUEST_CS_BASE_INDEX, "VMCS_N_GUEST_CS_BASE_INDEX" }, //                             0x6808
	{ VMCS_N_GUEST_SS_BASE_INDEX, "VMCS_N_GUEST_SS_BASE_INDEX" }, //                             0x680A
	{ VMCS_N_GUEST_DS_BASE_INDEX, "VMCS_N_GUEST_DS_BASE_INDEX" }, //                             0x680C
	{ VMCS_N_GUEST_FS_BASE_INDEX, "VMCS_N_GUEST_FS_BASE_INDEX" }, //                             0x680E
	{ VMCS_N_GUEST_GS_BASE_INDEX, "VMCS_N_GUEST_GS_BASE_INDEX" }, //                             0x6810
	{ VMCS_N_GUEST_LDTR_BASE_INDEX, "VMCS_N_GUEST_LDTR_BASE_INDEX" }, //                           0x6812
	{ VMCS_N_GUEST_TR_BASE_INDEX, "VMCS_N_GUEST_TR_BASE_INDEX" }, //                             0x6814
	{ VMCS_N_GUEST_GDTR_BASE_INDEX, "VMCS_N_GUEST_GDTR_BASE_INDEX" }, //                           0x6816
	{ VMCS_N_GUEST_IDTR_BASE_INDEX, "VMCS_N_GUEST_IDTR_BASE_INDEX" }, //                           0x6818
	{ VMCS_N_GUEST_DR7_INDEX, "VMCS_N_GUEST_DR7_INDEX" }, //                                 0x681A
	{ VMCS_N_GUEST_RSP_INDEX, "VMCS_N_GUEST_RSP_INDEX" }, //                                 0x681C
	{ VMCS_N_GUEST_RIP_INDEX, "VMCS_N_GUEST_RIP_INDEX" }, //                                 0x681E
	{ VMCS_N_GUEST_RFLAGS_INDEX, "VMCS_N_GUEST_RFLAGS_INDEX" }, //                              0x6820
	{ VMCS_N_GUEST_PENDING_DEBUG_EXCEPTIONS_INDEX, "VMCS_N_GUEST_PENDING_DEBUG_EXCEPTIONS_INDEX" }, //            0x6822
	{ VMCS_N_GUEST_IA32_SYSENTER_ESP_INDEX, "VMCS_N_GUEST_IA32_SYSENTER_ESP_INDEX" }, //                   0x6824
	{ VMCS_N_GUEST_IA32_SYSENTER_EIP_INDEX, "VMCS_N_GUEST_IA32_SYSENTER_EIP_INDEX" }, //                   0x6826
	{ VMCS_N_HOST_CR0_INDEX, "VMCS_N_HOST_CR0_INDEX" }, //                                  0x6C00
	{ VMCS_N_HOST_CR3_INDEX, "VMCS_N_HOST_CR3_INDEX" }, //                                  0x6C02
	{ VMCS_N_HOST_CR4_INDEX, "VMCS_N_HOST_CR4_INDEX" }, //                                  0x6C04
	{ VMCS_N_HOST_FS_BASE_INDEX, "VMCS_N_HOST_FS_BASE_INDEX" }, //                              0x6C06
	{ VMCS_N_HOST_GS_BASE_INDEX, "VMCS_N_HOST_GS_BASE_INDEX" }, //                              0x6C08
	{ VMCS_N_HOST_TR_BASE_INDEX, "VMCS_N_HOST_TR_BASE_INDEX" }, //                              0x6C0A
	{ VMCS_N_HOST_GDTR_BASE_INDEX, "VMCS_N_HOST_GDTR_BASE_INDEX" }, //                            0x6C0C
	{ VMCS_N_HOST_IDTR_BASE_INDEX, "VMCS_N_HOST_IDTR_BASE_INDEX" }, //                            0x6C0E
	{ VMCS_N_HOST_IA32_SYSENTER_ESP_INDEX, "VMCS_N_HOST_IA32_SYSENTER_ESP_INDEX" }, //                    0x6C10
	{ VMCS_N_HOST_IA32_SYSENTER_EIP_INDEX, "VMCS_N_HOST_IA32_SYSENTER_EIP_INDEX" }, //                    0x6C12
	{ VMCS_N_HOST_RSP_INDEX, "VMCS_N_HOST_RSP_INDEX" }, //                                  0x6C14
	{ VMCS_N_HOST_RIP_INDEX, "VMCS_N_HOST_RIP_INDEX" }, //                                  0x6C16
	{ 0xFFFF, NULL }
};
int strlen1(const char *str)
{
	const char * s;
	int counter = 0;
	for (s=str; (*s != 0) && (counter < 40); ++s, ++counter);
	return counter;  // only try 40
}

static UINT32 VmcsMapInit = 0;  // used to trigger VmcsMap initialization

void MapVmcs ()
{
	unsigned short index;
	unsigned int i;
	UINT64 CurrentVMCSSave;
	UINT64 FieldValue;
	UINT32 FieldOffset;
	UINT32 VmxRevId;
	char Line[150];
	char * EvalVmcs;

	if(VmcsMapInit == 1)
	{
		return;
	}

	VmcsMapInit = 1;
	// setup a dummy VMCS
	// BUG - need to check processor about proper VMCS size

	EvalVmcs = (char *) AllocatePages(VmcsSizeInPages);


	// fill the VMCS regions with 16bit values that will provide us 
	// with indexes into the VMCS

	for( index = 4; index < SIZE_4KB; index += 2 )
	{
		*(unsigned short *)(EvalVmcs + index) = index;
	}

	VmxRevId = AsmReadMsr32(IA32_VMX_BASIC_MSR_INDEX);
	memcpy(EvalVmcs, &VmxRevId, 4);

	AsmVmPtrStore(&CurrentVMCSSave);      // save off the current Vmcs pointer
	AsmVmClear(&CurrentVMCSSave);

	AsmVmPtrLoad((UINT64 *) &EvalVmcs);              // load our indexed Vmcs

	// scan through the table and determine each offset

	for( i = 0; 
		VmcsFieldOffsetTable[i].FieldEncoding != 0xFFFF;
		i++)
	{
		int count = 9;
		FieldValue = VmRead64(VmcsFieldOffsetTable[i].FieldEncoding);
		FieldOffset = FieldValue & 0xFFFFull;

		VmcsFieldOffsetTable[i].FieldOffset = FieldOffset;

	        memcpy(Line, "MapVmcs: ", count);
 		memcpy(&Line[count], VmcsFieldPrintTable[i].FieldPrint, strlen1(VmcsFieldPrintTable[i].FieldPrint));
		count = count + strlen1(VmcsFieldPrintTable[i].FieldPrint);
#define FORMAT1 " :   0x%08x : 0x%08lx : FV 0x%016llx\n"
		memcpy(&Line[count], FORMAT1, strlen1(FORMAT1));
		count = count + strlen1(FORMAT1);
		Line[count] = '\0';

		//DEBUG((EFI_D_ERROR, "MapVmcs: %s  :   %d : %016llx : FV %016llx\n", VmcsFieldPrintTable[i].FieldPrint,
		DEBUG((EFI_D_ERROR, Line,
			VmcsFieldOffsetTable[i].FieldEncoding, 
			VmcsFieldOffsetTable[i].FieldOffset,
			FieldValue));
	}

	AsmVmPtrLoad(&CurrentVMCSSave);       // Put back the orignal Vmcs
	AsmVmClear((UINT64 *)EvalVmcs);

	FreePages(EvalVmcs, VmcsSizeInPages);                  // free up the eval vmcs

	// validate with the current VMCS - especially since we just flushed it

	DEBUG((EFI_D_ERROR, "MapVmcs:  Field/Offset validation\n"));

	for( i = 0; 
		VmcsFieldOffsetTable[i].FieldEncoding != 0xFFFF;
		i++)
	{
		UINT64 VmReadValue;
		UINT64 OffReadValue;

		if(VmcsFieldOffsetTable[i].FieldOffset != 0) // zero offset is not valid
		{
		VmReadValue = VmRead64(VmcsFieldOffsetTable[i].FieldEncoding);
		OffReadValue = *(UINT64*)((UINTN)CurrentVMCSSave + (UINTN)VmcsFieldOffsetTable[i].FieldOffset);
			int count = 9;
			memcpy(Line, "MapVmcs: ", 9);
#define FORMAT2 "  VMREAD: 0x%016llx  Offset read: 0x%016llx\n"
			memcpy(&Line[count], VmcsFieldPrintTable[i].FieldPrint, strlen1(VmcsFieldPrintTable[i].FieldPrint));
			count = count + strlen1(VmcsFieldPrintTable[i].FieldPrint);
			memcpy(&Line[count], FORMAT2, strlen1(FORMAT2));
			count = count + strlen1(FORMAT2);
			Line[count] = '\0';

			//DEBUG((EFI_D_ERROR, "MapVmcs: %s  :   %d : %016llx : FV %016llx\n", VmcsFieldPrintTable[i].FieldPrint,
			DEBUG((EFI_D_ERROR, Line,
				VmReadValue, 
				OffReadValue));
		}
	}
}

// parses the VmcsFieldOffsetTable to find the offset based on the field_encoding
// an Offset of zero (0) indicates no match found (or the encoding is not valid for
// this processor family

UINT32 GetVmcsOffset( UINT32 field_encoding)
{
	int i = 0;

	while(VmcsFieldOffsetTable[i].FieldEncoding != 0xFFFF)
	{
		if(field_encoding == VmcsFieldOffsetTable[i].FieldEncoding)
		{
			DEBUG((EFI_D_ERROR, "GetVmcsOffset - encoding: 0x%lx offset 0x%lx\n",
				field_encoding,
				VmcsFieldOffsetTable[i].FieldOffset));
			return VmcsFieldOffsetTable[i].FieldOffset;
		}
		
		i++;
	}

	DEBUG((EFI_D_ERROR, "GetVmcsOffset - no VMCS offset fount for field encoding 0x%lx\n", field_encoding));
	return 0;   // offset of zero indicates no match found
}
