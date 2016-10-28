================================================================================
                      FEATURES
================================================================================
A. STM feature
1. STM binary need to be integreated to STM-BIOS.
2. STM is validated on MinnowMax platform X64 version BIOS, boot to UEFI Yacto.
3. STM package provides STM sample implementation for "STM User Guide, Revision 1.0".
   1) STM image generation.
   2) STM launch and STM teardown.
   3) VMCALL interface between BIOS and STM, such as exception handler.
   4) VMCALL interface between MLE and STM, such as MLE resource protection.
   5) SMRAM context generation.
   More detail is described in:
     https://firmware.intel.com/sites/default/files/A_Tour_Beyond_BIOS_Launching_STM_to_Monitor_SMM_in_EFI_Developer_Kit_II.pdf
4. High level structure for StmPkg:
   Core    -> The main part of STM, including Init part and Runtime part.
   EdkII   -> The EdkII libraries used by STM, including BaseTools, MdePkg, and
              PcAtChipsetPkg. They are lightweight version.
   Include -> STM include files.
   Library -> STM specific libraries, including MpSafeDebugLibSerialPort, 
              SimpleSynchronizationLib, and StmLib.
   Tool    -> STM specific build tool, including GenStm.

B. STM TEST Module feature
1. FRM binary is a tiny VMM to launch STM in UEFI environment for test purpose.
   It shows the concept on how VMM interacts with STM.
   1). FRM can also do TXT launch, if TXT is enabled and SINIT_ACM is provided by BIOS.

2. StmService provides the API between VMM and STM.
3. FrmLoader is the UEFI driver to load FRM binary.
   More detail for FRM is described in:
     https://firmware.intel.com/sites/default/files/A_Tour_Beyond_BIOS_Launching_VMM_in_EFI_Developer_Kit_II_0.pdf
4. DmaPkg is a sample solution to setup VTd for test purpose.
   It shows the concept on how to enable DMA protection in UEFI.
   More detail for DmaPkg is described in:
     https://firmware.intel.com/sites/default/files/resources/A_Tour_Beyond_BIOS_Using_Intel_VT-d_for_DMA_Protection.pdf

================================================================================
                                 HOW TO BUILD
================================================================================
A. How to build STM.
0. Install Visual Studio 2015.
1. Build STM package.
   1) Copy Stm dir to target dir, e.g. c:\StmCode
      The final directory layout is like below:
        c:\StmCode\StmPkg\Core
        c:\StmCode\StmPkg\EdkII
        ...
   2) Open command window, goto target dir, e.g. c:\StmCode
   3) Type "preparestm.bat" to prepare environment, this need run only once.
   4) Type "buildstm.bat" to build STM image.
      1) If user want debug build, please use "buildstm.bat" or "buildstm.bat DEBUG".
      2) If user want release build, please use "buildstm.bat RELEASE".
   5) NOTE: The default STM build uses a NULL instance StmPlatformLib.
      A platform BIOS may need override StmPlatformLib to handle some special MSR access,
      which must happen in VMX Root Mode if STM is enabled.
      If so, this platform owner need override the StmPlatformLib in StmPkg.dsc.
   6) NOTE: There might be a case that a user need enlarge STM heap size. If so, the user
      can modify StmPkg\Core\Stm.inf
         /STACK:0x8000,0x8000 /HEAP:0x140000,0x140000

2. Build STM tool (optional)
   1) Same as above
   2) Same as above
   3) Same as above
   4) Type "buildstmtool.bat" to build STM tool.

B. How to build STM test module.
1. Download EDKII repository.
2. Build STM test package.
   1) Copy FrmPkg to target dir, e.g. c:\EDKII
      The final directory layout is like below:
        c:\EDKII\FrmPkg\Core
   2) Open command window, goto target dir, e.g. c:\EDKII
   3) Type "edksetup.bat" to setup EDKII build env.
   4) Type "build -p FrmPkg\FrmPkg.dsc -a X64 -t VS2015x86" to build binary.
   
