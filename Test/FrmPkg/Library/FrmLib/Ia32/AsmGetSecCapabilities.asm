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
;    AsmGetSecCapabilities.asm
;
;------------------------------------------------------------------------------

.686P
.MODEL FLAT, C
.CODE

;------------------------------------------------------------------------------
;  UINT32
;  AsmGetSecCapabilities (
;    IN UINT32       Index
;    )
;------------------------------------------------------------------------------

AsmGetSecCapabilities PROC  PUBLIC
    push  ebx
    mov   ebx, [esp+8]
    mov   eax, 0 ; GET_SEC_CAPABILITIES
    DB 0fh, 37h ; GETSEC
    pop   ebx
    ret
AsmGetSecCapabilities  ENDP

END

