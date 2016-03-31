/** @file

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Base.h>
#include <Library\IoLib.h>
#include "FrmHandler.h"

typedef struct {
  UINT16             Port;
  UINTN              Size;
  IO_READ_HANDLER    Handler;
  VOID               *Context;
} IO_READ_HANDLER_ITEM;

typedef struct {
  UINT16             Port;
  UINTN              Size;
  IO_WRITE_HANDLER   Handler;
  VOID               *Context;
} IO_WRITE_HANDLER_ITEM;

#define MAX_IO_READ_HANDLER_NUM   0x100
#define MAX_IO_WRITE_HANDLER_NUM  0x100

UINTN                   mIoReadHandlerNum = 0;
UINTN                   mIoWriteHandlerNum = 0;
IO_READ_HANDLER_ITEM    mIoReadHandlers[MAX_IO_READ_HANDLER_NUM];
IO_WRITE_HANDLER_ITEM   mIoWriteHandlers[MAX_IO_WRITE_HANDLER_NUM];

/**

  This function set IO bitmap.

  @param Port   IO port

**/
VOID
SetIoBitmap (
  IN UINT16  Port
  );

/**

  This function register IO read handler.

  @param Port    IO port
  @param Size    IO port size
  @param Handler IO read handler
  @param Context Context for IO read handler

  @retval RETURN_SUCCESS           IO read handler is registered. 
  @retval RETURN_BUFFER_TOO_SMALL  IO read handler is not registered.

**/
RETURN_STATUS
RegisterIoRead (
  IN UINT16             Port,
  IN UINTN              Size,
  IN IO_READ_HANDLER    Handler,
  IN VOID               *Context
  )
{
  DEBUG ((EFI_D_INFO, "(FRM) RegisterIoRead - %x\n", (UINTN)Port));
  if (mIoReadHandlerNum == MAX_IO_READ_HANDLER_NUM) {
    CpuDeadLoop ();
    return RETURN_BUFFER_TOO_SMALL;
  }
  mIoReadHandlers[mIoReadHandlerNum].Port    = Port;
  mIoReadHandlers[mIoReadHandlerNum].Size    = Size;
  mIoReadHandlers[mIoReadHandlerNum].Handler = Handler;
  mIoReadHandlers[mIoReadHandlerNum].Context = Context;
  mIoReadHandlerNum++;

  SetIoBitmap (Port);

  return RETURN_SUCCESS;
}

/**

  This function return IO read handler according to port number.

  @param Port  IO Port
  @param Size  IO Port size

  @return IO read handler

**/
IO_READ_HANDLER_ITEM *
FindIoReadHandler (
  IN UINT16             Port,
  IN UINTN              Size
  )
{
  UINTN Index;

  for (Index = 0; Index < mIoReadHandlerNum; Index++) {
    if ((mIoReadHandlers[Index].Port == Port) && (mIoReadHandlers[Index].Size == Size)) {
      return &mIoReadHandlers[Index];
    }
  }

  return NULL;
}

/**

  This function register IO write handler.

  @param Port    IO port
  @param Size    IO port size
  @param Handler IO write handler
  @param Context Context for IO write handler

  @retval RETURN_SUCCESS           IO write handler is registered.
  @retval RETURN_BUFFER_TOO_SMALL  IO write handler is not registered.

**/
RETURN_STATUS
RegisterIoWrite (
  IN UINT16             Port,
  IN UINTN              Size,
  IN IO_WRITE_HANDLER   Handler,
  IN VOID               *Context
  )
{
  DEBUG ((EFI_D_INFO, "(FRM) RegisterIoWrite - %x\n", (UINTN)Port));
  if (mIoWriteHandlerNum == MAX_IO_WRITE_HANDLER_NUM) {
    CpuDeadLoop ();
    return RETURN_BUFFER_TOO_SMALL;
  }
  mIoWriteHandlers[mIoWriteHandlerNum].Port    = Port;
  mIoWriteHandlers[mIoWriteHandlerNum].Size    = Size;
  mIoWriteHandlers[mIoWriteHandlerNum].Handler = Handler;
  mIoWriteHandlers[mIoWriteHandlerNum].Context = Context;
  mIoWriteHandlerNum++;

  SetIoBitmap (Port);

  return RETURN_SUCCESS;
}

/**

  This function return IO write handler according to port number.

  @param Port  IO Port
  @param Size  IO Port size

  @return IO write handler

**/
IO_WRITE_HANDLER_ITEM *
FindIoWriteHandler (
  IN UINT16             Port,
  IN UINTN              Size
  )
{
  UINTN Index;

  for (Index = 0; Index < mIoWriteHandlerNum; Index++) {
    if ((mIoWriteHandlers[Index].Port == Port) && (mIoWriteHandlers[Index].Size == Size)) {
      return &mIoWriteHandlers[Index];
    }
  }

  return NULL;
}

