/** @file
  STM exception handler

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmRuntime.h"

//
// Error code flag indicating whether or not an error code will be 
// pushed on the stack if an exception occurs.
//
// 1 means an error code will be pushed, otherwise 0
//
// bit 0 - exception 0
// bit 1 - exception 1
// etc.
//
UINT32 mErrorCodeFlag = 0x00027d00;

EFI_EXCEPTION_CALLBACK          mExternalVectorTable[STM_MAX_IDT_NUM];
extern EFI_EXCEPTION_CALLBACK   *mExternalVectorTablePtr;
extern UINT32                   mExceptionHandlerLength;

/**

  This function is STM host exception handler.

**/
VOID
AsmExceptionHandlers (
  VOID
  );

/**

  This function dump X64 exception context.

**/
VOID
DumpExceptionX64 (
  IN EFI_EXCEPTION_TYPE   InterruptType, 
  IN EFI_SYSTEM_CONTEXT   SystemContext
  )
{
  //
  // Output context of all registers when exception occurs.
  //
  DEBUG ((
    EFI_D_ERROR,
    "!!!! X64 Exception Type - %016lx !!!!\n",
    InterruptType
    ));
  DEBUG ((
    EFI_D_ERROR,
    "RIP - %016lx, CS - %016lx, RFLAGS - %016lx\n",
    SystemContext.SystemContextX64->Rip,
    SystemContext.SystemContextX64->Cs,
    SystemContext.SystemContextX64->Rflags
    ));
  if ((mErrorCodeFlag & (1 << InterruptType)) != 0) {
    DEBUG ((
      EFI_D_ERROR,
      "ExceptionData - %016lx\n",
      SystemContext.SystemContextX64->ExceptionData
      ));
  }
  DEBUG ((
    EFI_D_ERROR,
    "RAX - %016lx, RCX - %016lx, RDX - %016lx\n",
    SystemContext.SystemContextX64->Rax,
    SystemContext.SystemContextX64->Rcx,
    SystemContext.SystemContextX64->Rdx
    ));
  DEBUG ((
    EFI_D_ERROR,
    "RBX - %016lx, RSP - %016lx, RBP - %016lx\n",
    SystemContext.SystemContextX64->Rbx,
    SystemContext.SystemContextX64->Rsp,
    SystemContext.SystemContextX64->Rbp
    ));
  DEBUG ((
    EFI_D_ERROR,
    "RSI - %016lx, RDI - %016lx\n",
    SystemContext.SystemContextX64->Rsi,
    SystemContext.SystemContextX64->Rdi
    ));
  DEBUG ((
    EFI_D_ERROR,
    "R8 - %016lx, R9 - %016lx, R10 - %016lx\n",
    SystemContext.SystemContextX64->R8,
    SystemContext.SystemContextX64->R9,
    SystemContext.SystemContextX64->R10
    ));
  DEBUG ((
    EFI_D_ERROR,
    "R11 - %016lx, R12 - %016lx, R13 - %016lx\n",
    SystemContext.SystemContextX64->R11,
    SystemContext.SystemContextX64->R12,
    SystemContext.SystemContextX64->R13
    ));
  DEBUG ((
    EFI_D_ERROR,
    "R14 - %016lx, R15 - %016lx\n",
    SystemContext.SystemContextX64->R14,
    SystemContext.SystemContextX64->R15
    ));
  DEBUG ((
    EFI_D_ERROR,
    "DS - %016lx, ES - %016lx, FS - %016lx\n",
    SystemContext.SystemContextX64->Ds,
    SystemContext.SystemContextX64->Es,
    SystemContext.SystemContextX64->Fs
    ));
  DEBUG ((
    EFI_D_ERROR,
    "GS - %016lx, SS - %016lx\n",
    SystemContext.SystemContextX64->Gs,
    SystemContext.SystemContextX64->Ss
    ));
  DEBUG ((
    EFI_D_ERROR,
    "GDTR - %016lx %016lx, LDTR - %016lx\n",
    SystemContext.SystemContextX64->Gdtr[0],
    SystemContext.SystemContextX64->Gdtr[1],
    SystemContext.SystemContextX64->Ldtr
    ));
  DEBUG ((
    EFI_D_ERROR,
    "IDTR - %016lx %016lx, TR - %016lx\n",
    SystemContext.SystemContextX64->Idtr[0],
    SystemContext.SystemContextX64->Idtr[1],
    SystemContext.SystemContextX64->Tr
    ));
  DEBUG ((
    EFI_D_ERROR,
    "CR0 - %016lx, CR2 - %016lx, CR3 - %016lx\n",
    SystemContext.SystemContextX64->Cr0,
    SystemContext.SystemContextX64->Cr2,
    SystemContext.SystemContextX64->Cr3
    ));
  DEBUG ((
    EFI_D_ERROR,
    "CR4 - %016lx, CR8 - %016lx\n",
    SystemContext.SystemContextX64->Cr4,
    SystemContext.SystemContextX64->Cr8
    ));
  DEBUG ((
    EFI_D_ERROR,
    "DR0 - %016lx, DR1 - %016lx, DR2 - %016lx\n",
    SystemContext.SystemContextX64->Dr0,
    SystemContext.SystemContextX64->Dr1,
    SystemContext.SystemContextX64->Dr2
    ));
  DEBUG ((
    EFI_D_ERROR,
    "DR3 - %016lx, DR6 - %016lx, DR7 - %016lx\n",
    SystemContext.SystemContextX64->Dr3,
    SystemContext.SystemContextX64->Dr6,
    SystemContext.SystemContextX64->Dr7
    ));
}

