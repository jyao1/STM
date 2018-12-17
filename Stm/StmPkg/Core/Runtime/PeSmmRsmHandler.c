/** @file

This function is RSM handler for PE.

@param CpuIndex CPU index

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
extern UINT32 PostPeVmProc(UINT32 rc, UINT32 CpuIndex, UINT32 mode);

VOID
	PeRsmHandler (
	IN UINT32  CpuIndex
	)
{
	UINTN             Rflags;
	UINT32			VmType;
	UINT32          pCpuIndex;

	EndTimeStamp = AsmReadTsc();
	VmType = mHostContextCommon.HostContextPerCpu[CpuIndex].GuestVmType;  // any VmType other than SMI_HANDLER is a PeVm

	if(VmType != SMI_HANDLER)
		pCpuIndex = 0;  // PeVm is always zero
	else
		pCpuIndex = CpuIndex;

	AsmVmPtrStore (&mGuestContextCommonSmm[VmType].GuestContextPerCpu[pCpuIndex].Vmcs);
	Rflags = AsmVmPtrLoad (&mGuestContextCommonSmi.GuestContextPerCpu[pCpuIndex].Vmcs);
	if ((Rflags & (RFLAGS_CF | RFLAGS_ZF)) != 0) {
		DEBUG ((EFI_D_ERROR, "%ld PeRsmHandler - ERROR: AsmVmPtrLoad %016lx : %08x\n", (UINTN)CpuIndex, mGuestContextCommonSmi.GuestContextPerCpu[CpuIndex].Vmcs, Rflags));
		DEBUG((EFI_D_ERROR, "%ld PeRsmHandler - CpuDeadLoop\n"));
		CpuDeadLoop ();
	}

	//STM_PERF_START (Index, 0, "ReadSyncSmmStateSaveArea", "RsmHandler");
	//ReadSyncSmmStateSaveArea (Index);
	//STM_PERF_END (Index, "ReadSyncSmmStateSaveArea", "RsmHandler");

	//#if 0
	DEBUG ((EFI_D_ERROR, "%ld PeRsmHandler start\n", (UINTN)CpuIndex));
	//#endif

	//STM_PERF_END (Index, "OsSmiHandler", "RsmHandler");

	// take care of any cleanup needed

	if(VmType == PE_PERM)
	{
		PostPeVmProc(PE_SUCCESS, CpuIndex, PRESERVE_VM);
	}
	else
	{
		PostPeVmProc(PE_SUCCESS, CpuIndex, RELEASE_VM);
	}
	DEBUG((EFI_D_ERROR, "%ld PeRsmHandler CpuDeadLoop\n", CpuIndex));
	CpuDeadLoop ();

	return ;
}