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
;    VmExit.asm
;
;------------------------------------------------------------------------------

EXTERNDEF      StmHandlerSmi:PROC
EXTERNDEF      StmHandlerSmm:PROC

.CODE

AsmHostEntrypointSmi PROC PUBLIC
  sub rsp, 512
  fxsave  [rsp]
  push r15
  push r14
  push r13
  push r12
  push r11
  push r10
  push r9
  push r8
  push rdi
  push rsi
  push rbp
  push rbp ; should be rsp
  push rbx
  push rdx
  push rcx
  push rax
  mov  rcx, rsp ; parameter
  sub  rsp, 20h
  call StmHandlerSmi
  add  rsp, 20h
  jmp $
AsmHostEntrypointSmi ENDP

AsmHostEntrypointSmm PROC PUBLIC
  sub rsp, 512
  fxsave  [rsp]
  push r15
  push r14
  push r13
  push r12
  push r11
  push r10
  push r9
  push r8
  push rdi
  push rsi
  push rbp
  push rbp ; should be rsp
  push rbx
  push rdx
  push rcx
  push rax
  mov  rcx, rsp ; parameter
  sub  rsp, 20h
  call StmHandlerSmm
  add  rsp, 20h
  jmp $
AsmHostEntrypointSmm ENDP

  END