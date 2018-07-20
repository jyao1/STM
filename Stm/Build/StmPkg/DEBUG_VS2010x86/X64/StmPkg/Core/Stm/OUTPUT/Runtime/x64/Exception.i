#line 1 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\Core\\Runtime\\x64\\Exception.asm"
#line 1 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\Build\\StmPkg\\DEBUG_VS2010x86\\X64\\StmPkg\\Core\\Stm\\DEBUG\\AutoGen.h"















#line 1 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\EdkII\\MdePkg\\Include\\Base.h"



























#line 1 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\EdkII\\MdePkg\\Include\\X64\\ProcessorBind.h"


























#pragma pack()
#line 29 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\EdkII\\MdePkg\\Include\\X64\\ProcessorBind.h"



























#line 57 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\EdkII\\MdePkg\\Include\\X64\\ProcessorBind.h"












#pragma warning ( disable : 4214 )




#pragma warning ( disable : 4100 )





#pragma warning ( disable : 4057 )




#pragma warning ( disable : 4127 )




#pragma warning ( disable : 4505 )




#pragma warning ( disable : 4206 )

#line 98 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\EdkII\\MdePkg\\Include\\X64\\ProcessorBind.h"



  
  
  

  
  
  
  typedef unsigned __int64    UINT64;
  
  
  
  typedef __int64             INT64;
  
  
  
  typedef unsigned __int32    UINT32;
  
  
  
  typedef __int32             INT32;
  
  
  
  typedef unsigned short      UINT16;
  
  
  
  
  typedef unsigned short      CHAR16;
  
  
  
  typedef short               INT16;
  
  
  
  
  typedef unsigned char       BOOLEAN;
  
  
  
  typedef unsigned char       UINT8;
  
  
  
  typedef char                CHAR8;
  
  
  
  typedef char                INT8;















































#line 199 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\EdkII\\MdePkg\\Include\\X64\\ProcessorBind.h"





typedef UINT64  UINTN;




typedef INT64   INTN;



































  
  
  
  
















#line 266 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\EdkII\\MdePkg\\Include\\X64\\ProcessorBind.h"







#line 274 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\EdkII\\MdePkg\\Include\\X64\\ProcessorBind.h"













#line 288 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\EdkII\\MdePkg\\Include\\X64\\ProcessorBind.h"

#line 29 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\EdkII\\MdePkg\\Include\\Base.h"



















extern UINT8 _VerifySizeofBOOLEAN[(sizeof(BOOLEAN) == (1)) / (sizeof(BOOLEAN) == (1))];
extern UINT8 _VerifySizeofINT8[(sizeof(INT8) == (1)) / (sizeof(INT8) == (1))];
extern UINT8 _VerifySizeofUINT8[(sizeof(UINT8) == (1)) / (sizeof(UINT8) == (1))];
extern UINT8 _VerifySizeofINT16[(sizeof(INT16) == (2)) / (sizeof(INT16) == (2))];
extern UINT8 _VerifySizeofUINT16[(sizeof(UINT16) == (2)) / (sizeof(UINT16) == (2))];
extern UINT8 _VerifySizeofINT32[(sizeof(INT32) == (4)) / (sizeof(INT32) == (4))];
extern UINT8 _VerifySizeofUINT32[(sizeof(UINT32) == (4)) / (sizeof(UINT32) == (4))];
extern UINT8 _VerifySizeofINT64[(sizeof(INT64) == (8)) / (sizeof(INT64) == (8))];
extern UINT8 _VerifySizeofUINT64[(sizeof(UINT64) == (8)) / (sizeof(UINT64) == (8))];
extern UINT8 _VerifySizeofCHAR8[(sizeof(CHAR8) == (1)) / (sizeof(CHAR8) == (1))];
extern UINT8 _VerifySizeofCHAR16[(sizeof(CHAR16) == (2)) / (sizeof(CHAR16) == (2))];







  
  
  
  
  
  







#line 80 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\EdkII\\MdePkg\\Include\\Base.h"
















#line 97 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\EdkII\\MdePkg\\Include\\Base.h"







#line 105 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\EdkII\\MdePkg\\Include\\Base.h"
  
#line 107 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\EdkII\\MdePkg\\Include\\Base.h"








  
#line 117 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\EdkII\\MdePkg\\Include\\Base.h"





typedef struct {
  UINT32  Data1;
  UINT16  Data2;
  UINT16  Data3;
  UINT8   Data4[8];
} GUID;




typedef UINT64 PHYSICAL_ADDRESS;




typedef struct _LIST_ENTRY LIST_ENTRY;




struct _LIST_ENTRY {
  LIST_ENTRY  *ForwardLink;
  LIST_ENTRY  *BackLink;
};























































































































































































 































































































































