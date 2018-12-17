
/** @file

Triple Fault Handler

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

void PeTripleFaultHandler( IN UINT32 CpuIndex)
{
	EndTimeStamp = AsmReadTsc();
	DEBUG((EFI_D_ERROR, "%ld PeTripleFaultHandler - Triple Fault occured in VM/PE - terminating\n", CpuIndex));
	PostPeVmProc(PE_VM_TRIPLE_FAULT, CpuIndex, PRESERVE_VM);        // bring the VM down
	return;

}