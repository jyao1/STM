add_library(PciLib SHARED
  PciLib.c
)

target_link_libraries(PciLib PUBLIC
  PciExpressLib
)

