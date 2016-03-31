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
#   AsmXSave.s
#
#------------------------------------------------------------------------------

ASM_GLOBAL ASM_PFX(AsmXSave)
ASM_PFX(AsmXSave):
    movl    4(%esp),  %eax # eax = mask[31:0]
    movl    8(%esp),  %edx # edx = mask[63:32]
    movl    12(%esp), %ecx # ecx = XStateBuffer
    xsave   (%ecx)
    ret
