/** @file
  Framework SMM CPU Save State protocol on SMST2 Thunk.

  This thunk driver produces Framework SMM CPU Save Status Protocol on SMST2.
  Some ECP platforms are still using Framework SMM CPU Save Status Protocol.

  Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials are licensed and made available under 
  the terms and conditions of the BSD License that accompanies this distribution.  
  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.                                          

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiSmm.h>
#include <Protocol/SmmCpuSaveState.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/SmmServicesTableLib.h>

///
/// SMM CPU Save State Protocol instance
///
EFI_SMM_CPU_SAVE_STATE_PROTOCOL  mSmmCpuSaveState = {
  NULL
};

/**
  Entry point of PI SMM Status Code Protocol on Framework SMM Status Code Protocol thunk driver.

  @param  ImageHandle       The firmware allocated handle for the EFI image.
  @param  SystemTable       A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS       The entry point is executed successfully.

**/
EFI_STATUS
EFIAPI
SmmCpuSaveStateProtocolOnSmst2Main (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS     Status;
  EFI_HANDLE     Handle;

  mSmmCpuSaveState.CpuSaveState = (EFI_SMM_CPU_STATE **)gSmst->CpuSaveState;

  Handle = NULL;
  Status = SystemTable->BootServices->InstallMultipleProtocolInterfaces(
                                        &Handle,
                                        &gEfiSmmCpuSaveStateProtocolGuid,
                                        &mSmmCpuSaveState,
                                        NULL
                                        );
  ASSERT_EFI_ERROR(Status);
  return EFI_SUCCESS;
}