C. How to build STM BIOS.
1. Download EDKII repository and MinnowMax binaries.
2. Build MinnowMax STM BIOS.
   1) Copy BIOS dir to target dir, e.g. c:\EDKII
      The final directory layout is like below:
        c:\EDKII\StmCpuPkg
        c:\EDKII\StmPlatformSamplePkg
   2) Copy Override\Vlv2BinaryPkg to Vlv2BinaryPkg.
        This MUST be done because default Vlv2BinaryPkg has limitation to block STM running.
   3) Update Vlv2TbltDevicePkg\PlatformPkgX64.dsc:
        Add "StmPlatformSamplePkg/MsegSmramPei/MsegSmramPei.inf" to "[Components.IA32]" section.
        Add "StmCpuPkg/PiSmmCpuDxeSmm/PiSmmCpuDxeSmm.inf {
              <LibraryClasses>
                SmmCpuPlatformHookLib|StmCpuPkg/Library/SmmCpuPlatformHookLibNull/SmmCpuPlatformHookLibNull.inf
                SmmCpuFeaturesLib|StmCpuPkg/Library/SmmCpuFeaturesLib/SmmCpuFeaturesLib.inf
            }" to "[Components.X64]" section.
        Add "StmPlatformSamplePkg/StmPlatformSmm/StmPlatformSmm.inf" to "[Components.X64]" section.
        Add "StmPlatformSamplePkg/Compatibility/EndOfDxeOnExitPmAuthThunk/EndOfDxeOnExitPmAuthThunk.inf" to "[Components.X64]" section.
        Add "StmPlatformSamplePkg/Compatibility/SmmCpuSaveStateProtocolOnSmst2/SmmCpuSaveStateProtocolOnSmst2.inf" to "[Components.X64]" section.
   4) Update Vlv2TbltDevicePkg\PlatformPkg.fdf:
        Add "INF StmPlatformSamplePkg/MsegSmramPei/MsegSmramPei.inf" to "[FV.FVRECOVERY]" section.
        Replace "INF RuleOverride = BINARY $(PLATFORM_BINARY_PACKAGE)/$(DXE_ARCHITECTURE)$(TARGET)/$(DXE_ARCHITECTURE)/PiSmmCpuDxeSmm.inf"
          by "INF StmCpuPkg/PiSmmCpuDxeSmm/PiSmmCpuDxeSmm.inf" in "[FV.MAIN]" section.
        Add "INF StmPlatformSamplePkg/StmPlatformSmm/StmPlatformSmm.inf" to "[FV.MAIN]" section.
        Add "INF StmPlatformSamplePkg/Compatibility/EndOfDxeOnExitPmAuthThunk/EndOfDxeOnExitPmAuthThunk.inf" to "[FV.MAIN]" section.
        Add "INF StmPlatformSamplePkg/Compatibility/SmmCpuSaveStateProtocolOnSmst2/SmmCpuSaveStateProtocolOnSmst2.inf" to "[FV.MAIN]" section.
        Add "FILE FREEFORM = PCD(gStmPlatformTokenSpaceGuid.PcdStmBinFile) {
               SECTION RAW = StmPlatformSamplePkg/StmBin/X64$(TARGET)/Stm.bin
            }" to "[FV.MAIN]" section.
        Add "INF USE=X64 StmPlatformSamplePkg/TestBin/X64$(TARGET)/Frm.inf" to "[FV.MAIN]" section.
        Add "INF USE=X64 StmPlatformSamplePkg/TestBin/X64$(TARGET)/StmService.inf" to "[FV.MAIN]" section.
        Add "INF USE=X64 StmPlatformSamplePkg/TestBin/X64$(TARGET)/FrmLoader.inf" to "[FV.MAIN]" section.
   5) Build MinnowMax BIOS accroding to MinnowMax release notes.
   6) NOTE:
      A platform BIOS may need override StmPlatformSamplePkg/StmPlatformSmm/StmPlatformResource.c
      to include the resource needed by platform SMI handlers.
      To be specific, if a platform overrides StmPlatformLib to access some special MSR, these MSR
      must be in the resource list.
   7) NOTE:
      A platform BIOS CPU driver may put APs to MWAIT state and assume no one touch this APs.
      This assumption is wrong, because FRM need wait up APs. FrmLoader uses mMpService->EnableDisableAP()
      to disable/enable AP.
      The CPU driver need use INIT-SIPI-SIPI to wake up AP, after the AP is disabled/enabled.
   8) NOTE:
      If STM need more MSEG size, the user need adjust PcdCpuMsegSize in platform.dsc.

D. How to build STM BIOS with TXT capability.
   NOTE: MinnowMax does not have TXT support. Below steps are for a platform which has TXT support.
   1) Make sure the platform BIOS has TXT support.
   2) Follow "C" to add required STM components.
   3) Update Platform.fdf:
      Add "FILE FREEFORM = 2CB32E6A-B0DB-496D-BD8D-2C890D5A73B8 {
             SECTION RAW = PlatformPkg/Bin/SINIT_ACM.bin
          }" to "[FV.MAIN]" section.
   4) Build platform BIOS.

================================================================================
                                  KNOWN LIMITATION
================================================================================
1. STM Domain type is NOT fully implemented.
   The purpose of StmPkg is to show the STM concept.
   Please do not include it in the production without full validation.
2. GCC build is not fully supported.
3. The FRM does not support S3, and FRM TXT support does not have a complete trusted boot chain.
   The purpose of FrmPkg is to validate STM with or without TXT support.
   Please do not include it in the production without full validation.

[END OF RELEASE NOTES]
