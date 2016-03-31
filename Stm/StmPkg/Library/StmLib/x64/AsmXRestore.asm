;------------------------------------------------------------------------------
;
; Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
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
;    AsmXRestore.asm
;
;------------------------------------------------------------------------------

.CODE

AsmXRestore PROC PUBLIC
    mov     r8,  rdx ; r8 = XStateBuffer
    mov     eax, ecx ; eax = mask[31:0]
    shr     rcx, 20h
    mov     edx, ecx ; edx = mask[63:32]
    mov     rcx, r8  ; rcx = XStateBuffer
    db      048h, 0fh, 0aeh, 028h+01 ; xrstor   [rcx]
    ret
AsmXRestore  ENDP

END
