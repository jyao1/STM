/** @file
  DCE header file

  Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _DCE_H_
#define _DCE_H_

#include <Base.h>
#include <Uefi.h>
#include <Library\BaseLib.h>
#include <Library\BaseMemoryLib.h>
#include <Library\IoLib.h>
#include <Library\DebugLib.h>
#include <Library\Smx.h>
#include <IndustryStandard\Acpi.h>
#include "CpuDef.h"

#define MAX_EVENT_LOG_BUFFER_SIZE  SIZE_4KB

#define MLE_PAGE_TABLE_PAGES  3
#define MLE_LOADER_PAGES      1
#define MLE_EVENT_LOG_PAGES   (6 * (MAX_EVENT_LOG_BUFFER_SIZE/EFI_PAGE_SIZE)) // 1 for TPM1.2 and 5 for TPM2.0

#define MAX_EVENT_LOG_TOTAL_BUFFER_SIZE  (EFI_PAGE_SIZE * MLE_EVENT_LOG_PAGES)

#define VARIABLE_MTRR_NUM_MAX    32

#define MLE_TEMP_STACK_SIZE_RLP  0x100

#pragma pack(1)

#define DCE_PRIVATE_DATA_SIGNATURE SIGNATURE_32 ('D', 'C', 'E', 'P')

typedef struct {
  UINT32                 Signature;

  //
  // Dce image
  //
  UINT64                 ImageBase;
  UINT64                 ImageSize;

  UINT64                 MeasuredImageSize;

  //
  // LCP PO policy
  //
  UINT64                 LcpPoBase;
  UINT64                 LcpPoSize;

  //
  // The DMA protected region for Dce.
  // It holds MLE page table, MLE header, Dce and LcpPo.
  // It could be either TXT DPR region or TXT PMR region.
  //
  UINT64                 DprBase;
  UINT64                 DprSize;

  //
  // The PMR low region to be set in MleToSinit.
  // They are all zero if there is enough TXT DPR region.
  // Or they are same as DprBase/DprSize if there is no enough TXT DPR region.
  //
  UINT64                 PmrLowBase;
  UINT64                 PmrLowSize;

  //
  // The PMR high region to be set in OsToSinit
  // All zero since we do not use PmrHigh in this version.
  //
  UINT64                 PmrHighBase;
  UINT64                 PmrHighSize;

  UINT64                 EventLogBase;
  UINT64                 EventLogSize;

  EFI_PHYSICAL_ADDRESS   DL_Entry;
  EFI_PHYSICAL_ADDRESS   DLME_Exit;

  // BIOS detected
  UINT32                 TpmType;
  UINT32                 ActivePcrBanks;

  // ACM capability
  UINT32                 AcmCapabilities;
  UINT32                 AcmTpmCapabilities;
  UINT16                 AcmTpmHashAlgoIDCount;
  UINT16                 AcmTpmHashAlgoID[5];

  // Event Log
  UINT16                 EventHashAlgoIDCount;
  UINT16                 EventHashAlgoID[5];

  // Cache data in heap
  TXT_HEAP_EVENTLOG_EXT_ELEMENT         *EventLogElement;
  TXT_HEAP_EVENT_LOG_POINTER_ELEMENT2   *EventLogPointerElement2;
  TXT_HEAP_EVENT_LOG_POINTER_ELEMENT2_1 *EventLogPointerElement2_1;

} DCE_PRIVATE_DATA;

//
// User define it
// All the data used after SENTER should be put here.
// It will be measured into PCR.
//

typedef struct {
  IA32_DESCRIPTOR  Gdtr;   
  IA32_DESCRIPTOR  Idtr;   
  UINT32           TempEsp;
  UINT32           TempEspRlp;
  UINT32           Cr3;
  UINT32           PostSinitOffset;
  UINT32           PostSinitSegment;
  UINT32           Ds;
  //
  // RLP Sync
  //
  UINT32           Lock;
  UINT32           RlpInitializedNumber;
  UINT32           RlpPostSinitOffset;
  UINT32           RlpPostSinitSegment;
  UINT32           RlpDs;
  UINT32           ApEntry;
  //
  // For register save requirement
  //
  UINT64           VariableMtrr[VARIABLE_MTRR_NUM_MAX * 2];
  UINT64           DefaultTypeMsr;
  UINT64           MiscEnableMsr;
  UINTN            Cr0;
  UINTN            Cr4;
  //
  // Other globle data to be used after SENTER.
  //
  UINT32                                       CpuNum;
  TXT_MLE_JOIN_DATA                            MleJoinData;
  EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER UefiRsdp;
  DCE_PRIVATE_DATA                             DcePrivateData;
} MLE_PRIVATE_DATA;

#define TXT_OS_TO_MLE_DATA_STRUCT_SIGNATURE SIGNATURE_32('T','O','M','D')
typedef struct {
  UINT32  Signature;
  UINT32  Reserved;
  UINT64  MlePrivateDataAddress;
  UINT64  Reserved2;
} TXT_OS_TO_MLE_DATA_STRUCT;

#pragma pack()

/**

  This function get MLE private data.

  @return MLE private data

**/
VOID *
GetMlePrivateData(
  VOID
  );

/**

  This function save MTRR informatin into TXT heap.

  @param MlePrivateData TXT MlePrivateData

**/
VOID
TxtSaveMtrr (
  OUT MLE_PRIVATE_DATA    *MlePrivateData
  );

