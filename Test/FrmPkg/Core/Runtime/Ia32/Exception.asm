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

  .686P
  .MMX
  .MODEL SMALL

EXTERNDEF  C    ExceptionHandlerC:PROC
EXTERNDEF  C    AsmNmiExceptionHandler:PROC
EXTERNDEF  C    mExceptionHandlerLength:DWORD
  
.DATA
mExceptionHandlerLength DD ExceptionHandlerEnd - _AsmExceptionHandlers

  .CODE

AsmNmiExceptionHandler PROC C PUBLIC
    iretd
AsmNmiExceptionHandler ENDP

;------------------------------------------------------------------------------
; ExceptionEntrypoint is the entry point for all exceptions
;
; Stack frame would be as follows as specified in IA32 manuals:
;   RFLAGS  +1ch
;   CS      +18h
;   RIP     +14h
;   ErrCode +10h
;   INT#    +0ch
;   EAX     +8
;   ECX     +4
;   EDX     +0
;
; RSP set to odd multiple of 8 means ErrCode PRESENT
;------------------------------------------------------------------------------
ExceptionEntrypoint PROC
jmp $
    push    eax
    push    ecx
    push    edx
    mov     ecx, [esp + 0ch]
    push    [esp + 10h]
    push    ecx
    call    ExceptionHandlerC
    pop     ecx
    pop     ecx
    pop     edx
    pop     ecx
    pop     eax
    add     esp, 8                      ; skip INT# & ErrCode
    iretd
ExceptionEntrypoint ENDP

_AsmExceptionHandlers PROC PUBLIC
IHDLRIDX = 0
    push    eax                         ; dummy error code
    push    IHDLRIDX
    DB      0e9h                        ; jmp disp32
    DD      ExceptionEntrypoint - ($ + 4)
    DD      090909090h
IHDLRIDX = IHDLRIDX + 1
ExceptionHandlerEnd::
REPEAT      7                           ; INT1 ~ INT7
    push    eax                         ; dummy error code
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
    push    eax                         ; dummy error code
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
    push    eax                         ; dummy error code
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
    push    eax                         ; dummy error code
    push    IHDLRIDX
    DB      0e9h                        ; jmp disp32
    DD      ExceptionEntrypoint - ($ + 4)
    DD      090909090h
IHDLRIDX = IHDLRIDX + 1
            ENDM
REPEAT      96                          ; INT32 ~ INT127
    push    eax                         ; dummy error code
    push    IHDLRIDX
    DB      0e9h                        ; jmp disp32
    DD      ExceptionEntrypoint - ($ + 4)
    DD      090909090h
IHDLRIDX = IHDLRIDX + 1
            ENDM
REPEAT      128                         ; INT128 ~ INT255
    push    eax                         ; dummy error code
    push    IHDLRIDX
    DB      0e9h                        ; jmp disp32
    DD      ExceptionEntrypoint - ($ + 4)
    DB      090h
IHDLRIDX = IHDLRIDX + 1
            ENDM
_AsmExceptionHandlers ENDP

  END

