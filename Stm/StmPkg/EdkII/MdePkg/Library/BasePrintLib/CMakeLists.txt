add_library(PrintLib STATIC
  PrintLibInternal.c
  PrintLib.c
)

target_link_libraries(PrintLib PUBLIC
  DebugLib
  BaseLib
)
