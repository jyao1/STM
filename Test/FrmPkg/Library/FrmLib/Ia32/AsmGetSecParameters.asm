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
;    AsmGetSecParameters.asm
;
;------------------------------------------------------------------------------

.686P
.MODEL FLAT, C
.CODE

;------------------------------------------------------------------------------
;  VOID
;  AsmGetSecParameters (
;    IN  UINT32       Index,
;    OUT UINT32       *RegEax,
;    OUT UINT32       *RegEbx,
;    OUT UINT32       *RegEcx
;    )
;------------------------------------------------------------------------------

AsmGetSecParameters PROC  PUBLIC
    push  ebx
    mov   ebx, [esp+8]
    mov   eax, 6 ; GET_SEC_PARAMETERS
    DB 0fh, 37h ; GETSEC
    mov   edx, [esp+0ch]
    mov   [edx], eax
    mov   edx, [esp+10h]
    mov   [edx], ebx
    mov   edx, [esp+14h]
    mov   [edx], ecx
    pop   ebx
    ret
AsmGetSecParameters  ENDP

END

