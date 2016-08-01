#------------------------------------------------------------------------------
#
# Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
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
#   AsmGetSecSmctrl.s
#
#------------------------------------------------------------------------------

ASM_GLOBAL ASM_PFX(AsmGetSecSmctrl)

#------------------------------------------------------------------------------
#  VOID
#  AsmGetSecSmctrl (
#    IN UINT32       Operation
#    )
#------------------------------------------------------------------------------

ASM_PFX(AsmGetSecSmctrl):
    push  %rbx
    movl  %ecx, %ebx
    movl  $7, %eax # GET_SEC_SMCTRL
    .byte 0x0F # GETSEC
    .byte 0x37
    pop   %rbx
    ret

