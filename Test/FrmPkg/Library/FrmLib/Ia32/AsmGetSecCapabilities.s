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
#   AsmGetSecCapabilities.s
#
#------------------------------------------------------------------------------

ASM_GLOBAL ASM_PFX(AsmGetSecCapabilities)

#------------------------------------------------------------------------------
#  UINT32
#  AsmGetSecCapabilities (
#    IN UINT32       Index
#    )
#------------------------------------------------------------------------------

ASM_PFX(AsmGetSecCapabilities):
    push  %ebx
    movl  8(%esp), %ebx
    movl  $0, %eax # GET_SEC_CAPABILITIES
    .byte 0x0F # GETSEC
    .byte 0x37
    pop   %ebx
    ret