/**

  This function restore MTRR informatin from TXT heap.

  @param MlePrivateData TXT MlePrivateData

**/
VOID
TxtRestoreMtrr (
  IN MLE_PRIVATE_DATA    *MlePrivateData
  );

/**

  This function configure MTRR environment for SENTER.

  @param MemBase ACM memory base
  @param MemSize ACM memory size
  @param MemType ACM memory type

**/
VOID
TxtConfigMtrr (
  IN UINT64                MemBase,
  IN UINT64                MemSize,
  IN UINT8                 MemType
  );

/**

  This function set TXT page table for MLE.

  @param MleLoadAddress  MLE address
  @param MleLoadSize     MLE size
  @param PageTableBase   page table base

**/
VOID
TxtSetupPageTable (
  IN UINT64  MleLoadAddress,
  IN UINT64  MleLoadSize,
  IN UINT64  PageTableBase
  );

/**

  This function wait up RLP.

  @param ApEntry   AP entrypoint

**/
VOID
TxtWakeUpRlps (
  IN UINT32 ApEntry
  );

//
// Mle entry
//
/**

  This function is MLE entrypoint.

**/
VOID
AsmMleEntryPoint (
  VOID
  );

/**

  This function is RLP wakeup code.

**/
VOID
AsmRlpWakeUpCode (
  VOID
  );

/**

  This function switch stack and launch DLME main.

  @param DlmeEntryPoint    A pointer to the entrypoint of DLME.
  @param DlmeArgs          A pointer to the Args of DLME.
  @param StackBufferTop    A pointer to the ending virtual address of a buffer to be used for the DLME's stack.

**/
VOID
AsmLaunchDlmeMain (
  IN UINTN                  DlmeEntryPoint,
  IN VOID                   *DlmeArgs,
  IN UINTN                  *StackBufferTop
  );

/**

  This function check if MLE is launched.

  @retval TRUE  MLE is launched
  @retval FALSE MLE is not launched

**/
BOOLEAN
IsMleLaunched(
  VOID
  );

/**

  This function entry DL environment.

  @param DlmeEntryPoint    A pointer to the entrypoint of DLME.
  @param DlmeArgs          A pointer to the Args of DLME.
  @param StackBufferTop    A pointer to the starting virtual address of a buffer to be used for the DLME's stack.

  @retval  non-0 fail to entry DL environment

**/
UINT32
DL_Entry (
  IN UINTN                  DlmeEntryPoint,
  IN VOID                   *DlmeArgs,
  IN UINTN                  *StackBufferTop
  );

/**

  This function exit DLME environment.

  @param SystemPowerState - An integer representing the system power state that the software will be transitioning in to.

  @retval  0 exit DLME successfully

**/
UINT32
DLME_Exit (
  IN UINT32 SystemPowerState
  );

/**

  This function dump raw data.

  @param  Data  raw data
  @param  Size  raw data size

**/
VOID
DumpData (
  IN UINT8  *Data,
  IN UINT32 Size
  );

/**

  This function dump raw data with colume format.

  @param  Data  raw data
  @param  Size  raw data size

**/
VOID
DumpHex (
  IN UINT8  *Data,
  IN UINT32 Size
  );

/**

  This function dump ACM binary info.

  @param  Acm   ACM binary

**/
VOID
DumpAcm (
  IN TXT_ACM_FORMAT   *Acm
  );

/**

  This function dump MLE header info.

  @param  MleHeader   MLE header

**/
VOID
DumpMleHeader (
  IN TXT_MLE_HEADER   *MleHeader
  );

/**

  This function dump TXT BiosToOs data.

  @param  Data   TXT BiosToOs data

**/
VOID
DumpBiosToOsData (
  IN UINT64  *Data
  );

/**

  This function dump TXT OsToSinit data.

  @param  Data   TXT OsToSinit data

**/
VOID
DumpOsToSinitData (
  IN UINT64  *Data
  );

/**

  This function dump TXT SinitToMle data.

  @param  Data   TXT SinitToMle data

**/
VOID
DumpSinitToMleData (
  IN UINT64  *Data
  );

/**

  This function dump GETSEC capabilities.

  @param  Index          GETSEC index
  @param  Capabilities   GETSEC capabilities

**/
VOID
DumpGetSecCapabilities (
  IN UINT32   Index,
  IN UINT32   Capabilities
  );

/**

  This function dump GETSEC parameters.

  @param  RegEax          GETSEC parameters RegEax
  @param  RegEbx          GETSEC parameters RegEbx
  @param  RegEcx          GETSEC parameters RegEcx

**/
VOID
DumpGetSecParameters (
  IN UINT32   RegEax,
  IN UINT32   RegEbx,
  IN UINT32   RegEcx
  );

/**

  This function dump TPM event log buffer.

**/
VOID
DumpTpmEventLogBuffer(
  VOID
  );

/**

  This fuction is entrypoint of DCE.

  @retval EFI_SUCCESS           load Dce successfully
  @retval EFI_UNSUPPORTED       not support load Dce

**/
EFI_STATUS
DceLoaderEntrypoint (
  VOID
  );

/**

  This function run DRTM DL_Entry.

**/
VOID
DlEntry (
  VOID
  );

#endif
