/** @file
  STM initialization header file

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _STM_INIT_H_
#define _STM_INIT_H_

#include "Stm.h"

/**

  This function is host entrypoint for SMI.

**/
VOID
AsmHostEntrypointSmi (
  VOID
  );

/**

  This function is host entrypoint for SMM.

**/
VOID
AsmHostEntrypointSmm (
  VOID
  );

/**

  This function create Ia32e page table for SMM guest.

  @return pages table address

**/
UINTN
CreateIa32ePageTable (
  VOID
  );

/**

  This function create compatible page table for SMM guest.

  @return pages table address

**/
UINTN
CreateCompatiblePageTable (
  VOID
  );

/**

  This function create compatible PAE page table for SMM guest.

  @return pages table address

**/
UINTN
CreateCompatiblePaePageTable (
  VOID
  );

/**

  This function create page table for STM host.
  The SINIT/StmLoader should already configured 4G paging, so here
  we just create >4G paging for X64 mode.

**/
VOID
CreateHostPaging (
  VOID
  );

/**
  Check if 1-GByte pages is supported by processor or not.

  @retval TRUE   1-GByte pages is supported.
  @retval FALSE  1-GByte pages is not supported.

**/
BOOLEAN
Is1GPageSupport (
  VOID
  );

/**

  This function initialize VMCS for SMI.

  @param Index CPU index
  @param Vmcs  VMCS pointer

**/
VOID
InitializeSmiVmcs (
  IN UINT32   Index,
  IN UINT64   *Vmcs
  );

/**

  This function initialize VMCS for SMM.

  @param Index CPU index
  @param Vmcs  VMCS pointer

**/
VOID
InitializeSmmVmcs (
  IN UINT32   Index,
  IN UINT64   *Vmcs
  );

/**

  This function initialize event log.

**/
VOID
InitializeEventLog (
  VOID
  );

/**

  This function run BIOS SMM provided SetupRip.

  @param Index CPU index

**/
VOID
SmmSetup (
  IN UINT32  Index
  );

/**

  This function initialize STM handle for SMI.

**/
VOID
InitStmHandlerSmi (
  VOID
  );

/**

  This function initialize STM handle for SMM.

**/
VOID
InitStmHandlerSmm (
  VOID
  );

/**

  This function initialize EPT.

**/
VOID
EptInit (
  VOID
  );

/**

  This function initialize IO bitmap.

**/
VOID
IoInit (
  VOID
  );

/**

  This function initialize MSR bitmap.

**/
VOID
MsrInit (
  VOID
  );

/**

  This function register BIOS resource list to VMCS.

  @param Resource          Resource list to be registered

**/
VOID
RegisterBiosResource (
  IN STM_RSC   *Resource
  );

#endif
