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
ASM_GLOBAL ASM_PFX(mLongModeEntryOffset)
ASM_GLOBAL ASM_PFX(mLongModeEntry)
ASM_GLOBAL ASM_PFX(mPageTableOffset)
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
ASM_PFX(mLongModeEntryOffset):         .long LongModeEntryOffset - ASM_PFX(AsmApWakeup32)
ASM_PFX(mLongModeEntry):               .long LongModeEntry - ASM_PFX(AsmApWakeup32)
ASM_PFX(mPageTableOffset):             .long PageTableOffset - ASM_PFX(AsmApWakeup32)
ASM_PFX(mCodeSel):                     .long CODE_SEL
ASM_PFX(mApGdtrOffset):                .long GDTR_OFFSET

.equ CODE_SEL,    CodeSeg32 - NullSeg
.equ DATA_SEL,    DataSeg32 - NullSeg
.equ CODE64_SEL,  CodeSeg64 - NullSeg
.equ DATA64_SEL,  DataSeg64 - NullSeg
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

            .quad   0                   # reserved by future use
            .quad   0                   # reserved by future use

DataSeg64:
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
#
# Now we are in Protected mode
#
  .byte   0x66, 0xB8
  .word   DATA_SEL
#  mov  ax, DATA_SEL
  .byte   0x66, 0x8E, 0xD8
#  mov  ds, ax
  .byte   0x66, 0x8E, 0xC0
#  mov  es, ax
  .byte   0x66, 0x8E, 0xD0
#  mov  ss, ax

  #
  # Enable the 64-bit page-translation-table entries by
  # setting CR4.PAE=1 (this is _required_ before activating
  # long mode). Paging is not enabled until after long mode
  # is enabled.
  #
  .byte 0xf, 0x20, 0xe0
#  mov eax, cr4
  bts $5, %eax
  .byte 0xf, 0x22, 0xe0
#  mov cr4, eax
  .byte  0xb8
PageTableOffset:
  .long  0  # page table, place holder
#  mov eax, 00000000h
  .byte  0xF, 0x22, 0xD8
#  mov cr3, eax

  #
  # Enable long mode (set EFER.LME=1).
  #
  .byte  0xb9
  .long  0xc0000080
#  mov   ecx, 0c0000080h # EFER MSR number.
  .byte 0xf, 0x32
#  rdmsr                 # Read EFER.
  .byte    0xf, 0xba, 0xe8, 0x8
#  bts   eax, 8          # Set LME=1.
  .byte 0xf, 0x30
#  wrmsr                 # Write EFER.

  #
  # Enable paging to activate long mode (set CR0.PG=1)
  #
  .byte    0xF, 0x20, 0xC0
#  mov   eax, cr0        # Read CR0.
  .byte    0xf, 0xba, 0xe8, 0x1f
#  bts   eax, 31         # Set PG=1.
  .byte    0xF, 0x22, 0xC0
#  mov   cr0, eax        # Write CR0.

# Go to long mode
    .byte      0x67
    .byte      0xea                # Far Jump $+9:Selector to reload CS
LongModeEntryOffset:
    .long      00000000            # TO BE FIXED
    .word      CODE64_SEL          #   Selector is our code selector, 38h

#
# Now we are in Long mode
#
LongModeEntry:
  movl $DATA64_SEL, %eax
  movw %ax, %ds
  movw %ax, %es
  movw %ax, %ss
# Search the stack pointer for the current CPU according to its
  movl $0xFEE00000, %eax
  movl 0x20(%eax), %edx
  shrl $24, %edx          # dl = LocalApicID

# set common stack bottom
  leaq mApStack, %rax
  movq (%rax), %rsp     # rsp = common stack bottom

# set CpuNum
  xorq %rcx, %rcx
  leaq mCpuNum, %rax
  movl (%rax), %ecx     # rcx = CpuNum (Max loop)

# search APIC ID in the list to get the CpuIndex
  leaq mApicIdList, %rax
  movq (%rax), %rsi     # rsi = ApicIdList
  movq %rsi, %rdi       # rdi = ApicIdList header
  
SearchApicId:           # rdi = ApicId ptr
  movb (%rdi), %bl      # bl = ApicId from ApicIdList
  cmpb %dl, %bl         # compare with this ApicId
  jz   ApicIdFound      # found
  addq $4, %rdi         # rdi = Next ApicId ptr
  loop SearchApicId

ApicIdNotFound:
  jmp .

ApicIdFound:
  subq %rsi, %rdi      # rdi = Index * 4
  shrq $2, %rdi        # rdi = Index
  movq %rdi, %rcx      # rcx = Index
  shlq $12, %rdi       # rsi = Index * STACK_SIZE (0x1000)
  addq %rdi, %rsp      # rsp = this stack bottom
  addq $0x1000, %rsp   # rsp = this stack top
  
  subq $0x20, %rsp
  call ApWakeupC
  addq $0x20, %rsp

  jmp .

ASM_GLOBAL ASM_PFX(AsmGuestApEntrypoint)
ASM_PFX(AsmGuestApEntrypoint):
  cli
  hlt
  jmp .

