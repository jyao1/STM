/** @file
  Ihis library uses TPM12 device to calculation hash.

Copyright (c) 2016, Intel Corporation. All rights reserved. <BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiPei.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/Tpm12CommandLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/Tpm12HashLib.h>
#include <Library/PcdLib.h>

#define TPMCMDBUFLENGTH             1024         // TPM command/reponse length

/**

  This function run TPM command to hash data.

  @param  Locality         TPM locality
  @param  Data             to be hashed data
  @param  DataLen          to be hashed data length
  @param  Digest           digest of data

  @retval EFI_SUCCESS        Command send successfully
  @retval EFI_DEVICE_ERROR   Something wrong with TPM

**/
EFI_STATUS
Tpm12Sha1HashAll (
  IN      UINT8                     *Data,
  IN      UINTN                     DataLen,
     OUT  TPM_DIGEST                *Digest
  )
{
  UINT32       MaxNumBytes;
  UINT8        *HashData;
  UINT32       Count;
  UINT32       LeftBytes;
  UINT32       Index;
  EFI_STATUS   Status;

  //
  // 1. Start - get parameter
  //
  Status = Tpm12Sha1Start (&MaxNumBytes);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "TpmSha1Start - %r\n", Status));
    return Status;
  }

  //
  // Tis implementation limitation - check it
  // And NumBytes must be a multiple of 64.
  //
  if (MaxNumBytes >= TPMCMDBUFLENGTH / 2) {
    MaxNumBytes = TPMCMDBUFLENGTH / 2;
  }
  HashData = Data;

  //
  // 2. Update - send with max
  //
  Count     = (UINT32)(DataLen / MaxNumBytes);
  LeftBytes = (UINT32)(DataLen % MaxNumBytes);
  for (Index = 0; Index < Count; Index++) {
    Status = Tpm12Sha1Update (MaxNumBytes, HashData);
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "TpmSha1Update - %r\n", Status));
      return Status;
    }
    HashData += MaxNumBytes;
  }

  //
  // 3. Complete, TpmSha1Complete need bytes less than 64.
  // Split them again.
  //
  MaxNumBytes = 64;
  Count     = (UINT32)(LeftBytes / MaxNumBytes);
  LeftBytes = (UINT32)(LeftBytes % MaxNumBytes);
  for (Index = 0; Index < Count; Index++) {
    Status = Tpm12Sha1Update (MaxNumBytes, HashData);
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "TpmSha1Update - %r\n", Status));
      return Status;
    }
    HashData += MaxNumBytes;
  }

  Status = Tpm12Sha1Complete (LeftBytes, HashData, Digest);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "TpmSha1Complete - %r\n", Status));
    return Status;
  }

  return EFI_SUCCESS;
}

/**

  This function hash data and extend it to PCR.

  @param PcrIndex        PCR index
  @param HashData        data to be hashed
  @param HashSize        size of data to be hashed
  @param DigestToEntend  digest of the data

  @retval EFI_SUCCESS      hash and extend successfully
  @retval EFI_DEVICE_ERROR something wrong with TPM

**/
EFI_STATUS
Tpm12HashAndExtend (
  IN TPM_PCRINDEX              PcrIndex,
  IN UINT8                     *HashData,
  IN UINTN                     HashSize,
  OUT TPM_DIGEST               *DigestToEntend
  )
{
  EFI_STATUS                Status;
  TPM_DIGEST                NewPcrValue;

  Status = Tpm12Sha1HashAll (
             HashData,
             HashSize,
             DigestToEntend
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "TpmSha1HashAll - %r\n", Status));
    return Status;
  }

  Status = Tpm12Extend (
             DigestToEntend,
             PcrIndex,
             &NewPcrValue
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Tpm12Extend %d - %r\n", (UINTN)PcrIndex, Status));
    return Status;
  }

  return EFI_SUCCESS;
}