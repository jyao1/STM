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

.686P
.MODEL FLAT, C
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
  push edi
  push esi
  push ebp
  push ebp ; should be esp
  push ebx
  push edx
  push ecx
  mov  eax, STM_API_INITIALIZE_PROTECTION
  push eax
  mov  ecx, esp ; parameter
  push ecx
  call InitializeSmmMonitor
  add  esp, 4
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
  push edi
  push esi
  push ebp
  push ebp ; should be esp
  push ebx
  push edx
  push ecx
  mov  eax, STM_API_START
  push eax
  mov  ecx, esp ; parameter
  push ecx
  call InitializeSmmMonitor
  add  esp, 4
  ; should never get here
DeadLoop:
  jmp $
_ModuleEntryPoint ENDP

END
