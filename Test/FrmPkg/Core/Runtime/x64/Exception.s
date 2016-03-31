#------------------------------------------------------------------------------
#
# Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
# This program and the accompanying materials
# are licensed and made available under the terms and conditions of the BSD License
# which accompanies this distribution.  The full text of the license may be found at
# http://opensource.org/licenses/bsd-license.php.
#
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
# Module Name:
# 
#    Exception.s
#
# Abstract:
#
#------------------------------------------------------------------------------

ASM_GLOBAL  ASM_PFX(ExceptionHandlerC)
ASM_GLOBAL  ASM_PFX(AsmNmiExceptionHandler)
ASM_GLOBAL  ASM_PFX(mExceptionHandlerLength)

.data
ASM_PFX(mExceptionHandlerLength): .long ExceptionHandlerEnd - ASM_PFX(AsmExceptionHandlers)

.text

ASM_PFX(AsmNmiExceptionHandler):
    iretq

#------------------------------------------------------------------------------
# ExceptionEntrypoint is the entry point for all exceptions
#
# Stack frame would be as follows as specified in IA32 manuals:
#   SS      +68h
#   RSP     +60h
#   RFLAGS  +58h
#   CS      +50h
#   RIP     +48h
#   ErrCode +40h
#   INT#    +38h
#   RAX     +30h
#   RCX     +28h
#   RDX     +20h
#   R8      +18h
#   R9      +10h
#   R10     +8
#   R11     +0
#
# RSP set to odd multiple of 8 means ErrCode PRESENT
#------------------------------------------------------------------------------
ExceptionEntrypoint:
jmp .
    push    %rax
    push    %rcx
    push    %rdx
    push    %r8
    push    %r9
    push    %r10
    push    %r11
    movq    0x38(%rsp), %rcx            # INT#
    movq    0x40(%rsp), %rdx            # ErrCode (if any)
    addq    $-0x20, %rsp
    call    ExceptionHandlerC
    addq    $0x20, %rsp
    pop     %r11
    pop     %r10
    pop     %r9
    pop     %r8
    pop     %rdx
    pop     %rcx
    pop     %rax
    addq    $0x10, %rsp                    # skip INT# & ErrCode
    iretq

ASM_GLOBAL  ASM_PFX(AsmExceptionHandlers)
ASM_PFX(AsmExceptionHandlers):
# No. 0
    pushq   %rax                         # dummy error code
    pushq   $0
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
ExceptionHandlerEnd:
# No. 1
    pushq   %rax                         # dummy error code
    pushq   $1
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 2
    pushq   %rax                         # dummy error code
    pushq   $2
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 3
    pushq   %rax                         # dummy error code
    pushq   $3
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 4
    pushq   %rax                         # dummy error code
    pushq   $4
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 5
    pushq   %rax                         # dummy error code
    pushq   $5
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 6
    pushq   %rax                         # dummy error code
    pushq   $6
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 7
    pushq   %rax                         # dummy error code
    pushq   $7
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 8
    pushq   $8
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .byte   0x90
    .long   0x090909090
# No. 9
    pushq   %rax                         # dummy error code
    pushq   $9
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 10
    pushq   $10
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .byte   0x90
    .long   0x090909090
# No. 11
    pushq   $11
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .byte   0x90
    .long   0x090909090
# No. 12
    pushq   $12
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .byte   0x90
    .long   0x090909090
# No. 13
    pushq   $13
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .byte   0x90
    .long   0x090909090
# No. 14
    pushq   $14
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .byte   0x90
    .long   0x090909090
# No. 15
    pushq   %rax                         # dummy error code
    pushq   $15
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 16
    pushq   %rax                         # dummy error code
    pushq   $16
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 17
    pushq   $17
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .byte   0x90
    .long   0x090909090
# No. 18
    pushq   %rax                         # dummy error code
    pushq   $18
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 19
    pushq   %rax                         # dummy error code
    pushq   $19
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 20
    pushq   %rax                         # dummy error code
    pushq   $20
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 21
    pushq   %rax                         # dummy error code
    pushq   $21
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 22
    pushq   %rax                         # dummy error code
    pushq   $22
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 23
    pushq   %rax                         # dummy error code
    pushq   $23
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 24
    pushq   %rax                         # dummy error code
    pushq   $24
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 25
    pushq   %rax                         # dummy error code
    pushq   $25
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 26
    pushq   %rax                         # dummy error code
    pushq   $26
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 27
    pushq   %rax                         # dummy error code
    pushq   $27
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 28
    pushq   %rax                         # dummy error code
    pushq   $28
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 29
    pushq   %rax                         # dummy error code
    pushq   $29
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 30
    pushq   %rax                         # dummy error code
    pushq   $30
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 31
    pushq   %rax                         # dummy error code
    pushq   $31
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090

