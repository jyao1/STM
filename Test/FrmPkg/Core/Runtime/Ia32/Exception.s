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
    iretl

#------------------------------------------------------------------------------
# ExceptionEntrypoint is the entry point for all exceptions
#
# Stack frame would be as follows as specified in IA32 manuals:
#   RFLAGS  +1ch
#   CS      +18h
#   RIP     +14h
#   ErrCode +10h
#   INT#    +0ch
#   EAX     +8
#   ECX     +4
#   EDX     +0
#
# RSP set to odd multiple of 8 means ErrCode PRESENT
#------------------------------------------------------------------------------
ExceptionEntrypoint:
jmp .
    push    %eax
    push    %ecx
    push    %edx
    movl    0xc(%esp), %ecx
    pushl   0x10(%esp)
    push    %ecx
    call    ExceptionHandlerC
    pop     %ecx
    pop     %ecx
    pop     %edx
    pop     %ecx
    pop     %eax
    addl    $8, %esp                      # skip INT# & ErrCode
    iretl

ASM_GLOBAL  ASM_PFX(AsmExceptionHandlers)
ASM_PFX(AsmExceptionHandlers):
# No. 0
    pushl   %eax                         # dummy error code
    pushl   $0
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
ExceptionHandlerEnd:
# No. 1
    pushl   %eax                         # dummy error code
    pushl   $1
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 2
    pushl   %eax                         # dummy error code
    pushl   $2
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 3
    pushl   %eax                         # dummy error code
    pushl   $3
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 4
    pushl   %eax                         # dummy error code
    pushl   $4
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 5
    pushl   %eax                         # dummy error code
    pushl   $5
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 6
    pushl   %eax                         # dummy error code
    pushl   $6
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 7
    pushl   %eax                         # dummy error code
    pushl   $7
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 8
    pushl   $8
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .byte   0x90
    .long   0x090909090
# No. 9
    pushl   %eax                         # dummy error code
    pushl   $9
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 10
    pushl   $10
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .byte   0x90
    .long   0x090909090
# No. 11
    pushl   $11
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .byte   0x90
    .long   0x090909090
# No. 12
    pushl   $12
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .byte   0x90
    .long   0x090909090
# No. 13
    pushl   $13
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .byte   0x90
    .long   0x090909090
# No. 14
    pushl   $14
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .byte   0x90
    .long   0x090909090
# No. 15
    pushl   %eax                         # dummy error code
    pushl   $15
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 16
    pushl   %eax                         # dummy error code
    pushl   $16
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 17
    pushl   $17
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .byte   0x90
    .long   0x090909090
# No. 18
    pushl   %eax                         # dummy error code
    pushl   $18
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 19
    pushl   %eax                         # dummy error code
    pushl   $19
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 20
    pushl   %eax                         # dummy error code
    pushl   $20
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 21
    pushl   %eax                         # dummy error code
    pushl   $21
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 22
    pushl   %eax                         # dummy error code
    pushl   $22
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 23
    pushl   %eax                         # dummy error code
    pushl   $23
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 24
    pushl   %eax                         # dummy error code
    pushl   $24
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 25
    pushl   %eax                         # dummy error code
    pushl   $25
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 26
    pushl   %eax                         # dummy error code
    pushl   $26
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 27
    pushl   %eax                         # dummy error code
    pushl   $27
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 28
    pushl   %eax                         # dummy error code
    pushl   $28
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 29
    pushl   %eax                         # dummy error code
    pushl   $29
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 30
    pushl   %eax                         # dummy error code
    pushl   $30
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090
# No. 31
    pushl   %eax                         # dummy error code
    pushl   $31
    .byte   0xe9                         # jmp disp32
    .long   ExceptionEntrypoint - (. + 4)
    .long   0x090909090

