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
;    AsmVmRead.asm
;
;------------------------------------------------------------------------------

  .686P
  .MMX
  .MODEL FLAT,C
  .CODE

AsmVmRead PROC PUBLIC
    mov   eax, [esp + 4]
    xor   ecx, ecx
    DB  0fh, 078h, 0c1h          ; VMREAD rcx, rax
    pushfd
    pop   eax
    mov   edx, [esp + 8]
    mov   [edx], ecx
    ret
AsmVmRead  ENDP

  END

