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
#include "FrmHandler.h"

//#define HOOK_KBC_RESET

#define KBC_RESET_IO_PORT      0x64
#define KBC_RESET_WRITE_VALUE  0xFE

/**

  This function is IO handler for reset.

  @param Index CPU index
  @param Port  Reset IO port address
  @param Data  Reset IO port data

**/
VOID
IoResetHandler (
  IN UINT32  Index,
  IN UINT16  Port,
  IN UINT8   Data
  )
{
  DEBUG ((EFI_D_INFO, "(FRM) !!!IoResetHandler!!!\n"));

  FrmTeardownBsp (Index, 0);
  AsmWbinvd ();

  //
  // Work-around for CTRL+ALT+DEL, it will pass 0x2.
  //
  Data = Data | 0x6;
  IoWrite8 (Port, Data);

  CpuDeadLoop ();

  return ;
}

/**

  This function is IO write handler for reset.

  @param Context Context for IO write handler
  @param Port    IO port
  @param Value   IO port value
  @param Action  IO write action

**/
VOID
IoResetWriteHandler (
  IN VOID      *Context,
  IN UINT16    Port,
  IN UINT32    Value,
  OUT UINT32   *Action
  )
{
  UINT32  Index;

  Index = ApicToIndex (ReadLocalApicId ());

  DEBUG ((EFI_D_INFO, "(FRM) !!!IoResetWriteHandler!!!\n"));

  FrmTeardownBsp (Index, 0);
  AsmWbinvd ();
  
  //
  // Work-around for CTRL+ALT+DEL, it will pass 0x2.
  //
  Value = Value | 0x6;
  IoWrite8 (Port, (UINT8)Value);

  CpuDeadLoop ();

  *Action = IO_ACTION_PASSTHROUGH;

  return ;
}

/**

  This function is IO write handler for KBC reset.

  @param Context Context for IO write handler
  @param Port    IO port
  @param Value   IO port value
  @param Action  IO write action

**/
VOID
KbcResetWriteHandler (
  IN VOID      *Context,
  IN UINT16    Port,
  IN UINT32    Value,
  OUT UINT32   *Action
  )
{
  UINT32  Index;

  Index = ApicToIndex (ReadLocalApicId ());

  DEBUG ((EFI_D_INFO, "(FRM) !!!KbcResetWriteHandler!!!\n"));

  if ((UINT8)Value != KBC_RESET_WRITE_VALUE) {
    // Normal KB
    *Action = IO_ACTION_PASSTHROUGH;
    return ;
  }

  // Reset
  FrmTeardownBsp (Index, 0);
  AsmWbinvd ();
  *Action = IO_ACTION_PASSTHROUGH;

  return ;
}

/**

  This function initialize IO handler for Reset.

**/
VOID
InitializeIoResetHandlers (
  VOID
  )
{
  if (mHostContextCommon.ResetIoPortBaseAddress != 0) {
    RegisterIoWrite (
      mHostContextCommon.ResetIoPortBaseAddress,       // Port
      sizeof(UINT8),                                   // Size = 8-bit
      IoResetWriteHandler,                             // WriteHandler
      NULL                                             // WriteContext
      );
  }
#ifdef HOOK_KBC_RESET
  RegisterIoWrite (
    KBC_RESET_IO_PORT,                               // Port
    sizeof(UINT8),                                   // Size = 8-bit
    KbcResetWriteHandler,                            // WriteHandler
    NULL                                             // WriteContext
    );
#endif
  return ;
}
