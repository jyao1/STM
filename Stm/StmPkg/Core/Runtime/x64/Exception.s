#------------------------------------------------------------------------------
#
# Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.<BR>
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
#   Exception.s
#
#------------------------------------------------------------------------------

ASM_GLOBAL ASM_PFX(AsmExceptionHandlers)
ASM_GLOBAL ASM_PFX(mExceptionHandlerLength)
ASM_GLOBAL ASM_PFX(mExternalVectorTablePtr)
ASM_GLOBAL ASM_PFX(mErrorCodeFlag)

.data
ASM_PFX(mExceptionHandlerLength): .long 8
ASM_PFX(mExternalVectorTablePtr): .quad 0

.text

.p2align 3

ASM_GLOBAL ASM_PFX(AsmExceptionHandlers)
ASM_PFX(AsmExceptionHandlers):
# The following segment repeats 32 times:
# No. 0
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 1
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 2
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 3
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 4
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 5
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 6
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 7
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 8
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 9
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 10
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 11
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 12
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 13
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 14
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 15
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 16
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 17
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 18
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 19
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 20
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 21
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 22
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 23
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 24
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 25
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 26
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 27
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 28
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 29
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 30
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop
# No. 31
        call  CommonInterruptEntry
        .word ( . - ASM_PFX(AsmExceptionHandlers) - 5 ) / 8
        nop

#---------------------------------------;
# CommonInterruptEntry                  ;
#---------------------------------------;
# The follow algorithm is used for the common interrupt routine.

#
# +---------------------+ <-- 16-byte aligned ensured by processor
# +    Old SS           +
# +---------------------+
# +    Old RSP          +
# +---------------------+
# +    RFlags           +
# +---------------------+
# +    CS               +
# +---------------------+
# +    RIP              +
# +---------------------+
# +    Error Code       +
# +---------------------+
# + RCX / Vector Number +
# +---------------------+
# +    RBP              +
# +---------------------+ <-- RBP, 16-byte aligned
#

CommonInterruptEntry:
  cli
  #
  # All interrupt handlers are invoked through interrupt gates, so
  # IF flag automatically cleared at the entry point
  #
  #
  # Calculate vector number
  #
  xchg    %rcx, (%rsp)
  movzwl  (%rcx), %ecx
  cmpl    $32,%ecx        # Intel reserved vector for exceptions?
  jae     NoErrorCode
  btl     %ecx, ASM_PFX(mErrorCodeFlag)
  jc      L1

NoErrorCode:
  #
  # Push a dummy error code on the stack
  # to maintain coherent stack map
  #
  pushq    (%rsp)
  movq     $0, 8(%rsp)
L1:
  pushq    %rbp
  movq    %rsp, %rbp

  #
  # Since here the stack pointer is 16-byte aligned, so
  # EFI_FX_SAVE_STATE_X64 of EFI_SYSTEM_CONTEXT_x64
  # is 16-byte aligned
  #

## UINT64  Rdi, Rsi, Rbp, Rsp, Rbx, Rdx, Rcx, Rax;
## UINT64  R8, R9, R10, R11, R12, R13, R14, R15;
  pushq %r15
  pushq %r14
  pushq %r13
  pushq %r12
  pushq %r11
  pushq %r10
  pushq %r9
  pushq %r8
  pushq %rax
  pushq 8(%rbp)
  pushq %rdx
  pushq %rbx
  pushq 48(%rbp)
  pushq (%rbp)
  pushq %rsi
  pushq %rdi

## UINT64  Gs, Fs, Es, Ds, Cs, Ss;  insure high 16 bits of each is zero
  movzwq   56(%rbp), %rax
  pushq    %rax
  movzwq   32(%rbp), %rax
  pushq    %rax
  movq     %ds, %rax
  pushq    %rax
  movq     %es, %rax
  pushq    %rax
  movq     %fs, %rax
  pushq    %rax
  movq     %gs, %rax
  pushq    %rax

   movq     %rcx, 8(%rbp)

## UINT64  Rip;
   pushq    24(%rbp)

## UINT64  Gdtr[2], Idtr[2];
   subq     $16, %rsp
   sidt    (%rsp)
   subq     $16, %rsp
   sgdt    (%rsp)

