add_library(SerialPortLib STATIC
  SerialPortLib.c
)

target_link_libraries(SerialPortLib PUBLIC 
	BaseLib
	IoLib
)
