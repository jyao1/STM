/** @file

PE Bad guest state handler

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

void PeBadGuestStateHandler( IN UINT32 CpuIndex)
{
	EndTimeStamp = AsmReadTsc();
	DEBUG((EFI_D_ERROR, "%ld PeBadGuestStateHandler - PE VmLaunch attempted with invalid guest state - VmInstruction Error field: %x\n",
		CpuIndex, VmRead32(VMCS_32_RO_VM_INSTRUCTION_ERROR_INDEX)));
	DEBUG((EFI_D_ERROR, "%ld PeBadGuestStateHandler - Exit qualification: %x\n", CpuIndex, VmReadN (VMCS_N_RO_EXIT_QUALIFICATION_INDEX)));

	PostPeVmProc(PE_VMLAUNCH_ERROR, CpuIndex, RELEASE_VM);
	// should not return

	DEBUG((EFI_D_ERROR, "%ld PeBadGuestStateHandler - CpuDeadLoop\n", CpuIndex));
	CpuDeadLoop ();

	return ;

}
