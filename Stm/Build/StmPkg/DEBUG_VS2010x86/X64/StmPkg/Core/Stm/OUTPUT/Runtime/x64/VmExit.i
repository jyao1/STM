#line 1 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\Core\\Runtime\\x64\\VmExit.asm"
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
#line 1 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\Core\\Runtime\\x64\\VmExit.asm"
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
;    VmExit.asm
;
;------------------------------------------------------------------------------

EXTERNDEF      StmHandlerSmi:PROC
EXTERNDEF      StmHandlerSmm:PROC

.CODE

AsmHostEntrypointSmi PROC PUBLIC
  sub rsp, 512
  fxsave  [rsp]
  push r15
  push r14
  push r13
  push r12
  push r11
  push r10
  push r9
  push r8
  push rdi
  push rsi
  push rbp
  push rbp ; should be rsp
  push rbx
  push rdx
  push rcx
  push rax
  mov  rcx, rsp ; parameter
  sub  rsp, 20h
  call StmHandlerSmi
  add  rsp, 20h
  jmp $
AsmHostEntrypointSmi ENDP

AsmHostEntrypointSmm PROC PUBLIC
  sub rsp, 512
  fxsave  [rsp]
  push r15
  push r14
  push r13
  push r12
  push r11
  push r10
  push r9
  push r8
  push rdi
  push rsi
  push rbp
  push rbp ; should be rsp
  push rbx
  push rdx
  push rcx
  push rax
  mov  rcx, rsp ; parameter
  sub  rsp, 20h
  call StmHandlerSmm
  add  rsp, 20h
  jmp $
AsmHostEntrypointSmm ENDP

  END