## UINT64  Ldtr, Tr;
   xorq     %rax, %rax
  strw    %ax
  pushq    %rax
  sldtw   %ax
  pushq    %rax

## UINT64  RFlags;
   pushq    40(%rbp)

## UINT64  Cr0, Cr1, Cr2, Cr3, Cr4, Cr8;
   movq    %cr8, %rax
  pushq    %rax
   movq    %cr4, %rax
   orq     $0x208, %rax
   movq    %rax, %cr4
  pushq    %rax
   movq    %cr3, %rax
  pushq    %rax
   movq    %cr2, %rax
  pushq    %rax
   xorq    %rax, %rax
  pushq    %rax
   movq    %cr0, %rax
  pushq    %rax

## UINT64  Dr0, Dr1, Dr2, Dr3, Dr6, Dr7;
   movq    %dr7, %rax
  pushq    %rax
   movq    %dr6, %rax
  pushq    %rax
   movq    %dr3, %rax
  pushq    %rax
   movq    %dr2, %rax
  pushq    %rax
   movq    %dr1, %rax
  pushq    %rax
   movq    %dr0, %rax
  pushq    %rax

## FX_SAVE_STATE_X64 FxSaveState;

   subq    $512, %rsp
   movq    %rsp, %rdi
   .byte   0x0f, 0xae, 0b00000111

## UEFI calling convention for x64 requires that Direction flag in EFLAGs is clear
   cld

## UINT32  ExceptionData;
   pushq   16(%rbp)

## call into exception handler
   movq    8(%rbp), %rcx
   movq    ASM_PFX(mExternalVectorTablePtr)(%rip), %rax
   movq    (%rax, %rcx, 8), %rax
   orq     %rax, %rax

  je    nonNullValue #

## Prepare parameter and call
   movq    %rsp, %rdx
  #
  # Per X64 calling convention, allocate maximum parameter stack space
  # and make sure RSP is 16-byte aligned
  #
   subq    $(4 * 8 + 8), %rsp
   call    *%rax
   addq    $(4 * 8 + 8), %rsp

nonNullValue:
  cli
## UINT64  ExceptionData;
   addq    $8, %rsp

## FX_SAVE_STATE_X64 FxSaveState;

   movq    %rsp, %rsi
   .byte   0x0f, 0xae, 0b00001110
   addq    $512, %rsp

## UINT64  Dr0, Dr1, Dr2, Dr3, Dr6, Dr7;
## Skip restoration of DRx registers to support in-circuit emualators
## or debuggers set breakpoint in interrupt/exception context
   addq     $48, %rsp

## UINT64  Cr0, Cr1, Cr2, Cr3, Cr4, Cr8;
   popq     %rax
   movq     %rax, %cr0
   addq     $8, %rsp
   popq     %rax
   movq     %rax, %cr2
   popq     %rax
   movq     %rax, %cr3
   popq     %rax
   movq     %rax, %cr4
   popq     %rax
   movq     %rax, %cr8

## UINT64  RFlags;
   popq    40(%rbp)

## UINT64  Ldtr, Tr;
## UINT64  Gdtr[2], Idtr[2];
## Best not let anyone mess with these particular registers...
   addq    $48, %rsp

## UINT64  Rip;
   popq    24(%rbp)

## UINT64  Gs, Fs, Es, Ds, Cs, Ss;
   popq     %rax
  # mov     gs, rax ; not for gs
   popq     %rax
  # mov     fs, rax ; not for fs
  # (X64 will not use fs and gs, so we do not restore it)
   popq     %rax
   movw     %ax, %es
   popq     %rax
   movw     %ax, %ds
   popq     32(%rbp)
   popq     56(%rbp)

## UINT64  Rdi, Rsi, Rbp, Rsp, Rbx, Rdx, Rcx, Rax;
## UINT64  R8, R9, R10, R11, R12, R13, R14, R15;
   popq     %rdi
   popq     %rsi
   addq     $8, %rsp
   popq     48(%rbp)
   popq     %rbx
   popq     %rdx
   popq     %rcx
   popq     %rax
   popq     %r8
   popq     %r9
   popq     %r10
   popq     %r11
   popq     %r12
   popq     %r13
   popq     %r14
   popq     %r15

   movq     %rbp, %rsp
   popq     %rbp
   addq     $16,  %rsp
  iretq

