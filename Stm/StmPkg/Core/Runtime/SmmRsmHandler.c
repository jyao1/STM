/** @file
SMM RSM handler

Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
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

extern PE_VM_DATA PeVmData[4];
extern UINT32 RestoreInterPeVm(UINT32 CpuIndex, UINT32 PeType);
extern PE_SMI_CONTROL PeSmiControl;
extern unsigned int CpuInSmiCount;

/**

This function is RSM handler for SMM.

@param Index CPU index

**/
VOID
	RsmHandler (
	IN UINT32  Index
	)
{
	UINTN                          Rflags = 0;
	UINT64                         ExecutiveVmcsPtr;
	UINT64                         VmcsLinkPtr;
	UINT32                         VmcsSize;
	UINT32                         PeType;

	VmcsSize = GetVmcsSize();
	ExecutiveVmcsPtr = VmRead64 (VMCS_64_CONTROL_EXECUTIVE_VMCS_PTR_INDEX);
  if (IsOverlap (ExecutiveVmcsPtr, VmcsSize, mHostContextCommon.TsegBase, mHostContextCommon.TsegLength)) {
			// Overlap TSEG
			DEBUG ((EFI_D_ERROR, "%ld RsmHandler - ExecutiveVmcsPtr violation (RsmHandler) - %016lx\n", Index, ExecutiveVmcsPtr));
			CpuDeadLoop() ;
	}

	VmcsLinkPtr = VmRead64 (VMCS_64_GUEST_VMCS_LINK_PTR_INDEX);
  if (IsOverlap (VmcsLinkPtr, VmcsSize, mHostContextCommon.TsegBase, mHostContextCommon.TsegLength)) {
			// Overlap TSEG
			DEBUG ((EFI_D_ERROR, "%ld RsmHandler - VmcsLinkPtr violation (RsmHandler) - %016lx\n", Index, VmcsLinkPtr));
			CpuDeadLoop() ;
	}

	if (mHostContextCommon.HostContextPerCpu[Index].JumpBufferValid) {
		//
		// return from Setup/TearDown
		//
		mHostContextCommon.HostContextPerCpu[Index].JumpBufferValid = FALSE;
		LongJump (&mHostContextCommon.HostContextPerCpu[Index].JumpBuffer, (UINTN)-1);
		// Should not get here
		CpuDeadLoop ();
	}

	AsmVmPtrStore (&mGuestContextCommonSmm[SMI_HANDLER].GuestContextPerCpu[Index].Vmcs);

	if((PeSmiControl.PeCpuIndex == ((INT32) Index)))
	{
		PeType = mHostContextCommon.HostContextPerCpu[Index].NonSmiHandler;
		//PeType = mHostContextCommon.HostContextPerCpu[Index].GuestVmType;
		DEBUG((EFI_D_ERROR, "%ld RsmHandler - VmPe Detected - PeType: %ld PeVmState: %ld\n", Index, PeType, PeVmData[PeType].PeVmState));

		switch(PeVmData[PeType].PeVmState)
		{
		case PE_VM_SUSPEND: // is this a suspended PE/VM?
			{
				// restore it - return to peer once it completes

				RestoreInterPeVm(Index, PeType);
				//should not return... will let the module handle the error processing

				// this will return in the case where the VM/PE was being created and it was interrupted by a SMI that was detected
				// while doing the processor state gathering.
				// we will come out and let it return so that the SMI can get fired and
				// when the SMI handler is done will reattempt to regather the processor info
				DEBUG((EFI_D_ERROR, "%ld RsmHandler ERROR - Failed to restart PE/VM after SMI, PeType: %ld\n", Index, PeType));
				break;
			}
		case PE_VM_IDLE:
		case PE_VM_AVAIL:
			{
				//DEBUG((EFI_D_ERROR, "%ld RsmHandler Idle VmPe: ignoring\n", Index));
				break;
			}
		default:
			{
				DEBUG((EFI_D_ERROR, " %ld RsmHandler - data structure inconsistency - suspended PE/VM not found\n", Index));
			}
		}
	}

	Rflags = AsmVmPtrLoad (&mGuestContextCommonSmi.GuestContextPerCpu[Index].Vmcs);
	if ((Rflags & (RFLAGS_CF | RFLAGS_ZF)) != 0) {
		DEBUG ((EFI_D_ERROR, "%ld RsmHandler - ERROR: AsmVmPtrLoad - %016lx : %08x\n", (UINTN)Index, mGuestContextCommonSmi.GuestContextPerCpu[Index].Vmcs, Rflags));
		CpuDeadLoop ();
	}

	STM_PERF_START (Index, 0, "ReadSyncSmmStateSaveArea", "RsmHandler");
	ReadSyncSmmStateSaveArea (Index);
	STM_PERF_END (Index, "ReadSyncSmmStateSaveArea", "RsmHandler");

#if 0
	DEBUG ((EFI_D_INFO, "RsmHandler Exit SmmHandler - %d\n", (UINTN)Index));
#endif

	// We should not WaitAllProcessorRendezVous() because we can not assume SMM will bring all CPU into BIOS SMM handler.
	//  WaitAllProcessorRendezVous (Index);

	STM_PERF_END (Index, "OsSmiHandler", "RsmHandler");

	CheckPendingMtf (Index);

	//
	// Launch back
	//
	Rflags = AsmVmResume (&mGuestContextCommonSmi.GuestContextPerCpu[Index].Register);
	// BUGBUG: - AsmVmLaunch if AsmVmResume fail
	if (VmRead32 (VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX) == VmxFailErrorVmResumeWithNonLaunchedVmcs) {
		DEBUG ((EFI_D_ERROR, "%ld :o(\n", (UINTN)Index));
		Rflags = AsmVmLaunch (&mGuestContextCommonSmi.GuestContextPerCpu[Index].Register);
	}

	AcquireSpinLock (&mHostContextCommon.DebugLock);
	DEBUG ((EFI_D_ERROR, "%ld !!!RsmHandler FAIL!!!\n", (UINTN)Index));
	DEBUG ((EFI_D_ERROR, "%ld RsmHandler Rflags: %08x\n", (UINTN)Index, Rflags));
	DEBUG ((EFI_D_ERROR, "%ld RsmHandler VMCS_32_RO_VM_INSTRUCTION_ERROR: %08x\n", (UINTN)Index, (UINTN)VmRead32 (VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX)));
	DumpVmcsAllField ();
	DumpRegContext (&mGuestContextCommonSmi.GuestContextPerCpu[Index].Register);
	DumpGuestStack(Index);
	ReleaseSpinLock (&mHostContextCommon.DebugLock);

	CpuDeadLoop ();

	return ;
}
