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
#   VmExit.s
#
#------------------------------------------------------------------------------

ASM_GLOBAL ASM_PFX(StmHandlerSmi)
ASM_GLOBAL ASM_PFX(StmHandlerSmm)
ASM_GLOBAL ASM_PFX(AsmHostEntrypointSmi)
ASM_GLOBAL ASM_PFX(AsmHostEntrypointSmm)

ASM_PFX(AsmHostEntrypointSmi):
  push %edi
  push %esi
  push %ebp
  push %ebp # should be esp
  push %ebx
  push %edx
  push %ecx
  push %eax
  movl %esp, %ecx # parameter
  push %ecx
  call ASM_PFX(StmHandlerSmi)
  addl $4, %esp
  jmp .

ASM_PFX(AsmHostEntrypointSmm):
  push %edi
  push %esi
  push %ebp
  push %ebp # should be rsp
  push %ebx
  push %edx
  push %ecx
  push %eax
  movl %esp, %ecx # parameter
  push %ecx
  call ASM_PFX(StmHandlerSmm)
  addl $4, %esp
  jmp .

