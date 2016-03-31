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
#   Exception.s
#
#------------------------------------------------------------------------------

ASM_GLOBAL ASM_PFX(AsmExceptionHandlers)
ASM_GLOBAL ASM_PFX(mExceptionHandlerLength)
ASM_GLOBAL ASM_PFX(mExternalVectorTablePtr)
ASM_GLOBAL ASM_PFX(mErrorCodeFlag)

.data
ASM_PFX(mExceptionHandlerLength): .long 8
ASM_PFX(mExternalVectorTablePtr): .long 0

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
# +---------------------+
# +    EFlags           +
# +---------------------+
# +    CS               +
# +---------------------+
# +    EIP              +
# +---------------------+
# +    Error Code       +
# +---------------------+
# + EAX / Vector Number +
# +---------------------+
# +    EBP              +
# +---------------------+ <-- EBP
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
  xchg    %eax, (%esp)
  movzwl  (%eax), %eax
  cmpl    $32,%eax        # Intel reserved vector for exceptions?
  jae     NoErrorCode
  btl     %eax, ASM_PFX(mErrorCodeFlag)
  jc      L1

NoErrorCode: 
  #
  # Push a dummy error code on the stack
  # to maintain coherent stack map
  #
  pushl   (%esp)
  movl    $0, 4(%esp)
L1:
  pushl   %ebp
  movl    %esp,%ebp

  #
  # Align stack to make sure that EFI_FX_SAVE_STATE_IA32 of EFI_SYSTEM_CONTEXT_IA32
  # is 16-byte aligned
  #
  andl    $0xfffffff0,%esp
  subl    $12,%esp

## UINT32  Edi, Esi, Ebp, Esp, Ebx, Edx, Ecx, Eax;
  pushl   0x4(%ebp)
  pushl   %ecx
  pushl   %edx
  pushl   %ebx
  leal    24(%ebp),%ecx
  pushl   %ecx                         # ESP
  pushl   (%ebp)
  pushl   %esi
  pushl   %edi

  movl    %eax,4(%ebp)                 # save vector number

## UINT32  Gs, Fs, Es, Ds, Cs, Ss;
  movl %ss,%eax
  pushl %eax
  movzwl 0x10(%ebp), %eax
  pushl %eax
  movl %ds,%eax
  pushl %eax
  movl %es,%eax
  pushl %eax
  movl %fs,%eax
  pushl %eax
  movl %gs,%eax
  pushl %eax

## UINT32  Eip;
  pushl    12(%ebp)

## UINT32  Gdtr[2], Idtr[2];
  subl $8,%esp
  sidt (%esp)
  subl $8,%esp
  sgdt (%esp)

## UINT32  Ldtr, Tr;
  xorl %eax,%eax
  strl %eax
  pushl %eax
  sldtl %eax
  pushl %eax

## UINT32  EFlags;
  pushl    20(%ebp)

## UINT32  Cr0, Cr1, Cr2, Cr3, Cr4;
  movl %cr4, %eax
  orl  $0x208,%eax
  movl %eax, %cr4
  pushl %eax
  movl %cr3, %eax
  pushl %eax
  movl %cr2, %eax
  pushl %eax
  xorl %eax,%eax
  pushl %eax
  movl %cr0, %eax
  pushl %eax

## UINT32  Dr0, Dr1, Dr2, Dr3, Dr6, Dr7;
  movl    %dr7, %eax
  pushl   %eax
  movl    %dr6, %eax
  pushl   %eax
  movl    %dr3, %eax
  pushl   %eax
  movl    %dr2, %eax
  pushl   %eax
  movl    %dr1, %eax
  pushl   %eax
  movl    %dr0, %eax
  pushl   %eax

## FX_SAVE_STATE_IA32 FxSaveState;
  subl $512,%esp
  movl %esp,%edi
  .byte 0x0f, 0xae, 0x07

## UEFI calling convention for IA32 requires that Direction flag in EFLAGs is clear
  cld

## UINT32  ExceptionData;
  pushl   8(%ebp)

## call into exception handler
  movl    4(%ebp),%ebx
  movl    ASM_PFX(mExternalVectorTablePtr), %eax
  movl    (%eax,%ebx,4),%eax
  orl     %eax,%eax               # NULL?
  je  nonNullValue #

## Prepare parameter and call
  movl    %esp,%edx
  pushl   %edx
  pushl   %ebx
  call    *%eax
  addl    $8,%esp

nonNullValue: 
  cli
## UINT32  ExceptionData;
  addl $4,%esp

## FX_SAVE_STATE_IA32 FxSaveState;
  movl %esp,%esi
  .byte 0x0f, 0xae, 0x0e
  addl $512,%esp

## UINT32  Dr0, Dr1, Dr2, Dr3, Dr6, Dr7;
## Skip restoration of DRx registers to support in-circuit emualators
## or debuggers set breakpoint in interrupt/exception context
  addl    $24, %esp

## UINT32  Cr0, Cr1, Cr2, Cr3, Cr4;
  popl    %eax
  movl    %eax, %cr0
  addl    $4,%esp   # not for Cr1
  popl    %eax
  movl    %eax, %cr2
  popl    %eax
  movl    %eax, %cr3
  popl    %eax
  movl    %eax, %cr4

## UINT32  EFlags;
  popl    20(%ebp)

## UINT32  Ldtr, Tr;
## UINT32  Gdtr[2], Idtr[2];
## Best not let anyone mess with these particular registers...
  addl    $24,%esp

## UINT32  Eip;
   pop     12(%ebp)

## UINT32  Gs, Fs, Es, Ds, Cs, Ss;
  popl    %gs
  popl    %fs
  popl    %es
  popl    %ds
  popl    16(%ebp)
  popl    %ss

## UINT32  Edi, Esi, Ebp, Esp, Ebx, Edx, Ecx, Eax;
  popl    %edi
  popl    %esi
  addl    $4,%esp  # not for ebp
  addl    $4,%esp  # not for esp
  popl    %ebx
  popl    %edx
  popl    %ecx
  popl    %eax

  movl    %ebp,%esp
  popl    %ebp
  addl    $8,%esp
  iretl

