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

/**

  This function is NMI handler.

  @param Index CPU index

**/
VOID
ExceptionNMIHandler (
  IN UINT32 Index
  )
{
  VM_EXIT_INFO_INTERRUPTION                    InterruptionInformation;

  InterruptionInformation.Uint32 = VmRead32 (VMCS_32_RO_VMEXIT_INTERRUPTION_INFO_INDEX);

  AcquireSpinLock (&mHostContextCommon.DebugLock);
  DEBUG ((EFI_D_INFO, "(FRM) !!!ExceptionNmiHandler %d!!!\n", Index));
  DEBUG ((EFI_D_INFO, "(FRM) InterruptType  %d, Vector %x\n", InterruptionInformation.Bits.InterruptType, InterruptionInformation.Bits.Vector));

  DumpVmcsAllField ();
  ReleaseSpinLock (&mHostContextCommon.DebugLock);
  CpuDeadLoop();
}

/**

  This function is interrupt windows handler.

  @param Index CPU index

**/
VOID
InterruptWindowHandler (
  UINT32 Index
  )
{
  VM_ENTRY_CONTROL_INTERRUPT                   InterruptControl;
  VM_EXEC_PROCESSOR_BASES_VMEXIT_CONTROLS      ProcessorBasedCtrls;
  VM_EXEC_PIN_BASES_VMEXIT_CONTROLS            PinBasedCtls;

  DEBUG ((EFI_D_INFO, "W"));

  InterruptControl.Uint32 = mGuestContextCommon.GuestContextPerCpu[Index].LastPendingInterrupt[mGuestContextCommon.GuestContextPerCpu[Index].LastPendingInterruptTop];

  if (InterruptControl.Bits.Valid == 0) {
    AcquireSpinLock (&mHostContextCommon.DebugLock);
    DEBUG ((EFI_D_ERROR, "(FRM) !!!InterruptWindowHandler Invalid Information - %d!!!\n", Index));
    DumpVmcsAllField ();
    ReleaseSpinLock (&mHostContextCommon.DebugLock);
    CpuDeadLoop ();
  }

  VmWrite32 (VMCS_32_CONTROL_VMENTRY_INTERRUPTION_INFO_INDEX, InterruptControl.Uint32);
  mGuestContextCommon.GuestContextPerCpu[Index].LastPendingInterruptTop--;

  //
  // Disable InterruptWindows, if LastPendingInterrupt
  //
  ProcessorBasedCtrls.Uint32 = VmRead32 (VMCS_32_CONTROL_PROCESSOR_BASED_VM_EXECUTION_INDEX);
  if (mGuestContextCommon.GuestContextPerCpu[Index].LastPendingInterruptTop == 0) {
    ProcessorBasedCtrls.Bits.InterruptWindow = 0;
  } else {
    ProcessorBasedCtrls.Bits.InterruptWindow = 1;
  }
  VmWrite32 (VMCS_32_CONTROL_PROCESSOR_BASED_VM_EXECUTION_INDEX,     ProcessorBasedCtrls.Uint32);
  PinBasedCtls.Uint32 = VmRead32 (VMCS_32_CONTROL_PIN_BASED_VM_EXECUTION_INDEX);
  PinBasedCtls.Bits.ExternalInterrupt = 1;
  VmWrite32 (VMCS_32_CONTROL_PIN_BASED_VM_EXECUTION_INDEX,           PinBasedCtls.Uint32);

  return ;
}

