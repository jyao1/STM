## @file StmPkg.dsc
# 
# Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.<BR>
# This program and the accompanying materials
# are licensed and made available under the terms and conditions of the BSD License
# which accompanies this distribution.  The full text of the license may be found at
# http://opensource.org/licenses/bsd-license.php.
#
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
##

[Defines]
  PLATFORM_NAME                  = StmPkg
  PLATFORM_GUID                  = CF6FFE1C-0F2B-470e-B708-56CF01FC4A00
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/StmPkg
  SUPPORTED_ARCHITECTURES        = IA32|X64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT

[LibraryClasses]
  BaseLib|StmPkg/EdkII/MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|StmPkg/EdkII/MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  PrintLib|StmPkg/EdkII/MdePkg/Library/BasePrintLib/BasePrintLib.inf
  IoLib|StmPkg/EdkII/MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
#  PciLib|StmPkg/EdkII/MdePkg/Library/BasePciLibCf8/BasePciLibCf8.inf
  PciLib|StmPkg/EdkII/MdePkg/Library/BasePciLibPciExpress/BasePciLibPciExpress.inf
  PciCf8Lib|StmPkg/EdkII/MdePkg/Library/BasePciCf8Lib/BasePciCf8Lib.inf
  PciExpressLib|StmPkg/EdkII/MdePkg/Library/BasePciExpressLib/BasePciExpressLib.inf
  PcdLib|StmPkg/EdkII/MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  DebugLib|StmPkg/EdkII/MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf

[LibraryClasses.common.USER_DEFINED]
  StmLib|StmPkg/Library/StmLib/StmLib.inf
  StmPlatformLib|StmPkg/Library/StmPlatformLibNull/StmPlatformLibNull.inf
  DebugLib|StmPkg/Library/MpSafeDebugLibSerialPort/MpSafeDebugLibSerialPort.inf
  SynchronizationLib|StmPkg/Library/SimpleSynchronizationLib/SimpleSynchronizationLib.inf
  SerialPortLib|StmPkg/EdkII/PcAtChipsetPkg/Library/SerialIoLib/SerialIoLib.inf

[Components]
  StmPkg/Core/Stm.inf {
    <LibraryClasses>
      BaseLib|StmPkg/EdkII/MdePkg/Library/BaseLib/BaseLib.inf
      BaseMemoryLib|StmPkg/EdkII/MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
      PrintLib|StmPkg/EdkII/MdePkg/Library/BasePrintLib/BasePrintLib.inf
      IoLib|StmPkg/EdkII/MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
#      PciLib|StmPkg/EdkII/MdePkg/Library/BasePciLibCf8/BasePciLibCf8.inf
      PciLib|StmPkg/EdkII/MdePkg/Library/BasePciLibPciExpress/BasePciLibPciExpress.inf
      PciCf8Lib|StmPkg/EdkII/MdePkg/Library/BasePciCf8Lib/BasePciCf8Lib.inf
      PciExpressLib|StmPkg/EdkII/MdePkg/Library/BasePciExpressLib/BasePciExpressLib.inf
      PcdLib|StmPkg/EdkII/MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
      StmLib|StmPkg/Library/StmLib/StmLib.inf
      DebugLib|StmPkg/Library/MpSafeDebugLibSerialPort/MpSafeDebugLibSerialPort.inf
      SynchronizationLib|StmPkg/Library/SimpleSynchronizationLib/SimpleSynchronizationLib.inf
      SerialPortLib|StmPkg/EdkII/PcAtChipsetPkg/Library/SerialIoLib/SerialIoLib.inf
  }

[PcdsFixedAtBuild]
  gEfiStmPkgTokenSpaceGuid.PcdDebugPropertyMask|0x0f
  gEfiStmPkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000040
  #gEfiStmPkgTokenSpaceGuid.PcdReportStatusCodePropertyMask|0x06
  gEfiStmPkgTokenSpaceGuid.PcdPerformanceLibraryPropertyMask|0x00
