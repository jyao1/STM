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
#   PeVmExit.s
#
#------------------------------------------------------------------------------

ASM_GLOBAL ASM_PFX(PeStmHandlerSmm)
ASM_GLOBAL ASM_PFX(AsmHostEntrypointSmmPe)

ASM_PFX(AsmHostEntrypointSmmPe):
  sub  %rsp, 512
  fxsave (%rsp)
  push %r15
  push %r14
  push %r13
  push %r12
  push %r11
  push %r10
  push %r9
  push %r8
  push %rdi
  push %rsi
  push %rbp
  push %rbp # should be rsp
  push %rbx
  push %rdx
  push %rcx
  push %rax
  movq %rsp, %rcx # parameter for MS
  movq %rsp, %rdi # parameter for GCC
  subq $0x30, %rsp
  call ASM_PFX(PeStmHandlerSmm)
  addq $0x30, %rsp
  jmp .

