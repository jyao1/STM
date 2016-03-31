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
#   AsmVmxResume.s
#
#------------------------------------------------------------------------------

ASM_GLOBAL ASM_PFX(AsmVmResume)
ASM_PFX(AsmVmResume):
    push   %ebx
    push   %ebp
    push   %edi
    push   %esi
    movl   20(%esp), %esi
    movl   0(%esi), %eax
    movl   4(%esi), %ecx
    movl   8(%esi), %edx
    movl   12(%esi), %ebx
    movl   20(%esi), %ebp
    movl   28(%esi), %edi
    movl   24(%esi), %esi
    .byte  0x0f, 0x01, 0xc3           # VMRESUME
    pushfl
    pop    %eax
    pop    %esi
    pop    %edi
    pop    %ebp
    pop    %ebx
    ret

