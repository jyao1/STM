@REM
@REM Copyright (c) 2015 - 2016, Intel Corporation
@REM All rights reserved. This program and the accompanying materials
@REM are licensed and made available under the terms and conditions of the BSD License
@REM which accompanies this distribution.  The full text of the license may be found at
@REM http://opensource.org/licenses/bsd-license.php.
@REM
@REM THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
@REM WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
@REM

@set WORKSPACE=%CD%
@set EDK_TOOLS_PATH=%WORKSPACE%\StmPkg\EdkII\BaseTools
@set WORKSPACE_TOOLS_PATH=%EDK_TOOLS_PATH%
@set BASE_TOOLS_PATH=%WORKSPACE_TOOLS_PATH%
@set PATH=%EDK_TOOLS_PATH%\Bin;%EDK_TOOLS_PATH%\Bin\Win32;%PATH%
@echo           PATH      = %PATH%
@echo      WORKSPACE      = %WORKSPACE%
@echo EDK_TOOLS_PATH      = %EDK_TOOLS_PATH%

@if NOT exist %WORKSPACE%\Conf (
  @mkdir %WORKSPACE%\Conf
)
@if NOT exist %WORKSPACE%\Conf\target.txt (
  @if NOT exist %EDK_TOOLS_PATH%\Conf\target.template (
    @echo Error: target.template is missing at folder %EDK_TOOLS_PATH%\Conf\
  )
  @copy %EDK_TOOLS_PATH%\Conf\target.template %WORKSPACE%\Conf\target.txt > nul
)
@if NOT exist %WORKSPACE%\Conf\tools_def.txt (
  @if NOT exist %EDK_TOOLS_PATH%\Conf\tools_def.template (
    @echo Error: tools_def.template is missing at folder %EDK_TOOLS_PATH%\Conf\
  )
  @copy %EDK_TOOLS_PATH%\Conf\tools_def.template %WORKSPACE%\Conf\tools_def.txt > nul
)
@if NOT exist %WORKSPACE%\Conf\build_rule.txt (
  @if NOT exist %EDK_TOOLS_PATH%\Conf\build_rule.template (
    @echo Error: build_rule.template is missing at folder %EDK_TOOLS_PATH%\Conf\
  )
  @copy %EDK_TOOLS_PATH%\Conf\build_rule.template %WORKSPACE%\Conf\build_rule.txt > nul
)
@if not exist %EDK_TOOLS_PATH%\Source\C\GenStm (
  @xcopy StmPkg\Tool\GenStm %EDK_TOOLS_PATH%\Source\C\GenStm /e/i/y > nul
)
@if not exist %EDK_TOOLS_PATH%\Bin\Win32\GenStm.exe (
  @copy StmPkg\Tool\GenStm\GenStm.exe %EDK_TOOLS_PATH%\Bin\Win32 /y > nul
)


@REM !!!choose your compiler here!!!
@if "%TOOL_CHAIN_TAG%" == "" (
  @if exist "C:\Program Files\Microsoft Visual Studio 8" (
    @REM VS2005:    IA32 Windows OS, IA32 VS2005
    @set TOOL_CHAIN_TAG=VS2005
  )
  @if exist "C:\Program Files\Microsoft Visual Studio 9.0" (
    @REM VS2008:    IA32 Windows OS, IA32 VS2008
    @set TOOL_CHAIN_TAG=VS2008
  )
  @if exist "C:\Program Files\Microsoft Visual Studio 10.0" (
    @REM VS2010:    IA32 Windows OS, IA32 VS2010
    @set TOOL_CHAIN_TAG=VS2010
  )
  @if exist "C:\Program Files\Microsoft Visual Studio 11.0" (
    @REM VS2010:    IA32 Windows OS, IA32 VS2012
    @set TOOL_CHAIN_TAG=VS2012
  )
  @if exist "C:\Program Files\Microsoft Visual Studio 12.0" (
    @REM VS2010:    IA32 Windows OS, IA32 VS2013
    @set TOOL_CHAIN_TAG=VS2013
  )
  @if exist "C:\Program Files\Microsoft Visual Studio 14.0" (
    @REM VS2010:    IA32 Windows OS, IA32 VS2015
    @set TOOL_CHAIN_TAG=VS2015
  )
  @if exist "C:\Program Files (x86)\Microsoft Visual Studio 8" (
    @REM VS2005x86: X64  Windows OS, IA32 VS2005
    @set TOOL_CHAIN_TAG=VS2005x86
  )
  @if exist "C:\Program Files (x86)\Microsoft Visual Studio 9.0" (
    @REM VS2008x86: X64  Windows OS, IA32 VS2008
    @set TOOL_CHAIN_TAG=VS2008x86
  )
  @if exist "C:\Program Files (x86)\Microsoft Visual Studio 10.0" (
    @REM VS2010x86: X64  Windows OS, IA32 VS2010
    @set TOOL_CHAIN_TAG=VS2010x86
  )
  @if exist "C:\Program Files (x86)\Microsoft Visual Studio 11.0" (
    @REM VS2010x86: X64  Windows OS, IA32 VS2012
    @set TOOL_CHAIN_TAG=VS2012x86
  )
  @if exist "C:\Program Files (x86)\Microsoft Visual Studio 12.0" (
    @REM VS2010x86: X64  Windows OS, IA32 VS2013
    @set TOOL_CHAIN_TAG=VS2013x86
  )
  @if exist "C:\Program Files (x86)\Microsoft Visual Studio 14.0" (
    @REM VS2010x86: X64  Windows OS, IA32 VS2015
    @set TOOL_CHAIN_TAG=VS2015x86
  )
)

@echo ################################################################################
@echo # Tool chain: %TOOL_CHAIN_TAG%
@echo ################################################################################
