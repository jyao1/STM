#------------------------------------------------------------------------------
#
# Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
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
#    MleEntrypoint.s
#
# Abstract:
#
#------------------------------------------------------------------------------

.include "Smx.i"

ASM_GLOBAL ASM_PFX(DL_Entry_Back)
ASM_GLOBAL ASM_PFX(PostInitAddr)

ASM_GLOBAL ASM_PFX(AsmMleEntryPoint)
ASM_GLOBAL ASM_PFX(AsmLaunchDlmeMain)

#------------------------------------------------------------------------------
# VOID 
# AsmMleEntryPoint (
#     VOID    
#     )
#------------------------------------------------------------------------------
ASM_PFX(AsmMleEntryPoint):
    cli
    movl    $TXT_PUBLIC_SPACE, %eax                  # eax = TXT Public Base Ptr
    addl    $TXT_HEAP_BASE, %eax                     # eax = HEAP Base Ptr
    movl    (%rax), %esi                             # esi = HEAP Base
    movl    (%rsi), %edx                             # edx = BiosOsDataSize
    addl    %edx, %esi                               # esi = OsMleDataSize Offset
    addl    $8, %esi                                 # esi = MlePrivateData Offset

    movl    %esi, %edi                                       # edi = MyOsMleData Offset
    addl    $_TXT_OS_TO_MLE_DATA_MlePrivateDataAddress, %edi # edi = MlePrivateDataAddress offset
    movl    (%rdi), %esi                                     # esi = TxtOsMleData Offset

    movl    %esi, %edi                               # edi = MlePrivateData Offset
    addl    $_MLE_PRIVATE_DATA_IdtrReg, %edi         # edi = IDTR offset
    lidt    (%rdi)                                   # Reload IDT
    movl    %esi, %edi                               # edi = MlePrivateData Offset
    addl    $_MLE_PRIVATE_DATA_GdtrReg, %edi         # edi = GDTR offset
    lgdt    (%rdi)                                   # Reload GDT

    movl    %esi, %edi                               # edi = MlePrivateData Offset
    addl    $_MLE_PRIVATE_DATA_DsSeg, %edi           # edi = DataSeg offset
    movw    (%rdi), %ax                              # ax = data segment
    movw    %ax, %ds
    movw    %ax, %ss
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs

    movl    %esi, %edi                               # edi = MlePrivateData Offset
    addl    $_MLE_PRIVATE_DATA_TempEsp, %edi         # edi = TempEsp offset
    movl    (%rdi), %eax                             # eax = temporary stack
    subl    $0x20, %eax

    # patch Offset/Segment

    subl    $4, %eax
    movl    %esi, %edi                                   # edi = MlePrivateData Offset
    addl    $_MLE_PRIVATE_DATA_PostSinitSegment, %edi    # edi = PostSinitSegment offset
    movl    (%rdi), %edx                                 # edx = PostSinitSegment
    movl    %edx, (%rax)

    subl    $4, %eax
    movl    %esi, %edi                                   # edi = MlePrivateData Offset
    addl    $_MLE_PRIVATE_DATA_PostSinitOffset, %edi     # edi = PostSinitOffset offset
    movl    (%rdi), %edx                                 # edx = PostSinitOffset
    movl    %edx, (%rax)

    movl    %eax, %esp

    movl    %esi, %edi                                 # edi = MlePrivateData Offset
    addl    $_MLE_PRIVATE_DATA_Cr3, %edi               # edi = CR3 offset
    movl    (%rdi), %ebx                               # ebx = CR3

    #
    # Enable the 64-bit page-translation-table entries by
    # setting CR4.PAE=1 (this is _required_ before activating
    # long mode). Paging is not enabled until after long mode
    # is enabled.
    #
    movq %cr4, %rax
    bts  $5, %eax
    bts  $9, %eax # enable XMM
    movq %rax, %cr4

    #
    # Get the long-mode page tables, and initialize the
    # 64-bit CR3 (page-table base address) to point to the base
    # of the PML4 page table. The PML4 page table must be located
    # below 4 Gbytes because only 32 bits of CR3 are loaded when
    # the processor is not in 64-bit mode.
    #
    movl %ebx, %eax            # Get Page Tables
    movq %rax, %cr3            # Initialize CR3 with PML4 base.

    #
    # Enable long mode (set EFER.LME=1).
    #
    movl $0xc0000080, %ecx # EFER MSR number.
    rdmsr                  # Read EFER.
    bts  $8, %eax          # Set LME=1.
    wrmsr                  # Write EFER.

    #
    # Enable paging to activate long mode (set CR0.PG=1)
    #
    movq %cr0, %rax # Read CR0.
    bts $31, %eax   # Set PG=1.
    movq %rax, %cr0 # Write CR0.

    # reload CS
    retf

.equ POST_INIT_ADDR, . - ASM_PFX(AsmMleEntryPoint)

    # use long jump to restore all the other register
    call  ASM_PFX(DL_Entry_Back)

    # Should not get here
    jmp .

#------------------------------------------------------------------------------
# VOID
# AsmLaunchDlmeMain (
#   IN UINTN                  DlmeEntryPoint,   // rcx/[rsp + 8]
#   IN VOID                   *DlmeArgs,        // rdx/[rsp + 10h]
#   IN UINTN                  *StackBufferTop   // r8 /[rsp + 18h]
#   )
#------------------------------------------------------------------------------
ASM_PFX(AsmLaunchDlmeMain):
  xorq %rax, %rax
  movq %rcx, %rax   # rax = DlmeMain
  
  movq %r8, %rbx
  addq $0xf, %rbx
  andw $0xFFF0, %bx     # rbx = new rsp
  subq $0x20, %rbx
  movq %rbx, %rsp

  movq %rdx, %rcx
  call *%rax
  # Should not get here
  jmp .

ASM_PFX(PostInitAddr):
  .long POST_INIT_ADDR