/**

  This function dump IA32 exception context.

**/
VOID
DumpExceptionIa32 (
  IN EFI_EXCEPTION_TYPE   InterruptType, 
  IN EFI_SYSTEM_CONTEXT   SystemContext
  )
{
  DEBUG ((
    EFI_D_ERROR,
    "!!!! IA32 Exception Type - %08x !!!!\n",
    InterruptType
    ));
  DEBUG ((
    EFI_D_ERROR,
    "EIP - %08x, CS - %08x, EFLAGS - %08x\n",
    SystemContext.SystemContextIa32->Eip,
    SystemContext.SystemContextIa32->Cs,
    SystemContext.SystemContextIa32->Eflags
    ));
  if ((mErrorCodeFlag & (1 << InterruptType)) != 0) {
    DEBUG ((
      EFI_D_ERROR,
      "ExceptionData - %08x\n",
      SystemContext.SystemContextIa32->ExceptionData
      ));
  }
  DEBUG ((
    EFI_D_ERROR,
    "EAX - %08x, ECX - %08x, EDX - %08x, EBX - %08x\n",
    SystemContext.SystemContextIa32->Eax,
    SystemContext.SystemContextIa32->Ecx,
    SystemContext.SystemContextIa32->Edx,
    SystemContext.SystemContextIa32->Ebx
    ));
  DEBUG ((
    EFI_D_ERROR,
    "ESP - %08x, EBP - %08x, ESI - %08x, EDI - %08x\n",
    SystemContext.SystemContextIa32->Esp,
    SystemContext.SystemContextIa32->Ebp,
    SystemContext.SystemContextIa32->Esi,
    SystemContext.SystemContextIa32->Edi
    ));
  DEBUG ((
    EFI_D_ERROR,
    "DS - %08x, ES - %08x, FS - %08x, GS - %08x, SS - %08x\n",
    SystemContext.SystemContextIa32->Ds,
    SystemContext.SystemContextIa32->Es,
    SystemContext.SystemContextIa32->Fs,
    SystemContext.SystemContextIa32->Gs,
    SystemContext.SystemContextIa32->Ss
    ));
  DEBUG ((
    EFI_D_ERROR,
    "GDTR - %08x %08x, IDTR - %08x %08x\n",
    SystemContext.SystemContextIa32->Gdtr[0],
    SystemContext.SystemContextIa32->Gdtr[1],
    SystemContext.SystemContextIa32->Idtr[0],
    SystemContext.SystemContextIa32->Idtr[1]
    ));
  DEBUG ((
    EFI_D_ERROR,
    "LDTR - %08x, TR - %08x\n",
    SystemContext.SystemContextIa32->Ldtr,
    SystemContext.SystemContextIa32->Tr
    ));
  DEBUG ((
    EFI_D_ERROR,
    "CR0 - %08x, CR2 - %08x, CR3 - %08x, CR4 - %08x\n",
    SystemContext.SystemContextIa32->Cr0,
    SystemContext.SystemContextIa32->Cr2,
    SystemContext.SystemContextIa32->Cr3,
    SystemContext.SystemContextIa32->Cr4
    ));
  DEBUG ((
    EFI_D_ERROR,
    "DR0 - %08x, DR1 - %08x, DR2 - %08x, DR3 - %08x\n",
    SystemContext.SystemContextIa32->Dr0,
    SystemContext.SystemContextIa32->Dr1,
    SystemContext.SystemContextIa32->Dr2,
    SystemContext.SystemContextIa32->Dr3
    ));
  DEBUG ((
    EFI_D_ERROR,
    "DR6 - %08x, DR7 - %08x\n",
    SystemContext.SystemContextIa32->Dr6,
    SystemContext.SystemContextIa32->Dr7
    ));
}

