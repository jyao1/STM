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
#    StmLaunch.s
#
# Abstract:
#
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#  UINT64
#  AsmLaunchStm (
#    UINT32  Eax,  // Rcx
#    UINT32  Ebx,  // Rdx
#    UINT32  Ecx,  // R8
#    UINT32  Edx   // R9
#    )
#------------------------------------------------------------------------------
ASM_GLOBAL ASM_PFX(AsmLaunchStm)
ASM_PFX(AsmLaunchStm):
    push     %rbx
    push     %rsi
    push     %rdi
    push     %rbp
    push     %r10
    push     %r11
    push     %r12
    push     %r13
    push     %r14
    push     %r15

    movl     %ecx, %eax
    movl     %edx, %ebx
    movl     %r8d, %ecx
    movl     %r9d, %edx
    .byte    0xf, 0x1, 0xc1 # VMCALL

    pop      %r15
    pop      %r14
    pop      %r13
    pop      %r12
    pop      %r11
    pop      %r10
    pop      %rbp
    pop      %rdi
    pop      %rsi
    pop      %rbx
    # last step: put EDX to high 32bit of RAX
    movq     $0xffffffff, %rcx
    andq     %rcx, %rax
    andq     %rcx, %rdx
    salq     $32, %rdx   # edx:eax -> rax
    orq      %rdx, %rax  # rax = edx:eax
    ret

ASM_GLOBAL ASM_PFX(AsmTeardownStm)
ASM_PFX(AsmTeardownStm):
    push     %rbx
    push     %rsi
    push     %rdi
    push     %rbp
    push     %r10
    push     %r11
    push     %r12
    push     %r13
    push     %r14
    push     %r15

    movl     %ecx, %eax
    .byte    0xf, 0x1, 0xc1 # VMCALL

    pop      %r15
    pop      %r14
    pop      %r13
    pop      %r12
    pop      %r11
    pop      %r10
    pop      %rbp
    pop      %rdi
    pop      %rsi
    pop      %rbx
    ret

