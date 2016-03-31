@REM
@REM Copyright (c) 2015, Intel Corporation
@REM All rights reserved. This program and the accompanying materials
@REM are licensed and made available under the terms and conditions of the BSD License
@REM which accompanies this distribution.  The full text of the license may be found at
@REM http://opensource.org/licenses/bsd-license.php.
@REM
@REM THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
@REM WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
@REM

@pushd "%BASE_TOOLS_PATH%"\Source\C
nmake
@popd

@echo STM tool build finish!
@echo ################################################################################
@echo # GENSTM.EXE : %BASE_TOOLS_PATH%\Bin\Win32\GenStm.exe
@echo ################################################################################

