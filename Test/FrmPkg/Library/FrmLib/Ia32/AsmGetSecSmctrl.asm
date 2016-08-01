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
;    AsmGetSecSmctrl.asm
;
;------------------------------------------------------------------------------

.686P
.MODEL FLAT, C
.CODE

;------------------------------------------------------------------------------
;  VOID
;  AsmGetSecSmctrl (
;    IN UINT32       Operation
;    )
;------------------------------------------------------------------------------

AsmGetSecSmctrl PROC  PUBLIC
    push  ebx
    mov   ebx, [esp+8]
    mov   eax, 7 ; GET_SEC_SMCTRL
    DB 0fh, 37h ; GETSEC
    pop   ebx
    ret
AsmGetSecSmctrl  ENDP

END

