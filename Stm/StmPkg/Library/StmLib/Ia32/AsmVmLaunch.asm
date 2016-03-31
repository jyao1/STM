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
;    AsmVmLaunch.asm
;
;------------------------------------------------------------------------------

  .686P
  .MMX
  .MODEL FLAT,C
  .CODE

AsmVmLaunch PROC PUBLIC
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
  DB  0fh, 01h, 0c2h           ; VMLAUNCH
  pushfd
  pop     eax
  pop     esi
  pop     edi
  pop     ebp
  pop     ebx
  ret
AsmVmLaunch ENDP

  END

