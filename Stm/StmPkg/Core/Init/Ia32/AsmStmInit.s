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
  push %edi
  push %esi
  push %ebp
  push %ebp # should be esp
  push %ebx
  push %edx
  push %ecx
  movl $STM_API_INITIALIZE_PROTECTION, %eax
  push %eax
  movl %esp, %ecx # parameter
  push %ecx
  call ASM_PFX(InitializeSmmMonitor)
  addl $4, %esp
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
  push %edi
  push %esi
  push %ebp
  push %ebp # should be esp
  push %ebx
  push %edx
  push %ecx
  movl $STM_API_START, %eax
  push %eax
  movl %esp, %ecx # parameter
  push %ecx
  call ASM_PFX(InitializeSmmMonitor)
  addl $4, %esp
  # should never get here
DeadLoop:
  jmp .

