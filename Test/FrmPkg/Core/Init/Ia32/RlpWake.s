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

  # Check DLE
  movl    $TXT_PUBLIC_SPACE, %eax
  addl    $TXT_HEAP_BASE, %eax                     # eax = HEAP Base Ptr
  movl    (%eax), %esi                             # esi = HEAP Base
  movl    (%esi), %edx                             # edx = BiosOsDataSize
  addl    %edx, %esi                               # esi = OsMleDataSize Offset
  addl    $8, %esi                                 # esi = MlePrivateData Offset
  
  movl    %esi, %edi                                       # edi = MyOsMleData Offset
  addl    $_TXT_OS_TO_MLE_DATA_MlePrivateDataAddress, %edi # edi = MlePrivateDataAddress offset
  movl    (%edi), %esi                                     # esi = TxtOsMleData Offset

  movl    %esi, %edi                               # edi = MlePrivateData Offset
  addl    $_MLE_PRIVATE_DATA_ApEntry, %edi         # edi = ApEntry Offset

  movl    (%edi), %eax
  cmpl    $0, %eax
  jz      NonDleAp

    movl    $TXT_PUBLIC_SPACE, %eax
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

    movl    _MLE_PRIVATE_DATA_RlpDsSeg(%esi), %eax # eax = data segment
    movw    %ax, %ds
    movw    %ax, %ss
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs

    movl    _MLE_PRIVATE_DATA_TempEspRlp(%esi), %eax # eax = temporary stack
    subl    $0x20, %eax

    # patch Offset/Segment

    subl    $4, %eax
    movl    _MLE_PRIVATE_DATA_RlpPostSinitSegment(%esi), %edx # edx = RlpPostSinitSegment
    movl    %edx, (%eax)

    subl    $4, %eax
    movl    _MLE_PRIVATE_DATA_RlpPostSinitOffset(%esi), %edx  # edx = RlpPostSinitOffset
    movl    %edx, (%eax)

    movl    %eax, %esp

    # reload CS
    retf

.equ POST_INIT_ADDR_RLP, . - ASM_PFX(AsmRlpWakeUpCode)

  #
  # Notify RLP wakeup
  #
# Critical Section - start -----------------------

  movl    $TXT_PUBLIC_SPACE, %eax
  addl    $TXT_HEAP_BASE, %eax                     # eax = HEAP Base Ptr
  movl    (%eax), %esi                             # esi = HEAP Base
  movl    (%esi), %edx                             # edx = BiosOsDataSize
  addl    %edx, %esi                               # esi = OsMleDataSize Offset
  addl    $8, %esi                                 # esi = MlePrivateData Offset

  movl    %esi, %edi                                       # edi = MyOsMleData Offset
  addl    $_TXT_OS_TO_MLE_DATA_MlePrivateDataAddress, %edi # edi = MlePrivateDataAddress offset
  movl    (%edi), %esi                                     # esi = TxtOsMleData Offset

  movl    %esi, %edi                               # edi = MlePrivateData Offset
  addl    $_MLE_PRIVATE_DATA_Lock, %edi            # edi = Lock Offset
  movl    %edi, %ebp                               # ebp = Lock Offset

# AcquireLock:    
  movb    $1, %al
TryGetLock:
  xchgb       %al, (%ebp)
  cmpb        $0, %al
  jz          LockObtained
#  pause
  jmp         TryGetLock       
LockObtained:

  movl    %esi, %edi                                       # edi = MlePrivateData Offset
  addl    $_MLE_PRIVATE_DATA_RlpInitializedNumber, %edi    # edi = RlpInitializedNumber Offset
  incl    (%edi)                                           # increase RlpInitializedNumber

# ReleaseLock:    
  movb        $0, %al
  xchg        %al, (%ebp)

# Critical Section - end -----------------------

  movl    $TXT_PUBLIC_SPACE, %eax
  addl    $TXT_HEAP_BASE, %eax                     # eax = HEAP Base Ptr
  movl    (%eax), %esi                             # esi = HEAP Base
  movl    (%esi), %edx                             # edx = BiosOsDataSize
  addl    %edx, %esi                               # esi = OsMleDataSize Offset
  addl    $8, %esi                                 # esi = MlePrivateData Offset

  movl    %esi, %edi                                       # edi = MyOsMleData Offset
  addl    $_TXT_OS_TO_MLE_DATA_MlePrivateDataAddress, %edi # edi = MlePrivateDataAddress offset
  movl    (%edi), %esi                                     # esi = TxtOsMleData Offset

  movl    %esi, %edi                               # edi = MlePrivateData Offset
  addl    $_MLE_PRIVATE_DATA_ApEntry, %edi         # edi = ApEntry Offset

  movl    (%edi), %eax
  jmp     *%eax
NonDleAp:

  # Should not get here
  jmp .

ASM_PFX(PostInitAddrRlp):
  .long POST_INIT_ADDR_RLP
