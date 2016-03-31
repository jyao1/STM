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
;    Exception.asm
;
; Abstract:
;
;------------------------------------------------------------------------------

EXTERNDEF      ExceptionHandlerC:PROC
EXTERNDEF      AsmNmiExceptionHandler:PROC
EXTERNDEF      mExceptionHandlerLength:DWORD

.DATA
mExceptionHandlerLength DD ExceptionHandlerEnd - AsmExceptionHandlers

.CODE

AsmNmiExceptionHandler PROC PUBLIC
    iretq
AsmNmiExceptionHandler ENDP

;------------------------------------------------------------------------------
; ExceptionEntrypoint is the entry point for all exceptions
;
; Stack frame would be as follows as specified in IA32 manuals:
;   SS      +68h
;   RSP     +60h
;   RFLAGS  +58h
;   CS      +50h
;   RIP     +48h
;   ErrCode +40h
;   INT#    +38h
;   RAX     +30h
;   RCX     +28h
;   RDX     +20h
;   R8      +18h
;   R9      +10h
;   R10     +8
;   R11     +0
;
; RSP set to odd multiple of 8 means ErrCode PRESENT
;------------------------------------------------------------------------------
ExceptionEntrypoint PROC PUBLIC
jmp $
    push    rax
    push    rcx
    push    rdx
    push    r8
    push    r9
    push    r10
    push    r11
    mov     rcx, [rsp + 38h]            ; INT#
    mov     rdx, [rsp + 40h]            ; ErrCode (if any)
    add     rsp, -20h
    call    ExceptionHandlerC
    add     rsp, 20h
    pop     r11
    pop     r10
    pop     r9
    pop     r8
    pop     rdx
    pop     rcx
    pop     rax
    add     rsp, 10h                    ; skip INT# & ErrCode
    iretq
ExceptionEntrypoint ENDP

AsmExceptionHandlers PROC PUBLIC
IHDLRIDX = 0
    push    rax                         ; dummy error code
    push    IHDLRIDX
    DB      0e9h                        ; jmp disp32
    DD      ExceptionEntrypoint - ($ + 4)
    DD      090909090h
IHDLRIDX = IHDLRIDX + 1
ExceptionHandlerEnd::
REPEAT      7                           ; INT1 ~ INT7
    push    rax                         ; dummy error code
    push    IHDLRIDX
    DB      0e9h                        ; jmp disp32
    DD      ExceptionEntrypoint - ($ + 4)
    DD      090909090h
IHDLRIDX = IHDLRIDX + 1
            ENDM
REPEAT      1                           ; INT8
    push    IHDLRIDX
    DB      0e9h                        ; jmp disp32
    DD      ExceptionEntrypoint - ($ + 4)
    DB      090h
    DD      090909090h
IHDLRIDX = IHDLRIDX + 1
            ENDM
REPEAT      1                           ; INT9
    push    rax                         ; dummy error code
    push    IHDLRIDX
    DB      0e9h                        ; jmp disp32
    DD      ExceptionEntrypoint - ($ + 4)
    DD      090909090h
IHDLRIDX = IHDLRIDX + 1
            ENDM
REPEAT      5                           ; INT10 ~ INT14
    push    IHDLRIDX
    DB      0e9h                        ; jmp disp32
    DD      ExceptionEntrypoint - ($ + 4)
    DB      090h
    DD      090909090h
IHDLRIDX = IHDLRIDX + 1
            ENDM
REPEAT      2                           ; INT15 ~ INT16
    push    rax                         ; dummy error code
    push    IHDLRIDX
    DB      0e9h                        ; jmp disp32
    DD      ExceptionEntrypoint - ($ + 4)
    DD      090909090h
IHDLRIDX = IHDLRIDX + 1
            ENDM
REPEAT      1                           ; INT17
    push    IHDLRIDX
    DB      0e9h                        ; jmp disp32
    DD      ExceptionEntrypoint - ($ + 4)
    DB      090h
    DD      090909090h
IHDLRIDX = IHDLRIDX + 1
            ENDM
REPEAT      14                          ; INT18 ~ INT31
    push    rax                         ; dummy error code
    push    IHDLRIDX
    DB      0e9h                        ; jmp disp32
    DD      ExceptionEntrypoint - ($ + 4)
    DD      090909090h
IHDLRIDX = IHDLRIDX + 1
            ENDM
REPEAT      96                          ; INT32 ~ INT127
    push    rax                         ; dummy error code
    push    IHDLRIDX
    DB      0e9h                        ; jmp disp32
    DD      ExceptionEntrypoint - ($ + 4)
    DD      090909090h
IHDLRIDX = IHDLRIDX + 1
            ENDM
REPEAT      128                         ; INT128 ~ INT255
    push    rax                         ; dummy error code
    push    IHDLRIDX
    DB      0e9h                        ; jmp disp32
    DD      ExceptionEntrypoint - ($ + 4)
    DB      090h
IHDLRIDX = IHDLRIDX + 1
            ENDM
AsmExceptionHandlers ENDP


  END