/**

  This function is external interrupt handler.

  @param Index CPU index

**/
VOID
ExternalInterruptHandler (
  UINT32 Index
  )
{
  VM_EXIT_INFO_INTERRUPTION                    InterruptionInformation;
//  VM_EXIT_INFO_IDT_VECTORING                   IdtVectoring;
  VM_ENTRY_CONTROL_INTERRUPT                   InterruptControl;
  VM_EXEC_PROCESSOR_BASES_VMEXIT_CONTROLS      ProcessorBasedCtrls;
  VM_EXEC_PIN_BASES_VMEXIT_CONTROLS            PinBasedCtls;

  DEBUG ((EFI_D_INFO, "I"));

  InterruptControl.Uint32 = 0;
#if 1
  InterruptionInformation.Uint32 = VmRead32 (VMCS_32_RO_VMEXIT_INTERRUPTION_INFO_INDEX);

  if (InterruptionInformation.Bits.Valid == 0) {
    AcquireSpinLock (&mHostContextCommon.DebugLock);
    DEBUG ((EFI_D_ERROR, "(FRM) !!!ExternalInterruptHandler Invaid Information - %d!!!\n", Index));
    DumpVmcsAllField ();
    ReleaseSpinLock (&mHostContextCommon.DebugLock);
    CpuDeadLoop ();
  }

  InterruptControl.Bits.Vector = InterruptionInformation.Bits.Vector;
  InterruptControl.Bits.InterruptType = InterruptionInformation.Bits.InterruptType;
  InterruptControl.Bits.DeliverErrorCode = InterruptionInformation.Bits.ErrorCodeValid;
  InterruptControl.Bits.Valid = InterruptionInformation.Bits.Valid;

  if (InterruptControl.Bits.DeliverErrorCode) {
    VmWrite32 (VMCS_32_CONTROL_VMENTRY_EXCEPTION_ERROR_CODE_INDEX, VmRead32 (VMCS_32_RO_VMEXIT_INTERRUPTION_ERROR_CODE_INDEX));
  }
#else
  IdtVectoring.Uint32 = VmRead32 (VMCS_32_RO_IDT_VECTORING_INFO_INDEX);

  InterruptControl.Bits.Vector = IdtVectoring.Bits.Vector;
  InterruptControl.Bits.InterruptType = IdtVectoring.Bits.InterruptType;
  InterruptControl.Bits.DeliverErrorCode = IdtVectoring.Bits.ErrorCodeValid;
  InterruptControl.Bits.Valid = IdtVectoring.Bits.Valid;

  if (InterruptControl.Bits.DeliverErrorCode) {
    VmWrite32 (VMCS_32_CONTROL_VMENTRY_EXCEPTION_ERROR_CODE_INDEX, VmRead32 (VMCS_32_RO_IDT_VECTORING_ERROR_CODE_INDEX));
  }
#endif

  if ((InterruptControl.Bits.InterruptType == INTERRUPT_TYPE_EXTERNAL_SOFTWARE_INTERRUPT) ||
      (InterruptControl.Bits.InterruptType == INTERRUPT_TYPE_EXTERNAL_PRIVILEDGED_SOFTWARE_EXCEPTION) ||
      (InterruptControl.Bits.InterruptType == INTERRUPT_TYPE_EXTERNAL_SOFTWARE_EXCEPTIONT)) {
    VmWrite32 (VMCS_32_CONTROL_VMENTRY_INSTRUCTION_LENGTH_INDEX, VmRead32 (VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX));
  } else {
    VmWrite32 (VMCS_32_CONTROL_VMENTRY_INSTRUCTION_LENGTH_INDEX, 0);
  }

  if (((VmReadN (VMCS_N_GUEST_RFLAGS_INDEX) & RFLAGS_IF) == 0) &&
      (InterruptControl.Bits.InterruptType == INTERRUPT_TYPE_EXTERNAL_EXTERNAL_INTERRUPT)) {
    //
    // Enable InterruptWindows, and pending ...
    //
//    DEBUG ((EFI_D_INFO, "P"));
    if (mGuestContextCommon.GuestContextPerCpu[Index].LastPendingInterruptTop != 0) {
      //
      // NOTE: The reason for code running here is:
      //   When ISR is set, the lower or equal IRQ will be masked, but higher IRQ will arrive to set IRR no matter EOI issued or not.
      //   When EOI is issued, only highest ISR will be cleared.
      //
      // The PIC/8259 IRQ priority is: 0 1 (8 9 10 11 12 13 14 15) 3 4 5 6 7
      // The IO-APIC IRQ priority is:
      //
//      AcquireSpinLock (&mHostContextCommon.DebugLock);
//      DEBUG ((EFI_D_INFO, "(FRM) !!!ExternalInterruptHandler Already Pending - %d!!!\n", Index));
//      DEBUG ((EFI_D_INFO, "(FRM) InterruptControl - %x\n", InterruptControl.Uint32));
//      DEBUG ((EFI_D_INFO, "(FRM) LastPendingInterrupt - %x\n", mGuestContextCommon.GuestContextPerCpu[Index].LastPendingInterrupt[mGuestContextCommon.GuestContextPerCpu[Index].LastPendingInterruptTop]));
//      DumpVmcsAllField ();
//      ReleaseSpinLock (&mHostContextCommon.DebugLock);
      DEBUG ((EFI_D_INFO, "D"));
    }
    mGuestContextCommon.GuestContextPerCpu[Index].LastPendingInterruptTop++;
    mGuestContextCommon.GuestContextPerCpu[Index].LastPendingInterrupt[mGuestContextCommon.GuestContextPerCpu[Index].LastPendingInterruptTop] = InterruptControl.Uint32;
    ProcessorBasedCtrls.Uint32 = VmRead32 (VMCS_32_CONTROL_PROCESSOR_BASED_VM_EXECUTION_INDEX);
    ProcessorBasedCtrls.Bits.InterruptWindow = 1;
    VmWrite32 (VMCS_32_CONTROL_PROCESSOR_BASED_VM_EXECUTION_INDEX,     ProcessorBasedCtrls.Uint32);
    PinBasedCtls.Uint32 = VmRead32 (VMCS_32_CONTROL_PIN_BASED_VM_EXECUTION_INDEX);
    PinBasedCtls.Bits.ExternalInterrupt = 0;
    VmWrite32 (VMCS_32_CONTROL_PIN_BASED_VM_EXECUTION_INDEX,           PinBasedCtls.Uint32);
  } else {
//    DEBUG ((EFI_D_INFO, "J"));
    //
    // Inject interrupt to guest
    //
    VmWrite32 (VMCS_32_CONTROL_VMENTRY_INTERRUPTION_INFO_INDEX, InterruptControl.Uint32);
  }

  return ;
}
