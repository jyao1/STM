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

  .686P
  .MMX
  .MODEL FLAT,C
  .CODE

;------------------------------------------------------------------------------
;  UINT32
;  AsmVmCall (
;    UINT32  Eax,  // [ESP + 8h]
;    UINT32  Ebx,  // [ESP + 0Ch]
;    UINT32  Ecx,  // [ESP + 10h]
;    UINT32  Edx   // [ESP + 14h]
;    )
;------------------------------------------------------------------------------
AsmVmCall PROC PUBLIC
    push     ebx
    mov      eax, [esp+8h]
    mov      ebx, [esp+0ch]
    mov      ecx, [esp+10h]
    mov      edx, [esp+14h]
    push     esi
    push     edi
    push     ebp
    DB  0fh, 01h, 0c1h           ; VMCALL
    pop      ebp
    pop      edi
    pop      esi
    pop      ebx
    ret
AsmVmCall  ENDP

  END

