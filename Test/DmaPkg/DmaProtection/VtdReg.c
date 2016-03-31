/** @file

  Copyright (c) 2014, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "DmaProtection.h"

UINT32                           mVtdUnitNumber;
VTD_UNIT_INFORMATION             mVtdUnitInformation[MAX_VTD_UNITS];

BOOLEAN  mVtdEnabled;

/**

  This function invalidate VTd IOTLBGlobal.

  @param VtdIndex      VTd engine index

  @retval EFI_SUCCESS invalidate VTd IOTLBGlobal successfully.
**/
EFI_STATUS
InvalidateVtdIOTLBGlobal (
  IN UINTN  VtdIndex
  )
{
  UINT64  Reg64;
  UINT32  Reg32;
  
  if (!mVtdEnabled) {
    return EFI_SUCCESS;
  }

  DEBUG((EFI_D_INFO, "InvalidateVtdIOTLBGlobal(%d)\n", VtdIndex));
    
  AsmWbinvd();

  //
  // Write Buffer Flush before invalidation
  //
  Reg32 = MmioRead32 ((UINTN)mVtdUnitInformation[VtdIndex].VtdUnitBaseAddress + R_CAP_REG);
  if ((Reg32 & B_CAP_REG_RWBF) != 0) {
    MmioWrite32 ((UINTN)mVtdUnitInformation[VtdIndex].VtdUnitBaseAddress + R_GCMD_REG, B_GMCD_REG_WBF);
  }

  //
  // Disable VTd
  //
  MmioWrite32 ((UINTN)mVtdUnitInformation[VtdIndex].VtdUnitBaseAddress + R_GCMD_REG, B_GMCD_REG_SRTP);
//  DEBUG((EFI_D_INFO, "InvalidateVtdIOTLBGlobal: waiting for RTPS bit to be set... \n"));
  do {
    Reg32 = MmioRead32 ((UINTN)mVtdUnitInformation[VtdIndex].VtdUnitBaseAddress + R_GSTS_REG);
  } while((Reg32 & B_GSTS_REG_RTPS) == 0);

  //
  // Invalidate the context cache
  //
  Reg64 = MmioRead64 ((UINTN)mVtdUnitInformation[VtdIndex].VtdUnitBaseAddress + R_CCMD_REG);
  if ((Reg64 & B_CCMD_REG_ICC) != 0) {
    DEBUG ((EFI_D_INFO,"ERROR: InvalidateVtdIOTLBGlobal: B_CCMD_REG_ICC is set for VTD(%d)\n",VtdIndex));
    return EFI_DEVICE_ERROR;
  }

  Reg64 &= ((~B_CCMD_REG_ICC) & (~B_CCMD_REG_CIRG_MASK));
  Reg64 |= (B_CCMD_REG_ICC | V_CCMD_REG_CIRG_GLOBAL);
  MmioWrite64 ((UINTN)mVtdUnitInformation[VtdIndex].VtdUnitBaseAddress + R_CCMD_REG, Reg64);

//  DEBUG((EFI_D_INFO, "InvalidateVtdIOTLBGlobal: Waiting B_CCMD_REG_ICC ...\n"));
  do {
    Reg64 = MmioRead64 ((UINTN)mVtdUnitInformation[VtdIndex].VtdUnitBaseAddress + R_CCMD_REG);
  } while ((Reg64 & B_CCMD_REG_ICC) != 0);

  //
  // Invalidate the IOTLB cache
  //
//  DEBUG((EFI_D_INFO, "InvalidateVtdIOTLBGlobal: IRO 0x%x\n", mVtdUnitInformation[VtdIndex].ECapReg.Bits.IRO));

  Reg64 = MmioRead64 ((UINTN)mVtdUnitInformation[VtdIndex].VtdUnitBaseAddress + (mVtdUnitInformation[VtdIndex].ECapReg.Bits.IRO * 16) + R_IOTLB_REG);
  if ((Reg64 & B_IOTLB_REG_IVT) != 0) {
    DEBUG ((EFI_D_INFO,"ERROR: InvalidateVtdIOTLBGlobal: B_IOTLB_REG_IVT is set for VTD(%d)\n", VtdIndex));
    return EFI_DEVICE_ERROR;
  }

  Reg64 &= ((~B_IOTLB_REG_IVT) & (~B_IOTLB_REG_IIRG_MASK));
  Reg64 |= (B_IOTLB_REG_IVT | V_IOTLB_REG_IIRG_GLOBAL);
  MmioWrite64 ((UINTN)mVtdUnitInformation[VtdIndex].VtdUnitBaseAddress + (mVtdUnitInformation[VtdIndex].ECapReg.Bits.IRO * 16) + R_IOTLB_REG, Reg64);
    
//  DEBUG((EFI_D_INFO, "InvalidateVtdIOTLBGlobal: Waiting B_IOTLB_REG_IVT ...\n"));
  do {
    Reg64 = MmioRead64 ((UINTN)mVtdUnitInformation[VtdIndex].VtdUnitBaseAddress + (mVtdUnitInformation[VtdIndex].ECapReg.Bits.IRO * 16) + R_IOTLB_REG);
  } while ((Reg64 & B_IOTLB_REG_IVT) != 0);
   
  //
  // Enable VTd
  //
  MmioWrite32 ((UINTN)mVtdUnitInformation[VtdIndex].VtdUnitBaseAddress + R_GCMD_REG, B_GMCD_REG_TE);
//  DEBUG((EFI_D_INFO, "InvalidateVtdIOTLBGlobal: Waiting B_GSTS_REG_TE ...\n"));
  do {
    Reg32 = MmioRead32 ((UINTN)mVtdUnitInformation[VtdIndex].VtdUnitBaseAddress + R_GSTS_REG);
  } while ((Reg32 & B_GSTS_REG_TE) == 0);

  return EFI_SUCCESS;
}

