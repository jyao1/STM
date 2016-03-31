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

.CODE

AsmVmRead PROC PUBLIC
    xor   rax, rax
    mov   eax, ecx
    xor   rcx, rcx
    DB  0fh, 078h, 0c1h          ; VMREAD rcx, rax
    pushfq
    pop   rax
    mov   [rdx], rcx
    ret
AsmVmRead  ENDP

END
