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
#   AsmGetSecSenter.s
#
#------------------------------------------------------------------------------

ASM_GLOBAL ASM_PFX(AsmGetSecSenter)

#------------------------------------------------------------------------------
#  VOID
#  AsmGetSecSenter (
#    IN UINT32       AcmBase,
#    IN UINT32       AcmSize,
#    IN UINT32       FunctionalityLevel
#)
#------------------------------------------------------------------------------

ASM_PFX(AsmGetSecSenter):
    push  %ebx
    movl  8(%esp), %ebx
    movl  12(%esp), %ecx
    movl  16(%esp), %edx
    movl  $4, %eax # GET_SEC_SENTER
    .byte 0x0F # GETSEC
    .byte 0x37
    pop   %ebx
    ret