/**

  This function invalidate VTd IOTLBDomain.

  @param VtdIndex          VTd engine index
  @param DomainIdentifier  VTd Domain Identifier

  @retval EFI_SUCCESS invalidate VTd IOTLBDomain successfully.
**/
EFI_STATUS
InvalidateVtdIOTLBDomain (
  IN UINTN  VtdIndex,
  IN UINT16 DomainIdentifier
  )
{
  UINT64  Reg64;

  if (!mVtdEnabled) {
    return EFI_SUCCESS;
  }

  DEBUG((EFI_D_INFO, "InvalidateVtdIOTLBDomain(%d): 0x%016lx (0x%04x)\n", VtdIndex, DomainIdentifier));
  
  //
  // Invalidate the context cache
  //
  Reg64 = MmioRead64 ((UINTN)mVtdUnitInformation[VtdIndex].VtdUnitBaseAddress + R_CCMD_REG);
  if ((Reg64 & B_CCMD_REG_ICC) != 0) {
    DEBUG ((EFI_D_INFO,"ERROR: InvalidateVtdIOTLBDomain: B_CCMD_REG_ICC is set for VTD(%d)\n",VtdIndex));
    return EFI_DEVICE_ERROR;
  }

  Reg64 &= ((~B_CCMD_REG_ICC) & (~B_CCMD_REG_CIRG_MASK));
  Reg64 |= (B_CCMD_REG_ICC | V_CCMD_REG_CIRG_DOMAIN);
  Reg64 |= (B_CCMD_REG_ICC | V_CCMD_REG_CIRG_DOMAIN);
  MmioWrite64 ((UINTN)mVtdUnitInformation[VtdIndex].VtdUnitBaseAddress + R_CCMD_REG, Reg64);

//  DEBUG((EFI_D_INFO, "InvalidateVtdIOTLBDomain: Waiting B_CCMD_REG_ICC ...\n"));
  do {
    Reg64 = MmioRead64 ((UINTN)mVtdUnitInformation[VtdIndex].VtdUnitBaseAddress + R_CCMD_REG);
  } while ((Reg64 & B_CCMD_REG_ICC) != 0);

  //
  // Invalidate the IOTLB cache
  //
//  DEBUG((EFI_D_INFO, "InvalidateVtdIOTLBDomain: IRO 0x%x\n", mVtdUnitInformation[VtdIndex].ECapReg.Bits.IRO));

  Reg64 = MmioRead64 ((UINTN)mVtdUnitInformation[VtdIndex].VtdUnitBaseAddress + (mVtdUnitInformation[VtdIndex].ECapReg.Bits.IRO * 16) + R_IOTLB_REG);
  if ((Reg64 & B_IOTLB_REG_IVT) != 0) {
    DEBUG ((EFI_D_INFO,"ERROR: InvalidateVtdIOTLBDomain: B_IOTLB_REG_IVT is set for VTD(%d)\n", VtdIndex));
    return EFI_DEVICE_ERROR;
  }

  Reg64 &= ((~B_IOTLB_REG_IVT) & (~B_IOTLB_REG_IIRG_MASK));
  Reg64 |= (B_IOTLB_REG_IVT | V_IOTLB_REG_IIRG_DOMAIN);
  Reg64 |= UEFI_DOMAIN_ID;
  MmioWrite64 ((UINTN)mVtdUnitInformation[VtdIndex].VtdUnitBaseAddress + (mVtdUnitInformation[VtdIndex].ECapReg.Bits.IRO * 16) + R_IOTLB_REG, Reg64);

  //  DEBUG((EFI_D_INFO, "InvalidateVtdIOTLBDomain: Waiting B_IOTLB_REG_IVT ...\n"));
  do {
    Reg64 = MmioRead64 ((UINTN)mVtdUnitInformation[VtdIndex].VtdUnitBaseAddress + (mVtdUnitInformation[VtdIndex].ECapReg.Bits.IRO * 16) + R_IOTLB_REG);
  } while ((Reg64 & B_IOTLB_REG_IVT) != 0);

  return EFI_SUCCESS;
}

