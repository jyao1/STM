/** @file
  PeSMM exception handler

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmRuntime.h"
#include "PeStm.h"
#include "PeLoadVm.h"

extern UINT64 EndTimeStamp;
extern UINT32 save_Inter_PeVm(UINT32 CpuIndex);
extern UINT32 PostPeVmProc(UINT32 rc, UINT32 CpuIndex, UINT32 mode);
extern PE_VM_DATA PeVmData[];

// define this here for now
#define INTERRUPT_VECTOR_NMI 2
#define INTERRUPT_VECTOR_PF  14
#define INTERRUPT_VECTOR_GPF 13

unsigned int StmVmPeNmiExCount = 0;

void PeExceptionHandler( IN UINT32 CpuIndex)
{
	VM_EXIT_INFO_INTERRUPTION IntInfo;
	UINT32 IntErr;
	UINTN address;
	UINT32 	VmType = mHostContextCommon.HostContextPerCpu[CpuIndex].GuestVmType;

	EndTimeStamp = AsmReadTsc();
	IntInfo.Uint32 = VmRead32(VMCS_32_RO_VMEXIT_INTERRUPTION_INFO_INDEX);
	IntErr  = VmRead32(VMCS_32_RO_VMEXIT_INTERRUPTION_ERROR_CODE_INDEX);
	StmVmPeNmiExCount++;   // free up the waiting smi processor

	DEBUG((EFI_D_ERROR, "%ld PeExceptionHandler VmexitInterruptionInfo: 0x%x  INTERRUPTION_ERROR_CODE 0x%x\n", CpuIndex, IntInfo.Uint32, IntErr));
	/*DEBUG*/DEBUG((EFI_D_ERROR, "%ld PeExceptionHandler - Exception Bitmap is: 0x%08lx\n", CpuIndex, VmRead32(VMCS_32_CONTROL_EXCEPTION_BITMAP_INDEX)));
	/*DEBUG*/DEBUG((EFI_D_ERROR, "%ld PeExceptionHandler - IDT Vectoring info 0x%08lx IDT Error Code 0x%08lx\n", CpuIndex, VmRead32(VMCS_32_RO_IDT_VECTORING_INFO_INDEX), VmRead32(VMCS_32_RO_IDT_VECTORING_ERROR_CODE_INDEX)));
	/*DEBUG*/DEBUG((EFI_D_ERROR, "%ld PeExceptionHandler - IDT Base 0x%016llx Limit 0x%016llx\n", CpuIndex, (UINT64)VmReadN(VMCS_N_GUEST_IDTR_BASE_INDEX), VmRead32(VMCS_32_GUEST_IDTR_LIMIT_INDEX)));

	if(IntInfo.Bits.Valid == 1)
	{

		switch(IntInfo.Bits.Vector)
		{
		case INTERRUPT_VECTOR_NMI:
			{
				// NMI means that (in this case) an external processor has received an SMI..

				DEBUG((EFI_D_ERROR, "%ld PeExceptionHandler - received NMI because SMI detected\n", CpuIndex));
				save_Inter_PeVm(CpuIndex);  // put the VM to sleep so that the SMI can be handled
				break;
			}
		case INTERRUPT_VECTOR_GPF:
			{
				// General Protection Fault- kill the PE/VM
				//DEBUG((EFI_D_ERROR, "%ld - PE/VM terminated because of an exception %x\n", CpuIndex, IntInfo.Uint32));

				DEBUG((EFI_D_ERROR, "%ld PeExceptionHandler - PE/VM General Protection Fault @ 0x%04lx:0x%016llx Address: 0x%016llx Info: 0x%lx\n",
					CpuIndex,
					VmRead16 (VMCS_16_GUEST_CS_INDEX),               
					VmReadN(VMCS_N_GUEST_RIP_INDEX),
					VmReadN(VMCS_N_RO_EXIT_QUALIFICATION_INDEX),
					IntInfo.Uint32));

				//DumpVmcsAllField();

				PostPeVmProc(PE_VM_GP_FAULT, CpuIndex, PRESERVE_VM);
				break;
			}

		case INTERRUPT_VECTOR_PF:
			{
				
				UINTN IDTLocation = VmReadN(VMCS_N_GUEST_IDTR_BASE_INDEX);  // find the IDT

				address = VmReadN(VMCS_N_RO_EXIT_QUALIFICATION_INDEX);

				DEBUG((EFI_D_ERROR, "%ld PeExceptionHandler - VM/PE Page Fault @ 0x%04lx:0x%016llx Address: 0x%016llx Info: 0x%lx\n",
					CpuIndex,
					VmRead16 (VMCS_16_GUEST_CS_INDEX),               
					VmReadN(VMCS_N_GUEST_RIP_INDEX),
					address,
					IntInfo.Uint32));
				
				if(( address >= IDTLocation) && (address < (IDTLocation + SIZE_4KB)))
				{
					DEBUG((EFI_D_ERROR, "%ld PeExceptionHandler - VM/PE Page Fault on IDT page - terminating VM\n", CpuIndex));
				}
				else
				{
					if(((PERM_VM_INJECT_INT & PeVmData[VmType].UserModule.VmConfig) == PERM_VM_INJECT_INT))// does the VM/PE want to handle its own page fault
					{
						AsmWriteCr2(address);   //CR2 holds the Page Fault address

						VmWrite32(VMCS_32_CONTROL_VMENTRY_INTERRUPTION_INFO_INDEX, IntInfo.Uint32);
						VmWrite32(VMCS_32_CONTROL_VMENTRY_EXCEPTION_ERROR_CODE_INDEX , IntErr);
						/*debug*/ DEBUG((EFI_D_ERROR, "%ld PeExceptionHandler - Injecting Page Fault\n", CpuIndex));
						return;
					}
				}

				DEBUG((EFI_D_ERROR, "     INTERRUPTION_ERROR_CODE: 0x%x\n",
					IntErr));
				if(IntErr & 0x00000001)
					DEBUG((EFI_D_ERROR, "Page-level protection violation\n"));
				else
					DEBUG((EFI_D_ERROR, "Non-present page\n"));
				if(IntErr & 0x00000002)
					DEBUG((EFI_D_ERROR, "Write-access attempted\n"));
				else
					DEBUG((EFI_D_ERROR, "read-access attempted\n"));
				if(IntErr & 0x00000004)
					DEBUG((EFI_D_ERROR, "USER mode\n"));
				else
					DEBUG((EFI_D_ERROR, "Supervisor mode\n"));
				if(IntErr & 0x00000008)
					DEBUG((EFI_D_ERROR, "Reserved bit set in paging structures\n"));
				if(IntErr & 0x00000010)
					DEBUG((EFI_D_ERROR, "Instruction FETCH\n"));
				else
					DEBUG((EFI_D_ERROR, "Data access\n"));

				// if the PT is 64-bit then dump the table for diagnostic purposes

#define Level4 0x0000FF8000000000ULL
#define Level3 0x0000007FC0000000ULL      
#define Level2 0x000000003FE00000ULL
#define Level1 0x00000000001FF000ULL
#define PAGING_4K_ADDRESS_MASK_64 0x000FFFFFFFFFF000ull

				if (sizeof(UINTN) == sizeof(UINT64)) 
				{
					UINTN                             PageTable;
					UINTN                             Pml4Index;
					UINTN							  PdpteIndex;
					UINTN							  PdeIndex;
					UINTN							  PteIndex;
					UINTN                            *Pde;
					UINTN                            *Pdpte;
					UINTN                            *Pml4;
					UINTN							 *Pte;
					UINTN                             BaseAddress;
					UINTN                             EndAddress;
					UINTN                             PhysBase;
					UINTN							  PdpteV;
					UINTN						      PdeV;
					UINTN							  PteV;
					UINTN							  Offset;

					PageTable = VmReadN (VMCS_N_GUEST_CR3_INDEX);
					//Pml4 = (UINT64 *)PageTable;

					Pml4Index = (address & Level4) >> 39;
					PdpteIndex = (UINTN)(address & Level3) >> 30;
					PdeIndex = (address & Level2) >> 21;
					PteIndex = (address & Level1) >> 12;

					DEBUG((EFI_D_ERROR, "Pagetable Chain causing the error\n"));
					DEBUG((EFI_D_ERROR, "    Pagetable Address (CR3): 0x%llx\n", PageTable));

					BaseAddress = (UINTN)PeVmData[VmType].UserModule.AddressSpaceStart;
					EndAddress  = BaseAddress + PeVmData[VmType].UserModule.AddressSpaceSize - 1;
					PhysBase = (UINT64)PeVmData[VmType].SmmBuffer;

					if(PageTable < BaseAddress || PageTable > (EndAddress - 4096))
					{
						DEBUG((EFI_D_ERROR, "    CR3 points out of VM region\n"));
						goto endpf;
					}

					// find out where the page table is located in the VM memory

					Offset = PageTable - BaseAddress;
					Pml4 = (UINTN *) (PhysBase + Offset);

					DEBUG((EFI_D_ERROR, "    Pml4[0x%x]: 0x%llx\n", Pml4Index, Pml4[Pml4Index]));

					PdpteV = Pml4[Pml4Index] & PAGING_4K_ADDRESS_MASK_64;

					if(PdpteV < BaseAddress || PdpteV > (EndAddress - 4096))
					{
						DEBUG((EFI_D_ERROR, "    Pdpte points out of VM region\n"));
						goto endpf;
					}

					Offset = PdpteV - BaseAddress;
					Pdpte = (UINTN *)(PhysBase + Offset);

					DEBUG((EFI_D_ERROR, "    Pdpte[0x%x]: 0x%llx\n", PdpteIndex, Pdpte[PdpteIndex]));

					PdeV = Pdpte[PdpteIndex] & PAGING_4K_ADDRESS_MASK_64;

					if(PdeV < BaseAddress || PdeV > (EndAddress - 4096))
					{
						DEBUG((EFI_D_ERROR, "    Pde points out of VM region\n"));
						goto endpf;
					}

					if(Pdpte[PdpteIndex] & IA32_PG_PS)
					{
						DEBUG((EFI_D_ERROR, "    Pdpte Index: [0x%x]\n", PdpteIndex));
						DEBUG((EFI_D_ERROR, "    Pte   Index: [0x%x]\n", PteIndex));
						goto endpf;
					}

					Offset = PdeV - BaseAddress;
					Pde = (UINTN *) (PhysBase + Offset);

					DEBUG((EFI_D_ERROR, "    Pde[0x%x]: 0x%llx\n", PdeIndex, Pde[PdeIndex]));

					PteV = Pde[PdeIndex] & PAGING_4K_ADDRESS_MASK_64;

					if(PteV < BaseAddress || PteV > (EndAddress - 4096))
					{
						DEBUG((EFI_D_ERROR, "    Pte points out of VM region\n"));
						goto endpf;
					}

					if(Pde[PdeIndex] & IA32_PG_PS)
					{
						DEBUG((EFI_D_ERROR, "    Pte Index: [0x%x]\n", PteIndex));
						goto endpf;
					}

					Offset = PteV - BaseAddress;
					Pte = (UINTN *) (PhysBase + Offset);

					DEBUG((EFI_D_ERROR, "    Pte[0x%x]: 0x%llx\n", PteIndex, Pte[PteIndex]));
				}
				//DumpVmcsAllField();
endpf:
				PostPeVmProc(PE_VM_PAGE_FAULT, CpuIndex, PRESERVE_VM);
				break;
			}
		default:
			{
				DEBUG((EFI_D_ERROR, "%ld PeExceptionHandler - PE/VM Unhandled Exception @ 0x%04lx:0x%016llx Address: 0x%016llx Info: 0x%lx\n",
					CpuIndex,
					VmRead16 (VMCS_16_GUEST_CS_INDEX),               
					VmReadN(VMCS_N_GUEST_RIP_INDEX),
					VmReadN(VMCS_N_RO_EXIT_QUALIFICATION_INDEX),
					IntInfo.Uint32));

				//DumpVmcsAllField();

				PostPeVmProc(PE_VM_GP_FAULT, CpuIndex, PRESERVE_VM);
			}
		}
		// should not get here

		DEBUG((EFI_D_ERROR, "%ld PeExceptionHandler - CpuDeadLoop\n", CpuIndex));
		CpuDeadLoop();
		return;
	}

	DEBUG((EFI_D_ERROR, "%ld PeExceptionHandler - Warning Info Valid bits not equal to 1 @ 0x%04lx:0x%016llx Address: 0x%016llx Info: 0x%lx\n",
		CpuIndex,
		VmRead16 (VMCS_16_GUEST_CS_INDEX),               
		VmReadN(VMCS_N_GUEST_RIP_INDEX),
		VmReadN(VMCS_N_RO_EXIT_QUALIFICATION_INDEX),
		IntInfo.Uint32));
	return;
}
#ifdef dummy
VOID
EventInjection (
  UINT32 Index
  )
{
  STM_PROTECTION_EXCEPTION_HANDLER          *StmProtectionExceptionHandler;
  STM_PROTECTION_EXCEPTION_STACK_FRAME_X64  *StackFrame;
  UINTN                                     Rflags;
  X86_REGISTER                              *Reg;
  UINT32									VmType = SMI_HANDLER;

  Reg = &mGuestContextCommonSmm[VmType].GuestContextPerCpu[Index].Register;

  StmProtectionExceptionHandler = &mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->StmProtectionExceptionHandler;

  //
  // Check valid
  //
  if (StmProtectionExceptionHandler->SpeRip == 0) {
    DEBUG ((EFI_D_INFO, "SpeRip unsupported!\n"));
    // Unsupported;
    return ;
  }

  //
  // Fill exception stack
  //
  StackFrame = (STM_PROTECTION_EXCEPTION_STACK_FRAME_X64 *)(UINTN)StmProtectionExceptionHandler->SpeRsp;
  StackFrame -= 1;

  //
  // make it 16bytes align
  //
  StackFrame = (STM_PROTECTION_EXCEPTION_STACK_FRAME_X64 *)(((UINTN)StackFrame - 0x10) & ~0xF);
  StackFrame = (STM_PROTECTION_EXCEPTION_STACK_FRAME_X64 *)((UINTN)StackFrame - 0x8);

  mGuestContextCommonSmm[VmType].GuestContextPerCpu[Index].InfoBasic.Uint32 = VmRead32 (VMCS_32_RO_EXIT_REASON_INDEX);

  mGuestContextCommonSmm[VmType].GuestContextPerCpu[Index].VmExitInstructionLength = VmRead32 (VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX);

  VmReadN (VMCS_N_RO_EXIT_QUALIFICATION_INDEX);
  VmRead32 (VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX);
  VmRead32 (VMCS_32_RO_VMEXIT_INSTRUCTION_INFO_INDEX);
  switch (mGuestContextCommonSmm[VmType].GuestContextPerCpu[Index].InfoBasic.Bits.Reason) {
  
  }

  
  if (mGuestContextCommonSmm[VmType].GuestContextPerCpu[Index].InfoBasic.Bits.Reason == VmExitReasonEptViolation) {
    // For SMM handle, linear addr == physical addr
    StackFrame->Cr2 = (UINTN)VmRead64(VMCS_64_RO_GUEST_PHYSICAL_ADDR_INDEX);
  } else {
    StackFrame->Cr2 = AsmReadCr2();
  }


 // inject page fault: (from vtlb.c)

  pf_error_code, PF_address

  vector = vmread32(VM_EXIT_INFO_IDT_VECTORING);
  interrrupt_info = vmread(VM_EXIT_INFO_EXCEPTION_INFO);

  stack->cr2 = PF_ADDRESS;   //???

  vmwrite(VM_ENTER_INTERRUPT_INFO, interrupt_info);
  vmwrite(VM_ENTER_EXCEPTION_ERROR_CODE, PF_ERROR_CODE);


  // inject interrupt

  ivector = vnread*(VM_EXIT_INFO_IDT_VECTORING);

  switch(ivector.bits.type)
  {
	case INT_TYPE_NMI:
		//handleed elsewhare...
		break;
	case INT_TYPE_EXCEPT:
		if(ivector.as_uint32 ==0x80000b0c)
		{
			// deal with non-PF exception

		}

		// we got here because we injected a PF that hit an IDT that was
		// we now reinject the PF so that we won't have to do this again
		// note: this is a shadow table issue...

		if(ivector.as_uint32 == 0x80000b0e)
		{
			idt_vectoring_code = Last_injected_PF_Code
				if(Last_injected_Ps_add != vstate->non_vmx_gs.cr2;)
				{
					// someting is wrong
				}
		}
		cr2 = exit_qualification
		vmwrite(VM_ENTER_INTERRUPT_INFO, ivector)
	    vmwrite(VM_ENTER_EXCEPTION_ERROR_CODE, idt_vectoring_code)
		break;
		// something was wrong with this...
	case INT_TYPE_SW:

		vmwrite(VM_ENTER_INTERRUPT_INFO, ivector);
		vmwrite(VM_ENTER_EXCEPTION_ERROR_CODE, idt_vectoring_code);
		vmread(VM_EXIT_INFO_INSTRUCTION_LENGTH, instruction_length);
		vmwrite(VM_ENTER_INSTRUCTION_LENGTH, instuction_length);
		break;
	case INT_TYPE_EXT:
		vmwrite(VM_ENTER_INTERRUPT_INFO, ivector);
		vmwrite(VM_ENTER_EXCEPTON_ERROR_CODE, idt_vectoring_code);
  }

  need to check guest interruptability -- VMCS_GUEST_INTERRUPTIBILITY
	  rflags=vmread(VMCS_GUEST_RFLAGS);









  Rflags = AsmVmResume (Reg);
  // BUGBUG: - AsmVmLaunch if AsmVmResume fail
  if (VmRead32 (VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX) == VmxFailErrorVmResumeWithNonLaunchedVmcs) {
//    DEBUG ((EFI_D_ERROR, "(STM):-(\n", (UINTN)Index));
    Rflags = AsmVmLaunch (Reg);
  }
  AcquireSpinLock (&mHostContextCommon.DebugLock);
  DEBUG ((EFI_D_ERROR, "!!!ResumeToBiosExceptionHandler FAIL!!!\n"));
  DEBUG ((EFI_D_ERROR, "Rflags: %08x\n", Rflags));
  DEBUG ((EFI_D_ERROR, "VMCS_32_RO_VM_INSTRUCTION_ERROR: %08x\n", (UINTN)VmRead32 (VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX)));
  DumpVmcsAllField ();
  DumpRegContext (Reg);
  DumpGuestStack(Index);
  ReleaseSpinLock (&mHostContextCommon.DebugLock);
  CpuDeadLoop ();
  return ;
}

