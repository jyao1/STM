/** @file

PE SMM handler - Handle VMEXITs from the running VM/PE

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

extern void PeSmmVmcallHandler ( IN UINT32  Index);
extern void PeRsmHandler( IN UINT32 Index);
extern void PeIoHandler( IN UINT32 CpuIndex);
extern void PeEPTViolationHandler( IN UINT32 CpuIndex);
extern void PeEPTMisconfigurationHandler( IN UINT32 CpuIndex);
extern void PeInvEPTHandler( IN UINT32 CpuIndex);
extern void PeBadGuestStateHandler( IN UINT32 CpuIndex);
extern void PeReadMsrHandler( IN UINT32 CpuIndex);
extern void PeWriteMsrHandler( IN UINT32 CpuIndex);
extern void PeCrHandler( IN UINT32 CpuIndex);
extern void PeExceptionHandler( IN UINT32 CpuIndex);
extern void PeCpuidHandler( IN UINT32 CpuIndex);
extern void PePreEmptionTimerHandler(IN UINT32 CpuIndex);
extern void PeTripleFaultHandler(IN UINT32 CpuIndex);

void InitCpuReadySync();

STM_HANDLER  mStmHandlerPeVm[VmExitReasonMax];

extern PE_SMI_CONTROL PeSmiControl;

/**

This function initialize STM/PE handle for SMM.

**/
VOID
	PeInitStmHandlerSmm (
	VOID
	)
{
	UINT32  Index;

	/* initialize the remainder of the guest contexts for the smm handlers */

	for(Index = SMI_HANDLER + 1; Index < NUM_PE_TYPE; Index++)
	{
		mGuestContextCommonSmm[Index].GuestContextPerCpu = AllocatePages (STM_SIZE_TO_PAGES(sizeof(STM_GUEST_CONTEXT_PER_CPU)) * mHostContextCommon.CpuNum);
	}

	DEBUG ((EFI_D_INFO, "PeInitStmHandlerSmm - initilizating PeSmmHandler Tables\n"));
	for (Index = 0; Index < VmExitReasonMax; Index++) {
		mStmHandlerPeVm[Index] = UnknownHandlerSmm;
	}

	mStmHandlerPeVm[VmExitReasonRsm] = PeRsmHandler;
	mStmHandlerPeVm[VmExitReasonVmCall] = PeSmmVmcallHandler;
	mStmHandlerPeVm[VmExitReasonExceptionNmi] = PeExceptionHandler;
	mStmHandlerPeVm[VmExitReasonCrAccess] = PeCrHandler;
	mStmHandlerPeVm[VmExitReasonEptViolation] = PeEPTViolationHandler;
	mStmHandlerPeVm[VmExitReasonEptMisConfiguration] = PeEPTMisconfigurationHandler;
	mStmHandlerPeVm[VmExitReasonInvEpt] = PeInvEPTHandler;
	mStmHandlerPeVm[VmExitReasonIoInstruction] = PeIoHandler;
	mStmHandlerPeVm[VmExitReasonCpuid] = PeCpuidHandler;
	mStmHandlerPeVm[VmExitReasonRdmsr] = PeReadMsrHandler;
	mStmHandlerPeVm[VmExitReasonWrmsr] = PeWriteMsrHandler;
	mStmHandlerPeVm[VmExitReasonVmEntryFailureDueToInvalidGuestState] = PeBadGuestStateHandler;
	mStmHandlerPeVm[VmExitReasonVmxPreEmptionTimerExpired] = PePreEmptionTimerHandler;
	mStmHandlerPeVm[VmExitReasonTripleFault] = PeTripleFaultHandler;

	DEBUG ((EFI_D_INFO, "PeInitStmHandlerSmm - PeSmmHandler Tables initialized\n"));

	InitCpuReadySync();
	DEBUG((EFI_D_INFO, "PeInitStmHandlerSmm - CpuReadySync Initialized\n"));
}

