add_library(BasePciCf8Lib STATIC
  PciCf8Lib.c
)

target_link_libraries(BasePciCf8Lib PUBLIC
  BaseLib
  DebugLib
  IoLib
  PcdData
)
