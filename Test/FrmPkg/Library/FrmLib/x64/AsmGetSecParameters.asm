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
    push  rbx
    mov   ebx, ecx
    mov   eax, 6 ; GET_SEC_PARAMETERS
    DB 0fh, 37h ; GETSEC
    mov   [rdx], eax
    mov   [r8],  ebx
    mov   [r9],  ecx
    pop   rbx
    ret
AsmGetSecParameters  ENDP

END

