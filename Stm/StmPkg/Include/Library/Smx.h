/** @file
  SMX header file

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _SMX_H_
#define _SMX_H_

#define TXT_PRIVATE_SPACE       0xFED20000
#define TXT_PUBLIC_SPACE        0xFED30000

#define TXT_STS                     0x0
#define   TXT_STS_SENTER_DONE       0x1
#define   TXT_STS_SEXIT_DONE        0x2
#define TXT_ESTS                    0x8
#define TXT_ERRORCODE               0x30
#define TXT_CMD_SYS_RESET           0x38
#define TXT_CMD_CLOSE_PRIVATE       0x48
#define TXT_VER_FSBIF               0x100
#define TXT_DIDVID                  0x110
#define TXT_CMD_UNLOCK_MEM_CONFIG   0x218
#define TXT_SINIT_BASE              0x270
#define TXT_SINIT_SIZE              0x278
#define TXT_MLE_JOIN                0x290
#define TXT_HEAP_BASE               0x300
#define TXT_HEAP_SIZE               0x308
#define TXT_DPR_REG                 0x330
#define   TXT_DPR_REG_LCK           0x1
#define   TXT_DPR_REG_SIZE_MASK     0xFF0
#define   TXT_DPR_REG_SIZE_OFFSET   16
#define   TXT_DPR_REG_BASE_MASK     0xFFF00000
#define TXT_CMD_OPEN_LOCALITY1      0x380
#define TXT_CMD_CLOSE_LOCALITY1     0x388
#define TXT_CMD_OPEN_LOCALITY2      0x390
#define TXT_CMD_CLOSE_LOCALITY2     0x398
#define TXT_CMD_SECRETS             0x8E0
#define TXT_CMD_NO_SECRETS          0x8E8
#define TXT_E2STS                   0x8F0

#define GETSEC_PARAMETER_TYPE_MASK  0x1F

#define GETSEC_PARAMETER_TYPE_ACM_VERSION       1
#define GETSEC_PARAMETER_TYPE_ACM_MAX_SIZE      2
#define GETSEC_PARAMETER_TYPE_EXTERN_MEM_TYPE   3
#define GETSEC_PARAMETER_TYPE_SENTER_DIS_CONTOL 4
#define GETSEC_PARAMETER_TYPE_EXTERNSION        5

#pragma pack (push, 1)

#define TXT_MLE_HEADER_GUID \
  { 0x9082AC5A, 0x74A7476F, 0xA2555C0F, 0x42B651CB }

typedef struct {
  UINT32    Guid0;
  UINT32    Guid1;
  UINT32    Guid2;
  UINT32    Guid3;
} TXT_GUID;

#define TXT_MLE_HEADER_VERSION_1     0x10000
#define TXT_MLE_HEADER_VERSION_1_1   0x10001
#define TXT_MLE_HEADER_VERSION_2     0x20000
#define TXT_MLE_HEADER_VERSION_2_1   0x20001
#define TXT_MLE_HEADER_VERSION       TXT_MLE_HEADER_VERSION_2

#define TXT_MLE_SINIT_CAPABILITY_GETSET_WAKEUP                1u
#define TXT_MLE_SINIT_CAPABILITY_MONITOR_ADDRESS_RLP_WAKEUP   (1u << 1)
#define TXT_MLE_SINIT_CAPABILITY_ECX_HAS_PAGE_TABLE           (1u << 2)
#define TXT_MLE_SINIT_CAPABILITY_STM                          (1u << 3)

typedef struct {
  TXT_GUID   Guid;
  UINT32     HeaderLen;
  UINT32     Version;
  UINT32     EntryPoint;
//#if (TXT_MLE_HEADER_VERSION >= TXT_MLE_HEADER_VERSION_1_1)
  UINT32     FirstValidPage;
  UINT32     MleStart;
  UINT32     MleEnd;
//#if (TXT_MLE_HEADER_VERSION >= TXT_MLE_HEADER_VERSION_2)
  UINT32     Capabilities;
//#if (TXT_MLE_HEADER_VERSION >= TXT_MLE_HEADER_VERSION_2_1)
  UINT32     CmdlineStart;
  UINT32     CmdlineEnd;
//#endif
//#endif
//#endif
} TXT_MLE_HEADER;

#define ACM_PKCS_1_5_RSA_SIGNATURE_SIZE  256
#define TXT_ACM_MODULE_TYPE_CHIPSET_ACM  2
#define TXT_ACM_MODULE_SUBTYPE_CAPABLE_OF_EXECUTE_AT_RESET  1
#define TXT_ACM_MODULE_FLAG_PREPRODUCTION  0x4000
#define TXT_ACM_MODULE_FLAG_DEBUG_SIGN     0x8000

typedef struct {
  UINT16     ModuleType;
  UINT16     ModuleSubType;
  UINT32     HeaderLen;
  UINT32     HeaderVersion;
  UINT16     ChipsetID;
  UINT16     Flags;
  UINT32     ModuleVendor;
  UINT32     Date;
  UINT32     Size;
  UINT32     Rsvd1;
  UINT32     CodeControl;
  UINT32     ErrorEntryPoint;
  UINT32     GDTLimit;
  UINT32     GDTBasePtr;
  UINT32     SegSel;
  UINT32     EntryPoint;
  UINT8      Rsvd2[64];
  UINT32     KeySize; // 64
  UINT32     ScratchSize; // 2 * KeySize + 15
//UINT8      RSAPubKey[64 * 4]; // KeySize * 4
//UINT32     RSAPubExp;
//UINT8      RSASig[256];
  // End of AC module header
//UINT8      Scratch[(64 * 2 + 15) * 4]; // ScratchSize * 4
  // User Area
//UINT8      UserArea[1];
} TXT_ACM_FORMAT;

#define TXT_CHIPSET_ACM_INFORMATION_TABLE_VERSION_2  0x02
#define TXT_CHIPSET_ACM_INFORMATION_TABLE_VERSION_3  0x03
#define TXT_CHIPSET_ACM_INFORMATION_TABLE_VERSION_4  0x04

#define TXT_CHIPSET_ACM_INFORMATION_TABLE_VERSION    TXT_CHIPSET_ACM_INFORMATION_TABLE_VERSION_3

#define TXT_CHIPSET_ACM_INFORMATION_TABLE_GUID_V03 \
  { 0x7FC03AAA, 0x18DB46A7, 0x8F69AC2E, 0x5A7F418D }

#define TXT_CHIPSET_ACM_INFORMATION_TABLE_GUID_V02 \
  { 0x8024D6CD, 0x2A624733, 0x893AF1D1, 0xBC82113B }

#define TXT_CHIPSET_ACM_TYPE_BIOS   0
#define TXT_CHIPSET_ACM_TYPE_SINIT  1

typedef struct {
  TXT_GUID   Guid;
  UINT8      ChipsetACMType;
  UINT8      Version;
  UINT16     Length;
  UINT32     ChipsetIDList;
  UINT32     OsSinitTableVer;
  UINT32     MinMleHeaderVer;
//#if (TXT_CHIPSET_ACM_INFORMATION_TABLE_VERSION >= TXT_CHIPSET_ACM_INFORMATION_TABLE_VERSION_3)
  UINT32     Capabilities;
  UINT8      AcmVersion;
  UINT8      Rsvd[3];
//#if (TXT_CHIPSET_ACM_INFORMATION_TABLE_VERSION >= TXT_CHIPSET_ACM_INFORMATION_TABLE_VERSION_4)
  UINT32     ProcessorIDList;
//#endif
//#endif
} TXT_CHIPSET_ACM_INFORMATION_TABLE;

#define TXT_ACM_CHIPSET_ID_REVISION_ID_MAKE  0x1

typedef struct {
  UINT32     Flags;
  UINT16     VendorID;
  UINT16     DeviceID;
  UINT16     RevisionID;
  UINT8      Reserved[2];
  UINT32     ExtendedID;
} TXT_ACM_CHIPSET_ID;

typedef struct {
  UINT32               Count;
  TXT_ACM_CHIPSET_ID   ChipsetID[1];
} TXT_CHIPSET_ID_LIST;

typedef struct {
  UINT32     FMS;
  UINT32     FMSMask;
  UINT64     PlatformID;
  UINT64     PlatformMask;
} TXT_ACM_PROCESSOR_ID;

typedef struct {
  UINT32               Count;
  TXT_ACM_PROCESSOR_ID ProcessorID[1];
} TXT_PROCESSOR_ID_LIST;

#define TXT_BIOS_TO_OS_DATA_VERSION_1   1
#define TXT_BIOS_TO_OS_DATA_VERSION_2   2
#define TXT_BIOS_TO_OS_DATA_VERSION_3   3
#define TXT_BIOS_TO_OS_DATA_VERSION_4   4 // For optional element
#define TXT_BIOS_TO_OS_DATA_VERSION     TXT_BIOS_TO_OS_DATA_VERSION_3

typedef struct {
  UINT32     Version;
  UINT32     BiosSinitSize;
//#if (TXT_BIOS_TO_OS_DATA_VERSION >= TXT_BIOS_TO_OS_DATA_VERSION_2)
  UINT64     LcpPdBase;
  UINT64     LcpPdSize;
  UINT32     NumLogProcs;
//#if (TXT_BIOS_TO_OS_DATA_VERSION >= TXT_BIOS_TO_OS_DATA_VERSION_3)
  UINT64     Flags;
//#if (TXT_BIOS_TO_OS_DATA_VERSION >= TXT_BIOS_TO_OS_DATA_VERSION_4)
//TXT_HEAP_EXT_DATA_ELEMENT  ExtDataElements[];
//#endif
//#endif
//#endif
} TXT_BIOS_TO_OS_DATA;

typedef struct {
  UINT32 Type; // HEAP_EXTDATA_TYPE_* (global for all of heap)
  UINT32 Size;
//UINT8 Data[Size - 12];
} TXT_HEAP_EXT_DATA_ELEMENT;

#define TXT_HEAP_EXTDATA_TYPE_END 0
typedef struct {
  UINT32 Type; // = 0
  UINT32 Size; // = 8
} TXT_HEAP_END_ELEMENT;

#define TXT_HEAP_EXTDATA_TYPE_BIOS_SPEC_VER 1
typedef struct {          // For TXT BIOS Spec version 1.9
  UINT16 SpecVerMajor;    // 01 (Decimal)
  UINT16 SpecVerMinor;    // 09 (Decimal)
  UINT16 SpecVerRevision; // 00 (Decimal)
} TXT_HEAP_BIOS_SPEC_VER_ELEMENT;

#define TXT_HEAP_EXTDATA_TYPE_BIOSACM 2
typedef struct {
  UINT32 NumAcms;               // Number of BIOS ACMs carried by BIOS
//UINT64 BiosAcmAddrs[NumAcms]; // 64bit physical address of BIOS ACM(s)
} TXT_HEAP_BIOSACM_ELEMENT;

#define TXT_HEAP_EXTDATA_TYPE_BIOS_EXT 3
typedef struct {
  UINT8  StmSpecVerMajor; // <major>.<minor> current = 0x00010000
  UINT8  StmSpecVerMinor;
  UINT16 BiosSmmFlags;
  UINT16 StmFeatureFlags;
  UINT32 RequiredStmSmmRevId;
  UINT32 Reserved;
  UINT8  GetBiosAcStatusCmd;
  UINT8  UpdateBiosAcCmd;
  UINT8  GetSinitAcStatusCmd;
  UINT8  UpdateSinitAcCmd;
  UINT8  GetStmStatusCmd;
  UINT8  UpdateStmCmd;
  UINT8  ReservedCmd[20];
  UINT8  ChangeBiosResourcesCmd;
  UINT8  AccessResourcesCmd;
  UINT8  LoadStmCmd;
  UINT8  ReservedCmdForDebug[3];
} TXT_HEAP_BIOS_EXT_ELEMENT;

#define TXT_HEAP_EXTDATA_TYPE_CUSTOM 4
typedef struct {
  UINT32 Data1;
  UINT16 Data2;
  UINT16 Data3;
  UINT16 Data4;
  UINT8  Data5[6];
} TXT_UUID;
typedef struct {
  TXT_UUID  Uuid;
//UINT8     Data[];
} TXT_HEAP_CUSTOM_ELEMENT;

#define TXT_OS_TO_SINIT_DATA_VERSION_1   1
#define TXT_OS_TO_SINIT_DATA_VERSION_3   3
#define TXT_OS_TO_SINIT_DATA_VERSION_4   4
#define TXT_OS_TO_SINIT_DATA_VERSION_5   5
#define TXT_OS_TO_SINIT_DATA_VERSION     TXT_OS_TO_SINIT_DATA_VERSION_5

typedef struct {
  UINT32     Version;
  UINT32     Reserved;
  UINT64     MLEPageTableBase;
  UINT64     MLESize;
  UINT64     MLEHeaderBase;
//#if (TXT_OS_TO_SINIT_DATA_VERSION >= TXT_OS_TO_SINIT_DATA_VERSION_3)
  UINT64     PMRLowBase;
  UINT64     PMRLowSize;
  UINT64     PMRHighBase;
  UINT64     PMRHighSize;
  UINT64     LCPPOBase;
  UINT64     LCPPOSize;
//#if (TXT_OS_TO_SINIT_DATA_VERSION >= TXT_OS_TO_SINIT_DATA_VERSION_4)
  UINT32     Capabilities;
//#if (TXT_OS_TO_SINIT_DATA_VERSION >= TXT_OS_TO_SINIT_DATA_VERSION_5)
  UINT64     RsdpPtr;
//#endif
//#endif
//#endif
} TXT_OS_TO_SINIT_DATA;

#define TXT_SINIT_TO_MLE_DATA_VERSION_1   1
#define TXT_SINIT_TO_MLE_DATA_VERSION_3   3
#define TXT_SINIT_TO_MLE_DATA_VERSION_5   5
#define TXT_SINIT_TO_MLE_DATA_VERSION_6   6
#define TXT_SINIT_TO_MLE_DATA_VERSION_7   7
#define TXT_SINIT_TO_MLE_DATA_VERSION_8   8

#define TXT_SINIT_TO_MLE_DATA_VERSION   TXT_SINIT_TO_MLE_DATA_VERSION_8

typedef struct {
  UINT32     Version;
  UINT8      BiosAcmID[20];
  UINT32     EdxSenterFlags;
  UINT64     MsegValid;
  UINT8      SinitHash[20];
  UINT8      MleHash[20];
  UINT8      StmHash[20];
//#if (TXT_SINIT_TO_MLE_DATA_VERSION >= TXT_SINIT_TO_MLE_DATA_VERSION_3)
  UINT8      LcpPolicyHash[20];
  UINT32     PolicyControl;
//#if (TXT_SINIT_TO_MLE_DATA_VERSION >= TXT_SINIT_TO_MLE_DATA_VERSION_5)
  UINT32     RlpWakeupAddr;     // Write non-0 will wakeup AP.
  UINT32     Reserved;
//#else
//  UINT64     Reserved;
//#endif
  UINT32     NumberOfSinitMdrs;
  UINT32     SinitMdrTableOffset;
  UINT32     SinitVtdDmarTableSize;
  UINT32     SinitVtdDmarTableOffset;
//#if (TXT_SINIT_TO_MLE_DATA_VERSION >= TXT_SINIT_TO_MLE_DATA_VERSION_8)
  UINT32     ProcessorSCRTMStatus;
//#endif
//#endif
} TXT_SINIT_TO_MLE_DATA;

#define TXT_SINIT_MDR_TYPE_USABLE_MEMORY        0
#define TXT_SINIT_MDR_TYPE_OVERLAYED_SMRAM      1
#define TXT_SINIT_MDR_TYPE_NON_OVERLAYED_SMRAM  2
#define TXT_SINIT_MDR_TYPE_PCIE                 3

typedef struct {
  UINT64     Address;
  UINT64     Length;
  UINT8      Type;
  UINT8      Reserved[7];
} TXT_SINIT_MEMORY_DESCRIPTOR_RECORD;

typedef struct {
  UINT32    GDTLimit;
  UINT32    GDTBasePtr;
  UINT32    Cs;
  UINT32    LinearEntryPoint;
} TXT_MLE_JOIN_DATA;

typedef struct {
  UINT32  ChipsetPresent:1;
  UINT32  Undefined1:1;
  UINT32  EnterAccs:1;
  UINT32  ExitAc:1;
  UINT32  Senter:1;
  UINT32  Sexit:1;
  UINT32  Parameters:1;
  UINT32  Smctrl:1;
  UINT32  Wakeup:1;
  UINT32  Undefined2:22;
  UINT32  ExtendedLeafs:1;
} TXT_GETSEC_CAPABILITIES_BITS;

typedef union {
  TXT_GETSEC_CAPABILITIES_BITS Bits;
  UINT32                       Uint32;
} TXT_GETSEC_CAPABILITIES;

typedef struct {
  UINT16     VendorID;
  UINT16     DeviceID;
  UINT16     RevisionID;
  UINT16     ExtendedID;
} TXT_DID_VID_BITS;

typedef union {
  TXT_DID_VID_BITS Bits;
  UINT64           Uint64;
} TXT_DID_VID;

#pragma pack (pop)

//
// Function
//

/**

  This function read TXT public space.

  @param Offset TXT public space register

  @return TXT public space data

**/
UINT32
TxtPubRead32 (
  IN UINTN  Offset
  );

