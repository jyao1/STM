#line 1 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\Library\\StmLib\\Ia32\\AsmVmResume.asm"
#line 1 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\Build\\StmPkg\\DEBUG_VS2010x86\\IA32\\StmPkg\\Library\\StmLib\\StmLib\\DEBUG\\AutoGen.h"















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


#line 17 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\Build\\StmPkg\\DEBUG_VS2010x86\\IA32\\StmPkg\\Library\\StmLib\\StmLib\\DEBUG\\AutoGen.h"

extern GUID  gEfiCallerIdGuid;
extern CHAR8 *gEfiCallerBaseName;







#line 28 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\Build\\StmPkg\\DEBUG_VS2010x86\\IA32\\StmPkg\\Library\\StmLib\\StmLib\\DEBUG\\AutoGen.h"
#line 1 "z:\\stm_reference\\stmpegit\\stm-pe\\stm\\StmPkg\\Library\\StmLib\\Ia32\\AsmVmResume.asm"
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
;    AsmVmResume.asm
;
;------------------------------------------------------------------------------

  .686P
  .MMX
  .MODEL FLAT,C
  .CODE

AsmVmResume PROC PUBLIC
  push    ebx
  push    ebp
  push    edi
  push    esi
  mov     esi, [esp + 4 + 16]
  mov     eax, [esi + 0]
  mov     ecx, [esi + 4]
  mov     edx, [esi + 8]
  mov     ebx, [esi + 12]
  mov     ebp, [esi + 20]
  mov     edi, [esi + 28]
  mov     esi, [esi + 24]
  DB  0fh, 01h, 0c3h           ; VMRESUME
  pushfd
  pop     eax
  pop     esi
  pop     edi
  pop     ebp
  pop     ebx
  ret
AsmVmResume ENDP

  END

