#include "StmRuntime.h"
#include "PeStm.h"

extern UINT64 EndTimeStamp;

void PeCrHandler( IN UINT32 CpuIndex)
{
	EndTimeStamp = AsmReadTsc();
	DEBUG((EFI_D_ERROR, "%ld PeCrHandler - CR Handler not implemented\n", CpuIndex));
	DEBUG((EFI_D_ERROR, "%ld PeCrHandler - CpuDeadLoop\n", CpuIndex));
	CpuDeadLoop();
	return;
}