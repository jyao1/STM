#line 1 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\Core\\Runtime\\Ia32\\Exception.asm"
#line 1 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\Build\\StmPkg\\DEBUG_VS2010x86\\IA32\\StmPkg\\Core\\Stm\\DEBUG\\AutoGen.h"















#line 1 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\EdkII\\MdePkg\\Include\\Base.h"



























#line 1 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\EdkII\\MdePkg\\Include\\Ia32\\ProcessorBind.h"


























#pragma pack()
#line 29 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\EdkII\\MdePkg\\Include\\Ia32\\ProcessorBind.h"


























#line 56 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\EdkII\\MdePkg\\Include\\Ia32\\ProcessorBind.h"












#pragma warning ( disable : 4214 )




#pragma warning ( disable : 4100 )





#pragma warning ( disable : 4057 )




#pragma warning ( disable : 4127 )




#pragma warning ( disable : 4505 )




#pragma warning ( disable : 4206 )

#line 97 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\EdkII\\MdePkg\\Include\\Ia32\\ProcessorBind.h"




  
  
  

  
  
  
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















































#line 199 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\EdkII\\MdePkg\\Include\\Ia32\\ProcessorBind.h"





typedef UINT32  UINTN;




typedef INT32   INTN;


































  
  
  
  







#line 256 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\EdkII\\MdePkg\\Include\\Ia32\\ProcessorBind.h"







#line 264 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\EdkII\\MdePkg\\Include\\Ia32\\ProcessorBind.h"













#line 278 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\EdkII\\MdePkg\\Include\\Ia32\\ProcessorBind.h"

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


#line 17 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\Build\\StmPkg\\DEBUG_VS2010x86\\IA32\\StmPkg\\Core\\Stm\\DEBUG\\AutoGen.h"
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

#line 18 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\Build\\StmPkg\\DEBUG_VS2010x86\\IA32\\StmPkg\\Core\\Stm\\DEBUG\\AutoGen.h"

extern GUID  gEfiCallerIdGuid;
extern CHAR8 *gEfiCallerBaseName;





extern GUID gEfiStmPkgTokenSpaceGuid;







extern const  UINT8  _gPcd_FixedAtBuild_PcdPerformanceLibraryPropertyMask;





extern volatile   UINT64  _gPcd_BinaryPatch_PcdPciExpressBaseAddress;



extern UINTN _gPcd_BinaryPatch_Size_PcdPciExpressBaseAddress; 










#line 56 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\Build\\StmPkg\\DEBUG_VS2010x86\\IA32\\StmPkg\\Core\\Stm\\DEBUG\\AutoGen.h"
#line 1 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\Core\\Runtime\\Ia32\\Exception.asm"
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
  or      eax, eax                ; ((void *) 0)?
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

