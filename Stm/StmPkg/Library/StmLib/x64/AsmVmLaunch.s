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
#   AsmVmLaunch.s
#
#------------------------------------------------------------------------------

ASM_GLOBAL ASM_PFX(AsmVmLaunch)
ASM_PFX(AsmVmLaunch):
    push   %rbx
    push   %rbp
    push   %rdi
    push   %rsi
    push   %r12
    push   %r13
    push   %r14
    push   %r15
    movq   0(%rcx), %rax
    movq   16(%rcx), %rdx
    movq   24(%rcx), %rbx
    movq   40(%rcx), %rbp
    movq   48(%rcx), %rsi
    movq   56(%rcx), %rdi
    movq   64(%rcx), %r8
    movq   72(%rcx), %r9
    movq   80(%rcx), %r10
    movq   88(%rcx), %r11
    movq   96(%rcx), %r12
    movq   104(%rcx), %r13
    movq   112(%rcx), %r14
    movq   120(%rcx), %r15
    .byte  0x0f, 0xae, 0x89, 0x80, 0x00, 0x00, 0x00 # fxrstor 128(%rcx)
    movq   8(%rcx), %rcx
    .byte  0x0f, 0x01, 0xc2           # VMLAUNCH
    pushfq
    pop    %rax
    pop    %r15
    pop    %r14
    pop    %r13
    pop    %r12
    pop    %rsi
    pop    %rdi
    pop    %rbp
    pop    %rbx
    ret
