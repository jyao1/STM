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
#    Smx.i
#
# Abstract:
#
#------------------------------------------------------------------------------

.equ GET_SEC_CAPABILITIES,  0
.equ GET_SEC_ENTERACCS,     2
.equ GET_SEC_EXITAC,        3
.equ GET_SEC_SENTER,        4
.equ GET_SEC_SEXIT,         5
.equ GET_SEC_PARAMETERS,    6
.equ GET_SEC_SMCTRL,        7
.equ GET_SEC_WAKEUP,        8

.equ TXT_PUBLIC_SPACE,      0xFED30000
.equ TXT_HEAP_BASE,         0x300

.equ _TXT_OS_TO_MLE_DATA_Signature,                0
.equ _TXT_OS_TO_MLE_DATA_MlePrivateDataAddress,    8

# do not know how to define structure
.equ _MLE_PRIVATE_DATA_GdtrReg, 0
.equ _MLE_PRIVATE_DATA_IdtrReg, 6
.equ _MLE_PRIVATE_DATA_TempEsp, 12
.equ _MLE_PRIVATE_DATA_TempEspRlp, 16
.equ _MLE_PRIVATE_DATA_Cr3, 20
.equ _MLE_PRIVATE_DATA_PostSinitOffset, 24
.equ _MLE_PRIVATE_DATA_PostSinitSegment, 28
.equ _MLE_PRIVATE_DATA_DsSeg, 32
.equ _MLE_PRIVATE_DATA_Lock, 36
.equ _MLE_PRIVATE_DATA_RlpInitializedNumber, 40
.equ _MLE_PRIVATE_DATA_RlpPostSinitOffset, 44
.equ _MLE_PRIVATE_DATA_RlpPostSinitSegment, 48
.equ _MLE_PRIVATE_DATA_RlpDsSeg, 52
.equ _MLE_PRIVATE_DATA_ApEntry, 56

