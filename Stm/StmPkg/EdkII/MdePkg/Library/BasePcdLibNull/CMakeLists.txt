add_library(PcdLib STATIC
	PcdLib.c
)

target_link_libraries(PcdLib PUBLIC
  DebugLib
  BaseMemoryLib
)

target_include_directories(PcdLib PRIVATE
        ../../Include/
        ../../Include/X64
)