#line 458 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\EdkII\\MdePkg\\Include\\Base.h"
















#line 475 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\EdkII\\MdePkg\\Include\\Base.h"




typedef CHAR8 *VA_LIST;

















































#line 530 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\EdkII\\MdePkg\\Include\\Base.h"




typedef UINTN  *BASE_LIST;










































































































  




































typedef UINTN RETURN_STATUS;



















































































































































































































































































#line 955 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\EdkII\\MdePkg\\Include\\Base.h"


#line 17 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\Build\\StmPkg\\DEBUG_VS2010x86\\X64\\StmPkg\\Core\\Stm\\DEBUG\\AutoGen.h"
#line 1 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\EdkII\\MdePkg\\Include\\Library/PcdLib.h"






































































































































































































































































































































































































































































































































                                            






























































































































































































































































                                         
















UINTN
__cdecl
LibPcdSetSku (
   UINTN   SkuId
  );












UINT8
__cdecl
LibPcdGet8 (
   UINTN             TokenNumber
  );












UINT16
__cdecl
LibPcdGet16 (
   UINTN             TokenNumber
  );












UINT32
__cdecl
LibPcdGet32 (
   UINTN             TokenNumber
  );












UINT64
__cdecl
LibPcdGet64 (
   UINTN             TokenNumber
  );












void *
__cdecl
LibPcdGetPtr (
   UINTN             TokenNumber
  );












BOOLEAN 
__cdecl
LibPcdGetBool (
   UINTN             TokenNumber
  );










UINTN
__cdecl
LibPcdGetSize (
   UINTN             TokenNumber
  );
















UINT8
__cdecl
LibPcdGetEx8 (
   const GUID        *Guid,
   UINTN             TokenNumber
  );
















UINT16
__cdecl
LibPcdGetEx16 (
   const GUID        *Guid,
   UINTN             TokenNumber
  );













UINT32
__cdecl
LibPcdGetEx32 (
   const GUID        *Guid,
   UINTN             TokenNumber
  );
















UINT64
__cdecl
LibPcdGetEx64 (
   const GUID        *Guid,
   UINTN             TokenNumber
  );
















void *
__cdecl
LibPcdGetExPtr (
   const GUID        *Guid,
   UINTN             TokenNumber
  );
















BOOLEAN
__cdecl
LibPcdGetExBool (
   const GUID        *Guid,
   UINTN             TokenNumber
  );
















UINTN
__cdecl
LibPcdGetExSize (
   const GUID        *Guid,
   UINTN             TokenNumber
  );














UINT8
__cdecl
LibPcdSet8 (
   UINTN             TokenNumber,
   UINT8             Value
  );














UINT16
__cdecl
LibPcdSet16 (
   UINTN             TokenNumber,
   UINT16            Value
  );














UINT32
__cdecl
LibPcdSet32 (
   UINTN             TokenNumber,
   UINT32            Value
  );














UINT64
__cdecl
LibPcdSet64 (
   UINTN             TokenNumber,
   UINT64            Value
  );
























void *
__cdecl
LibPcdSetPtr (
          UINTN             TokenNumber,
       UINTN             *SizeOfBuffer,
   const  void              *Buffer
  );














BOOLEAN
__cdecl
LibPcdSetBool (
   UINTN             TokenNumber,
   BOOLEAN           Value
  );


















UINT8
__cdecl
LibPcdSetEx8 (
   const GUID        *Guid,
   UINTN             TokenNumber,
   UINT8             Value
  );


















UINT16
__cdecl
LibPcdSetEx16 (
   const GUID        *Guid,
   UINTN             TokenNumber,
   UINT16            Value
  );


















UINT32
__cdecl
LibPcdSetEx32 (
   const GUID        *Guid,
   UINTN             TokenNumber,
   UINT32            Value
  );

















UINT64
__cdecl
LibPcdSetEx64 (
   const GUID        *Guid,
   UINTN             TokenNumber,
   UINT64            Value
  );
























void *
__cdecl
LibPcdSetExPtr (
        const GUID        *Guid,
        UINTN             TokenNumber,
     UINTN             *SizeOfBuffer,
        void              *Buffer
  );


















BOOLEAN
__cdecl
LibPcdSetExBool (
   const GUID        *Guid,
   UINTN             TokenNumber,
   BOOLEAN           Value
  );


















typedef
void
(__cdecl *PCD_CALLBACK)(
          const GUID        *CallBackGuid, 
          UINTN             CallBackToken,
       void              *TokenData,
          UINTN             TokenDataSize
  );


















