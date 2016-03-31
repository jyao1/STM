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


#define FRM_COMMUNICATION_DATA_SIGNATURE SIGNATURE_32 ('A', 'M', 'B', 'R')

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

#endif
