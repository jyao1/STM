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
; Abstract:
;
;------------------------------------------------------------------------------

  .686P
  .MMX
  .MODEL SMALL
  .CODE

EXTERNDEF  C   FrmHandlerC:PROC

AsmHostEntrypoint PROC C PUBLIC
  push edi
  push esi
  push ebp
  push ebp ; should be esp
  push ebx
  push edx
  push ecx
  push eax
  mov  ecx, esp ; parameter
  push ecx
  call FrmHandlerC
  add  esp, 4
  jmp $
AsmHostEntrypoint ENDP

  END

