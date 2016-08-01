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
    movl    (%eax), %esi                             # esi = HEAP Base
    movl    (%esi), %edx                             # edx = BiosOsDataSize
    addl    %edx, %esi                               # esi = OsMleDataSize Offset
    addl    $8, %esi                                 # esi = MlePrivateData Offset

    movl    %esi, %edi                                       # edi = MyOsMleData Offset
    addl    $_TXT_OS_TO_MLE_DATA_MlePrivateDataAddress, %edi # edi = MlePrivateDataAddress offset
    movl    (%edi), %esi                                     # esi = TxtOsMleData Offset

    movl    %esi, %edi                               # edi = MlePrivateData Offset
    addl    $_MLE_PRIVATE_DATA_IdtrReg, %edi         # edi = IDTR offset
    lidt    (%edi)                                   # Reload IDT
    movl    %esi, %edi                               # edi = MlePrivateData Offset
    addl    $_MLE_PRIVATE_DATA_GdtrReg, %edi         # edi = GDTR offset
    lgdt    (%edi)                                   # Reload GDT

    movl    _MLE_PRIVATE_DATA_DsSeg(%esi), %eax   # eax = data segment
    movw    %ax, %ds
    movw    %ax, %ss
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs

    movl    _MLE_PRIVATE_DATA_TempEsp(%esi), %eax # eax = temporary stack
    subl    $0x20, %eax

    # patch Offset/Segment

    subl    $4, %eax
    movl    _MLE_PRIVATE_DATA_PostSinitSegment(%esi), %edx # edx = PostSinitSegment
    movl    %edx, (%eax)

    subl    $4, %eax
    movl    _MLE_PRIVATE_DATA_PostSinitOffset(%esi), %edx  # edx = PostSinitOffset
    movl    %edx, (%eax)

    movl    %eax, %esp

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
#   IN UINTN                  DlmeEntryPoint,   // [esp + 4]
#   IN VOID                   *DlmeArgs,        // [esp + 8]
#   IN UINTN                  *StackBufferTop   // [esp + 0ch]
#   )
#------------------------------------------------------------------------------
ASM_PFX(AsmLaunchDlmeMain):
  movl %esp, %ebp      # ebp = old esp
  movl 4(%ebp), %eax   # eax = DlmeMain
  
  movl 0xc(%esp), %ebx
  addl $0xf, %ebx
  andw $0xFFF0, %bx     # ebx = new esp
  movl %ebx, %esp
  
  movl 8(%ebp), %edx
  push %edx

  call *%eax
  # Should not get here
  jmp .

ASM_PFX(PostInitAddr):
  .long POST_INIT_ADDR