/**

  This function return from BIOS exception handler.

  @param Index CPU index

**/
VOID
ReturnFromBiosExceptionHandler (
  UINT32 Index
  )
{
  STM_PROTECTION_EXCEPTION_HANDLER          *StmProtectionExceptionHandler;
  STM_PROTECTION_EXCEPTION_STACK_FRAME_X64  *StackFrame;
  UINTN                                     Rflags;
  X86_REGISTER                              *Reg;
  UINT32                                    VmType = SMI_HANDLER;

  Reg = &mGuestContextCommonSmm[VmType].GuestContextPerCpu[Index].Register;

  StmProtectionExceptionHandler = &mHostContextCommon.HostContextPerCpu[Index].TxtProcessorSmmDescriptor->StmProtectionExceptionHandler;

  //
  // Check valid
  //
  if (StmProtectionExceptionHandler->SpeRip == 0) {
    // Unsupported;
    return ;
  }

  //
  // Get exception stack
  //
  StackFrame = (STM_PROTECTION_EXCEPTION_STACK_FRAME_X64 *)(UINTN)StmProtectionExceptionHandler->SpeRsp;
  StackFrame -= 1;

  //
  // make it 16bytes align
  //
  StackFrame = (STM_PROTECTION_EXCEPTION_STACK_FRAME_X64 *)(((UINTN)StackFrame - 0x10) & ~0xF);
  StackFrame = (STM_PROTECTION_EXCEPTION_STACK_FRAME_X64 *)((UINTN)StackFrame - 0x8);

  Reg->R15 = StackFrame->R15;
  Reg->R14 = StackFrame->R14;
  Reg->R13 = StackFrame->R13;
  Reg->R12 = StackFrame->R12;
  Reg->R11 = StackFrame->R11;
  Reg->R10 = StackFrame->R10;
  Reg->R9  = StackFrame->R9;
  Reg->R8  = StackFrame->R8;
  Reg->Rdi = StackFrame->Rdi;
  Reg->Rsi = StackFrame->Rsi;
  Reg->Rbp = StackFrame->Rbp;
  Reg->Rdx = StackFrame->Rdx;
  Reg->Rcx = StackFrame->Rcx;
  Reg->Rbx = StackFrame->Rbx;
  Reg->Rax = StackFrame->Rax;
//  AsmWriteCr8 (StackFrame->Cr8);
  VmWriteN (VMCS_N_GUEST_CR3_INDEX, StackFrame->Cr3);
  AsmWriteCr2 (StackFrame->Cr2);
  VmWriteN (VMCS_N_GUEST_CR0_INDEX, StackFrame->Cr0);
  VmWriteN (VMCS_N_GUEST_RIP_INDEX, StackFrame->Rip);
  VmWrite16 (VMCS_16_GUEST_CS_INDEX, (UINT16)StackFrame->Cs);
  VmWriteN (VMCS_N_GUEST_RFLAGS_INDEX, StackFrame->Rflags);
  VmWriteN (VMCS_N_GUEST_RSP_INDEX, StackFrame->Rsp);
  VmWrite16 (VMCS_16_GUEST_SS_INDEX, (UINT16)StackFrame->Ss);

  DEBUG ((EFI_D_INFO, "ReturnFromBiosExceptionHandler - %d\n", (UINTN)Index));

  STM_PERF_START (Index, 0, "BiosSmmHandler", "ReturnFromBiosExceptionHandler");

  //
  // Start resume back to BiosExceptionHandler
  //
  Rflags = AsmVmResume (Reg);
  // BUGBUG: - AsmVmLaunch if AsmVmResume fail
  if (VmRead32 (VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX) == VmxFailErrorVmResumeWithNonLaunchedVmcs) {
//    DEBUG ((EFI_D_ERROR, "(STM):-(\n", (UINTN)Index));
    Rflags = AsmVmLaunch (Reg);
  }
  AcquireSpinLock (&mHostContextCommon.DebugLock);
  DEBUG ((EFI_D_ERROR, "!!!ReturnFromBiosExceptionHandler FAIL!!!\n"));
  DEBUG ((EFI_D_ERROR, "Rflags: %08x\n", Rflags));
  DEBUG ((EFI_D_ERROR, "VMCS_32_RO_VM_INSTRUCTION_ERROR: %08x\n", (UINTN)VmRead32 (VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX)));
  DumpVmcsAllField ();
  DumpRegContext (Reg);
  DumpGuestStack(Index);
  ReleaseSpinLock (&mHostContextCommon.DebugLock);
  CpuDeadLoop ();
  return ;
}
#endif