/**

  This function invalidate VTd IOTLBPage.

  @param VtdIndex          VTd engine index
  @param Address           Address
  @param AddressMode       Address mode

  @retval EFI_SUCCESS invalidate VTd IOTLBPage successfully.
**/
EFI_STATUS
InvalidateVtdIOTLBPage (
  IN UINTN  VtdIndex,
  IN UINT64 Address,
  IN UINT8  AddressMode
  )
{
  UINT64  Reg64;
  UINT64  Data64;
  
  if (!mVtdEnabled) {
    return EFI_SUCCESS;
  }

  DEBUG((EFI_D_INFO, "InvalidateVtdIOTLBPage(%d): 0x%016lx (0x%02x)\n", VtdIndex, Address, AddressMode));

  if (mVtdUnitInformation[VtdIndex].CapReg.Bits.PSI != 0) {
    Reg64 = MmioRead64 ((UINTN)mVtdUnitInformation[VtdIndex].VtdUnitBaseAddress + (mVtdUnitInformation[VtdIndex].ECapReg.Bits.IRO * 16) + R_IOTLB_REG);
    if ((Reg64 & B_IOTLB_REG_IVT) != 0) {
      DEBUG ((EFI_D_INFO,"ERROR: InvalidateVtdIOTLBPage: B_IOTLB_REG_IVT is set for VTD(%d)\n", VtdIndex));
      return EFI_DEVICE_ERROR;
    }

    Data64 = Address | AddressMode;
    MmioWrite64 ((UINTN)mVtdUnitInformation[VtdIndex].VtdUnitBaseAddress + (mVtdUnitInformation[VtdIndex].ECapReg.Bits.IRO * 16) + R_IVA_REG, Data64);

    Reg64 &= ((~B_IOTLB_REG_IVT) & (~B_IOTLB_REG_IIRG_MASK));
    Reg64 |= (B_IOTLB_REG_IVT | V_IOTLB_REG_IIRG_PAGE);
    Reg64 |= LShiftU64 (UEFI_DOMAIN_ID, 32);
    MmioWrite64 ((UINTN)mVtdUnitInformation[VtdIndex].VtdUnitBaseAddress + (mVtdUnitInformation[VtdIndex].ECapReg.Bits.IRO * 16) + R_IOTLB_REG, Reg64);

  //  DEBUG((EFI_D_INFO, "InvalidateVtdIOTLBPage: Waiting B_IOTLB_REG_IVT ...\n"));
    do {
      Reg64 = MmioRead64 ((UINTN)mVtdUnitInformation[VtdIndex].VtdUnitBaseAddress + (mVtdUnitInformation[VtdIndex].ECapReg.Bits.IRO * 16) + R_IOTLB_REG);
    } while ((Reg64 & B_IOTLB_REG_IVT) != 0);
  } else {
    InvalidateVtdIOTLBGlobal (VtdIndex);
  }

  return EFI_SUCCESS;
}

