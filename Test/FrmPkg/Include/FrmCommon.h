/** @file

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _FRM_COMMON_H_
#define _FRM_COMMON_H_

#define MAX_EVENT_LOG_BUFFER_SIZE  SIZE_4KB

#define MLE_PAGE_TABLE_PAGES  3
#define MLE_LOADER_PAGES      1
#define MLE_EVENT_LOG_PAGES   (6 * (MAX_EVENT_LOG_BUFFER_SIZE/EFI_PAGE_SIZE)) // 1 for TPM1.2 and 5 for TPM2.0

#define MAX_EVENT_LOG_TOTAL_BUFFER_SIZE  (EFI_PAGE_SIZE * MLE_EVENT_LOG_PAGES)


#define FRM_COMMUNICATION_DATA_SIGNATURE SIGNATURE_32 ('A', 'M', 'B', 'R')

#define FRM_TPM_TYPE_NONE   0
#define FRM_TPM_TYPE_TPM12  1
#define FRM_TPM_TYPE_TPM2   2

typedef struct _FRM_COMMUNICATION_DATA {
  UINT32                 Signature;

  PHYSICAL_ADDRESS       HighMemoryBase; // < 4G memory used for FRM
  UINT64                 HighMemorySize;
  PHYSICAL_ADDRESS       LowMemoryBase; // < 1M memory used for FRM
  UINT64                 LowMemorySize;

  PHYSICAL_ADDRESS       ImageBase;
  UINT64                 ImageSize;
  PHYSICAL_ADDRESS       ImageEntrypoint;

  UINT64                 TimerPeriod; // Amount of time that passes in femtoseconds 10^(-15) for each increament of TSC
  UINT64                 AcpiRsdp;

  PHYSICAL_ADDRESS       SmMonitorServiceImageBase;
  UINT64                 SmMonitorServiceImageSize;
  PHYSICAL_ADDRESS       SmMonitorServiceProtocol;

  //
  // TXT Launch
  //
  UINT64                 MeasuredImageSize;

  UINT64                 SinitAcmBase;
  UINT64                 SinitAcmSize;

  UINT64                 LcpPoBase;
  UINT64                 LcpPoSize;

  UINT64                 EventLogBase;
  UINT64                 EventLogSize;

  UINT64                 DprBase;
  UINT64                 DprSize;

  UINT64                 PmrLowBase;
  UINT64                 PmrLowSize;

  UINT64                 PmrHighBase;
  UINT64                 PmrHighSize;

  // TPM
  UINT32                 TpmType;
  UINT32                 ActivePcrBanks;

} FRM_COMMUNICATION_DATA;

typedef
RETURN_STATUS
(* FRM_ENTRYPOINT) (
  IN FRM_COMMUNICATION_DATA    *CommunicationData
  );

#define FRM_FILE_GUID_NAME  \
  { 0xf50b4b1, 0xc62f, 0x4902, {0xb3, 0xaf, 0xe7, 0x9d, 0xbe, 0xd2, 0x1, 0x9 } }
// {0F50B4B1-C62F-4902-B3AF-E79DBED20109}

#define STM_SERVICE_FILE_GUID_NAME  \
  { 0x4E95A7C, 0x07ED, 0x48A7, { 0x94, 0x62, 0x0E, 0x5B, 0x1C, 0x3B, 0x04, 0x9F } }
// {04E95A7C-07ED-48A7-9462-0E5B1C3B049F}

#define SINIT_ACM_FILE_GUID_NAME  \
  { 0x2cb32e6a, 0xb0db, 0x496d, { 0xbd, 0x8d, 0x2c, 0x89, 0xd, 0x5a, 0x73, 0xb8 } }
// {2CB32E6A-B0DB-496D-BD8D-2C890D5A73B8}

#define LCP_PO_FILE_GUID_NAME  \
  { 0x9887e5f7, 0x8e8, 0x4abe, { 0x80, 0xc2, 0x9, 0x5a, 0xc9, 0x7a, 0xc8, 0x87 } }
// {9887E5F7-08E8-4ABE-80C2-095AC97AC887}

#endif
