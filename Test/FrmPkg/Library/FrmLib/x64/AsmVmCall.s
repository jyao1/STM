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
#   AsmVmCall.s
#
#------------------------------------------------------------------------------

ASM_GLOBAL ASM_PFX(AsmVmCall)
#------------------------------------------------------------------------------
#  UINT32
#  AsmVmCall (
#    UINT32  Eax,  // [ESP + 8h]
#    UINT32  Ebx,  // [ESP + 0Ch]
#    UINT32  Ecx,  // [ESP + 10h]
#    UINT32  Edx   // [ESP + 14h]
#    )
#------------------------------------------------------------------------------
ASM_PFX(AsmVmCall):
    push     %rbx
    push     %rsi
    push     %rdi
    push     %rbp
    push     %r12
    push     %r13
    push     %r14
    push     %r15

    movl     %ecx, %eax
    movl     %edx, %ebx
    movl     %r8d, %ecx
    movl     %r9d, %edx
    .byte  0x0f, 0x01, 0xc1           # VMCALL

    pop      %r15
    pop      %r14
    pop      %r13
    pop      %r12
    pop      %rbp
    pop      %rdi
    pop      %rsi
    pop      %rbx

    ret
