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
#    VmExit.asm
#
# Abstract:
#
#------------------------------------------------------------------------------


ASM_GLOBAL ASM_PFX(FrmHandlerC)

.text

ASM_GLOBAL ASM_PFX(AsmHostEntrypoint)
ASM_PFX(AsmHostEntrypoint):
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
  call FrmHandlerC
  addl $4, %esp
  jmp .

