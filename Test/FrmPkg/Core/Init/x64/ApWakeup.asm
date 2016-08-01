;------------------------------------------------------------------------------
;
; Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
; This program and the accompanying materials
; are licensed and made available under the terms and conditions of the BSD License
; which accompanies this distribution.  The full text of the license may be found at
; http://opensource.org/licenses/bsd-license.php.
;
; THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
; WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
;
; Module Name:
; 
;    ApWakeup.asm
;
; Abstract:
;
;------------------------------------------------------------------------------

EXTERNDEF      mApWakeupSegmentOffset:DWORD
EXTERNDEF      mApProtectedModeEntryOffset:DWORD
EXTERNDEF      mApGdtBaseOffset:DWORD
EXTERNDEF      mApGdtBase:DWORD
EXTERNDEF      mLongModeEntryOffset:DWORD
EXTERNDEF      mLongModeEntry:DWORD
EXTERNDEF      mPageTableOffset:DWORD
EXTERNDEF      mCodeSel:DWORD
EXTERNDEF      mApGdtrOffset:DWORD

EXTERNDEF      mCpuNum:NEAR
EXTERNDEF      mApStack:NEAR
EXTERNDEF      mApicIdList:NEAR

EXTERNDEF      ApWakeupC:PROC

  .DATA

mApWakeupSegmentOffset       DD WakeupSegmentOffset - AsmApWakeup
mApProtectedModeEntryOffset  DD ProtectedModeEntryOffset - AsmApWakeup
mApGdtBaseOffset             DD GdtBaseOffset - AsmApWakeup
mApGdtBase                   DD NullSeg - AsmApWakeup
mLongModeEntryOffset         DD LongModeEntryOffset - AsmApWakeup32
mLongModeEntry               DD LongModeEntry - AsmApWakeup32
mPageTableOffset             DD PageTableOffset - AsmApWakeup32
mCodeSel                     DD CODE_SEL
mApGdtrOffset                DD GDTR_OFFSET

CODE_SEL    = offset CodeSeg32 - offset NullSeg
DATA_SEL    = offset DataSeg32 - offset NullSeg
CODE64_SEL  = offset CodeSeg64 - offset NullSeg
DATA64_SEL  = offset DataSeg64 - offset NullSeg
GDTR_OFFSET = GdtrOffset - AsmApWakeup

.CODE

AsmApWakeup PROC PUBLIC
  db 0b8h                      ; mov ax, imm16 
WakeupSegmentOffset::
  dw 00                        ; TO BE FIXED
  db 8eh, 0d8h                 ; mov ds, ax
  db 8Dh, 36h, GDTR_OFFSET, 00 ; lea  si, GdtrOffset
  db 0Fh, 01, 14h              ; lgdt fword ptr [si]
  db 0Fh, 20h, 0C0h            ; mov eax, cr0
  db 0ch, 01                   ; or  al, 1
  db 0Fh, 22h, 0C0h            ; mov cr0, eax
  db 66h ; far jump
  db 0EAh
ProtectedModeEntryOffset::
  dd 00000000h ; TO BE FIXED
  dw CODE_SEL

  dd 0 ; dummy
GdtrOffset::
  dw GDT_SIZE - 1
GdtBaseOffset::
  dd 0h ; TO BE FIXED
  dw 0 ; dummy
NullSeg     DQ      0                   ; reserved by architecture
            DQ      0                   ; reserved by future use
CodeSeg32   LABEL   QWORD
            DW      -1                  ; LimitLow
            DW      0                   ; BaseLow
            DB      0                   ; BaseMid
            DB      9bh
            DB      0cfh                ; LimitHigh
            DB      0                   ; BaseHigh
DataSeg32   LABEL   QWORD
            DW      -1                  ; LimitLow
            DW      0                   ; BaseLow
            DB      0                   ; BaseMid
            DB      93h
            DB      0cfh                ; LimitHigh
            DB      0                   ; BaseHigh

            DQ      0                   ; reserved by future use
            DQ      0                   ; reserved by future use

DataSeg64   LABEL   QWORD
            DW      -1                  ; LimitLow
            DW      0                   ; BaseLow
            DB      0                   ; BaseMid
            DB      93h
            DB      0cfh                ; LimitHigh
            DB      0                   ; BaseHigh
