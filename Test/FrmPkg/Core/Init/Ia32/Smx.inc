;------------------------------------------------------------------------------
;
; Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
; This program and the accompanying materials
; are licensed and made available under the terms and conditions of the BSD License
; which accompanies this distribution.  The full text of the license may be found at
; http://opensource.org/licenses/bsd-license.php.
;
; THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
; WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
;
; Module Name:
; 
;    Smx.inc
;
; Abstract:
;
;------------------------------------------------------------------------------

GET_SEC_CAPABILITIES  EQU  0
GET_SEC_ENTERACCS     EQU  2
GET_SEC_EXITAC        EQU  3
GET_SEC_SENTER        EQU  4
GET_SEC_SEXIT         EQU  5
GET_SEC_PARAMETERS    EQU  6
GET_SEC_SMCTRL        EQU  7
GET_SEC_WAKEUP        EQU  8

TXT_PUBLIC_SPACE      EQU 0FED30000h
TXT_HEAP_BASE         EQU 0300h

_IA32_DESCRIPTOR      STRUCT 2
_Limit                WORD            ?
_Base                 DWORD           ?
_IA32_DESCRIPTOR      ENDS

_TXT_OS_TO_MLE_DATA    STRUCT 2
_Signature             DWORD             0
_Reserved              DWORD             0
_MlePrivateDataAddress QWORD             0
_TXT_OS_TO_MLE_DATA    ENDS

_MLE_PRIVATE_DATA   STRUCT 2
_GdtrReg              _IA32_DESCRIPTOR  <>
_IdtrReg              _IA32_DESCRIPTOR  <>
_TempEsp              DWORD             0
_TempEspRlp           DWORD             0
_Cr3                  DWORD             0
_PostSinitOffset      DWORD             0
_PostSinitSegment     DWORD             0
_DsSeg                DWORD             0
_Lock                 DWORD             0
_RlpInitializedNumber DWORD             0
_RlpPostSinitOffset   DWORD             0
_RlpPostSinitSegment  DWORD             0
_RlpDsSeg             DWORD             0
_ApEntry              DWORD             0
_MLE_PRIVATE_DATA     ENDS