/**

  This function prepare VTd configuration.

  @return EFI_SUCCESS  prepare VTd configuration successfully.

**/
EFI_STATUS
PrepareVtdConfig (
  VOID
  )
{
  UINTN         Index;

  for (Index = 0; Index < mVtdUnitNumber; Index++) {
    DEBUG ((EFI_D_INFO, "Dump VTd Capability (%d)\n", Index));
    mVtdUnitInformation[Index].CapReg.Uint64 = MmioRead64 ((UINTN)mVtdUnitInformation[Index].VtdUnitBaseAddress + R_CAP_REG);
    DumpVtdCapRegs (&mVtdUnitInformation[Index].CapReg);
    mVtdUnitInformation[Index].ECapReg.Uint64 = MmioRead64 ((UINTN)mVtdUnitInformation[Index].VtdUnitBaseAddress + R_ECAP_REG);
    DumpVtdECapRegs (&mVtdUnitInformation[Index].ECapReg);

    if ((mVtdUnitInformation[Index].CapReg.Bits.SLLPS & BIT0) == 0) {
      DEBUG((EFI_D_INFO, "!!!! 2MB super page is not supported on VTD %d !!!!\n", Index));
      // return EFI_UNSUPPORTED;
    }
    if ((mVtdUnitInformation[Index].CapReg.Bits.SAGAW & BIT2) == 0) {
      DEBUG((EFI_D_INFO, "!!!! 4-level page-table is not supported on VTD %d !!!!\n", Index));
      return EFI_UNSUPPORTED;
    }

    if (!mVtdUnitInformation[Index].PciDeviceInfo.IncludeAllFlag) {
      //
      // BUGBUG: Workaround for VideoController, 2M page table does not work. :-(
      //
      mVtdUnitInformation[Index].CapReg.Bits.SLLPS = 0x00;
    }
  }
  return EFI_SUCCESS;
}