CodeSeg64   LABEL   QWORD
            DW      -1                  ; LimitLow
            DW      0                   ; BaseLow
            DB      0                   ; BaseMid
            DB      9bh
            DB      0afh                ; LimitHigh
            DB      0                   ; BaseHigh

            DQ      0                   ; reserved for future use
GDT_SIZE = $ - offset NullSeg

AsmApWakeup ENDP

AsmApWakeup32 PROC  PUBLIC
;
; Now we are in Protected mode
;
  db   66h, 0B8h
  dw   DATA_SEL
;  mov  ax, DATA_SEL
  db   66h, 8Eh, 0D8h
;  mov  ds, ax
  db   66h, 8Eh, 0C0h
;  mov  es, ax
  db   66h, 8Eh, 0D0h
;  mov  ss, ax

  ;
  ; Enable the 64-bit page-translation-table entries by
  ; setting CR4.PAE=1 (this is _required_ before activating
  ; long mode). Paging is not enabled until after long mode
  ; is enabled.
  ;
  db 0fh, 20h, 0e0h
;  mov eax, cr4
  bts eax, 5
  db 0fh, 22h, 0e0h
;  mov cr4, eax
  db  0b8h
PageTableOffset::
  dd  00000000h  ; page table, place holder
;  mov eax, 00000000h
  db  0Fh, 22h, 0D8h
;  mov cr3, eax

  ;
  ; Enable long mode (set EFER.LME=1).
  ;
  db  0b9h
  dd  0c0000080h
;  mov   ecx, 0c0000080h ; EFER MSR number.
  db 0fh, 32h
;  rdmsr                 ; Read EFER.
  db    0fh, 0bah, 0e8h, 08h
;  bts   eax, 8          ; Set LME=1.
  db 0fh, 30h
;  wrmsr                 ; Write EFER.

  ;
  ; Enable paging to activate long mode (set CR0.PG=1)
  ;
  db    0Fh, 20h, 0C0h
;  mov   eax, cr0        ; Read CR0.
  db    0fh, 0bah, 0e8h, 01fh
;  bts   eax, 31         ; Set PG=1.
  db    0Fh, 22h, 0C0h
;  mov   cr0, eax        ; Write CR0.

; Go to long mode
    db      067h
    db      0eah                ; Far Jump $+9:Selector to reload CS
LongModeEntryOffset::
    dd      00000000            ; TO BE FIXED
    dw      CODE64_SEL          ;   Selector is our code selector, 38h

;
; Now we are in Long mode
;
LongModeEntry::
  mov  eax,DATA64_SEL
  mov  ds,ax
  mov  es,ax
  mov  ss,ax
; Search the stack pointer for the current CPU according to its
  mov  eax, 0FEE00000h
  mov  edx, [eax + 020h]
  shr  edx, 24          ; dl = LocalApicID

; set common stack bottom
  lea  rax, mApStack
  mov  rsp, [rax]       ; rsp = common stack bottom

; set CpuNum
  xor  rcx, rcx
  lea  rax, mCpuNum
  mov  ecx, [rax]       ; rcx = CpuNum (Max loop)

; search APIC ID in the list to get the CpuIndex
  lea  rax, mApicIdList
  mov  rsi, [rax]       ; rsi = ApicIdList
  mov  rdi, rsi         ; rdi = ApicIdList header
  
SearchApicId:           ; rdi = ApicId ptr
  mov  bl, [rdi]        ; bl = ApicId from ApicIdList
  cmp  bl, dl           ; compare with this ApicId
  jz   ApicIdFound      ; found
  add  rdi, 4           ; rdi = Next ApicId ptr
  loop SearchApicId

ApicIdNotFound:
  jmp $

ApicIdFound:
  sub rdi, rsi      ; rdi = Index * 4
  shr rdi, 2        ; rdi = Index
  mov rcx, rdi      ; rcx = Index
  shl rdi, 12       ; rsi = Index * STACK_SIZE (0x1000)
  add rsp, rdi      ; rsp = this stack bottom
  add rsp, 1000h    ; rsp = this stack top
  
  sub rsp, 20h
  call ApWakeupC
  add rsp, 20h

  jmp $
AsmApWakeup32 ENDP

AsmGuestApEntrypoint PROC PUBLIC
  cli
  hlt
  jmp $
AsmGuestApEntrypoint ENDP

  END
