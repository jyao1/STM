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
; It is 32bit protected mode here
    cli
    mov     eax, TXT_PUBLIC_SPACE
    add     eax, TXT_HEAP_BASE                       ; eax = HEAP Base Ptr
    mov     esi, [rax]                               ; esi = HEAP Base
    mov     edx, [rsi]                               ; edx = BiosOsDataSize
    add     esi, edx                                 ; esi = OsMleDataSize Offset
    add     esi, 8                                   ; esi = MlePrivateData Offset

    mov     edi, esi                                        ; edi = MyOsMleData Offset
    add     edi, _TXT_OS_TO_MLE_DATA._MlePrivateDataAddress ; edi = MlePrivateDataAddress offset
    mov     esi, [rdi]                                      ; esi = TxtOsMleData Offset

    mov     edi, esi                                 ; edi = MlePrivateData Offset
    add     edi, _MLE_PRIVATE_DATA._IdtrReg          ; edi = IDTR offset
    lidt    fword ptr [rdi]                          ; Reload IDT
    mov     edi, esi                                 ; edi = MlePrivateData Offset
    add     edi, _MLE_PRIVATE_DATA._GdtrReg          ; edi = GDTR offset
    lgdt    fword ptr [rdi]                          ; Reload GDT

    mov     edi, esi                                 ; edi = MlePrivateData Offset
    add     edi, _MLE_PRIVATE_DATA._DsSeg            ; edi = DataSeg offset
    mov     ax, [rdi]                                ; ax = data segment
    mov     ds, ax
    mov     ss, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax

    mov     edi, esi                                 ; edi = MlePrivateData Offset
    add     edi, _MLE_PRIVATE_DATA._TempEsp          ; edi = TempEsp offset
    mov     eax, [rdi]                               ; eax = temporary stack
    sub     eax, 20h

    ; patch Offset/Segment

    sub     eax, 4
    mov     edi, esi                                   ; edi = MlePrivateData Offset
    add     edi, _MLE_PRIVATE_DATA._PostSinitSegment   ; edi = PostSinitSegment offset
    mov     edx, [rdi]                                 ; edx = PostSinitSegment
    mov     [rax], edx

    sub     eax, 4
    mov     edi, esi                                   ; edi = MlePrivateData Offset
    add     edi, _MLE_PRIVATE_DATA._PostSinitOffset    ; edi = PostSinitOffset offset
    mov     edx, [rdi]                                 ; edx = PostSinitOffset
    mov     [rax], edx

    mov     esp, eax

    mov     edi, esi                                 ; edi = MlePrivateData Offset
    add     edi, _MLE_PRIVATE_DATA._Cr3              ; edi = CR3 offset
    mov     ebx, [rdi]                               ; ebx = CR3

    ;
    ; Enable the 64-bit page-translation-table entries by
    ; setting CR4.PAE=1 (this is _required_ before activating
    ; long mode). Paging is not enabled until after long mode
    ; is enabled.
    ;
    mov rax, cr4
    bts eax, 5
    bts eax, 9 ; enable XMM
    mov cr4, rax

    ;
    ; Get the long-mode page tables, and initialize the
    ; 64-bit CR3 (page-table base address) to point to the base
    ; of the PML4 page table. The PML4 page table must be located
    ; below 4 Gbytes because only 32 bits of CR3 are loaded when
    ; the processor is not in 64-bit mode.
    ;
    mov eax, ebx            ; Get Page Tables
    mov cr3, rax            ; Initialize CR3 with PML4 base.

    ;
    ; Enable long mode (set EFER.LME=1).
    ;
    mov ecx, 0c0000080h ; EFER MSR number.
    rdmsr               ; Read EFER.
    bts eax, 8          ; Set LME=1.
    wrmsr               ; Write EFER.

    ;
    ; Enable paging to activate long mode (set CR0.PG=1)
    ;
    mov rax, cr0 ; Read CR0.
    bts eax, 31  ; Set PG=1.
    mov cr0, rax ; Write CR0.

    ; reload CS
    retf

POST_INIT_ADDR  = $ - offset AsmMleEntryPoint

; It is 64bit long mode here
    call  DL_Entry_Back

    ; Should not get here
    jmp $
AsmMleEntryPoint ENDP

;------------------------------------------------------------------------------
; VOID
; AsmLaunchDlmeMain (
;   IN UINTN                  DlmeEntryPoint,   // rcx/[rsp + 8]
;   IN VOID                   *DlmeArgs,        // rdx/[rsp + 10h]
;   IN UINTN                  *StackBufferTop   // r8 /[rsp + 18h]
;   )
;------------------------------------------------------------------------------
AsmLaunchDlmeMain PROC  PUBLIC
  xor rax, rax
  mov rax, rcx   ; rax = DlmeMain
  
  mov rbx, r8
  add rbx, 0fh
  and bx, 0fff0h       ; rbx = new rsp
  sub rbx, 20h
  mov rsp, rbx

  mov  rcx, rdx
  call rax
  ; Should not get here
  jmp $
AsmLaunchDlmeMain ENDP

PostInitAddr LABEL DWORD
  DD POST_INIT_ADDR

END
