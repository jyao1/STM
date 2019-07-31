  
add_library(StmInit STATIC
  #StmInit.c
  Paging.c
  #Memory.c
  VmcsInit.c
  IoInit.c
  MsrInit.c
  Relocate.c
  PeVmcsInit.c
  x64/AsmStmInit.S
)

target_include_directories(StmInit PUBLIC
	../Runtime
)

target_link_libraries(StmInit PUBLIC
 #BaseLib
 #BaseMemoryLib
 #IoLib
 #PciLib
 SynchronizationLib
 #DebugLib
 #StmLib
 #PcdLib
 #StmPlatformLib
 StmRuntime
 Resource
)
