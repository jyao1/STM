/** @file
  DrtmTpm lib header file

  Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _DRTM_TPM_H_
#define _DRTM_TPM_H_

#include <IndustryStandard/Tpm12.h>
#include <IndustryStandard/Tpm20.h>
#include <IndustryStandard/TpmTis.h>
#include <IndustryStandard/TpmPtp.h>
#include <IndustryStandard/UefiTcgPlatform.h>

#include <Library/Tpm12DeviceLib.h>
#include <Library/Tpm2DeviceLib.h>
#include <Library/Tpm12CommandLib.h>
#include <Library/Tpm2CommandLib.h>
#include <Library/Tpm12HashLib.h>
#include <Library/HashLib.h>

#define TPM_LOCALITY_0             0
#define TPM_LOCALITY_1             1
#define TPM_LOCALITY_2             2
#define TPM_LOCALITY_3             3
#define TPM_LOCALITY_4             4

#define TPM_BASE_ADDRESS            0xfed40000
#define TPM_ACCESS(x)               ((x) << 12)

#define EV_EVENT_TAG                ((TCG_EVENTTYPE) 0x00000006)

/**

  This function request use TPM.

  @param  Locality         TPM locality

  @retval EFI_SUCCESS        Command send successfully
  @retval EFI_DEVICE_ERROR   Something wrong with TPM

**/
EFI_STATUS
TpmCommRequestUseTpm (
  IN      TPM_LOCALITY_SELECTION    Locality
  );

/**

  This function qelinquish locality.

  @param  Locality         TPM locality

  @retval EFI_SUCCESS        Command send successfully
  @retval EFI_DEVICE_ERROR   Something wrong with TPM

**/
EFI_STATUS
TpmCommRelinquishLocality (
  IN      TPM_LOCALITY_SELECTION    Locality
  );

/**

  This function cap sensitive TPM PCR values.

**/
VOID
CapPcrs (
  VOID
  );

/**

  This function extend required items to PCRs.

**/
VOID
DoMeasurement (
  VOID
  );

#endif  // _TPM_H_
