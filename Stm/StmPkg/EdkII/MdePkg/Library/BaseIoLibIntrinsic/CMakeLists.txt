add_library(IoLib STATIC
  IoLibMmioBuffer.c
  IoHighLevel.c

  IoLibGcc.c 
  IoLib.c
)

target_link_libraries(IoLib PUBLIC
  DebugLib
  BaseLib
)
