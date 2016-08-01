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

#define SLEEP_EN  0x2000
#define SLEEP_MSK (0x1C00 | SLEEP_EN)
#define SLEEP_S1  (0x0400 | SLEEP_EN)
#define SLEEP_S3  (0x1400 | SLEEP_EN)
#define SLEEP_S4  (0x1800 | SLEEP_EN)
#define SLEEP_S5  (0x1C00 | SLEEP_EN)

/**

  This function is IO handler for ACPI.

  @param Index CPU index
  @param Port  ACPI IO port address
  @param Data  ACPI IO port data

**/
VOID
IoAcpiHandler (
  IN UINT32  Index,
  IN UINT16  Port,
  IN UINT32  Data
  )
{
  DEBUG ((EFI_D_INFO, "(FRM) !!!IoAcpiHandler %d - %04x<-%08x\n", (UINTN)Index, (UINTN)Port, (UINTN)Data));

  switch (Data & SLEEP_MSK) {
  case SLEEP_S1:
    // S1: Passthrough
    break;
  case SLEEP_S3:
    // S3
    FrmTeardownBsp (Index, 3);
    // Pass throuth
    break;
  case SLEEP_S4:
  case SLEEP_S5:
    // S4/S5
    FrmTeardownBsp (Index, 5);
    break;
  default:
    // S0: Passthrough
    break;
  }

  AsmWbinvd ();

  return ;
}

/**

  This function is IO write handler for ACPI.

  @param Context Context for IO write handler
  @param Port    IO port
  @param Value   IO port value
  @param Action  IO write action

**/
VOID
IoAcpiWriteHandler (
  IN VOID      *Context,
  IN UINT16    Port,
  IN UINT32    Value,
  OUT UINT32   *Action
  )
{
  UINT32  Index;

  Index = ApicToIndex (ReadLocalApicId ());

  DEBUG ((EFI_D_INFO, "(FRM) !!!IoAcpiHandler %d - %04x<-%08x\n", (UINTN)Index, (UINTN)Port, (UINTN)Value));

  switch (Value & SLEEP_MSK) {
  case SLEEP_S1:
    // S1: Passthrough
    break;
  case SLEEP_S3:
    // S3
    FrmTeardownBsp (Index, 3);
    // Pass throuth
    break;
  case SLEEP_S4:
  case SLEEP_S5:
    // S4/S5
    FrmTeardownBsp (Index, 5);
    break;
  default:
    // S0: Passthrough
#if 0
    {
      VM_EXEC_PIN_BASES_VMEXIT_CONTROLS            PinBasedCtls;
      VM_EXIT_CONTROLS                             VmExitCtrls;

      // enable VmxTimer
      PinBasedCtls.Uint32 = AsmVmRead32 (VMCS_32_CONTROL_PIN_BASED_VM_EXECUTION_INDEX);
      PinBasedCtls.Bits.VmxPreemptionTimer = 1; // Timer
      AsmVmWrite32 (VMCS_32_CONTROL_PIN_BASED_VM_EXECUTION_INDEX,           PinBasedCtls.Uint32);
      VmExitCtrls.Uint32 = AsmVmRead32 (VMCS_32_CONTROL_VMEXIT_CONTROLS_INDEX);
      if (PinBasedCtls.Bits.VmxPreemptionTimer) {
        VmExitCtrls.Bits.SaveVmxPreemptionTimerValue = 1;
      }
      AsmVmWrite32 (VMCS_32_CONTROL_VMEXIT_CONTROLS_INDEX,                  VmExitCtrls.Uint32);
    }
#endif
    break;
  }

  AsmWbinvd ();
  *Action = IO_ACTION_PASSTHROUGH;

  return ;
}

/**

  This function initialize IO handler for ACPI.

**/
VOID
InitializeIoAcpiHandlers (
  VOID
  )
{
  RegisterIoWrite (
    mHostContextCommon.AcpiPmControlIoPortBaseAddress, // Port
    sizeof(UINT32),                                  // Size = 32-bit
    IoAcpiWriteHandler,                              // WriteHandler
    NULL                                             // WriteContext
    );

  return ;
}
