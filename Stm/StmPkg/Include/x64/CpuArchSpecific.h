/** @file
  CPU arch specific definition

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _CPU_ARCH_SPECIFIC_H_
#define _CPU_ARCH_SPECIFIC_H_

#pragma pack (push, 1)
typedef struct _X86_REGISTER {
    UINTN Rax;       //  0/0
    UINTN Rcx;       //  4/8
    UINTN Rdx;       //  8/16
    UINTN Rbx;       //  12/24
    UINTN Rsp;       //  16/32
    UINTN Rbp;       //  20/40
    UINTN Rsi;       //  24/48
    UINTN Rdi;       //  28/56
    UINTN R8;        // 64
    UINTN R9;        // 72
    UINTN R10;       // 80
    UINTN R11;       // 88
    UINTN R12;       // 96
    UINTN R13;       // 104
    UINTN R14;       // 112
    UINTN R15;       // 120
    IA32_FX_BUFFER   FxBuffer;  // 128, X64 calling convention allow using XMM0~15
} X86_REGISTER;
#pragma pack (pop)

#endif
