add_library(PciExpressLib STATIC
  PciExpressLib.c
)

target_link_libraries(PciExpressLib PUBLIC
	BaseLib
	PcdLib
	DebugLib
	IoLib
	PcdData
)

