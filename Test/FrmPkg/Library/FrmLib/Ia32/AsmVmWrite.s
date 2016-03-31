#------------------------------------------------------------------------------
#
# Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
# This program and the accompanying materials
# are licensed and made available under the terms and conditions of the BSD License
# which accompanies this distribution.  The full text of the license may be found at
# http://opensource.org/licenses/bsd-license.php.
#
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
# Module Name:
#
#   AsmVmxWrite.s
#
#------------------------------------------------------------------------------

ASM_GLOBAL ASM_PFX(AsmVmWrite)
ASM_PFX(AsmVmWrite):
    movl   4(%esp), %eax
    movl   8(%esp), %ecx
    .byte  0x0f, 0x79, 0xc1          # VMWRITE rax, rcx
    pushfl
    pop    %eax
    ret