void
__cdecl
LibPcdCallbackOnSet (
   const GUID               *Guid,       
   UINTN                    TokenNumber,
   PCD_CALLBACK             NotificationFunction
  );















void
__cdecl
LibPcdCancelCallback (
   const GUID               *Guid,       
   UINTN                    TokenNumber,
   PCD_CALLBACK             NotificationFunction
  );





















UINTN           
__cdecl
LibPcdGetNextToken (
   const GUID               *Guid,       
   UINTN                    TokenNumber
  );
















GUID *
__cdecl
LibPcdGetNextTokenSpace (
   const GUID  *TokenSpaceGuid
  );

























void *
__cdecl
LibPatchPcdSetPtr (
          void        *PatchVariable,
          UINTN       MaximumDatumSize,
       UINTN       *SizeOfBuffer,
   const  void        *Buffer
  );

#line 1507 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\EdkII\\MdePkg\\Include\\Library/PcdLib.h"

#line 18 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\Build\\StmPkg\\DEBUG_VS2010x86\\X64\\StmPkg\\Core\\Stm\\DEBUG\\AutoGen.h"

extern GUID  gEfiCallerIdGuid;
extern CHAR8 *gEfiCallerBaseName;





extern GUID gEfiStmPkgTokenSpaceGuid;







extern const  UINT8  _gPcd_FixedAtBuild_PcdPerformanceLibraryPropertyMask;





extern volatile   UINT64  _gPcd_BinaryPatch_PcdPciExpressBaseAddress;



extern UINTN _gPcd_BinaryPatch_Size_PcdPciExpressBaseAddress; 










#line 56 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\Build\\StmPkg\\DEBUG_VS2010x86\\X64\\StmPkg\\Core\\Stm\\DEBUG\\AutoGen.h"
#line 1 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\Core\\Runtime\\x64\\Exception.asm"
;------------------------------------------------------------------------------
;
; Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
; This program and the accompanying materials
; are licensed and made available under the terms and conditions of the BSD License
; which accompanies this distribution.  The full text of the license may be found at
; http:
;
; THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
; WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
;
; Module Name:
; 
;    Exception.asm
;
;------------------------------------------------------------------------------

EXTERNDEF      mExceptionHandlerLength:DWORD
EXTERNDEF      mExternalVectorTablePtr:QWORD

EXTRN mErrorCodeFlag:DWORD ; Error code flags for exceptions

.DATA
mExceptionHandlerLength DD 8

mExternalVectorTablePtr QWORD 0 ; point to the external interrupt vector table

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
; +---------------------+ <-- 16-byte aligned ensured by processor
; +    Old SS           +
; +---------------------+
; +    Old RSP          +
; +---------------------+
; +    RFlags           +
; +---------------------+
; +    CS               +
; +---------------------+
; +    RIP              +
; +---------------------+
; +    Error Code       +
; +---------------------+
; + RCX / Vector Number +
; +---------------------+
; +    RBP              +
; +---------------------+ <-- RBP, 16-byte aligned
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
  xchg    rcx, [rsp] ; get the return address of call, actually, it is the address of vector number.
  movzx   ecx, word ptr [rcx]        
  cmp     ecx, 32         ; Intel reserved vector for exceptions?
  jae     NoErrorCode
  bt      mErrorCodeFlag, ecx
  jc      @F

NoErrorCode:
  ;
  ; Push a dummy error code on the stack
  ; to maintain coherent stack map
  ;
  push    [rsp]
  mov     qword ptr [rsp + 8], 0
@@:       
  push    rbp
  mov     rbp, rsp

  ;
  ; Since here the stack pointer is 16-byte aligned, so
  ; EFI_FX_SAVE_STATE_X64 of EFI_SYSTEM_CONTEXT_x64
  ; is 16-byte aligned
  ;       

;; UINT64  Rdi, Rsi, Rbp, Rsp, Rbx, Rdx, Rcx, Rax;
;; UINT64  R8, R9, R10, R11, R12, R13, R14, R15;
  push r15
  push r14
  push r13
  push r12
  push r11
  push r10
  push r9
  push r8
  push rax
  push qword ptr [rbp + 8]   ; RCX
  push rdx
  push rbx
  push qword ptr [rbp + 48]  ; RSP
  push qword ptr [rbp]       ; RBP
  push rsi
  push rdi

;; UINT64  Gs, Fs, Es, Ds, Cs, Ss;  insure high 16 bits of each is zero
  movzx   rax, word ptr [rbp + 56]
  push    rax                      ; for ss
  movzx   rax, word ptr [rbp + 32]
  push    rax                      ; for cs
  mov     rax, ds
  push    rax
  mov     rax, es
  push    rax
  mov     rax, fs
  push    rax
  mov     rax, gs
  push    rax

  mov     [rbp + 8], rcx               ; save vector number

