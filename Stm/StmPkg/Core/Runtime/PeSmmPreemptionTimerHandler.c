
#include "StmRuntime.h"
#include "PeStm.h"
#include "PeLoadVm.h"

extern UINT64 EndTimeStamp;
extern UINT32 PostPeVmProc(UINT32 rc, UINT32 CpuIndex, UINT32 mode);

void PePreEmptionTimerHandler(IN UINT32 CpuIndex)
{
	EndTimeStamp = AsmReadTsc();
	DEBUG((EFI_D_ERROR, "%ld - PE/VM terminated because of a Premption Timer Runout \n", CpuIndex));
	DumpVmcsAllField();
	PostPeVmProc(PE_SUCCESS, CpuIndex, RELEASE_VM);
}