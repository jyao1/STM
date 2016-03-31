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
;    StmLaunch.asm
;
; Abstract:
;
;------------------------------------------------------------------------------

  .CODE

;------------------------------------------------------------------------------
;  UINT64
;  AsmLaunchStm (
;    UINT32  Eax,  // Rcx
;    UINT32  Ebx,  // Rdx
;    UINT32  Ecx,  // R8
;    UINT32  Edx   // R9
;    )
;------------------------------------------------------------------------------
AsmLaunchStm PROC PUBLIC
    push     rbx
    push     rsi
    push     rdi
    push     rbp
    push     r10
    push     r11
    push     r12
    push     r13
    push     r14
    push     r15

    mov      eax, ecx
    mov      ebx, edx
    mov      ecx, r8d
    mov      edx, r9d
    DB  0fh, 01h, 0c1h ; VMCALL

    pop      r15
    pop      r14
    pop      r13
    pop      r12
    pop      r11
    pop      r10
    pop      rbp
    pop      rdi
    pop      rsi
    pop      rbx
    ; last step: put EDX to high 32bit of RAX
    and      rax, 0ffffffffh
    and      rdx, 0ffffffffh
    sal      rdx, 32   ; edx:eax -> rax
    or       rax, rdx  ; rax = edx:eax
    ret
AsmLaunchStm  ENDP

AsmTeardownStm PROC PUBLIC
    push     rbx
    push     rsi
    push     rdi
    push     rbp
    push     r10
    push     r11
    push     r12
    push     r13
    push     r14
    push     r15

    mov      eax, ecx
    DB  0fh, 01h, 0c1h ; VMCALL

    pop      r15
    pop      r14
    pop      r13
    pop      r12
    pop      r11
    pop      r10
    pop      rbp
    pop      rdi
    pop      rsi
    pop      rbx
    ret
AsmTeardownStm ENDP

  END

