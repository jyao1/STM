add_library(DebugLib STATIC
  DebugLib.c
)

target_link_libraries(DebugLib PUBLIC
  SerialPortLib
  BaseMemoryLib
  PcdLib
  PrintLib
  BaseLib
  SynchronizationLib
)

