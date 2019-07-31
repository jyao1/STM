
add_library(SmmExcept STATIC
	 x64/SmmException.c
	 x64/Exception.S
)

target_include_directories(SmmExcept PUBLIC
	.
)

add_library(StmRuntime STATIC
  SmmMp.c
  SmmHandler.c
  SmmStateSync.c
  SmmSetup.c
  SmmTearDown.c
  SmmRsmHandler.c
  SmmVmcallHandler.c
  SmmExceptionHandler.c
  SmmCacheHandler.c
  SmmCpuidHandler.c
  SmmCrHandler.c
  #SmmEptHandler.c
  SmmIoHandler.c
  SmmMsrHandler.c
  SmmTaskSwitchHandler.c
  SmiHandler.c
  SmiEventHandler.c
  SmiVmcallHandler.c
  StmTearDown.c
  StmExceptionHandler.c
  PageTable.c
  #PeEptHandler.c
  PeSmmHandler.c
  PeSmmVmcallHandler.c
  PeSmmRsmHandler.c
  PeSmmIoHandler.c
  PeSmmBadGuestStateHandler.c
  PeSmmMsrHandler.c
  PeSmmCrHandler.c
  PeSmmExceptionHandler.c
  PeSmmCpuidHandler.c
  PeSmmPreemptionTimerHandler.c
  PeSmmTripleFaultHandler.c
  PeSmiVmcallHandler.c
  PeSmiHandler.c
  PeVmxState.c
  PeApicHandler.c
  PePciHandler.c
  PeLoadVm.c
  PeVmcsInit.c
  VmcsMapper.c
  #x64/VmExit.asm
  x64/VmExit.S
  #x64/PeVmExit.asm
  x64/PeVmExit.S
  #x64/Exception.asm
  #x64/Exception.S
  #x64/SmmException.c
  x64/SmmStateSyncIa32eGpr.c
)

target_include_directories(StmRuntime PUBLIC
	../Init
	.)

target_link_libraries(StmRuntime PUBLIC
 SynchronizationLib
 StmPlatformLib
 EventLog
 StmLib
)

