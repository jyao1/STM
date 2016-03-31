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
;    AsmXRestore.asm
;
;------------------------------------------------------------------------------

.686p
.MODEL flat,c
.CODE

AsmXRestore PROC PUBLIC
    mov     eax, [esp + 4]  ; eax = mask[31:0]
    mov     edx, [esp + 8]  ; edx = mask[63:32]
    mov     ecx, [esp + 12] ; ecx = XStateBuffer
    db      0fh, 0aeh, 028h+01 ; xrstor [ecx]
    ret
AsmXRestore  ENDP

END
