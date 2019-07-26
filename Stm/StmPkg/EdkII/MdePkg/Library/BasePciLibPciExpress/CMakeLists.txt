add_library(PciLib STATIC
  PciLib.c
)

target_link_libraries(PciLib PUBLIC
  PciExpressLib
)

