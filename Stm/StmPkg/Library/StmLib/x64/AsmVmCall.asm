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
;    AsmVmCall.asm
;
;------------------------------------------------------------------------------

.CODE

;------------------------------------------------------------------------------
;  UINT32
;  AsmVmCall (
;    UINT32  Eax,  // Rcx
;    UINT32  Ebx,  // Rdx
;    UINT32  Ecx,  // R8
;    UINT32  Edx   // R9
;    )
;------------------------------------------------------------------------------
AsmVmCall PROC PUBLIC
    push     rbx
    push     rsi
    push     rdi
    push     rbp
    push     r12
    push     r13
    push     r14
    push     r15

    mov      eax, ecx
    mov      ebx, edx
    mov      ecx, r8d
    mov      edx, r9d
    DB  0fh, 01h, 0c1h           ; VMCALL

    pop      r15
    pop      r14
    pop      r13
    pop      r12
    pop      rbp
    pop      rdi
    pop      rsi
    pop      rbx

    ret
AsmVmCall  ENDP

END
