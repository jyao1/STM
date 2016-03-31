#
# Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
# This program and the accompanying materials
# are licensed and made available under the terms and conditions of the BSD License
# which accompanies this distribution.  The full text of the license may be found at
# http://opensource.org/licenses/bsd-license.php.
# 
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#

build -p StmPkg/StmPkg.dsc -a IA32 -a X64 -t UNIXGCC

BaseTools/Source/C/bin/GenStm -e --debug 5 -o Build/StmPkg/DEBUG_UNIXGCC/IA32/Stm.bin Build/StmPkg/DEBUG_UNIXGCC/IA32/StmPkg/Core/Stm/DEBUG/Stm.dll
BaseTools/Source/C/bin/GenStm -e --debug 5 -o Build/StmPkg/DEBUG_UNIXGCC/X64/Stm.bin Build/StmPkg/DEBUG_UNIXGCC/X64/StmPkg/Core/Stm/DEBUG/Stm.dll