/**

This function is STM/PE handler for SMM VMEXITS

@param Register X86 register context

**/
VOID
	PeStmHandlerSmm (
	IN X86_REGISTER *Register
	)
{
	UINT32              Index;
	UINTN               Rflags;
	VM_EXIT_INFO_BASIC  InfoBasic;
	X86_REGISTER        *Reg;
	UINT32			  VmType;

	UINT32             pIndex;

	Index = ApicToIndex (ReadLocalApicId ());
	VmType = mHostContextCommon.HostContextPerCpu[Index].GuestVmType;  // any VmType other than SMI_HANDLER is a PeVm

	if(VmType != SMI_HANDLER)
		pIndex = 0;
	else
	{
		DEBUG((EFI_D_ERROR, "%ld PeStmHandlerSmm - Warning SMI_HANDLER type used in Pe handler\n", Index));
		pIndex = Index;
	}
	// make sure no one fires an SMI our way

	PeSmiControl.PeExec = 0;
	PeSmiControl.PeNmiBreak = 1;

	//STM_PERF_END (Index, "BiosSmmHandler", "StmHandlerSmm");

	Reg = &mGuestContextCommonSmm[VmType].GuestContextPerCpu[pIndex].Register;
	Register->Rsp = VmReadN (VMCS_N_GUEST_RSP_INDEX);
	CopyMem (Reg, Register, sizeof(X86_REGISTER));//
	//#if 0
	//DEBUG ((EFI_D_INFO, "%ld PeStmHandlerSmm - Started\n", (UINTN)Index));
	//#endif
	//
	// Dispatch
	//
	InfoBasic.Uint32 = VmRead32 (VMCS_32_RO_EXIT_REASON_INDEX);
	//DEBUG((EFI_D_ERROR, "%d PeStmHandlerSmm - InfoBasic: 0x%0l8x Reason: %d\n", Index, InfoBasic.Uint32, InfoBasic.Bits.Reason));

	if (InfoBasic.Bits.Reason >= VmExitReasonMax) {
		DEBUG ((EFI_D_ERROR, "%ld PeStmHandlerSmm - !!!Unknown VmExit Reason!!!\n", Index));
		DumpVmcsAllField (Index);
		DEBUG((EFI_D_ERROR, "%ld PeStmHandlerSmm - CpuDeadLoop\n", Index));

		CpuDeadLoop ();
	}

	//
	// Call dispatch handler
	//
	if(mStmHandlerPeVm[InfoBasic.Bits.Reason] == NULL)
	{
		DEBUG((EFI_D_INFO, "%ld PeStmHandlerSmm - ***WARNING*** mStmHandlerPeVm[%x] is NULL- aborting STM \n", Index, InfoBasic.Bits.Reason));
		WriteUnaligned32 ((UINT32 *)&Reg->Rax, 0xFFFFFFFF);
		VmWriteN (VMCS_N_GUEST_RIP_INDEX, VmReadN (VMCS_N_GUEST_RIP_INDEX) + VmRead32 (VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX));
		DEBUG((EFI_D_ERROR, "%ld PeStmHandlerSmm - CpuDeadLoop\n", Index));
		CpuDeadLoop();
	}
	else
	{
		mStmHandlerPeVm[InfoBasic.Bits.Reason] (Index); // PE VM 
	}
	VmWriteN (VMCS_N_GUEST_RSP_INDEX, Reg->Rsp); // sync RSP

	// STM_PERF_START (Index, InfoBasic.Bits.Reason, "BiosSmmHandler", "StmHandlerSmm");

	//
	// Resume
	//
	Rflags = AsmVmResume (&mGuestContextCommonSmm[VmType].GuestContextPerCpu[pIndex].Register);
	// BUGBUG: - AsmVmLaunch if AsmVmResume fail
	if (VmRead32 (VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX) == VmxFailErrorVmResumeWithNonLaunchedVmcs) {
		//    DEBUG ((EFI_D_ERROR, "(STM):-(\n", (UINTN)Index));
		Rflags = AsmVmLaunch (&mGuestContextCommonSmm[VmType].GuestContextPerCpu[pIndex].Register);
	}

	AcquireSpinLock (&mHostContextCommon.DebugLock);

	DEBUG ((EFI_D_ERROR, "%ld PeStmHandlerSmm - !!!ResumePeGuestSmm FAIL!!!\n", (UINTN)Index));
	DEBUG ((EFI_D_ERROR, "%ld PeStmHandlerSmm - Rflags: %08x\n", Index, Rflags));
	DEBUG ((EFI_D_ERROR, "%ld PeStmHandlerSmm - VMCS_32_RO_VM_INSTRUCTION_ERROR: %08x\n", Index, (UINTN)VmRead32 (VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX)));
	DumpVmcsAllField (Index);
	DumpRegContext (&mGuestContextCommonSmm[VmType].GuestContextPerCpu[pIndex].Register, Index);
	DumpGuestStack(Index);
	ReleaseSpinLock (&mHostContextCommon.DebugLock);
	DEBUG((EFI_D_ERROR, "%ld PeStmHandlerSmm - CpuDeadLoop\n", Index));
	CpuDeadLoop ();

	return ;
}