/**

  This function enable DMAR.

  @return EFI_SUCCESS enable DMAR successfully.

**/
EFI_STATUS
EnableDmar (
  VOID
  )
{
  UINTN     Index;
  UINT64    Reg64;
  UINT32    Reg32;

  AsmWbinvd();

  for (Index = 0; Index < mVtdUnitNumber; Index++) {
    if (!mVtdUnitInformation[Index].PciDeviceInfo.IncludeAllFlag) {
      continue;
    }

    DEBUG((EFI_D_INFO, ">>>>>>EnableDmar() for engine [%d] \n", Index));

    if (mVtdUnitInformation[Index].ExtRootEntryTable != NULL) {
      DEBUG((EFI_D_INFO, "ExtRootEntryTable 0x%x \n", mVtdUnitInformation[Index].ExtRootEntryTable));
      MmioWrite64 ((UINTN)mVtdUnitInformation[Index].VtdUnitBaseAddress + R_RTADDR_REG, (UINT64)(UINTN)mVtdUnitInformation[Index].ExtRootEntryTable | BIT11);
    } else {
      DEBUG((EFI_D_INFO, "RootEntryTable 0x%x \n", mVtdUnitInformation[Index].RootEntryTable));
      MmioWrite64 ((UINTN)mVtdUnitInformation[Index].VtdUnitBaseAddress + R_RTADDR_REG, (UINT64)(UINTN)mVtdUnitInformation[Index].RootEntryTable);
    }

    MmioWrite32 ((UINTN)mVtdUnitInformation[Index].VtdUnitBaseAddress + R_GCMD_REG, B_GMCD_REG_SRTP);

    DEBUG((EFI_D_INFO, "EnableDmar: waiting for RTPS bit to be set... \n"));
    do {
      Reg32 = MmioRead32 ((UINTN)mVtdUnitInformation[Index].VtdUnitBaseAddress + R_GSTS_REG);
    } while((Reg32 & B_GSTS_REG_RTPS) == 0);

    //
    // Init DMAr Fault Event and Data registers
    //
    Reg32 = MmioRead32 ((UINTN)mVtdUnitInformation[Index].VtdUnitBaseAddress + R_FEDATA_REG);

    //
    // Write Buffer Flush before invalidation
    //
    Reg32 = MmioRead32 ((UINTN)mVtdUnitInformation[Index].VtdUnitBaseAddress + R_CAP_REG);
    if ((Reg32 & B_CAP_REG_RWBF) != 0) {
      MmioWrite32 ((UINTN)mVtdUnitInformation[Index].VtdUnitBaseAddress + R_GCMD_REG, B_GMCD_REG_WBF);
    }

    //
    // Invalidate the context cache
    //
    Reg64 = MmioRead64 ((UINTN)mVtdUnitInformation[Index].VtdUnitBaseAddress + R_CCMD_REG);
    if ((Reg64 & B_CCMD_REG_ICC) != 0) {
      DEBUG ((EFI_D_INFO,"ERROR: EnableDmar: B_CCMD_REG_ICC is set for VTD(%d)\n",Index));
      return EFI_DEVICE_ERROR;
    }

    Reg64 &= ((~B_CCMD_REG_ICC) & (~B_CCMD_REG_CIRG_MASK));
    Reg64 |= (B_CCMD_REG_ICC | V_CCMD_REG_CIRG_GLOBAL);
    MmioWrite64 ((UINTN)mVtdUnitInformation[Index].VtdUnitBaseAddress + R_CCMD_REG, Reg64);

    DEBUG((EFI_D_INFO, "EnableDmar: Waiting B_CCMD_REG_ICC ...\n"));
    do {
      Reg64 = MmioRead64 ((UINTN)mVtdUnitInformation[Index].VtdUnitBaseAddress + R_CCMD_REG);
    } while ((Reg64 & B_CCMD_REG_ICC) != 0);

    //
    // Invalidate the IOTLB cache
    //
    DEBUG((EFI_D_INFO, "EnableDmar: IRO 0x%x\n", mVtdUnitInformation[Index].ECapReg.Bits.IRO));

    Reg64 = MmioRead64 ((UINTN)mVtdUnitInformation[Index].VtdUnitBaseAddress + (mVtdUnitInformation[Index].ECapReg.Bits.IRO * 16) + R_IOTLB_REG);
    if ((Reg64 & B_IOTLB_REG_IVT) != 0) {
      DEBUG ((EFI_D_INFO,"ERROR: EnableDmar: B_IOTLB_REG_IVT is set for VTD(%d)\n", Index));
      return EFI_DEVICE_ERROR;
    }

    Reg64 &= ((~B_IOTLB_REG_IVT) & (~B_IOTLB_REG_IIRG_MASK));
    Reg64 |= (B_IOTLB_REG_IVT | V_IOTLB_REG_IIRG_GLOBAL);
    MmioWrite64 ((UINTN)mVtdUnitInformation[Index].VtdUnitBaseAddress + (mVtdUnitInformation[Index].ECapReg.Bits.IRO * 16) + R_IOTLB_REG, Reg64);

    DEBUG((EFI_D_INFO, "EnableDmar: Waiting B_IOTLB_REG_IVT ...\n"));
    do {
      Reg64 = MmioRead64 ((UINTN)mVtdUnitInformation[Index].VtdUnitBaseAddress + (mVtdUnitInformation[Index].ECapReg.Bits.IRO * 16) + R_IOTLB_REG);
    } while ((Reg64 & B_IOTLB_REG_IVT) != 0);

    //
    // Enable VTd
    //
    MmioWrite32 ((UINTN)mVtdUnitInformation[Index].VtdUnitBaseAddress + R_GCMD_REG, B_GMCD_REG_TE);
    DEBUG((EFI_D_INFO, "EnableDmar: Waiting B_GSTS_REG_TE ...\n"));
    do {
      Reg32 = MmioRead32 ((UINTN)mVtdUnitInformation[Index].VtdUnitBaseAddress + R_GSTS_REG);
    } while ((Reg32 & B_GSTS_REG_TE) == 0);

    DEBUG ((EFI_D_INFO,"VTD (%d) enabled!<<<<<<\n",Index));
  }

  mVtdEnabled = TRUE;

  return EFI_SUCCESS;
}

