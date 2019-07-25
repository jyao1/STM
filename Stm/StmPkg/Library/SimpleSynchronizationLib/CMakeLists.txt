add_library(SynchronizationLib STATIC
  
  #X64/InterlockedCompareExchange64.asm | INTEL
  #X64/InterlockedCompareExchange32.asm | INTEL
  
  #X64/InterlockedDecrement.asm | INTEL 
  #X64/InterlockedIncrement.asm | INTEL 
  #Synchronization.c | INTEL 

  X64/GccInline.c 
  SynchronizationGcc.c 
)
 
target_link_libraries(SynchronizationLib PUBLIC 
  PcdLib
  DebugLib
  BaseMemoryLib
)
