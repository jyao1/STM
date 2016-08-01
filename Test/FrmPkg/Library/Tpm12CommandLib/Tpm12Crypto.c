/** @file
  Implement TPM1.2 Crypto related commands.

Copyright (c) 2016, Intel Corporation. All rights reserved. <BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiPei.h>
#include <Library/Tpm12CommandLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/Tpm12DeviceLib.h>

#define TPMCMDBUFLENGTH             1024         // TPM command/reponse length

#pragma pack(1)

typedef struct {
  TPM_RQU_COMMAND_HDR   Hdr;
} TPM_CMD_SHA1_START;

typedef struct {
  TPM_RSP_COMMAND_HDR   Hdr;
  UINT32                MaxNumBytes;
} TPM_RSP_SHA1_START;

typedef struct {
  TPM_RQU_COMMAND_HDR   Hdr;
  UINT32                NumBytes;
  UINT8                 Data[TPMCMDBUFLENGTH];
} TPM_CMD_SHA1_UPDATE;

typedef struct {
  TPM_RSP_COMMAND_HDR   Hdr;
} TPM_RSP_SHA1_UPDATE;

typedef struct {
  TPM_RQU_COMMAND_HDR   Hdr;
  UINT32                HashDataSize;
  UINT8                 Data[64];
} TPM_CMD_SHA1_COMPLETE;

typedef struct {
  TPM_RSP_COMMAND_HDR   Hdr;
  TPM_DIGEST            HashValue;
} TPM_RSP_SHA1_COMPLETE;

typedef struct {
  TPM_RQU_COMMAND_HDR   Hdr;
  UINT32                BytesRequested;
} TPM_CMD_GET_RANDOM;

typedef struct {
  TPM_RSP_COMMAND_HDR   Hdr;
  TPM_DIGEST            HashValue;
  UINT32                RandomBytesSize;
  UINT8                 RandomBytes[TPMCMDBUFLENGTH];
} TPM_RSP_GET_RANDOM;

#pragma pack()

/**

  This function run TPM command Sha1 start.

  @param  MaxNumBytes      max number bytes

  @retval EFI_SUCCESS        Command send successfully
  @retval EFI_DEVICE_ERROR   Something wrong with TPM

**/
EFI_STATUS
Tpm12Sha1Start(
  OUT  UINT32                    *MaxNumBytes
  )
{
  EFI_STATUS          Status;
  TPM_CMD_SHA1_START  Command;
  TPM_RSP_SHA1_START  Response;
  UINT32              Length;

  //
  // send Tpm command TPM_ORD_SHA1Start
  //
  Command.Hdr.tag = SwapBytes16(TPM_TAG_RQU_COMMAND);
  Command.Hdr.paramSize = SwapBytes32(sizeof(Command));
  Command.Hdr.ordinal = SwapBytes32(TPM_ORD_SHA1Start);
  Length = sizeof(Response);
  Status = Tpm12SubmitCommand(sizeof(Command), (UINT8 *)&Command, &Length, (UINT8 *)&Response);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  if (Response.Hdr.returnCode != 0) {
    return EFI_DEVICE_ERROR;
  }

  *MaxNumBytes = Response.MaxNumBytes;

  return Status;
}

