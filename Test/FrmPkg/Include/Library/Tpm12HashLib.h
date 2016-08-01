/** @file
  Ihis library abstract TPM2 hash calculation.
  The platform can choose multiply hash, while caller just need invoke these API.
  Then all hash value will be returned and/or extended.

Copyright (c) 2013, Intel Corporation. All rights reserved. <BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _TPM12_HASH_LIB_H_
#define _TPM12_HASH_LIB_H_

#include <Uefi.h>
#include <Protocol/Hash.h>

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
  );

#endif
