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
;    AsmStmInit.asm
;
;------------------------------------------------------------------------------

.CODE

externdef InitializeSmmMonitor:NEAR
externdef _ModuleEntryPoint:NEAR

STM_API_START                 EQU 00010001h
STM_API_INITIALIZE_PROTECTION EQU 00010007h

STM_STACK_SIZE                EQU 08000h

;------------------------------------------------------------------------------
; VOID
; AsmInitializeSmmMonitor (
;   VOID
;   )
_ModuleEntryPoint PROC PUBLIC
  cmp eax, STM_API_INITIALIZE_PROTECTION ; for BSP
  jz  GoBsp
  cmp eax, STM_API_START ; for AP
  jz  GoAp
  jmp DeadLoop

GoBsp:
  ; Assume ThisOffset is 0
  ; ESP is pointer to stack bottom, NOT top
  mov  eax, STM_STACK_SIZE     ; eax = STM_STACK_SIZE, 
  lock xadd [esp], eax         ; eax = ThisOffset, ThisOffset += STM_STACK_SIZE (LOCK instruction)
  add  eax, STM_STACK_SIZE     ; eax = ThisOffset + STM_STACK_SIZE
  add  esp, eax                ; esp += ThisOffset + STM_STACK_SIZE

  ;
  ; Jump to C code
  ;
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
  mov  eax, STM_API_INITIALIZE_PROTECTION
  push rax
  mov  rcx, rsp ; parameter
  sub  rsp, 20h
  call InitializeSmmMonitor
  add  rsp, 20h
  ; should never get here
  jmp  DeadLoop

GoAp:
  ;
  ; assign unique ESP for each processor
  ;
; |------------|<-ESP (PerProc)
; | Reg        |
; |------------|
; | Stack      |
; |------------|
; | ThisOffset |
; +------------+<-ESP (Common)
; | Heap       |

  ; Assume ThisOffset is 0
  ; ESP is pointer to stack bottom, NOT top
  mov  eax, STM_STACK_SIZE     ; eax = STM_STACK_SIZE, 
  lock xadd [esp], eax         ; eax = ThisOffset, ThisOffset += STM_STACK_SIZE (LOCK instruction)
  add  eax, STM_STACK_SIZE     ; eax = ThisOffset + STM_STACK_SIZE
  add  esp, eax                ; esp += ThisOffset + STM_STACK_SIZE

  ;
  ; Jump to C code
  ;
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
  mov  eax, STM_API_START
  push rax
  mov  rcx, rsp ; parameter
  sub  rsp, 20h
  call InitializeSmmMonitor
  add  rsp, 20h
  ; should never get here
DeadLoop:
  jmp $
_ModuleEntryPoint ENDP

END
