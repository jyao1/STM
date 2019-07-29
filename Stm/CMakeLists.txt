
cmake_minimum_required(VERSION 3.5)

project(stm C ASM)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdlib -fno-jump-tables -fPIC -fno-stack-protector -fno-stack-check -include PcdData.h")

set(CMAKE_ASM_FLAGS "-include BaseAsm.h")
set(CMAKE_ASM_CREATE_SHARED_LIBRARY "gcc ${CFLAGS}  -o *.o")

include_directories("${PROJECT_SOURCE_DIR}/StmPkg/Core"
	"${PROJECT_SOURCE_DIR}/StmPkg/EdkII/MdePkg/Include/"
	"${PROJECT_SOURCE_DIR}/StmPkg/EdkII/MdePkg/Include/X64"
        "${PROJECT_SOURCE_DIR}/StmPkg/Include"
        "${PROJECT_SOURCE_DIR}/StmPkg/Include/x64"
	"$(PROJECT_SOURCE_DIR}/StmPkg/Core/Runtime")

add_subdirectory(StmPkg/EdkII/MdePkg/Library/BaseLib)
add_subdirectory(StmPkg/EdkII/MdePkg/Library/BaseMemoryLib)
add_subdirectory(StmPkg/EdkII/MdePkg/Library/BasePrintLib)
add_subdirectory(StmPkg/EdkII/MdePkg/Library/BaseIoLibIntrinsic)
add_subdirectory(StmPkg/EdkII/MdePkg/Library/BasePciLibPciExpress)
add_subdirectory(StmPkg/EdkII/MdePkg/Library/BasePciCf8Lib)
add_subdirectory(StmPkg/EdkII/MdePkg/Library/BasePciExpressLib)
add_subdirectory(StmPkg/Library/StmLib)
add_subdirectory(StmPkg/Library/MpSafeDebugLibSerialPort)
add_subdirectory(StmPkg/Library/SimpleSynchronizationLib)
add_subdirectory(StmPkg/EdkII/PcAtChipsetPkg/Library/SerialIoLib)
add_subdirectory(StmPkg/EdkII/MdePkg/Library/BasePcdLibNull)
add_subdirectory(StmPkg/Core)
add_subdirectory(StmPkg/Library/StmPlatformLibNull)

