/** @file
  Implement TPM2 Startup related command.

Copyright (c) 2012, Intel Corporation. All rights reserved. <BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <IndustryStandard/UefiTcgPlatform.h>
#include <Library/Tpm2CommandLib.h>
#include <Library/Tpm2DeviceLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>

#pragma pack(1)

typedef struct {
  TPM2_COMMAND_HEADER  Header;
  UINT16               BytesRequested;
} TPM2_GETRANDOM_COMMAND;

typedef struct {
  TPM2_RESPONSE_HEADER Header;
  TPM2B_DIGEST         RandomBytes;
} TPM2_GETRANDOM_RESPONSE;

#pragma pack()

/**
  Send GetRandom command to TPM2.

  @param BytesRequested        BytesRequested
  @param RandomBytes           RandomBytes

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2GetRandom (
  IN      UINT16             BytesRequested,
     OUT  TPM2B_DIGEST       *RandomBytes
  )
{
  EFI_STATUS                        Status;
  TPM2_GETRANDOM_COMMAND            Cmd;
  TPM2_GETRANDOM_RESPONSE           Res;
  UINT32                            ResultBufSize;

  Cmd.Header.tag         = SwapBytes16(TPM_ST_NO_SESSIONS);
  Cmd.Header.paramSize   = SwapBytes32(sizeof(Cmd));
  Cmd.Header.commandCode = SwapBytes32(TPM_CC_GetRandom);
  Cmd.BytesRequested     = SwapBytes16(BytesRequested);

  ResultBufSize = sizeof(Res);
  Status = Tpm2SubmitCommand (sizeof(Cmd), (UINT8 *)&Cmd, &ResultBufSize, (UINT8 *)&Res);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  RandomBytes->size = SwapBytes16(Res.RandomBytes.size);
  CopyMem (RandomBytes->buffer, Res.RandomBytes.buffer, RandomBytes->size);

  return Status;
}
