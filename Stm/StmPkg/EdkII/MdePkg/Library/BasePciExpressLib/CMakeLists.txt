add_library(PciExpressLib SHARED
  PciExpressLib.c
)

target_link_libraries(PciExpressLib PUBLIC
	BaseLib
	PcdLib
	DebugLib
	IoLib
	PcdData
)

