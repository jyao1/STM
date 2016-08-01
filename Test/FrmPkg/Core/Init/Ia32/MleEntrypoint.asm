;------------------------------------------------------------------------------
;
; Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
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
;    MleEntrypoint.asm
;
; Abstract:
;
;------------------------------------------------------------------------------

include Smx.inc

.686P
.MODEL FLAT, C
.CODE

EXTERNDEF DL_Entry_Back:NEAR
EXTERNDEF PostInitAddr:DWORD

;------------------------------------------------------------------------------
; VOID 
; AsmMleEntryPoint (
;     VOID    
;     )
;------------------------------------------------------------------------------
AsmMleEntryPoint PROC  PUBLIC
    cli
    mov     eax, TXT_PUBLIC_SPACE
    add     eax, TXT_HEAP_BASE                       ; eax = HEAP Base Ptr
    mov     esi, [eax]                               ; esi = HEAP Base
    mov     edx, [esi]                               ; edx = BiosOsDataSize
    add     esi, edx                                 ; esi = OsMleDataSize Offset
    add     esi, 8                                   ; esi = MlePrivateData Offset

    mov     edi, esi                                        ; edi = MyOsMleData Offset
    add     edi, _TXT_OS_TO_MLE_DATA._MlePrivateDataAddress ; edi = MlePrivateDataAddress offset
    mov     esi, [edi]                                      ; esi = TxtOsMleData Offset

    mov     edi, esi                                 ; edi = MlePrivateData Offset
    add     edi, _MLE_PRIVATE_DATA._IdtrReg          ; edi = IDTR offset
    lidt    fword ptr [edi]                          ; Reload IDT
    mov     edi, esi                                 ; edi = MlePrivateData Offset
    add     edi, _MLE_PRIVATE_DATA._GdtrReg          ; edi = GDTR offset
    lgdt    fword ptr [edi]                          ; Reload GDT

    mov     eax, (_MLE_PRIVATE_DATA PTR [esi])._DsSeg ; eax = data segment
    mov     ds, ax
    mov     ss, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax

    mov     eax, (_MLE_PRIVATE_DATA PTR [esi])._TempEsp ; eax = temporary stack
    sub     eax, 20h

    ; patch Offset/Segment

    sub     eax, 4
    mov     edx, (_MLE_PRIVATE_DATA PTR [esi])._PostSinitSegment ; edx = PostSinitSegment
    mov     [eax], edx

    sub     eax, 4
    mov     edx, (_MLE_PRIVATE_DATA PTR [esi])._PostSinitOffset  ; edx = PostSinitOffset
    mov     [eax], edx

    mov     esp, eax

    ; reload CS
    retf

POST_INIT_ADDR  = $ - offset AsmMleEntryPoint

    ; use long jump to restore all the other register
    call  DL_Entry_Back

    ; Should not get here
    jmp $
AsmMleEntryPoint ENDP

;------------------------------------------------------------------------------
; VOID
; AsmLaunchDlmeMain (
;   IN UINTN                  DlmeEntryPoint,   // [esp + 4]
;   IN VOID                   *DlmeArgs,        // [esp + 8]
;   IN UINTN                  *StackBufferTop   // [esp + 0ch]
;   )
;------------------------------------------------------------------------------
AsmLaunchDlmeMain PROC  PUBLIC
  mov ebp, esp         ; ebp = old esp
  mov eax, [ebp + 4]   ; eax = DlmeMain
  
  mov ebx, [esp + 0Ch]
  add ebx, 0fh
  and bx, 0fff0h       ; ebx = new esp
  mov esp, ebx
  
  mov  edx, [ebp + 8h]
  push edx

  call eax
  ; Should not get here
  jmp $
AsmLaunchDlmeMain ENDP

PostInitAddr LABEL DWORD
  DD POST_INIT_ADDR

END
