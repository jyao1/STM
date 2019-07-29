# Install script for directory: /home/test/Documents/STMPE/STM-PE/Stm

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/test/Documents/STMPE/STM-PE/Stm/build/StmPkg/EdkII/MdePkg/Library/BaseLib/cmake_install.cmake")
  include("/home/test/Documents/STMPE/STM-PE/Stm/build/StmPkg/EdkII/MdePkg/Library/BaseMemoryLib/cmake_install.cmake")
  include("/home/test/Documents/STMPE/STM-PE/Stm/build/StmPkg/EdkII/MdePkg/Library/BasePrintLib/cmake_install.cmake")
  include("/home/test/Documents/STMPE/STM-PE/Stm/build/StmPkg/EdkII/MdePkg/Library/BaseIoLibIntrinsic/cmake_install.cmake")
  include("/home/test/Documents/STMPE/STM-PE/Stm/build/StmPkg/EdkII/MdePkg/Library/BasePciLibPciExpress/cmake_install.cmake")
  include("/home/test/Documents/STMPE/STM-PE/Stm/build/StmPkg/EdkII/MdePkg/Library/BasePciCf8Lib/cmake_install.cmake")
  include("/home/test/Documents/STMPE/STM-PE/Stm/build/StmPkg/EdkII/MdePkg/Library/BasePciExpressLib/cmake_install.cmake")
  include("/home/test/Documents/STMPE/STM-PE/Stm/build/StmPkg/Library/StmLib/cmake_install.cmake")
  include("/home/test/Documents/STMPE/STM-PE/Stm/build/StmPkg/Library/MpSafeDebugLibSerialPort/cmake_install.cmake")
  include("/home/test/Documents/STMPE/STM-PE/Stm/build/StmPkg/Library/SimpleSynchronizationLib/cmake_install.cmake")
  include("/home/test/Documents/STMPE/STM-PE/Stm/build/StmPkg/EdkII/PcAtChipsetPkg/Library/SerialIoLib/cmake_install.cmake")
  include("/home/test/Documents/STMPE/STM-PE/Stm/build/StmPkg/EdkII/MdePkg/Library/BasePcdLibNull/cmake_install.cmake")
  include("/home/test/Documents/STMPE/STM-PE/Stm/build/StmPkg/Core/cmake_install.cmake")
  include("/home/test/Documents/STMPE/STM-PE/Stm/build/StmPkg/Library/StmPlatformLibNull/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/test/Documents/STMPE/STM-PE/Stm/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
