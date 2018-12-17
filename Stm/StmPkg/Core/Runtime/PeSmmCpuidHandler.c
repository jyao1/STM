/** @file

CPUID Handler

This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/
#include "StmRuntime.h"
#include "PeStm.h"

extern UINT64 EndTimeStamp;

void PeCpuidHandler( IN UINT32 CpuIndex)
{
	EndTimeStamp = AsmReadTsc();
	DEBUG((EFI_D_ERROR, "%ld PE - CPUID Handler not implemented\n", CpuIndex));
	DEBUG((EFI_D_ERROR, "%ld PeCpuidHandler - CpuDeadLoop\n", CpuIndex));
	CpuDeadLoop();
	return;

}