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
;    AsmXSetBv.asm
;
;------------------------------------------------------------------------------

.CODE

AsmXSetBv PROC PUBLIC
    mov     rax, rdx       ; meanwhile, rax <- return value
    shr     rdx, 20h       ; edx:eax contains the value to write
    db      0fh, 01h, 0d1h ; xsetbv
    ret
AsmXSetBv  ENDP

END
