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
;    AsmGetSecSenter.asm
;
;------------------------------------------------------------------------------

.686P
.MODEL FLAT, C
.CODE

;------------------------------------------------------------------------------
;  VOID
;  AsmGetSecSenter (
;    IN UINT32       AcmBase,
;    IN UINT32       AcmSize,
;    IN UINT32       FunctionalityLevel
;    )
;------------------------------------------------------------------------------

AsmGetSecSenter PROC  PUBLIC
    push  ebx
    mov   ebx, [esp+8]
    mov   ecx, [esp+12]
    mov   edx, [esp+16]
    mov   eax, 4 ; GET_SEC_SENTER
    DB 0fh, 37h ; GETSEC
    pop   ebx
    ret
AsmGetSecSenter  ENDP

END

