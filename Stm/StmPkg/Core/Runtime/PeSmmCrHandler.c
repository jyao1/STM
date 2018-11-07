#include "StmRuntime.h"
#include "PeStm.h"

extern UINT64 EndTimeStamp;

VOID
SmmCrHandler (
  IN UINT32 Index
  );

void PeCrHandler( IN UINT32 CpuIndex)
{
	DEBUG((EFI_D_ERROR, "%ld PeCrHandler - Entered\n", CpuIndex));
	SmmCrHandler(CpuIndex);   // use the intel handler since it provides the necessary functionality
	DEBUG((EFI_D_ERROR, "%ld PeCrHander - done\n", CpuIndex));
	return;
}