/**

  This function run TPM command Sha1 update.

  @param  NumBytes         to be hashed data bytes
  @param  HashData         to be hashed data

  @retval EFI_SUCCESS        Command send successfully
  @retval EFI_DEVICE_ERROR   Something wrong with TPM

**/
EFI_STATUS
Tpm12Sha1Update(
  IN   UINT32                    NumBytes,
  IN   UINT8                     *HashData
  )
{
  EFI_STATUS          Status;
  TPM_CMD_SHA1_UPDATE Command;
  TPM_RSP_SHA1_UPDATE Response;
  UINT32              Length;

  ASSERT(NumBytes <= sizeof(Command.Data));

  //
  // send Tpm command TPM_ORD_SHA1Update
  //
  Command.Hdr.tag = SwapBytes16(TPM_TAG_RQU_COMMAND);
  Command.Hdr.paramSize = SwapBytes32(sizeof(Command.Hdr) + sizeof(UINT32) + NumBytes);
  Command.Hdr.ordinal = SwapBytes32(TPM_ORD_SHA1Update);
  Command.NumBytes = SwapBytes32(NumBytes);
  CopyMem (Command.Data, HashData, NumBytes);
  Length = sizeof(Response);
  Status = Tpm12SubmitCommand(sizeof(Command.Hdr) + sizeof(UINT32) + NumBytes, (UINT8 *)&Command, &Length, (UINT8 *)&Response);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  if (Response.Hdr.returnCode != 0) {
    return EFI_DEVICE_ERROR;
  }

  return Status;
}

/**

  This function run TPM command Sha1 complete.

  @param  Locality         TPM locality
  @param  NumBytes         to be hashed data bytes
  @param  HashData         to be hashed data
  @param  Digest           digest of data

  @retval EFI_SUCCESS        Command send successfully
  @retval EFI_DEVICE_ERROR   Something wrong with TPM

**/
EFI_STATUS
Tpm12Sha1Complete(
  IN   UINT32                    NumBytes,
  IN   UINT8                     *HashData,
  OUT  TPM_DIGEST                *Digest
  )
{
  EFI_STATUS             Status;
  TPM_CMD_SHA1_COMPLETE  Command;
  TPM_RSP_SHA1_COMPLETE  Response;
  UINT32                 Length;

  ASSERT(NumBytes <= sizeof(Command.Data));

  //
  // send Tpm command TPM_ORD_SHA1Complete
  //
  Command.Hdr.tag = SwapBytes16(TPM_TAG_RQU_COMMAND);
  Command.Hdr.paramSize = SwapBytes32(sizeof(Command.Hdr) + sizeof(UINT32) + NumBytes);
  Command.Hdr.ordinal = SwapBytes32(TPM_ORD_SHA1Complete);
  Command.HashDataSize = SwapBytes32(NumBytes);
  CopyMem(Command.Data, HashData, NumBytes);
  Length = sizeof(Response);
  Status = Tpm12SubmitCommand(sizeof(Command.Hdr) + sizeof(UINT32) + NumBytes, (UINT8 *)&Command, &Length, (UINT8 *)&Response);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  if (Response.Hdr.returnCode != 0) {
    return EFI_DEVICE_ERROR;
  }

  CopyMem (Digest, &Response.HashValue, sizeof(*Digest));

  return Status;
}

/**

  This function run TPM command get random.

  @param  DataLen          random data length
  @param  Data             random data

  @retval EFI_SUCCESS        Command send successfully
  @retval EFI_DEVICE_ERROR   Something wrong with TPM

**/
EFI_STATUS
Tpm12GetRandom (
  IN   UINT32                    DataLen,
  OUT  UINT8                     *Data
  )
{
  EFI_STATUS          Status;
  TPM_CMD_GET_RANDOM  Command;
  TPM_RSP_GET_RANDOM  Response;
  UINT32              Length;

  ASSERT(DataLen <= sizeof(Response.RandomBytes));

  //
  // send Tpm command TPM_ORD_SHA1Complete
  //
  Command.Hdr.tag = SwapBytes16(TPM_TAG_RQU_COMMAND);
  Command.Hdr.paramSize = SwapBytes32(sizeof(Command));
  Command.Hdr.ordinal = SwapBytes32(TPM_ORD_GetRandom);
  Command.BytesRequested = SwapBytes32(DataLen);
  Length = sizeof(Response);
  Status = Tpm12SubmitCommand(sizeof(Command), (UINT8 *)&Command, &Length, (UINT8 *)&Response);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  if (Response.Hdr.returnCode != 0) {
    return EFI_DEVICE_ERROR;
  }

  CopyMem (Data, &Response.RandomBytes, SwapBytes32(Response.RandomBytesSize));

  return Status;
}