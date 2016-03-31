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
;    Exception.asm
;
;------------------------------------------------------------------------------

  .686P
  .MMX
  .MODEL FLAT,C

EXTERNDEF      mExceptionHandlerLength:DWORD
EXTERNDEF      mExternalVectorTablePtr:DWORD

EXTRN mErrorCodeFlag:DWORD             ; Error code flags for exceptions

.DATA
mExceptionHandlerLength DD 8

mExternalVectorTablePtr DWORD 0 ; point to the external interrupt vector table

  .CODE

ALIGN   8

PUBLIC AsmExceptionHandlers
        
AsmExceptionHandlers LABEL BYTE
REPEAT  32
        call  CommonInterruptEntry
        dw ( $ - AsmExceptionHandlers - 5 ) / 8 ; vector number
        nop
ENDM

;---------------------------------------;
; CommonInterruptEntry                  ;
;---------------------------------------;
; The follow algorithm is used for the common interrupt routine.

;
; +---------------------+
; +    EFlags           +
; +---------------------+
; +    CS               +
; +---------------------+
; +    EIP              +
; +---------------------+
; +    Error Code       +
; +---------------------+
; + EAX / Vector Number +
; +---------------------+
; +    EBP              +
; +---------------------+ <-- EBP
;

CommonInterruptEntry PROC 
  cli
  ;
  ; All interrupt handlers are invoked through interrupt gates, so
  ; IF flag automatically cleared at the entry point
  ;
  ;
  ; Calculate vector number
  ;
  xchg    eax, [esp] ; get the return address of call, actually, it is the address of vector number.
  movzx   eax, word ptr [eax]        
  cmp     eax, 32         ; Intel reserved vector for exceptions?
  jae     NoErrorCode
  bt      mErrorCodeFlag, eax
  jc      @F

NoErrorCode:
  ;
  ; Push a dummy error code on the stack
  ; to maintain coherent stack map
  ;
  push    [esp]
  mov     dword ptr [esp + 4], 0
@@:       
  push    ebp
  mov     ebp, esp

  ;
  ; Align stack to make sure that EFI_FX_SAVE_STATE_IA32 of EFI_SYSTEM_CONTEXT_IA32
  ; is 16-byte aligned
  ;
  and     esp, 0fffffff0h
  sub     esp, 12

;; UINT32  Edi, Esi, Ebp, Esp, Ebx, Edx, Ecx, Eax;
  push    dword ptr [ebp + 4]          ; EAX
  push    ecx
  push    edx
  push    ebx
  lea     ecx, [ebp + 24]
  push    ecx                          ; ESP
  push    dword ptr [ebp]              ; EBP
  push    esi
  push    edi

  mov     [ebp + 4], eax               ; save vector number

;; UINT32  Gs, Fs, Es, Ds, Cs, Ss;
  mov  eax, ss
  push eax
  movzx eax, word ptr [ebp + 16]
  push eax
  mov  eax, ds
  push eax
  mov  eax, es
  push eax
  mov  eax, fs
  push eax
  mov  eax, gs
  push eax

;; UINT32  Eip;
  push    dword ptr [ebp + 12]

;; UINT32  Gdtr[2], Idtr[2];
  sub  esp, 8
  sidt fword ptr [esp]
  sub  esp, 8
  sgdt fword ptr [esp]

;; UINT32  Ldtr, Tr;
  xor  eax, eax
  str  ax
  push eax
  sldt ax
  push eax

;; UINT32  EFlags;
  push    dword ptr [ebp + 20]

;; UINT32  Cr0, Cr1, Cr2, Cr3, Cr4;
  mov  eax, cr4
  or   eax, 208h
  mov  cr4, eax
  push eax
  mov  eax, cr3
  push eax
  mov  eax, cr2
  push eax
  xor  eax, eax
  push eax
  mov  eax, cr0
  push eax

;; UINT32  Dr0, Dr1, Dr2, Dr3, Dr6, Dr7;
  mov     eax, dr7
  push    eax
  mov     eax, dr6
  push    eax
  mov     eax, dr3
  push    eax
  mov     eax, dr2
  push    eax
  mov     eax, dr1
  push    eax
  mov     eax, dr0
  push    eax

;; FX_SAVE_STATE_IA32 FxSaveState;
  sub esp, 512
  mov edi, esp
  db 0fh, 0aeh, 00000111y ;fxsave [edi]

;; UEFI calling convention for IA32 requires that Direction flag in EFLAGs is clear
  cld

;; UINT32  ExceptionData;
  push    dword ptr [ebp + 8]

;; call into exception handler
  mov     ebx, [ebp + 4]
  mov     eax, mExternalVectorTablePtr
  mov     eax, [eax + ebx * 4]
  or      eax, eax                ; NULL?
  je  nonNullValue;

;; Prepare parameter and call
  mov     edx, esp
  push    edx
  push    ebx
  call    eax
  add     esp, 8

nonNullValue:
  cli
;; UINT32  ExceptionData;
  add esp, 4

;; FX_SAVE_STATE_IA32 FxSaveState;
  mov esi, esp
  db 0fh, 0aeh, 00001110y ; fxrstor [esi]
  add esp, 512

;; UINT32  Dr0, Dr1, Dr2, Dr3, Dr6, Dr7;
;; Skip restoration of DRx registers to support in-circuit emualators
;; or debuggers set breakpoint in interrupt/exception context
  add     esp, 4 * 6

;; UINT32  Cr0, Cr1, Cr2, Cr3, Cr4;
  pop     eax
  mov     cr0, eax
  add     esp, 4    ; not for Cr1
  pop     eax
  mov     cr2, eax
  pop     eax
  mov     cr3, eax
  pop     eax
  mov     cr4, eax

;; UINT32  EFlags;
  pop     dword ptr [ebp + 20]

;; UINT32  Ldtr, Tr;
;; UINT32  Gdtr[2], Idtr[2];
;; Best not let anyone mess with these particular registers...
  add     esp, 24

;; UINT32  Eip;
  pop     dword ptr [ebp + 12]

;; UINT32  Gs, Fs, Es, Ds, Cs, Ss;
  pop     gs
  pop     fs
  pop     es
  pop     ds
  pop     dword ptr [ebp + 16]
  pop     ss

;; UINT32  Edi, Esi, Ebp, Esp, Ebx, Edx, Ecx, Eax;
  pop     edi
  pop     esi
  add     esp, 4   ; not for ebp
  add     esp, 4   ; not for esp
  pop     ebx
  pop     edx
  pop     ecx
  pop     eax

  mov     esp, ebp
  pop     ebp
  add     esp, 8
  iretd

CommonInterruptEntry ENDP

  END

