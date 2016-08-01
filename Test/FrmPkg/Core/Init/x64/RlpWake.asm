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
;    RlpWake.asm
;
; Abstract:
;
;------------------------------------------------------------------------------

include Smx.inc

.CODE

EXTERNDEF PostInitAddrRlp:DWORD

;------------------------------------------------------------------------------
; VOID 
; AsmRlpWakeUpCode (
;     VOID    
;     )
;------------------------------------------------------------------------------
AsmRlpWakeUpCode PROC PUBLIC
; It is 32bit protected mode here
  cli

  ;
  ; Enable SMI
  ;
  mov   ebx, 0
  mov   eax, GET_SEC_SMCTRL
  DB 0fh, 37h ; GETSEC

  ; Check DLE 64
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
  add     edi, _MLE_PRIVATE_DATA._ApEntry          ; edi = ApEntry Offset

  mov     eax, [rdi]
  cmp     eax, 0
  jz      NonDleAp

  ; Launch DLE64

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
    add     edi, _MLE_PRIVATE_DATA._RlpDsSeg         ; edi = DataSeg offset
    mov     ax, [rdi]                                ; ax = data segment
    mov     ds, ax
    mov     ss, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax

    mov     edi, esi                                 ; edi = MlePrivateData Offset
    add     edi, _MLE_PRIVATE_DATA._TempEspRlp       ; edi = TempEspRlp offset
    mov     eax, [rdi]                               ; eax = temporary stack
    sub     eax, 20h

    ; patch Offset/Segment

    sub     eax, 4
    mov     edi, esi                                      ; edi = MlePrivateData Offset
    add     edi, _MLE_PRIVATE_DATA._RlpPostSinitSegment   ; edi = RlpPostSinitSegment offset
    mov     edx, [rdi]                                    ; edx = RlpPostSinitSegment
    mov     [rax], edx

    sub     eax, 4
    mov     edi, esi                                      ; edi = MlePrivateData Offset
    add     edi, _MLE_PRIVATE_DATA._RlpPostSinitOffset    ; edi = RlpPostSinitOffset offset
    mov     edx, [rdi]                                    ; edx = RlpPostSinitOffset
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

POST_INIT_ADDR_RLP  = $ - offset AsmRlpWakeUpCode

; It is 64bit long mode here


  ;
  ; Notify RLP wakeup
  ;
; Critical Section - start -----------------------

  xor     rax, rax
  xor     rsi, rsi
  xor     rdi, rdi
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
  add     edi, _MLE_PRIVATE_DATA._Lock             ; edi = Lock Offset
  mov     ebp, edi                                 ; ebp = Lock Offset

; AcquireLock:    
  mov         al, 1
TryGetLock:
  xchg        al, byte ptr [rbp]
  cmp         al, 0
  jz          LockObtained
;  pause
  jmp         TryGetLock       
LockObtained:

  mov     edi, esi                                       ; edi = MlePrivateData Offset
  add     edi, _MLE_PRIVATE_DATA._RlpInitializedNumber   ; edi = RlpInitializedNumber Offset
  inc     DWORD PTR [rdi]                                ; increase RlpInitializedNumber

; ReleaseLock:    
  mov         al, 0
  xchg        al, byte ptr [rbp]

; Critical Section - end -----------------------


  xor     rax, rax
  xor     rsi, rsi
  xor     rdi, rdi
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
  add     edi, _MLE_PRIVATE_DATA._ApEntry          ; edi = ApEntry Offset

  mov     eax, [rdi]
  jmp     rax

NonDleAp:
  ; Should not get here
  jmp $
AsmRlpWakeUpCode ENDP

PostInitAddrRlp LABEL DWORD
  DD POST_INIT_ADDR_RLP

END
