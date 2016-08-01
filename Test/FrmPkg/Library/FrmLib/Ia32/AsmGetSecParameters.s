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
#   AsmGetSecParameters.s
#
#------------------------------------------------------------------------------

ASM_GLOBAL ASM_PFX(AsmGetSecParameters)

#------------------------------------------------------------------------------
#  VOID
#  AsmGetSecParameters (
#    IN  UINT32       Index,
#    OUT UINT32       *RegEax,
#    OUT UINT32       *RegEbx,
#    OUT UINT32       *RegEcx
#    )
#------------------------------------------------------------------------------

ASM_PFX(AsmGetSecParameters):
    push  %ebx
    movl  8(%esp), %ebx
    movl  $6, %eax # GET_SEC_PARAMETERS
    .byte 0x0F # GETSEC
    .byte 0x37
    movl  0xc(%esp), %edx
    movl  %eax, (%edx)
    movl  0x10(%esp), %edx
    movl  %ebx, (%edx)
    movl  0x14(%esp), %edx
    movl  %ecx, (%edx)
    pop   %ebx
    ret

