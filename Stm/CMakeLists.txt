
cmake_minimum_required(VERSION 3.13)

project(stm C ASM)

if("${BIOS}" STREQUAL "coreboot")
	add_definitions( -DCOREBOOT32 )
	message("Building for Coreboot")
	endif()

if("${UART}")
	add_definitions( -DUARTBASE=${UART} )
	message("UART Base: ${UART}")
	endif()

if("${HEAPSIZE}")
	message("Custom Heapsize set: ${HEAPSIZE}")
	else()
	set(HEAPSIZE 0x246000)
	message("Default Heapsize set: ${HEAPSIZE}")
	endif()

if("${BUILD}" STREQUAL "release")
	add_definitions( -DRELEASE )
	message("Building with no debug messages")
	else()
	message("Building with debug messages")
	endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Xlinker -Map=stm.map -Os -falign-functions -ffreestanding -s -pie --entry _ModuleEntryPoint -u _ModuleEntryPoint -nostdlib -n -z common-page-size=0x40 -fno-asynchronous-unwind-tables  -fno-jump-tables -fPIC -fno-stack-protector -fno-stack-check -include PcdData.h -T ${PROJECT_SOURCE_DIR}/StmPkg/Core/Stm.lds")

set(CMAKE_ASM_FLAGS "-include BaseAsm.h -fPIC")
set(CMAKE_ASM_CREATE_SHARED_LIBRARY "gcc ${CFLAGS} -o *.o")

include_directories("${PROJECT_SOURCE_DIR}/StmPkg/Core"
	"${PROJECT_SOURCE_DIR}/StmPkg/EDKII/BaseTools/Source/C/Include/X64"
	"${PROJECT_SOURCE_DIR}/StmPkg/EdkII/MdePkg/Include/"
	"${PROJECT_SOURCE_DIR}/StmPkg/EdkII/MdePkg/Include/X64"
        "${PROJECT_SOURCE_DIR}/StmPkg/Include"
        "${PROJECT_SOURCE_DIR}/StmPkg/Include/x64"
	"${PROJECT_SOURCE_DIR}/StmPkg/Core/Runtime"
)

add_subdirectory(StmPkg/EdkII/MdePkg/Library/BaseLib)
add_subdirectory(StmPkg/EdkII/MdePkg/Library/BaseMemoryLib)
add_subdirectory(StmPkg/EdkII/MdePkg/Library/BasePrintLib)
add_subdirectory(StmPkg/EdkII/MdePkg/Library/BaseIoLibIntrinsic)
add_subdirectory(StmPkg/EdkII/MdePkg/Library/BasePciLibPciExpress)
#add_subdirectory(StmPkg/EdkII/MdePkg/Library/BasePciCf8Lib)
add_subdirectory(StmPkg/EdkII/MdePkg/Library/BasePciExpressLib)
add_subdirectory(StmPkg/Library/StmLib)
add_subdirectory(StmPkg/Library/MpSafeDebugLibSerialPort)
add_subdirectory(StmPkg/Library/SimpleSynchronizationLib)
add_subdirectory(StmPkg/EdkII/PcAtChipsetPkg/Library/SerialIoLib)
add_subdirectory(StmPkg/EdkII/MdePkg/Library/BasePcdLibNull)
add_subdirectory(StmPkg/Core)
add_subdirectory(StmPkg/Library/StmPlatformLibNull)

