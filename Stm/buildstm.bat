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

@REM !!!choose DEBUG/RELEASE mode here!!!
@if "%1" == "" (
  @set STM_TARGET=DEBUG
) else (
  @if /I "%1" == "DEBUG" (
    @set STM_TARGET=DEBUG
  ) else (
    @if /I "%1" == "RELEASE" (
      @set STM_TARGET=RELEASE
    ) else (
      @echo "%1" build is UNSUPPORTED. Please use DEBUG or RELEASE.
      goto end
    )
  )
)

@if /I "%STM_TARGET%" == "DEBUG" (
  @set STM_ACTIVE_PLATFORM=StmPkg\StmPkg.dsc
) else (
  @set STM_ACTIVE_PLATFORM=StmPkg\StmPkgRelease.dsc
)

build -p %STM_ACTIVE_PLATFORM% -a IA32 -a X64 -b %STM_TARGET% -t %TOOL_CHAIN_TAG%

@if not %ERRORLEVEL% equ 0 (
  goto end
)
@REM we should not use Stm.efi, because we need refer Stack/Heap size in PE image header.
@REM we have to copy it to Stm.efi to support ITP debug.

copy   %WORKSPACE%\Build\StmPkg\%STM_TARGET%_%TOOL_CHAIN_TAG%\IA32\StmPkg\Core\Stm\DEBUG\Stm.dll  %WORKSPACE%\Build\StmPkg\%STM_TARGET%_%TOOL_CHAIN_TAG%\IA32\StmPkg\Core\Stm\DEBUG\Stm.efi
copy   %WORKSPACE%\Build\StmPkg\%STM_TARGET%_%TOOL_CHAIN_TAG%\X64\StmPkg\Core\Stm\DEBUG\Stm.dll  %WORKSPACE%\Build\StmPkg\%STM_TARGET%_%TOOL_CHAIN_TAG%\X64\StmPkg\Core\Stm\DEBUG\Stm.efi

@REM generate final STM.BIN
GenStm -e --debug 5 -o %WORKSPACE%\Build\StmPkg\%STM_TARGET%_%TOOL_CHAIN_TAG%\IA32\Stm.bin %WORKSPACE%\Build\StmPkg\%STM_TARGET%_%TOOL_CHAIN_TAG%\IA32\StmPkg\Core\Stm\DEBUG\Stm.dll
GenStm -e --debug 5 -o %WORKSPACE%\Build\StmPkg\%STM_TARGET%_%TOOL_CHAIN_TAG%\X64\Stm.bin %WORKSPACE%\Build\StmPkg\%STM_TARGET%_%TOOL_CHAIN_TAG%\X64\StmPkg\Core\Stm\DEBUG\Stm.dll

@echo ################################################################################
@echo # STM.BIN (IA32): %WORKSPACE%\Build\StmPkg\%STM_TARGET%_%TOOL_CHAIN_TAG%\IA32\Stm.bin
@echo # STM.BIN (x64) : %WORKSPACE%\Build\StmPkg\%STM_TARGET%_%TOOL_CHAIN_TAG%\x64\Stm.bin
@echo ################################################################################

:end
