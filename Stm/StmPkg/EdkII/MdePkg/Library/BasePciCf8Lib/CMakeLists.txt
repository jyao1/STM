add_library(BasePciCf8Lib SHARED
  PciCf8Lib.c
)

target_link_libraries(BasePciCf8Lib PUBLIC
  BaseLib
  DebugLib
  IoLib
  PcdData
)
