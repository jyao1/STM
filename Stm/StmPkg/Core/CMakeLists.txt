add_subdirectory(Runtime)
add_subdirectory(Init)
 
add_library(Stm MODULE
  Cpu.c
  Dump.c
  VmcsRecord.c
  StmPerformance.c
  Init/Memory.c
  Init/StmInit.c
)

add_library(Resource STATIC
  StmResource.c
)

add_library(EventLog STATIC
  EventLog.c
)

add_library(EptLib STATIC
  Init/EptInit.c
  Runtime/SmmEptHandler.c
  Runtime/PeEptHandler.c
)

target_include_directories(EptLib PUBLIC
  Runtime
  Init
)

target_link_libraries(Stm PUBLIC 
 #BaseLib 
 #BaseMemoryLib 
 #IoLib 
 PciLib 
 #SynchronizationLib 
 #DebugLib 
 StmLib 
 PcdLib
 StmPlatformLib
 StmRuntime
 Resource
 EptLib
 StmInit
 SmmExcept
)

target_include_directories(Stm PUBLIC
	Runtime)

target_include_directories(Resource PUBLIC
	Runtime)

add_library(PcdData STATIC PcdData.c)

#GCC:*_*_X64_DLINK_FLAGS  == -o $(DEBUG_DIR)/Stm.dll -nostdlib -n -q --gc-sections -z common-page-size=0x40 --entry _ModuleEntryPoint -u _ModuleEntryPoint -Map $(DEST_DIR_DEBUG)/$(BASE_NAME).map -m elf_x86_64 --oformat=elf64-x86-64 --defsym=PECOFF_HEADER_SIZE=0x228 #--script=$(MODULE_DIR)/Stm.lds
#GCC:*_*_IA32_DLINK_FLAGS == -o $(DEBUG_DIR)/Stm.dll -nostdlib -n -q --gc-sections -z common-page-size=0x40 --entry _ModuleEntryPoint -u _ModuleEntryPoint -Map $(DEST_DIR_DEBUG)/$(BASE_NAME).map -m elf_i386   --oformat=elf32-i386   --defsym=PECOFF_HEADER_SIZE=0x220 #--script=$(MODULE_DIR)/Stm.lds