;; UINT64  Rip;
  push    qword ptr [rbp + 24]

;; UINT64  Gdtr[2], Idtr[2];
  sub     rsp, 16
  sidt    fword ptr [rsp]
  sub     rsp, 16
  sgdt    fword ptr [rsp]

;; UINT64  Ldtr, Tr;
  xor     rax, rax
  str     ax
  push    rax
  sldt    ax
  push    rax

;; UINT64  RFlags;
  push    qword ptr [rbp + 40]

;; UINT64  Cr0, Cr1, Cr2, Cr3, Cr4, Cr8;
  mov     rax, cr8
  push    rax
  mov     rax, cr4
  or      rax, 208h
  mov     cr4, rax
  push    rax
  mov     rax, cr3
  push    rax
  mov     rax, cr2
  push    rax
  xor     rax, rax
  push    rax
  mov     rax, cr0
  push    rax

;; UINT64  Dr0, Dr1, Dr2, Dr3, Dr6, Dr7;
  mov     rax, dr7
  push    rax
  mov     rax, dr6
  push    rax
  mov     rax, dr3
  push    rax
  mov     rax, dr2
  push    rax
  mov     rax, dr1
  push    rax
  mov     rax, dr0
  push    rax

;; FX_SAVE_STATE_X64 FxSaveState;

  sub rsp, 512
  mov rdi, rsp
  db 0fh, 0aeh, 00000111y ;fxsave [rdi]

;; UEFI calling convention for x64 requires that Direction flag in EFLAGs is clear
  cld

;; UINT32  ExceptionData;
  push    qword ptr [rbp + 16]

;; call into exception handler
  mov     rcx, [rbp + 8]
  mov     rax, mExternalVectorTablePtr  ; get the interrupt vectors base
  mov     rax, [rax + rcx * 8]       
  or      rax, rax                        ; ((void *) 0)?

  je    nonNullValue;

;; Prepare parameter and call
;  mov     rcx, [rbp + 8]
  mov     rdx, rsp
  ;
  ; Per X64 calling convention, allocate maximum parameter stack space
  ; and make sure RSP is 16-byte aligned
  ;
  sub     rsp, 4 * 8 + 8
  call    rax
  add     rsp, 4 * 8 + 8

nonNullValue:
  cli
;; UINT64  ExceptionData;
  add     rsp, 8

;; FX_SAVE_STATE_X64 FxSaveState;

  mov rsi, rsp
  db 0fh, 0aeh, 00001110y ; fxrstor [rsi]
  add rsp, 512

;; UINT64  Dr0, Dr1, Dr2, Dr3, Dr6, Dr7;
;; Skip restoration of DRx registers to support in-circuit emualators
;; or debuggers set breakpoint in interrupt/exception context
  add     rsp, 8 * 6

;; UINT64  Cr0, Cr1, Cr2, Cr3, Cr4, Cr8;
  pop     rax
  mov     cr0, rax
  add     rsp, 8   ; not for Cr1
  pop     rax
  mov     cr2, rax
  pop     rax
  mov     cr3, rax
  pop     rax
  mov     cr4, rax
  pop     rax
  mov     cr8, rax

;; UINT64  RFlags;
  pop     qword ptr [rbp + 40]

;; UINT64  Ldtr, Tr;
;; UINT64  Gdtr[2], Idtr[2];
;; Best not let anyone mess with these particular registers...
  add     rsp, 48

;; UINT64  Rip;
  pop     qword ptr [rbp + 24]

;; UINT64  Gs, Fs, Es, Ds, Cs, Ss;
  pop     rax
  ; mov     gs, rax ; not for gs
  pop     rax
  ; mov     fs, rax ; not for fs
  ; (X64 will not use fs and gs, so we do not restore it)
  pop     rax
  mov     es, rax
  pop     rax
  mov     ds, rax
  pop     qword ptr [rbp + 32]  ; for cs
  pop     qword ptr [rbp + 56]  ; for ss

;; UINT64  Rdi, Rsi, Rbp, Rsp, Rbx, Rdx, Rcx, Rax;
;; UINT64  R8, R9, R10, R11, R12, R13, R14, R15;
  pop     rdi
  pop     rsi
  add     rsp, 8               ; not for rbp
  pop     qword ptr [rbp + 48] ; for rsp
  pop     rbx
  pop     rdx
  pop     rcx
  pop     rax
  pop     r8
  pop     r9
  pop     r10
  pop     r11
  pop     r12
  pop     r13
  pop     r14
  pop     r15

  mov     rsp, rbp
  pop     rbp
  add     rsp, 16
  iretq

CommonInterruptEntry ENDP

  END

