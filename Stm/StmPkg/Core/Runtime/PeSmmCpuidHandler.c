
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