/**

  This function write TXT public space.

  @param Offset TXT public space register
  @param Data   TXT public space data

**/
VOID
TxtPubWrite32 (
  IN UINTN  Offset,
  IN UINT32 Data
  );

/**

  This function read TXT public space.

  @param Offset TXT public space register

  @return TXT public space data

**/
UINT64
TxtPubRead64 (
  IN UINTN  Offset
  );

/**

  This function write TXT public space.

  @param Offset TXT public space register
  @param Data   TXT public space data

**/
VOID
TxtPubWrite64 (
  IN UINTN  Offset,
  IN UINT64 Data
  );

/**

  This function read TXT private space.

  @param Offset TXT private space register

  @return TXT private space data

**/
UINT32
TxtPriRead32 (
  IN UINTN  Offset
  );

/**

  This function write TXT private space.

  @param Offset TXT private space register
  @param Data   TXT private space data

**/
VOID
TxtPriWrite32 (
  IN UINTN  Offset,
  IN UINT32 Data
  );

/**

  This function read TXT private space.

  @param Offset TXT private space register

  @return TXT private space data

**/
UINT64
TxtPriRead64 (
  IN UINTN  Offset
  );

/**

  This function write TXT private space.

  @param Offset TXT private space register
  @param Data   TXT private space data

**/
VOID
TxtPriWrite64 (
  IN UINTN  Offset,
  IN UINT64 Data
  );

