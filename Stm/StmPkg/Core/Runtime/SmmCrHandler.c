/** @file
  SMM CR handler

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmRuntime.h"

/**

  This function sync Ia32PAE page table for EPT.

  @param Index CPU index

**/
VOID
Ia32PAESync (
  IN UINT32  Index
  );

/**

  This function is CR access handler for SMM.

  @param Index CPU index

**/
VOID
SmmCrHandler (
  IN UINT32 Index
  )
{
  VM_EXIT_QUALIFICATION   Qualification;
  UINTN                   *GptRegPtr;
  VM_ENTRY_CONTROLS       VmEntryControls;
  STM_REGISTER_VIOLATION_DESC     RegisterViolation;

  Qualification.UintN = VmReadN (VMCS_N_RO_EXIT_QUALIFICATION_INDEX);
  GptRegPtr = (UINTN *)&mGuestContextCommonSmm.GuestContextPerCpu[Index].Register;

  switch (Qualification.CrAccess.CrNum) {
  case 3: // Cr3
    if (Qualification.CrAccess.AccessType == 0) { // MOV to CR

      if ((!mGuestContextCommonSmm.GuestContextPerCpu[Index].UnrestrictedGuest) &&
          ((mGuestContextCommonSmm.GuestContextPerCpu[Index].Cr0 & CR0_PG) == 0)) {
        //
        // Need cache current Setting
        //
      } else {
        VmWriteN (VMCS_N_GUEST_CR3_INDEX, GptRegPtr[Qualification.CrAccess.GpReg]);
      }

      // special for EPT
      Ia32PAESync (Index);

      //
      // Save current data as old data
      //
      mGuestContextCommonSmm.GuestContextPerCpu[Index].Cr3 = GptRegPtr[Qualification.CrAccess.GpReg];
      goto Ret;
    } else if (Qualification.CrAccess.AccessType == 1) { // MOV from CR
      GptRegPtr[Qualification.CrAccess.GpReg] = mGuestContextCommonSmm.GuestContextPerCpu[Index].Cr3;
      goto Ret;
    }
    break;
  case 0: // Cr0
    if (Qualification.CrAccess.AccessType == 0) { // MOV to CR

      if ((!mGuestContextCommonSmm.GuestContextPerCpu[Index].UnrestrictedGuest) &&
          ((GptRegPtr[Qualification.CrAccess.GpReg] & CR0_PE) == 0)) {
        //
        // Disabling PE is not support when UnrestrictedGuest is OFF.
        // We allow other case because it may support run INT10Thunk in SMM mode.
        //
        // However, this can be supported if we launch VM86 in STM in the future.
        // Moreover, SMM guest can use VM86 mode to run INT10Thunk, so disabling PE is not needed.
        //
        DEBUG ((EFI_D_ERROR, "CR violation!\n"));
        ZeroMem (&RegisterViolation, sizeof(RegisterViolation));
        RegisterViolation.Hdr.RscType = REGISTER_VIOLATION;
        RegisterViolation.Hdr.Length = sizeof(RegisterViolation);
        RegisterViolation.RegisterType = StmRegisterCr0;
        AddEventLogForResource (EvtHandledProtectionException, (STM_RSC *)&RegisterViolation);
        SmmExceptionHandler (Index);
        CpuDeadLoop ();
      }


      VmWriteN (VMCS_N_GUEST_CR0_INDEX, GptRegPtr[Qualification.CrAccess.GpReg] | (UINTN)(AsmReadMsr64 (IA32_VMX_CR0_FIXED0_MSR_INDEX) & AsmReadMsr64 (IA32_VMX_CR0_FIXED1_MSR_INDEX) & ~CR0_PG & ~CR0_PE));

      //
      // Check IA32e mode switch
      //
      VmEntryControls.Uint32 = VmRead32 (VMCS_32_CONTROL_VMENTRY_CONTROLS_INDEX);
      if (((mGuestContextCommonSmm.GuestContextPerCpu[Index].Efer & IA32_EFER_MSR_MLE) != 0)&& 
          ((GptRegPtr[Qualification.CrAccess.GpReg] & CR0_PG) != 0)) {
        mGuestContextCommonSmm.GuestContextPerCpu[Index].Efer |= IA32_EFER_MSR_MLA;
        VmEntryControls.Bits.Ia32eGuest = 1;
      } else {
        mGuestContextCommonSmm.GuestContextPerCpu[Index].Efer &= ~IA32_EFER_MSR_MLA;
        VmEntryControls.Bits.Ia32eGuest = 0;
      }
      VmWrite32 (VMCS_32_CONTROL_VMENTRY_CONTROLS_INDEX, VmEntryControls.Uint32);
      VmWrite64 (VMCS_64_GUEST_IA32_EFER_INDEX,          mGuestContextCommonSmm.GuestContextPerCpu[Index].Efer);

      // check CD
      if (GptRegPtr[Qualification.CrAccess.GpReg] & CR0_CD) {
//        DEBUG ((EFI_D_INFO, "!!!CrHandler - Cr0: CD!!!\n"));
        AsmWbinvd ();
      }

      // update shadow
      VmWriteN (VMCS_N_CONTROL_CR0_READ_SHADOW_INDEX, VmReadN (VMCS_N_GUEST_CR0_INDEX));

      //
      // Check UnrestrictedGuest
      //
      if (!mGuestContextCommonSmm.GuestContextPerCpu[Index].UnrestrictedGuest) {
        //
        // Need check PG and PE
        //
        if (((mGuestContextCommonSmm.GuestContextPerCpu[Index].Cr0 & CR0_PG) != 0) &&
            ((GptRegPtr[Qualification.CrAccess.GpReg] & CR0_PG) == 0)) {
          //
          // Disable Paging, but still PE, or disable PE at same time.
          //
          if ((GptRegPtr[Qualification.CrAccess.GpReg] & CR0_PE) != 0) {
//            DEBUG ((EFI_D_INFO, "-PG"));
          } else {
//            DEBUG ((EFI_D_INFO, "-PGE"));
          }
          mGuestContextCommonSmm.GuestContextPerCpu[Index].Cr3 = VmReadN (VMCS_N_GUEST_CR3_INDEX);
          if ((VmReadN (VMCS_N_GUEST_CR4_INDEX) & CR4_PAE) != 0) {
            VmWrite64 (VMCS_64_GUEST_IA32_EFER_INDEX, VmRead64 (VMCS_64_GUEST_IA32_EFER_INDEX) & ~IA32_EFER_MSR_MLE);
            mGuestContextCommonSmm.GuestContextPerCpu[Index].Cr4 = VmReadN (VMCS_N_GUEST_CR4_INDEX) & ~CR4_VMXE & ~CR4_SMXE;
            VmWriteN (VMCS_N_GUEST_CR4_INDEX, VmReadN (VMCS_N_GUEST_CR4_INDEX) & ~CR4_PAE);
            VmWriteN (VMCS_N_GUEST_CR3_INDEX, mGuestContextCommonSmm.CompatiblePageTable);
          } else {
            VmWriteN (VMCS_N_GUEST_CR3_INDEX, mGuestContextCommonSmm.CompatiblePageTable);
          }

          VmWriteN (VMCS_N_GUEST_CR0_INDEX, VmReadN (VMCS_N_GUEST_CR0_INDEX) | CR0_PG | CR0_PE);
          if ((GptRegPtr[Qualification.CrAccess.GpReg] & CR0_PE) != 0) {
            VmWriteN (VMCS_N_CONTROL_CR0_READ_SHADOW_INDEX, VmReadN (VMCS_N_CONTROL_CR0_READ_SHADOW_INDEX) & ~CR0_PG);
          } else {
            mGuestContextCommonSmm.GuestContextPerCpu[Index].Cr0 = GptRegPtr[Qualification.CrAccess.GpReg];
            VmWriteN (VMCS_N_CONTROL_CR0_READ_SHADOW_INDEX, VmReadN (VMCS_N_CONTROL_CR0_READ_SHADOW_INDEX) & ~CR0_PE & ~CR0_PG);

            // LaunchVm86Monitor (Index);
            CpuDeadLoop (); // never returned
          }

        } else if (((mGuestContextCommonSmm.GuestContextPerCpu[Index].Cr0 & CR0_PG) == 0) &&
                   ((GptRegPtr[Qualification.CrAccess.GpReg] & CR0_PG) != 0)) {
          //
          // Enable Paging, from PE
          //
//          DEBUG ((EFI_D_INFO, "+PG"));
          VmWriteN (VMCS_N_GUEST_CR0_INDEX, VmReadN (VMCS_N_GUEST_CR0_INDEX) | CR0_PG | CR0_PE);
          VmWriteN (VMCS_N_GUEST_CR3_INDEX, mGuestContextCommonSmm.GuestContextPerCpu[Index].Cr3);
          VmWriteN (VMCS_N_GUEST_CR4_INDEX, mGuestContextCommonSmm.GuestContextPerCpu[Index].Cr4 | (UINTN)(AsmReadMsr64 (IA32_VMX_CR4_FIXED0_MSR_INDEX) & AsmReadMsr64 (IA32_VMX_CR4_FIXED1_MSR_INDEX)));

          VmWrite64 (VMCS_64_GUEST_IA32_EFER_INDEX, mGuestContextCommonSmm.GuestContextPerCpu[Index].Efer);
          if ((mGuestContextCommonSmm.GuestContextPerCpu[Index].Efer & IA32_EFER_MSR_MLE) != 0) {
            mGuestContextCommonSmm.GuestContextPerCpu[Index].Efer |= IA32_EFER_MSR_MLA;
            VmEntryControls.Uint32 = VmRead32 (VMCS_32_CONTROL_VMENTRY_CONTROLS_INDEX);
            VmEntryControls.Bits.Ia32eGuest = 1;
            VmWrite32 (VMCS_32_CONTROL_VMENTRY_CONTROLS_INDEX, VmEntryControls.Uint32);
          }

          VmWriteN (VMCS_N_CONTROL_CR4_READ_SHADOW_INDEX, VmReadN (VMCS_N_GUEST_CR4_INDEX) & ~CR4_VMXE & ~CR4_SMXE);

        } else if (((mGuestContextCommonSmm.GuestContextPerCpu[Index].Cr0 & CR0_PE) != 0) &&
                   ((GptRegPtr[Qualification.CrAccess.GpReg] & CR0_PE) == 0)) {
          //
          // Disable protection
          //
//          DEBUG ((EFI_D_INFO, "-PE"));
          mGuestContextCommonSmm.GuestContextPerCpu[Index].Cr0 = GptRegPtr[Qualification.CrAccess.GpReg];
          VmWriteN (VMCS_N_GUEST_CR0_INDEX, VmReadN (VMCS_N_GUEST_CR0_INDEX) | CR0_PG | CR0_PE);
          VmWriteN (VMCS_N_CONTROL_CR0_READ_SHADOW_INDEX, VmReadN (VMCS_N_CONTROL_CR0_READ_SHADOW_INDEX) & ~CR0_PE & ~CR0_PG);

          // LaunchVm86Monitor (Index);
          CpuDeadLoop (); // never returned
        } else if (((mGuestContextCommonSmm.GuestContextPerCpu[Index].Cr0 & CR0_PE) == 0) &&
                   ((GptRegPtr[Qualification.CrAccess.GpReg] & CR0_PE) != 0)) {
          //
          // Enable protection
          // Should not happen, because Vm86Monitor should detect this and terminate via VmCall.
          //
          CpuDeadLoop ();
        }
      }

      //
      // Save current data as old data
      //
      mGuestContextCommonSmm.GuestContextPerCpu[Index].Cr0 = GptRegPtr[Qualification.CrAccess.GpReg];

      // special for EPT
      Ia32PAESync (Index);

      goto Ret;
#if 0
    } else if (Qualification.CrAccess.AccessType == 1) { // MOV from CR
      GptRegPtr[Qualification.CrAccess.GpReg] = VmReadN (VMCS_N_GUEST_CR0_INDEX);
      goto Ret;
#endif
    }
    break;
  case 4: // Cr4
    if (Qualification.CrAccess.AccessType == 0) { // MOV to CR
      if ((!mGuestContextCommonSmm.GuestContextPerCpu[Index].UnrestrictedGuest) &&
          ((mGuestContextCommonSmm.GuestContextPerCpu[Index].Cr0 & CR0_PG) == 0)) {
        //
        // Need cache current Setting
        //
      } else {
        VmWriteN (VMCS_N_GUEST_CR4_INDEX, GptRegPtr[Qualification.CrAccess.GpReg] | (UINTN)(AsmReadMsr64 (IA32_VMX_CR4_FIXED0_MSR_INDEX) & AsmReadMsr64 (IA32_VMX_CR4_FIXED1_MSR_INDEX)));
      }

      // update shadow
      VmWriteN (VMCS_N_CONTROL_CR4_READ_SHADOW_INDEX, VmReadN (VMCS_N_GUEST_CR4_INDEX) & ~CR4_VMXE & ~CR4_SMXE);

      //
      // Save current data as old data
      //
      mGuestContextCommonSmm.GuestContextPerCpu[Index].Cr4 = GptRegPtr[Qualification.CrAccess.GpReg];
      goto Ret;
#if 0
    } else if (Qualification.CrAccess.AccessType == 1) { // MOV from CR
      GptRegPtr[Qualification.CrAccess.GpReg] = VmReadN (VMCS_N_GUEST_CR4_INDEX);
      goto Ret;
#endif
    }
    break;

  default:
    break;
  }

  DEBUG ((EFI_D_INFO, "!!!CrAccessHandler!!!\n"));
  DumpVmcsAllField ();

  CpuDeadLoop ();

Ret:
  VmWriteN (VMCS_N_GUEST_RIP_INDEX, VmReadN(VMCS_N_GUEST_RIP_INDEX) + VmRead32(VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX));
  return ;
}