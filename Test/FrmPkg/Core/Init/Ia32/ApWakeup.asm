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

  .686P
  .MMX
  .MODEL SMALL

EXTERNDEF   C   mApWakeupSegmentOffset:DWORD
EXTERNDEF   C   mApProtectedModeEntryOffset:DWORD
EXTERNDEF   C   mApGdtBaseOffset:DWORD
EXTERNDEF   C   mApGdtBase:DWORD
EXTERNDEF   C   mCodeSel:DWORD
EXTERNDEF   C   mApGdtrOffset:DWORD

EXTERNDEF   C   mCpuNum:NEAR
EXTERNDEF   C   mApStack:NEAR
EXTERNDEF   C   mApicIdList:NEAR

EXTERNDEF   C   ApWakeupC:PROC

  .DATA
  
mApWakeupSegmentOffset       DD WakeupSegmentOffset - AsmApWakeup
mApProtectedModeEntryOffset  DD ProtectedModeEntryOffset - AsmApWakeup
mApGdtBaseOffset             DD GdtBaseOffset - AsmApWakeup
mApGdtBase                   DD NullSeg - AsmApWakeup
mCodeSel                     DD CODE_SEL
mApGdtrOffset                DD GDTR_OFFSET

CODE_SEL    = offset CodeSeg32 - offset NullSeg
DATA_SEL    = offset DataSeg32 - offset NullSeg
GDTR_OFFSET = GdtrOffset - AsmApWakeup

  .CODE

AsmApWakeup PROC C PUBLIC
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

AsmApWakeup32 PROC C PUBLIC
  mov  ax,DATA_SEL
  mov  ds,ax
  mov  es,ax
  mov  ss,ax
; Search the stack pointer for the current CPU according to its
  mov  eax, 0FEE00000h
  mov  edx, [eax + 020h]
  shr  edx, 24          ; dl = LocalApicID

; set common stack bottom
  lea  eax, mApStack
  mov  esp, [eax]       ; esp = common stack bottom

; set CpuNum
  lea  eax, mCpuNum
  mov  ecx, [eax]       ; ecx = CpuNum (Max loop)

; search APIC ID in the list to get the CpuIndex
  lea  eax, mApicIdList
  mov  esi, [eax]       ; esi = ApicIdList
  mov  edi, esi         ; edi = ApicIdList header
  
SearchApicId:           ; edi = ApicId ptr
  mov  bl, [edi]        ; bl = ApicId from gApicIdList
  cmp  bl, dl           ; compare with this ApicId
  jz   ApicIdFound      ; found
  add  edi, 4           ; edi = Next ApicId ptr
  loop SearchApicId

ApicIdNotFound:
  jmp $

ApicIdFound:
  sub edi, esi      ; edi = Index * 4
  shr edi, 2        ; edi = Index
  mov ecx, edi      ; ecx = Index
  shl edi, 12       ; esi = Index * STACK_SIZE (0x1000)
  add esp, edi      ; esp = this stack bottom
  add esp, 1000h    ; esp = this stack top
  
  push ecx
  call ApWakeupC
  add  esp, 4

  jmp $
AsmApWakeup32 ENDP

AsmGuestApEntrypoint PROC C PUBLIC
  cli
  hlt
  jmp $
AsmGuestApEntrypoint ENDP

  END