/**

  This function disable DMAR.

  @return EFI_SUCCESS disable DMAR successfully.

**/
EFI_STATUS
DisableDmar (
  VOID
  )
{
  UINTN     Index;
  UINT32    Reg32;

  AsmWbinvd();

  for (Index = 0; Index < mVtdUnitNumber; Index++) {
    if (!mVtdUnitInformation[Index].PciDeviceInfo.IncludeAllFlag) {
      continue;
    }

    DEBUG((EFI_D_INFO, ">>>>>>DisableDmar() for engine [%d] \n", Index));

    //
    // Write Buffer Flush before invalidation
    //
    Reg32 = MmioRead32 ((UINTN)mVtdUnitInformation[Index].VtdUnitBaseAddress + R_CAP_REG);
    if ((Reg32 & B_CAP_REG_RWBF) != 0) {
      MmioWrite32 ((UINTN)mVtdUnitInformation[Index].VtdUnitBaseAddress + R_GCMD_REG, B_GMCD_REG_WBF);
    }
    
    //
    // Disable VTd
    //
    MmioWrite32 ((UINTN)mVtdUnitInformation[Index].VtdUnitBaseAddress + R_GCMD_REG, B_GMCD_REG_SRTP);
  //  DEBUG((EFI_D_INFO, "DisableDmar: waiting for RTPS bit to be set... \n"));
    do {
      Reg32 = MmioRead32 ((UINTN)mVtdUnitInformation[Index].VtdUnitBaseAddress + R_GSTS_REG);
    } while((Reg32 & B_GSTS_REG_RTPS) == 0);

    Reg32 = MmioRead32 ((UINTN)mVtdUnitInformation[Index].VtdUnitBaseAddress + R_GSTS_REG);
    DEBUG((EFI_D_INFO, "DisableDmar: GSTS_REG - 0x%08x\n", Reg32));

    DEBUG ((EFI_D_INFO,"VTD (%d) Disabled!<<<<<<\n",Index));
  }

  mVtdEnabled = FALSE;

  return EFI_SUCCESS;
}

/**

  This function dump VTd cap register.

  @param CapReg VTd cap register

**/
VOID
DumpVtdCapRegs (
  IN VTD_CAP_REG *CapReg
  )
{
  DEBUG((EFI_D_INFO, "  CapReg:\n", CapReg->Uint64));
  DEBUG((EFI_D_INFO, "    ND     - 0x%x\n", CapReg->Bits.ND));
  DEBUG((EFI_D_INFO, "    AFL    - 0x%x\n", CapReg->Bits.AFL));
  DEBUG((EFI_D_INFO, "    RWBF   - 0x%x\n", CapReg->Bits.RWBF));
  DEBUG((EFI_D_INFO, "    PLMR   - 0x%x\n", CapReg->Bits.PLMR));
  DEBUG((EFI_D_INFO, "    PHMR   - 0x%x\n", CapReg->Bits.PHMR));
  DEBUG((EFI_D_INFO, "    CM     - 0x%x\n", CapReg->Bits.CM));
  DEBUG((EFI_D_INFO, "    SAGAW  - 0x%x\n", CapReg->Bits.SAGAW));
  DEBUG((EFI_D_INFO, "    MGAW   - 0x%x\n", CapReg->Bits.MGAW));
  DEBUG((EFI_D_INFO, "    ZLR    - 0x%x\n", CapReg->Bits.ZLR));
  DEBUG((EFI_D_INFO, "    FRO    - 0x%x\n", CapReg->Bits.FRO_Lo + (CapReg->Bits.FRO_Hi << 8)));
  DEBUG((EFI_D_INFO, "    SLLPS  - 0x%x\n", CapReg->Bits.SLLPS));
  DEBUG((EFI_D_INFO, "    PSI    - 0x%x\n", CapReg->Bits.PSI));
  DEBUG((EFI_D_INFO, "    NFR    - 0x%x\n", CapReg->Bits.NFR));
  DEBUG((EFI_D_INFO, "    MAMV   - 0x%x\n", CapReg->Bits.MAMV));
  DEBUG((EFI_D_INFO, "    DWD    - 0x%x\n", CapReg->Bits.DWD));
  DEBUG((EFI_D_INFO, "    DRD    - 0x%x\n", CapReg->Bits.DRD));
  DEBUG((EFI_D_INFO, "    FL1GP  - 0x%x\n", CapReg->Bits.FL1GP));
  DEBUG((EFI_D_INFO, "    PI     - 0x%x\n", CapReg->Bits.PI));
}

