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

  This function sync Ia32PAE page table for EPT.

  @param Index CPU index

**/
VOID
Ia32PAESync (
  IN UINT32  Index
  );

/**

  This function is CR access handler.

  @param Index CPU index

**/
VOID
CrHandler (
  IN UINT32 Index
  )
{
  VM_EXIT_QUALIFICATION   Qualification;
  UINTN                   *GptRegPtr;
  VM_ENTRY_CONTROLS       VmEntryControls;
  UINTN                   Data;

  Qualification.UintN = VmReadN (VMCS_N_RO_EXIT_QUALIFICATION_INDEX);
  GptRegPtr = (UINTN *)&mGuestContextCommon.GuestContextPerCpu[Index].Register;

  switch (Qualification.CrAccess.CrNum) {
  case 3: // Cr3
    if (Qualification.CrAccess.AccessType == 0) { // MOV to CR

      if ((!mGuestContextCommon.GuestContextPerCpu[Index].UnrestrictedGuest) &&
          ((mGuestContextCommon.GuestContextPerCpu[Index].Cr0 & CR0_PG) == 0)) {
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
      mGuestContextCommon.GuestContextPerCpu[Index].Cr3 = GptRegPtr[Qualification.CrAccess.GpReg];
      goto Ret;
    } else if (Qualification.CrAccess.AccessType == 1) { // MOV from CR
      GptRegPtr[Qualification.CrAccess.GpReg] = mGuestContextCommon.GuestContextPerCpu[Index].Cr3;
      goto Ret;
    }
    break;
  case 0: // Cr0
    if (Qualification.CrAccess.AccessType == 0) { // MOV to CR
#if 0
      DEBUG ((EFI_D_INFO, "(FRM) !!!CrHandler - Cr0!!! %08x->%08x\n", VmReadN (VMCS_N_GUEST_CR0_INDEX), GptRegPtr[Qualification.CrAccess.GpReg]));
#endif
      VmWriteN (VMCS_N_GUEST_CR0_INDEX, GptRegPtr[Qualification.CrAccess.GpReg] | (UINTN)(AsmReadMsr64 (IA32_VMX_CR0_FIXED0_MSR_INDEX) & AsmReadMsr64 (IA32_VMX_CR0_FIXED1_MSR_INDEX) & ~CR0_PG & ~CR0_PE));

      //
      // Check IA32e mode switch
      //
      VmEntryControls.Uint32 = VmRead32 (VMCS_32_CONTROL_VMENTRY_CONTROLS_INDEX);
      if ((mGuestContextCommon.GuestContextPerCpu[Index].EFER & IA32_EFER_MSR_MLE) && 
          (GptRegPtr[Qualification.CrAccess.GpReg] & CR0_PG)) {
        mGuestContextCommon.GuestContextPerCpu[Index].EFER |= IA32_EFER_MSR_MLA;
        VmEntryControls.Bits.Ia32eGuest = 1;
      } else {
        mGuestContextCommon.GuestContextPerCpu[Index].EFER &= ~IA32_EFER_MSR_MLA;
        VmEntryControls.Bits.Ia32eGuest = 0;
      }
      VmWrite32 (VMCS_32_CONTROL_VMENTRY_CONTROLS_INDEX, VmEntryControls.Uint32);
      VmWrite64 (VMCS_64_GUEST_IA32_EFER_INDEX,          mGuestContextCommon.GuestContextPerCpu[Index].EFER);

      // check CD
      if (GptRegPtr[Qualification.CrAccess.GpReg] & CR0_CD) {
//        DEBUG ((EFI_D_ERROR, "(FRM) !!!CrHandler - Cr0: CD!!!\n"));
        AsmWbinvd ();
      }

      // update shadow
      VmWriteN (VMCS_N_CONTROL_CR0_READ_SHADOW_INDEX, VmReadN (VMCS_N_GUEST_CR0_INDEX));

      //
      // Check UnrestrictedGuest
      //
      if (!mGuestContextCommon.GuestContextPerCpu[Index].UnrestrictedGuest) {
        //
        // Need check PG and PE
        //
        if ((mGuestContextCommon.GuestContextPerCpu[Index].Cr0 & CR0_PG) &&
            ((GptRegPtr[Qualification.CrAccess.GpReg] & CR0_PG) == 0) /*&&
            (GptRegPtr[Qualification.CrAccess.GpReg] & CR0_PE)*/) {
          //
          // Disable Paging, but still PE
          //
#if 0
          if (GptRegPtr[Qualification.CrAccess.GpReg] & CR0_PE) {
            DEBUG ((EFI_D_INFO, "-PG"));
          } else {
            DEBUG ((EFI_D_INFO, "-PGE"));
          }
#endif
          mGuestContextCommon.GuestContextPerCpu[Index].Cr3 = VmReadN (VMCS_N_GUEST_CR3_INDEX);
          if (VmReadN (VMCS_N_GUEST_CR4_INDEX) & CR4_PAE) {
            VmWrite64 (VMCS_64_GUEST_IA32_EFER_INDEX, VmRead64 (VMCS_64_GUEST_IA32_EFER_INDEX) & ~IA32_EFER_MSR_MLE);
            mGuestContextCommon.GuestContextPerCpu[Index].Cr4 = VmReadN (VMCS_N_GUEST_CR4_INDEX) & ~CR4_VMXE & ~CR4_SMXE;
            VmWriteN (VMCS_N_GUEST_CR4_INDEX, VmReadN (VMCS_N_GUEST_CR4_INDEX) & ~CR4_PAE);
            VmWriteN (VMCS_N_GUEST_CR3_INDEX, mGuestContextCommon.CompatiblePageTable);
          } else {
            VmWriteN (VMCS_N_GUEST_CR3_INDEX, mGuestContextCommon.CompatiblePageTable);
          }

          VmWriteN (VMCS_N_GUEST_CR0_INDEX, VmReadN (VMCS_N_GUEST_CR0_INDEX) | CR0_PG | CR0_PE);
          if (GptRegPtr[Qualification.CrAccess.GpReg] & CR0_PE) {
            VmWriteN (VMCS_N_CONTROL_CR0_READ_SHADOW_INDEX, VmReadN (VMCS_N_CONTROL_CR0_READ_SHADOW_INDEX) & ~CR0_PG);
          } else {
            mGuestContextCommon.GuestContextPerCpu[Index].Cr0 = GptRegPtr[Qualification.CrAccess.GpReg];
            VmWriteN (VMCS_N_CONTROL_CR0_READ_SHADOW_INDEX, VmReadN (VMCS_N_CONTROL_CR0_READ_SHADOW_INDEX) & ~CR0_PE & ~CR0_PG);

            CpuDeadLoop (); // never returned
          }

        } else if (((mGuestContextCommon.GuestContextPerCpu[Index].Cr0 & CR0_PG) == 0) &&
//                   (mGuestContextCommon.GuestContextPerCpu[Index].Cr0 & CR0_PE) &&
                   (GptRegPtr[Qualification.CrAccess.GpReg] & CR0_PG)) {
          //
          // Enable Paging, from PE
          //
#if 0
          DEBUG ((EFI_D_INFO, "+PG"));
#endif
          VmWriteN (VMCS_N_GUEST_CR0_INDEX, VmReadN (VMCS_N_GUEST_CR0_INDEX) | CR0_PG | CR0_PE);
          VmWriteN (VMCS_N_GUEST_CR3_INDEX, mGuestContextCommon.GuestContextPerCpu[Index].Cr3);
          VmWriteN (VMCS_N_GUEST_CR4_INDEX, mGuestContextCommon.GuestContextPerCpu[Index].Cr4 | (UINTN)(AsmReadMsr64 (IA32_VMX_CR4_FIXED0_MSR_INDEX) & AsmReadMsr64 (IA32_VMX_CR4_FIXED1_MSR_INDEX)));

          VmWrite64 (VMCS_64_GUEST_IA32_EFER_INDEX, mGuestContextCommon.GuestContextPerCpu[Index].EFER);
          if (mGuestContextCommon.GuestContextPerCpu[Index].EFER & IA32_EFER_MSR_MLE) {
            mGuestContextCommon.GuestContextPerCpu[Index].EFER |= IA32_EFER_MSR_MLA;
            VmEntryControls.Uint32 = VmRead32 (VMCS_32_CONTROL_VMENTRY_CONTROLS_INDEX);
            VmEntryControls.Bits.Ia32eGuest = 1;
            VmWrite32 (VMCS_32_CONTROL_VMENTRY_CONTROLS_INDEX, VmEntryControls.Uint32);
          }

          VmWriteN (VMCS_N_CONTROL_CR4_READ_SHADOW_INDEX, VmReadN (VMCS_N_GUEST_CR4_INDEX) & ~CR4_VMXE & ~CR4_SMXE);

        } else if ((mGuestContextCommon.GuestContextPerCpu[Index].Cr0 & CR0_PE) &&
                   ((GptRegPtr[Qualification.CrAccess.GpReg] & CR0_PE) == 0)) {
          //
          // Disable protection
          //
#if 0
          DEBUG ((EFI_D_INFO, "-PE"));
#endif
          mGuestContextCommon.GuestContextPerCpu[Index].Cr0 = GptRegPtr[Qualification.CrAccess.GpReg];
          VmWriteN (VMCS_N_GUEST_CR0_INDEX, VmReadN (VMCS_N_GUEST_CR0_INDEX) | CR0_PG | CR0_PE);
          VmWriteN (VMCS_N_CONTROL_CR0_READ_SHADOW_INDEX, VmReadN (VMCS_N_CONTROL_CR0_READ_SHADOW_INDEX) & ~CR0_PE & ~CR0_PG);

          CpuDeadLoop (); // never returned
        } else if (((mGuestContextCommon.GuestContextPerCpu[Index].Cr0 & CR0_PE) == 0) &&
                   (GptRegPtr[Qualification.CrAccess.GpReg] & CR0_PE)) {
          //
          // Enable protection
          // Should not happen
          //
          CpuDeadLoop ();
        }
      }

      //
      // Save current data as old data
      //
      mGuestContextCommon.GuestContextPerCpu[Index].Cr0 = GptRegPtr[Qualification.CrAccess.GpReg];

      // special for EPT
      Ia32PAESync (Index);

      goto Ret;
#if 1
    } else if (Qualification.CrAccess.AccessType == 1) { // MOV from CR
      GptRegPtr[Qualification.CrAccess.GpReg] = VmReadN (VMCS_N_GUEST_CR0_INDEX);
      goto Ret;
#endif
    } else if (Qualification.CrAccess.AccessType == 3) { // LMSW
      if (mGuestContextCommon.GuestContextPerCpu[Index].UnrestrictedGuest) {
        //
        // Save current data as old data
        //
        DEBUG ((EFI_D_INFO, "(FRM) !!!Cpu%d Exeuct LMSW!!!\n", Index));
//        DumpVmcsAllField ();

        Data = VmReadN (VMCS_N_GUEST_CR0_INDEX);
        Data = (Data & ~0xF) | (Qualification.CrAccess.LmswSource & 0xF);
        VmWriteN (VMCS_N_GUEST_CR0_INDEX, Data);// GptRegPtr[Qualification.CrAccess.GpReg] | (UINTN)(AsmReadMsr64 (IA32_VMX_CR0_FIXED0_MSR_INDEX) & AsmReadMsr64 (IA32_VMX_CR0_FIXED1_MSR_INDEX) & ~CR0_PG & ~CR0_PE));

        //
        // Check IA32e mode switch
        //
        VmEntryControls.Uint32 = VmRead32 (VMCS_32_CONTROL_VMENTRY_CONTROLS_INDEX);
        if ((mGuestContextCommon.GuestContextPerCpu[Index].EFER & IA32_EFER_MSR_MLE) && 
            (GptRegPtr[Qualification.CrAccess.GpReg] & CR0_PG)) {
          mGuestContextCommon.GuestContextPerCpu[Index].EFER |= IA32_EFER_MSR_MLA;
          VmEntryControls.Bits.Ia32eGuest = 1;
        } else {
          mGuestContextCommon.GuestContextPerCpu[Index].EFER &= ~IA32_EFER_MSR_MLA;
          VmEntryControls.Bits.Ia32eGuest = 0;
        }
        VmWrite32 (VMCS_32_CONTROL_VMENTRY_CONTROLS_INDEX, VmEntryControls.Uint32);
        VmWrite64 (VMCS_64_GUEST_IA32_EFER_INDEX,          mGuestContextCommon.GuestContextPerCpu[Index].EFER);

        // check CD
        if (Qualification.CrAccess.LmswSource & CR0_CD) {
//          DEBUG ((EFI_D_INFO, "(FRM) !!!CrHandler - Cr0: CD!!!\n"));
          AsmWbinvd ();
        }

        // update shadow
        VmWriteN (VMCS_N_CONTROL_CR0_READ_SHADOW_INDEX, VmReadN (VMCS_N_GUEST_CR0_INDEX));

        mGuestContextCommon.GuestContextPerCpu[Index].Cr0 = (mGuestContextCommon.GuestContextPerCpu[Index].Cr0 & ~0xFul) | (Qualification.CrAccess.LmswSource & 0xF);
        // special for EPT
        Ia32PAESync (Index);
        goto Ret;
      }
      DEBUG ((EFI_D_INFO, "(FRM) !!!CrAccessHandler - Cr%d!!!\n", Qualification.CrAccess.CrNum));
      DEBUG ((EFI_D_INFO, "(FRM) !!!Qualification.CrAccess.AccessType - %d!!!\n", Qualification.CrAccess.AccessType));
      DEBUG ((EFI_D_INFO, "(FRM) !!!Qualification.CrAccess.LmswSource - %x!!!\n", Qualification.CrAccess.LmswSource));
      Data = VmReadN (VMCS_N_GUEST_CR0_INDEX);
      DEBUG ((EFI_D_INFO, "(FRM) !!!VMCS_N_GUEST_CR0 - %x!!!\n", Data));
    }
    break;
  case 4: // Cr4
    if (Qualification.CrAccess.AccessType == 0) { // MOV to CR
#if 0
      DEBUG ((EFI_D_INFO, "(FRM) !!!CrHandler - Cr4!!! %08x->%08x\n", VmReadN (VMCS_N_GUEST_CR4_INDEX), GptRegPtr[Qualification.CrAccess.GpReg]));
#endif

      if ((!mGuestContextCommon.GuestContextPerCpu[Index].UnrestrictedGuest) &&
          ((mGuestContextCommon.GuestContextPerCpu[Index].Cr0 & CR0_PG) == 0)) {
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
      mGuestContextCommon.GuestContextPerCpu[Index].Cr4 = GptRegPtr[Qualification.CrAccess.GpReg];

      //
      // We need sync GuestCr4.OSXSAVE to HostCr4. Or we will get exception in XsetbvHandler.
      // (No need to sync OSXMMEXCPT or OSFXSR, we can always enablt it)
      //
      if (mGuestContextCommon.GuestContextPerCpu[Index].Cr4 & CR4_OSXSAVE) {
        if (IsXStateSupoprted ()) {
          AsmWriteCr4 (AsmReadCr4() | CR4_OSXSAVE);
        }
      } else {
        AsmWriteCr4 (AsmReadCr4() & ~CR4_OSXSAVE);
      }
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

  DEBUG ((EFI_D_ERROR, "(FRM) !!!CrAccessHandler - Cr%d!!!\n", Qualification.CrAccess.CrNum));
  DumpVmcsAllField ();

  CpuDeadLoop ();

Ret:
  VmWriteN (VMCS_N_GUEST_RIP_INDEX, VmReadN(VMCS_N_GUEST_RIP_INDEX) + VmRead32(VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX));
  return ;
}