/**

  This function open locality2.

**/
VOID
OpenLocality2 (
  VOID
  );

/**

  This function close locality2.

**/
VOID
CloseLocality2 (
  VOID
  );

/**

  This function open locality1.

**/
VOID
OpenLocality1 (
  VOID
  );

/**

  This function close locality1.

**/
VOID
CloseLocality1 (
  VOID
  );

/**

  This function set secrets.

**/
VOID
SetSecrets (
  VOID
  );

/**

  This function set no-secrets.

**/
VOID
SetNoSecrets (
  VOID
  );

/**

  This function unlock memory configuration.

**/
VOID
UnlockMemConfig (
  VOID
  );

/**

  This function close private.

**/
VOID
ClosePrivate (
  VOID
  );

/**

  This function return TXT heap.

  @return TXT heap

**/
VOID *
GetTxtHeap (
  VOID
  );

/**

  This function return TXT heap size.

  @return TXT heap size

**/
UINTN
GetTxtHeapSize (
  VOID
  );

/**

  This function return TXT BiosToOs region.

  @return TXT BiosToOs region

**/
TXT_BIOS_TO_OS_DATA *
GetTxtBiosToOsData (
  VOID
  );

/**

  This function return TXT OsToMle region.

  @return TXT OsToMle region

**/
VOID *
GetTxtOsToMleData (
  VOID
  );

/**

  This function return TXT OsToSinit region.

  @return TXT OsToSinit region

**/
TXT_OS_TO_SINIT_DATA *
GetTxtOsToSinitData (
  VOID
  );

/**

  This function return TXT SinitToMle region.

  @return TXT SinitToMle region

**/
TXT_SINIT_TO_MLE_DATA *
GetTxtSinitToMleData (
  VOID
  );

/**

  This function return TXT Heap occupied size.

  @return TXT Heap occupied size

**/
UINTN
GetTxtHeapOccupiedSize (
  VOID
  );

#endif