/**

  This function dump VTd Ecap register.

  @param ECapReg VTd Ecap register

**/
VOID
DumpVtdECapRegs (
  IN VTD_ECAP_REG *ECapReg
  )
{
  DEBUG((EFI_D_INFO, "  ECapReg:\n", ECapReg->Uint64));
  DEBUG((EFI_D_INFO, "    C      - 0x%x\n", ECapReg->Bits.C));
  DEBUG((EFI_D_INFO, "    QI     - 0x%x\n", ECapReg->Bits.QI));
  DEBUG((EFI_D_INFO, "    DT     - 0x%x\n", ECapReg->Bits.DT));
  DEBUG((EFI_D_INFO, "    IR     - 0x%x\n", ECapReg->Bits.IR));
  DEBUG((EFI_D_INFO, "    EIM    - 0x%x\n", ECapReg->Bits.EIM));
  DEBUG((EFI_D_INFO, "    PT     - 0x%x\n", ECapReg->Bits.PT));
  DEBUG((EFI_D_INFO, "    SC     - 0x%x\n", ECapReg->Bits.SC));
  DEBUG((EFI_D_INFO, "    IRO    - 0x%x\n", ECapReg->Bits.IRO));
  DEBUG((EFI_D_INFO, "    MHMV   - 0x%x\n", ECapReg->Bits.MHMV));
  DEBUG((EFI_D_INFO, "    ECS    - 0x%x\n", ECapReg->Bits.ECS));
  DEBUG((EFI_D_INFO, "    MTS    - 0x%x\n", ECapReg->Bits.MTS));
  DEBUG((EFI_D_INFO, "    NEST   - 0x%x\n", ECapReg->Bits.NEST));
  DEBUG((EFI_D_INFO, "    DIS    - 0x%x\n", ECapReg->Bits.DIS));
  DEBUG((EFI_D_INFO, "    PASID  - 0x%x\n", ECapReg->Bits.PASID));
  DEBUG((EFI_D_INFO, "    PRS    - 0x%x\n", ECapReg->Bits.PRS));
  DEBUG((EFI_D_INFO, "    ERS    - 0x%x\n", ECapReg->Bits.ERS));
  DEBUG((EFI_D_INFO, "    SRS    - 0x%x\n", ECapReg->Bits.SRS));
  DEBUG((EFI_D_INFO, "    NWFS   - 0x%x\n", ECapReg->Bits.NWFS));
  DEBUG((EFI_D_INFO, "    EAFS   - 0x%x\n", ECapReg->Bits.EAFS));
  DEBUG((EFI_D_INFO, "    PSS    - 0x%x\n", ECapReg->Bits.PSS));
}

