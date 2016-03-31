/** @file
  The definition for VTD register.
  It is defined in "Intel VT for Direct IO Architecture Specification".

  Copyright (c) 2014, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __EFI_VTD_REG_H__
#define __EFI_VTD_REG_H__

//
// Translation Structure Formats
//
#define VTD_ROOT_ENTRY_NUMBER       256
#define VTD_CONTEXT_ENTRY_NUMBER    256

typedef union {
  struct {
    UINT64  Present:1;
    UINT64  Reserved_1:11;
    UINT64  ContextTablePointer:52;

    UINT64  Reserved_64;
  } Bits;
  struct {
    UINT64  Uint64Lo;
    UINT64  Uint64Hi;
  } Uint128;
} VTD_ROOT_ENTRY;

typedef union {
  struct {
    UINT64  LowerPresent:1;
    UINT64  Reserved_1:11;
    UINT64  LowerContextTablePointer:52;

    UINT64  UpperPresent:1;
    UINT64  Reserved_65:11;
    UINT64  UpperContextTablePointer:52;
  } Bits;
  struct {
    UINT64  Uint64Lo;
    UINT64  Uint64Hi;
  } Uint128;
} VTD_EXT_ROOT_ENTRY;

typedef union {
  struct {
    UINT64  Present:1;
    UINT64  FaultProcessingDisable:1;
    UINT64  TranslationType:2;
    UINT64  Reserved_4:8;
    UINT64  SecondLevelPageTranslationPointer:52;

    UINT64  AddressWidth:3;
    UINT64  Ignored_67:4;
    UINT64  Reserved_71:1;
    UINT64  DomainIdentifier:16;
    UINT64  Reserved_88:40;
  } Bits;
  struct {
    UINT64  Uint64Lo;
    UINT64  Uint64Hi;
  } Uint128;
} VTD_CONTEXT_ENTRY;

typedef union {
  struct {
    UINT64  Present:1;
    UINT64  FaultProcessingDisable:1;
    UINT64  TranslationType:3;
    UINT64  ExtendedMemoryType:3;
    UINT64  DeferredInvalidateEnable:1;
    UINT64  PageRequestEnable:1;
    UINT64  NestedTranslationEnable:1;
    UINT64  PASIDEnable:1;
    UINT64  SecondLevelPageTranslationPointer:52;

    UINT64  AddressWidth:3;
    UINT64  PageGlobalEnable:1;
    UINT64  NoExecuteEnable:1;
    UINT64  WriteProtectEnable:1;
    UINT64  CacheDisable:1;
    UINT64  ExtendedMemoryTypeEnable:1;
    UINT64  DomainIdentifier:16;
    UINT64  SupervisorModeExecuteProtection:1;
    UINT64  ExtendedAccessedFlagEnable:1;
    UINT64  ExecuteRequestsEnable:1;
    UINT64  SecondLevelExecuteEnable:1;
    UINT64  Reserved_92:4;
    UINT64  PageAttributeTable0:3;
    UINT64  Reserved_Pat0:1;
    UINT64  PageAttributeTable1:3;
    UINT64  Reserved_Pat1:1;
    UINT64  PageAttributeTable2:3;
    UINT64  Reserved_Pat2:1;
    UINT64  PageAttributeTable3:3;
    UINT64  Reserved_Pat3:1;
    UINT64  PageAttributeTable4:3;
    UINT64  Reserved_Pat4:1;
    UINT64  PageAttributeTable5:3;
    UINT64  Reserved_Pat5:1;
    UINT64  PageAttributeTable6:3;
    UINT64  Reserved_Pat6:1;
    UINT64  PageAttributeTable7:3;
    UINT64  Reserved_Pat7:1;

    UINT64  PASIDTableSize:4;
    UINT64  Reserved_132:8;
    UINT64  PASIDTablePointer:52;

    UINT64  Reserved_192:12;
    UINT64  PASIDStateTablePointer:52;
  } Bits;
  struct {
    UINT64  Uint64_1;
    UINT64  Uint64_2;
    UINT64  Uint64_3;
    UINT64  Uint64_4;
  } Uint256;
} VTD_EXT_CONTEXT_ENTRY;

typedef union {
  struct {
    UINT64  Present:1;
    UINT64  Reserved_1:2;
    UINT64  PageLevelCacheDisable:1;
    UINT64  PageLevelWriteThrough:1;
    UINT64  Reserved_5:6;
    UINT64  SupervisorRequestsEnable:1;
    UINT64  FirstLevelPageTranslationPointer:52;
  } Bits;
  UINT64    Uint64;
} VTD_PASID_ENTRY;

typedef union {
  struct {
    UINT64  Reserved_0:32;
    UINT64  ActiveReferenceCount:16;
    UINT64  Reserved_48:15;
    UINT64  DeferredInvalidate:1;
  } Bits;
  UINT64    Uint64;
} VTD_PASID_STATE_ENTRY;

typedef union {
  struct {
    UINT64  Present:1;
    UINT64  ReadWrite:1;
    UINT64  UserSupervisor:1;
    UINT64  PageLevelWriteThrough:1;
    UINT64  PageLevelCacheDisable:1;
    UINT64  Accessed:1;
    UINT64  Dirty:1;
    UINT64  PageSize:1; // It is PageAttribute:1 for 4K page entry
    UINT64  Global:1;
    UINT64  Ignored_9:1;
    UINT64  ExtendedAccessed:1;
    UINT64  Ignored_11:1;
    // NOTE: There is PageAttribute:1 as bit12 for 1G page entry and 2M page entry
    UINT64  Address:40;
    UINT64  Ignored_52:11;
    UINT64  ExecuteDisable:1;
  } Bits;
  UINT64    Uint64;
} VTD_FIRST_LEVEL_PAGING_ENTRY;

typedef union {
  struct {
    UINT64  Read:1;
    UINT64  Write:1;
    UINT64  Execute:1;
    UINT64  ExtendedMemoryType:3;
    UINT64  IgnorePAT:1;
    UINT64  PageSize:1;
    UINT64  Ignored_8:3;
    UINT64  Snoop:1;
    UINT64  Address:40;
    UINT64  Ignored_52:10;
    UINT64  TransientMapping:1;
    UINT64  Ignored_63:1;
  } Bits;
  UINT64    Uint64;
} VTD_SECOND_LEVEL_PAGING_ENTRY;

//
// Register Descriptions
//
#define R_VER_REG        0x00
#define R_CAP_REG        0x08
#define   B_CAP_REG_RWBF       BIT4
#define R_ECAP_REG       0x10
#define R_GCMD_REG       0x18
#define   B_GMCD_REG_WBF       BIT27
#define   B_GMCD_REG_SRTP      BIT30
#define   B_GMCD_REG_TE        BIT31
#define R_GSTS_REG       0x1C
#define   B_GSTS_REG_WBF       BIT27
#define   B_GSTS_REG_RTPS      BIT30
#define   B_GSTS_REG_TE        BIT31
#define R_RTADDR_REG     0x20
#define R_CCMD_REG       0x28
#define   B_CCMD_REG_CIRG_MASK    (BIT62|BIT61)
#define   V_CCMD_REG_CIRG_GLOBAL  BIT61
#define   V_CCMD_REG_CIRG_DOMAIN  BIT62
#define   V_CCMD_REG_CIRG_DEVICE  (BIT62|BIT61)
#define   B_CCMD_REG_ICC          BIT63
#define R_FSTS_REG       0x34
#define R_FECTL_REG      0x38
#define R_FEDATA_REG     0x3C
#define R_FEADDR_REG     0x40
#define R_FEUADDR_REG    0x44
#define R_AFLOG_REG      0x58
#define R_PMEN_REG       0x64

#define R_IVA_REG        0x00 // + IRO
#define   B_IVA_REG_AM_MASK       (BIT0|BIT1|BIT2|BIT3|BIT4|BIT5)
#define   B_IVA_REG_AM_4K         0 // 1 page
#define   B_IVA_REG_AM_2M         9 // 2M page
#define   B_IVA_REG_IH            BIT6
#define R_IOTLB_REG      0x08 // + IRO
#define   B_IOTLB_REG_IIRG_MASK   (BIT61|BIT60)
#define   V_IOTLB_REG_IIRG_GLOBAL BIT60
#define   V_IOTLB_REG_IIRG_DOMAIN BIT61
#define   V_IOTLB_REG_IIRG_PAGE   (BIT61|BIT60)
#define   B_IOTLB_REG_IVT         BIT63

#define R_FRCD_REG       0x00 // + FRO

typedef union {
  struct {
    UINT32        ND:3; // Number of domains supported
    UINT32        AFL:1; // Advanced Fault Logging
    UINT32        RWBF:1; // Required Write-Buffer Flushing
    UINT32        PLMR:1; // Protected Low-Memory Region
    UINT32        PHMR:1; // Protected High-Memory Region
    UINT32        CM:1; // Caching Mode

    UINT32        SAGAW:5; // Supported Adjusted Guest Address Widths
    UINT32        Rsvd_13:3;

    UINT32        MGAW:6; // Maximum Guest Address Width
    UINT32        ZLR:1; // Zero Length Read
    UINT32        Rsvd_23:1;

    UINT32        FRO_Lo:8; // Fault-recording Register offset (low bit)
    UINT32        FRO_Hi:2; // Fault-recording Register offset (high bit)
    UINT32        SLLPS:4; // Second Level Large Page Support
    UINT32        Rsvd_38:1;
    UINT32        PSI:1; // Page Selective Invalidation

    UINT32        NFR:8; // Number of Fault-recording Registers

    UINT32        MAMV:6; // Maximum Address Mask Value
    UINT32        DWD:1; // Write Draining
    UINT32        DRD:1; // Read Draining

    UINT32        FL1GP:1; // First Level 1-GByte Page Support
    UINT32        Rsvd_57:2;
    UINT32        PI:1; // Posted Interrupts Support
    UINT32        Rsvd_60:4;
  } Bits;
  UINT64     Uint64;
} VTD_CAP_REG;

typedef union {
  struct {
    UINT32        C:1; // Page-walk Coherency
    UINT32        QI:1; // Queued Invalidation support
    UINT32        DT:1; // Device-TLB support
    UINT32        IR:1; // Interrupt Remapping support
    UINT32        EIM:1; // Extended Interrupt Mode
    UINT32        Rsvd_5:1;
    UINT32        PT:1; // Pass Through
    UINT32        SC:1; // Snoop Control

    UINT32        IRO:10; // IOTLB Register Offset
    UINT32        Rsvd_18:2;
    UINT32        MHMV:4; // Maximum Handle Mask Value

    UINT32        ECS:1; // Extended Context Support
    UINT32        MTS:1; // Memory Type Support
    UINT32        NEST:1; // Nested Translation Support
    UINT32        DIS:1; // Deferred Invalidate Support
    UINT32        PASID:1; // Process Address Space ID Support
    UINT32        PRS:1; // Page Request Support
    UINT32        ERS:1; // Execute Request Support
    UINT32        SRS:1; // Supervisor Request Support

    UINT32        Rsvd_32:1;
    UINT32        NWFS:1; // No Write Flag Support
    UINT32        EAFS:1; // Extended Accessed Flag Support
    UINT32        PSS:5; // PASID Size Supported

    UINT32        Rsvd_40:24;
  } Bits;
  UINT64     Uint64;
} VTD_ECAP_REG;

typedef union {
  struct {
    UINT16   Function:3;
    UINT16   Device:5;
    UINT16   Bus:8;
  } Bits;
  UINT16     Uint16;
} VTD_SOURCE_ID;

#endif