/**

  This function is exception handler for STM.

  This function is called by primitive interrupt handler in assembly code, and
  parameters of InterruptType and SystemContext are prepared by it beforehand.
  This function outputs context of all registers when exception occurs. It then
  reports status code for the exception.

  @param  InterruptType  Exception type.
  @param  SystemContext  System context data.

**/
VOID
EFIAPI
HostExceptionHandler (
  IN EFI_EXCEPTION_TYPE   InterruptType, 
  IN EFI_SYSTEM_CONTEXT   SystemContext
  )
{
  if (sizeof(UINTN) == sizeof(UINT64)) {
    DumpExceptionX64 (InterruptType, SystemContext);
  } else {
    DumpExceptionIa32 (InterruptType, SystemContext);
  }

  CpuDeadLoop ();

  return ;
}

/**

  Registers a function to be called when a given processor exception occurs.

  @param  ExceptionType         Specifies which processor exception to hook.                       
  @param  ExceptionCallback     A pointer to a function of type EXCEPTION_CALLBACK that is called
                                when the processor exception specified by ExceptionType occurs.  

  @retval RETURN_SUCCESS           The function completed successfully.  
  @retval RETURN_UNSUPPORTED       The exception specified by ExceptionType is not supported.

**/
RETURN_STATUS
RegisterExceptionHandler (
  IN EFI_EXCEPTION_TYPE            ExceptionType,
  IN EFI_EXCEPTION_CALLBACK        ExceptionCallback
  )
{
  if ((UINT32)ExceptionType >= STM_MAX_IDT_NUM) {
    return RETURN_UNSUPPORTED;
  }
  mExternalVectorTable[ExceptionType] = ExceptionCallback;
  return RETURN_SUCCESS;
}

/**

  Initialize external vector table pointer.

**/
VOID
InitializeExternalVectorTablePtr (
  IN IA32_IDT_GATE_DESCRIPTOR       *IdtGate
  )
{
  UINT32                         Index;

  mExternalVectorTablePtr = mExternalVectorTable;
  for (Index = 0; Index < STM_MAX_IDT_NUM; Index++) {
    IdtGate[Index].Bits.Selector = AsmReadCs ();
    IdtGate[Index].Bits.GateType = IA32_IDT_GATE_TYPE_INTERRUPT_32;
    IdtGate[Index].Bits.OffsetLow = (UINT16)((UINTN)AsmExceptionHandlers + Index * mExceptionHandlerLength);
    IdtGate[Index].Bits.OffsetHigh = (UINT16)(((UINTN)AsmExceptionHandlers + Index * mExceptionHandlerLength) >> 16);
    if (sizeof(UINTN) == sizeof(UINT64)) {
#if defined (MDE_CPU_X64)
      IdtGate[Index].Bits.OffsetUpper = (UINT32)RShiftU64 ((UINTN)AsmExceptionHandlers + Index * mExceptionHandlerLength, 32);
#endif
    }
    RegisterExceptionHandler (Index, HostExceptionHandler);
  }

}