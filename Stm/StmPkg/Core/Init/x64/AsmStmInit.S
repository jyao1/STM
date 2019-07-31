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
#   AsmStmInit.s
#
#------------------------------------------------------------------------------

ASM_GLOBAL ASM_PFX(InitializeSmmMonitor)
ASM_GLOBAL ASM_PFX(_ModuleEntryPoint)

.equ STM_API_START,                 0x00010001
.equ STM_API_INITIALIZE_PROTECTION, 0x00010007

.equ STM_STACK_SIZE,                0x020000

#------------------------------------------------------------------------------
# VOID
# AsmInitializeSmmMonitor (
#   VOID
#   )
ASM_PFX(_ModuleEntryPoint):
  cmpl $STM_API_INITIALIZE_PROTECTION, %eax # for BSP
  jz  GoBsp
  cmpl $STM_API_START, %eax # for AP
  jz  GoAp
  jmp DeadLoop

GoBsp:
  # Assume ThisOffset is 0
  # ESP is pointer to stack bottom, NOT top
  movl $STM_STACK_SIZE, %eax     # eax = STM_STACK_SIZE, 
  lock xaddl %eax, (%esp)        # eax = ThisOffset, ThisOffset += STM_STACK_SIZE (LOCK instruction)
  addl $STM_STACK_SIZE, %eax     # eax = ThisOffset + STM_STACK_SIZE
  addl %eax, %esp                # esp += ThisOffset + STM_STACK_SIZE

  #
  # Jump to C code
  #
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
  movl $STM_API_INITIALIZE_PROTECTION, %eax
  push %rax
  movq %rsp, %rcx # parameter
  subq $0x20, %rsp
  call ASM_PFX(InitializeSmmMonitor)
  addq $0x20, %rsp
  # should never get here
  jmp  DeadLoop

GoAp:
  #
  # assign unique ESP for each processor
  #
# |------------|<-ESP (PerProc)
# | Reg        |
# |------------|
# | Stack      |
# |------------|
# | ThisOffset |
# +------------+<-ESP (Common)
# | Heap       |

  # Assume ThisOffset is 0
  # ESP is pointer to stack bottom, NOT top
  movl $STM_STACK_SIZE, %eax      # eax = STM_STACK_SIZE, 
  lock xaddl %eax, (%esp)         # eax = ThisOffset, ThisOffset += STM_STACK_SIZE (LOCK instruction)
  addl $STM_STACK_SIZE, %eax      # eax = ThisOffset + STM_STACK_SIZE
  addl %eax, %esp                 # esp += ThisOffset + STM_STACK_SIZE

  #
  # Jump to C code
  #
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
  movl $STM_API_START, %eax
  push %rax
  movq %rsp, %rcx # parameter
  subq $0x20, %rsp
  call ASM_PFX(InitializeSmmMonitor)
  addq $0x20, %rsp
  # should never get here
DeadLoop:
  jmp .

