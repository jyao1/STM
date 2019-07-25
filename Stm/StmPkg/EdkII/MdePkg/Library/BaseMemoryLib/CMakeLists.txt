add_library(BaseMemoryLib STATIC
  SetMem.c
  ScanMem64Wrapper.c
  ScanMem32Wrapper.c
  ScanMem16Wrapper.c
  ScanMem8Wrapper.c
  ZeroMemWrapper.c
  CompareMemWrapper.c
  SetMem64Wrapper.c
  SetMem32Wrapper.c
  SetMem16Wrapper.c
  SetMemWrapper.c
  CopyMemWrapper.c
  MemLibGeneric.c
  MemLibGuid.c
  CopyMem.c
)

target_link_libraries(BaseMemoryLib PUBLIC  
  DebugLib
  BaseLib
)