/**

  This function dump VTd register.

**/
VOID
DumpVtdRegs (
  VOID
  )
{
  UINTN       Num;
  UINTN       Index;
  UINT64      Reg64;
  UINT64      Reg64_2;
  VTD_CAP_REG CapReg;
  UINT32      Reg32;

  for (Num = 0; Num < mVtdUnitNumber; Num++) {
    DEBUG((EFI_D_INFO, "#### DumpVtdRegs(%d) Begin ####\n", Num));

    Reg32 = MmioRead32 ((UINTN)mVtdUnitInformation[Num].VtdUnitBaseAddress + R_VER_REG);
    DEBUG((EFI_D_INFO, "  VER_REG     - 0x%08x\n", Reg32));

    CapReg.Uint64 = MmioRead64 ((UINTN)mVtdUnitInformation[Num].VtdUnitBaseAddress + R_CAP_REG);
    DEBUG((EFI_D_INFO, "  CAP_REG     - 0x%016lx\n", CapReg.Uint64));

    Reg64 = MmioRead64 ((UINTN)mVtdUnitInformation[Num].VtdUnitBaseAddress + R_ECAP_REG);
    DEBUG((EFI_D_INFO, "  ECAP_REG    - 0x%016lx\n", Reg64));

    Reg32 = MmioRead32 ((UINTN)mVtdUnitInformation[Num].VtdUnitBaseAddress + R_GSTS_REG);
    DEBUG((EFI_D_INFO, "  GSTS_REG    - 0x%08x \n", Reg32));

    Reg64 = MmioRead64 ((UINTN)mVtdUnitInformation[Num].VtdUnitBaseAddress + R_RTADDR_REG);
    DEBUG((EFI_D_INFO, "  RTADDR_REG  - 0x%016lx\n", Reg64));

    Reg64 = MmioRead64 ((UINTN)mVtdUnitInformation[Num].VtdUnitBaseAddress + R_CCMD_REG);
    DEBUG((EFI_D_INFO, "  CCMD_REG    - 0x%016lx\n", Reg64));

    Reg32 = MmioRead32 ((UINTN)mVtdUnitInformation[Num].VtdUnitBaseAddress + R_FSTS_REG);
    DEBUG((EFI_D_INFO, "  FSTS_REG    - 0x%08x\n", Reg32));

    Reg32 = MmioRead32 ((UINTN)mVtdUnitInformation[Num].VtdUnitBaseAddress + R_FECTL_REG);
    DEBUG((EFI_D_INFO, "  FECTL_REG   - 0x%08x\n", Reg32));

    Reg32 = MmioRead32 ((UINTN)mVtdUnitInformation[Num].VtdUnitBaseAddress + R_FEDATA_REG);
    DEBUG((EFI_D_INFO, "  FEDATA_REG  - 0x%08x\n", Reg32));

    Reg32 = MmioRead32 ((UINTN)mVtdUnitInformation[Num].VtdUnitBaseAddress + R_FEADDR_REG);
    DEBUG((EFI_D_INFO, "  FEADDR_REG  - 0x%08x\n",Reg32));

    Reg32 = MmioRead32 ((UINTN)mVtdUnitInformation[Num].VtdUnitBaseAddress + R_FEUADDR_REG);
    DEBUG((EFI_D_INFO, "  FEUADDR_REG - 0x%08x\n",Reg32));

    for (Index = 0; Index < CapReg.Bits.NFR + 1; Index++) {
      Reg64 = MmioRead64 ((UINTN)mVtdUnitInformation[Num].VtdUnitBaseAddress + (((CapReg.Bits.FRO_Lo + (CapReg.Bits.FRO_Hi << 8)) * 16) + (Index * 16) + R_FRCD_REG));
      Reg64_2 = MmioRead64 ((UINTN)mVtdUnitInformation[Num].VtdUnitBaseAddress + (((CapReg.Bits.FRO_Lo + (CapReg.Bits.FRO_Hi << 8)) * 16) + (Index * 16) + R_FRCD_REG + sizeof(UINT64)));
      DEBUG((EFI_D_INFO, "  FRCD_REG[%d] - 0x%016lx %016lx\n", Index, Reg64_2, Reg64));
    }

    Reg64 = MmioRead64 ((UINTN)mVtdUnitInformation[Num].VtdUnitBaseAddress + (mVtdUnitInformation[Num].ECapReg.Bits.IRO * 16) + R_IVA_REG);
    DEBUG((EFI_D_INFO, "  IVA_REG     - 0x%016lx\n",Reg64));

    Reg64 = MmioRead64 ((UINTN)mVtdUnitInformation[Num].VtdUnitBaseAddress + (mVtdUnitInformation[Num].ECapReg.Bits.IRO * 16) + R_IOTLB_REG);
    DEBUG((EFI_D_INFO, "  IOTLB_REG   - 0x%016lx\n",Reg64));

    DEBUG((EFI_D_INFO, "#### DumpVtdRegs(%d) End ####\n", Num));
  }
}

