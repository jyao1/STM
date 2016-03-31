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
#   AsmTestAndReset.s
#
#------------------------------------------------------------------------------

ASM_GLOBAL ASM_PFX(AsmTestAndReset)
ASM_PFX(AsmTestAndReset):
    xorl     %eax, %eax
    lock btr %ecx, (%rdx)
    adcl     %eax, %eax
    ret
