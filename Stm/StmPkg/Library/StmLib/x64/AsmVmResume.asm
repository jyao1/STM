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
;    AsmVmResume.asm
;
;------------------------------------------------------------------------------

.CODE

AsmVmResume PROC PUBLIC
  push    rbx
  push    rbp
  push    rdi
  push    rsi
  push    r12
  push    r13
  push    r14
  push    r15
  mov     rax, [rcx + 0]
  mov     rdx, [rcx + 16]
  mov     rbx, [rcx + 24]
  mov     rbp, [rcx + 40]
  mov     rsi, [rcx + 48]
  mov     rdi, [rcx + 56]
  mov     r8,  [rcx + 64]
  mov     r9,  [rcx + 72]
  mov     r10, [rcx + 80]
  mov     r11, [rcx + 88]
  mov     r12, [rcx + 96]
  mov     r13, [rcx + 104]
  mov     r14, [rcx + 112]
  mov     r15, [rcx + 120]
  fxrstor [rcx + 128]
  mov     rcx, [rcx + 8]
  DB  0fh, 01h, 0c3h           ; VMRESUME
  pushfq
  pop     rax
  pop     r15
  pop     r14
  pop     r13
  pop     r12
  pop     rsi
  pop     rdi
  pop     rbp
  pop     rbx
  ret
AsmVmResume ENDP

END
