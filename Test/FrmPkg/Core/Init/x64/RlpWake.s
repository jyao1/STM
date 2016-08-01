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
#    RlpWake.s
#
# Abstract:
#
#------------------------------------------------------------------------------

.include "Smx.i"

ASM_GLOBAL ASM_PFX(AsmRlpWakeUpCode)
ASM_GLOBAL ASM_PFX(PostInitAddrRlp)

#------------------------------------------------------------------------------
# VOID 
# AsmRlpWakeUpCode (
#     VOID    
#     )
#------------------------------------------------------------------------------
ASM_PFX(AsmRlpWakeUpCode):
  cli

  #
  # Enable SMI
  #
  movl  $0, %ebx
  movl  $GET_SEC_SMCTRL, %eax
  .byte 0x0F # GETSEC
  .byte 0x37

  # Check DLE 64
  movl    $TXT_PUBLIC_SPACE, %eax
  addl    $TXT_HEAP_BASE, %eax                     # eax = HEAP Base Ptr
  movl    (%rax), %esi                             # esi = HEAP Base
  movl    (%rsi), %edx                             # edx = BiosOsDataSize
  addl    %edx, %esi                               # esi = OsMleDataSize Offset
  addl    $8, %esi                                 # esi = MlePrivateData Offset
  
  movl    %esi, %edi                                       # edi = MyOsMleData Offset
  addl    $_TXT_OS_TO_MLE_DATA_MlePrivateDataAddress, %edi # edi = MlePrivateDataAddress offset
  movl    (%rdi), %esi                                     # esi = TxtOsMleData Offset

  movl    %esi, %edi                               # edi = MlePrivateData Offset
  addl    $_MLE_PRIVATE_DATA_ApEntry, %edi         # edi = ApEntry Offset

  movl    (%rdi), %eax
  cmpl    $0, %eax
  jz      NonDleAp

  # Launch DLE64

    movl    $TXT_PUBLIC_SPACE, %eax
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
    addl    $_MLE_PRIVATE_DATA_RlpDsSeg, %edi        # edi = DataSeg offset
    movw    (%rdi), %ax                              # ax = data segment
    movw    %ax, %ds
    movw    %ax, %ss
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs

    movl    %esi, %edi                               # edi = MlePrivateData Offset
    addl    $_MLE_PRIVATE_DATA_TempEspRlp, %edi      # edi = TempEspRlp offset
    movl    (%rdi), %eax                             # eax = temporary stack
    subl    $0x20, %eax

    # patch Offset/Segment

    subl    $4, %eax
    movl    %esi, %edi                                      # edi = MlePrivateData Offset
    addl    $_MLE_PRIVATE_DATA_RlpPostSinitSegment, %edi    # edi = RlpPostSinitSegment offset
    movl    (%rdi), %edx                                    # edx = RlpPostSinitSegment
    movl    %edx, (%rax)

    subl    $4, %eax
    movl    %esi, %edi                                      # edi = MlePrivateData Offset
    addl    $_MLE_PRIVATE_DATA_RlpPostSinitOffset, %edi     # edi = RlpPostSinitOffset offset
    movl    (%rdi), %edx                                    # edx = RlpPostSinitOffset
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

.equ POST_INIT_ADDR_RLP, . - ASM_PFX(AsmRlpWakeUpCode)

# It is 64bit long mode here


  #
  # Notify RLP wakeup
  #
# Critical Section - start -----------------------

  xorq    %rax, %rax
  xorq    %rsi, %rsi
  xorq    %rdi, %rdi
  movl    $TXT_PUBLIC_SPACE, %eax
  addl    $TXT_HEAP_BASE, %eax                     # eax = HEAP Base Ptr
  movl    (%rax), %esi                             # esi = HEAP Base
  movl    (%rsi), %edx                             # edx = BiosOsDataSize
  addl    %edx, %esi                               # esi = OsMleDataSize Offset
  addl    $8, %esi                                 # esi = MlePrivateData Offset

  movl    %esi, %edi                                       # edi = MyOsMleData Offset
  addl    $_TXT_OS_TO_MLE_DATA_MlePrivateDataAddress, %edi # edi = MlePrivateDataAddress offset
  movl    (%rdi), %esi                                     # esi = TxtOsMleData Offset

  movl    %esi, %edi                               # edi = MlePrivateData Offset
  addl    $_MLE_PRIVATE_DATA_Lock, %edi            # edi = Lock Offset
  movl    %edi, %ebp                               # ebp = Lock Offset

# AcquireLock:    
  movb    $1, %al
TryGetLock:
  xchg        %al, (%rbp)
  cmpb        $0, %al
  jz          LockObtained
#  pause
  jmp         TryGetLock       
LockObtained:

  movl    %esi, %edi                                      # edi = MlePrivateData Offset
  addl    $_MLE_PRIVATE_DATA_RlpInitializedNumber, %edi   # edi = RlpInitializedNumber Offset
  incl    (%rdi)                                          # increase RlpInitializedNumber

# ReleaseLock:    
  movb        $0, %al
  xchg        %al, (%rbp)

# Critical Section - end -----------------------


  xorq    %rax, %rax
  xorq    %rsi, %rsi
  xorq    %rdi, %rdi
  movl    $TXT_PUBLIC_SPACE, %eax
  addl    $TXT_HEAP_BASE, %eax                     # eax = HEAP Base Ptr
  movl    (%rax), %esi                             # esi = HEAP Base
  movl    (%rsi), %edx                             # edx = BiosOsDataSize
  addl    %edx, %esi                               # esi = OsMleDataSize Offset
  addl    $8, %esi                                 # esi = MlePrivateData Offset

  movl    %esi, %edi                                       # edi = MyOsMleData Offset
  addl    $_TXT_OS_TO_MLE_DATA_MlePrivateDataAddress, %edi # edi = MlePrivateDataAddress offset
  movl    (%rdi), %esi                                     # esi = TxtOsMleData Offset

  movl    %esi, %edi                               # edi = MlePrivateData Offset
  addl    $_MLE_PRIVATE_DATA_ApEntry, %edi         # edi = ApEntry Offset

  movl    (%rdi), %eax
  jmp     *%rax
NonDleAp:
  # Should not get here
  jmp .

ASM_PFX(PostInitAddrRlp):
  .long POST_INIT_ADDR_RLP
