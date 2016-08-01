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
#    ApWakeup.s
#
# Abstract:
#
#------------------------------------------------------------------------------

ASM_GLOBAL ASM_PFX(mApWakeupSegmentOffset)
ASM_GLOBAL ASM_PFX(mApProtectedModeEntryOffset)
ASM_GLOBAL ASM_PFX(mApGdtBaseOffset)
ASM_GLOBAL ASM_PFX(mApGdtBase)
ASM_GLOBAL ASM_PFX(mCodeSel)
ASM_GLOBAL ASM_PFX(mApGdtrOffset)

ASM_GLOBAL ASM_PFX(mCpuNum)
ASM_GLOBAL ASM_PFX(mApStack)
ASM_GLOBAL ASM_PFX(mApicIdList)

ASM_GLOBAL ASM_PFX(ApWakeupC)

.data

ASM_PFX(mApWakeupSegmentOffset):       .long WakeupSegmentOffset - ASM_PFX(AsmApWakeup)
ASM_PFX(mApProtectedModeEntryOffset):  .long ProtectedModeEntryOffset - ASM_PFX(AsmApWakeup)
ASM_PFX(mApGdtBaseOffset):             .long GdtBaseOffset - ASM_PFX(AsmApWakeup)
ASM_PFX(mApGdtBase):                   .long NullSeg - ASM_PFX(AsmApWakeup)
ASM_PFX(mCodeSel):                     .long CODE_SEL
ASM_PFX(mApGdtrOffset):                .long GDTR_OFFSET

.equ CODE_SEL,    CodeSeg32 - NullSeg
.equ DATA_SEL,    DataSeg32 - NullSeg
.equ GDTR_OFFSET, GdtrOffset - ASM_PFX(AsmApWakeup)

.text

ASM_GLOBAL ASM_PFX(AsmApWakeup)
ASM_PFX(AsmApWakeup):
  .byte 0xb8                      # mov ax, imm16 
WakeupSegmentOffset:
  .word 00                        # TO BE FIXED
  .byte 0x8e, 0xd8                # mov ds, ax
  .byte 0x8D, 0x36, GDTR_OFFSET, 0 # lea  si, GdtrOffset
  .byte 0xF, 0x1, 0x14            # lgdt fword ptr [si]
  .byte 0xF, 0x20, 0xC0           # mov eax, cr0
  .byte 0xc, 0x1                  # or  al, 1
  .byte 0xF, 0x22, 0xC0           # mov cr0, eax
  .byte 0x66 # far jump
  .byte 0xEA
ProtectedModeEntryOffset:
  .long 0 # TO BE FIXED
  .word CODE_SEL

  .long 0 # dummy
GdtrOffset:
  .word GDT_SIZE - 1
GdtBaseOffset:
  .long 0 # TO BE FIXED
  .word 0 # dummy
NullSeg:    .quad   0                   # reserved by architecture
            .quad   0                   # reserved by future use
CodeSeg32:
            .word  -1                   # LimitLow
            .word   0                   # BaseLow
            .byte   0                   # BaseMid
            .byte   0x9b
            .byte   0xcf                # LimitHigh
            .byte   0                   # BaseHigh
DataSeg32:
            .word   -1                  # LimitLow
            .word   0                   # BaseLow
            .byte   0                   # BaseMid
            .byte   0x93
            .byte   0xcf                # LimitHigh
            .byte   0                   # BaseHigh
CodeSeg64:
            .word   -1                  # LimitLow
            .word   0                   # BaseLow
            .byte   0                   # BaseMid
            .byte   0x9b
            .byte   0xaf                # LimitHigh
            .byte   0                   # BaseHigh

            .quad   0                   # reserved for future use
.equ GDT_SIZE, . - NullSeg

ASM_GLOBAL ASM_PFX(AsmApWakeup32)
ASM_PFX(AsmApWakeup32):
  movw $DATA_SEL, %ax
  movw %ax, %ds
  movw %ax, %es
  movw %ax, %ss
# Search the stack pointer for the current CPU according to its
  movl $0xFEE00000, %eax
  movl 0x20(%eax), %edx
  shrl $24, %edx        # dl = LocalApicID

# set common stack bottom
  leal mApStack, %eax
  movl (%eax), %esp     # esp = common stack bottom

# set CpuNum
  leal mCpuNum, %eax
  movl (%eax), %ecx     # ecx = CpuNum (Max loop)

# search APIC ID in the list to get the CpuIndex
  leal mApicIdList, %eax
  movl (%eax), %esi     # esi = ApicIdList
  movl %esi, %edi       # edi = ApicIdList header
  
SearchApicId:           # edi = ApicId ptr
  movb (%edi), %bl      # bl = ApicId from gApicIdList
  cmpb %dl, %bl         # compare with this ApicId
  jz   ApicIdFound      # found
  addl $4, %edi         # edi = Next ApicId ptr
  loop SearchApicId

ApicIdNotFound:
  jmp .

ApicIdFound:
  subl %esi, %edi      # edi = Index * 4
  shrl $2, %edi        # edi = Index
  movl %edi, %ecx      # ecx = Index
  shll $12, %edi       # esi = Index * STACK_SIZE (0x1000)
  addl %edi, %esp      # esp = this stack bottom
  addl $0x1000, %esp   # esp = this stack top
  
  push %ecx
  call ApWakeupC
  addl $4, %esp

  jmp .

ASM_GLOBAL ASM_PFX(AsmGuestApEntrypoint)
ASM_PFX(AsmGuestApEntrypoint):
  cli
  hlt
  jmp .