/**

  This function is IO instruction handler.

  @param Index CPU index

**/
VOID
IoHandler (
  IN UINT32 Index
  )
{
  VM_EXIT_QUALIFICATION   Qualification;
  UINT16                  Port;
  UINTN                   *DataPtr;
  IO_WRITE_HANDLER_ITEM   *IoWriteHandler;
  IO_READ_HANDLER_ITEM    *IoReadHandler;
  UINT32                  Action;
  UINT32                  Value;
  UINTN                   LinearAddr;

  Qualification.UintN = VmReadN (VMCS_N_RO_EXIT_QUALIFICATION_INDEX);

  Port = (UINT16)Qualification.IoInstruction.PortNum;
  DataPtr = (UINTN *)&mGuestContextCommon.GuestContextPerCpu[Index].Register.Rax;

  if (Qualification.IoInstruction.Rep) {
    UINT64 RcxMask;

    RcxMask = 0xFFFFFFFFFFFFFFFFull;
    if ((mGuestContextCommon.GuestContextPerCpu[Index].EFER & IA32_EFER_MSR_MLA) == 0) {
      RcxMask = 0xFFFFFFFFull;
    }
    if ((mGuestContextCommon.GuestContextPerCpu[Index].Register.Rcx & RcxMask) == 0) {
      // Skip
      VmWriteN (VMCS_N_GUEST_RIP_INDEX, VmReadN(VMCS_N_GUEST_RIP_INDEX) + VmRead32(VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX));
      return ;
    }
  }

  if (Port == 0xB2) {
    DEBUG ((EFI_D_INFO, "(FRM) !!! SmiCmd(0xb2) - 0x%02x !!!\n", (UINT8)*DataPtr));
  }

  if ((mHostContextCommon.ResetIoPortBaseAddress != 0) && (Port == mHostContextCommon.ResetIoPortBaseAddress) && (Qualification.IoInstruction.Direction == 0)) {
    IoResetHandler (Index, Port, (UINT8)*DataPtr);
  } else if ((Port == mHostContextCommon.AcpiPmControlIoPortBaseAddress) && (Qualification.IoInstruction.Direction == 0)) {
    IoAcpiHandler (Index, Port, (UINT32)*DataPtr);
  }

  if (Qualification.IoInstruction.String) {
    LinearAddr = VmReadN (VMCS_N_RO_GUEST_LINEAR_ADDR_INDEX);
    if (VmReadN (VMCS_N_GUEST_CR0_INDEX) & CR0_PG) {
      DataPtr = (UINTN *)(UINTN)GuestVirtualToGuestPhysical (Index, LinearAddr);
    } else {
      DataPtr = (UINTN *)LinearAddr;
    }

    if (VmReadN (VMCS_N_GUEST_RFLAGS_INDEX) & RFLAGS_DF) {
      if (Qualification.IoInstruction.Direction) {
        mGuestContextCommon.GuestContextPerCpu[Index].Register.Rdi -= Qualification.IoInstruction.Size + 1;
      } else {
        mGuestContextCommon.GuestContextPerCpu[Index].Register.Rsi -= Qualification.IoInstruction.Size + 1;
      }
    } else {
      if (Qualification.IoInstruction.Direction) {
        mGuestContextCommon.GuestContextPerCpu[Index].Register.Rdi += Qualification.IoInstruction.Size + 1;
      } else {
        mGuestContextCommon.GuestContextPerCpu[Index].Register.Rsi += Qualification.IoInstruction.Size + 1;
      }
    }
  }

  if (Qualification.IoInstruction.Direction) { // IN
    IoReadHandler = FindIoReadHandler (Port, Qualification.IoInstruction.Size + 1);
    if (IoReadHandler != NULL) {
      Action = IO_ACTION_NO_ACTION;
      Value = 0;
      IoReadHandler->Handler (IoReadHandler->Context, Port, &Value, &Action);
      if (Action == IO_ACTION_NO_ACTION) {
        switch (Qualification.IoInstruction.Size) {
        case 0:
          *(UINT8 *)DataPtr = (UINT8)Value;
          goto Ret;
          break;
        case 1:
          *(UINT16 *)DataPtr = (UINT16)Value;
          goto Ret;
          break;
        case 3:
          *(UINT32 *)DataPtr = Value;
          goto Ret;
          break;
        default:
          break;
        }
        goto Ret;
      } else {
        // Passthrough
      }
    }
    switch (Qualification.IoInstruction.Size) {
    case 0:
      *(UINT8 *)DataPtr = IoRead8 (Port);
      goto Ret;
      break;
    case 1:
      *(UINT16 *)DataPtr = IoRead16 (Port);
      goto Ret;
      break;
    case 3:
      *(UINT32 *)DataPtr = IoRead32 (Port);
      goto Ret;
      break;
    default:
      break;
    }
  } else { // OUT
    IoWriteHandler = FindIoWriteHandler (Port, Qualification.IoInstruction.Size + 1);
    if (IoWriteHandler != NULL) {
      Action = IO_ACTION_NO_ACTION;
      Value = (UINT32)*DataPtr;
      IoWriteHandler->Handler (IoWriteHandler->Context, Port, Value, &Action);
      if (Action == IO_ACTION_NO_ACTION) {
        goto Ret;
      } else {
        // Passthrough
      }
    }
    switch (Qualification.IoInstruction.Size) {
    case 0:
      IoWrite8 (Port, (UINT8)*DataPtr);
      goto Ret;
      break;
    case 1:
      IoWrite16 (Port, (UINT16)*DataPtr);
      goto Ret;
      break;
    case 3:
      IoWrite32 (Port, (UINT32)*DataPtr);
      goto Ret;
      break;
    default:
      break;
    }
  }

  DEBUG ((EFI_D_ERROR, "(FRM) !!!IoHandler!!!\n"));
  DumpVmcsAllField ();

  CpuDeadLoop ();

Ret:
  if (Qualification.IoInstruction.Rep) {
    // replay
    mGuestContextCommon.GuestContextPerCpu[Index].Register.Rcx --;
    return ;
  }

  VmWriteN (VMCS_N_GUEST_RIP_INDEX, VmReadN(VMCS_N_GUEST_RIP_INDEX) + VmRead32(VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX));
  return ;
}
