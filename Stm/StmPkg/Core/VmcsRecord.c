/** @file
  STM VMCS resource management

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Stm.h"

/**

  This function return VMCS record from VMCS database.

  @param VmcsDatabase VMCS database
  @param Vmcs         VMCS to be found

  @return VMCS record

**/
VMCS_RECORD_STRUCTURE *
GetVmcsRecord (
  IN UINT64  VmcsDatabase,
  IN UINT64  Vmcs
  )
/*
  VMCS Database ---->+----------------------+
                     | VMCS Database Table  |
                     +----------------------+
                     | VMCS Database Table  |
                     +----------------------+
                     |                      |
                     +----------------------+
*/
{
  VMCS_RECORD_STRUCTURE              *VmcsDatabaseTable;
  UINTN                              Index;

  if (VmcsDatabase == 0) {
    return NULL;
  }

  VmcsDatabaseTable = (VMCS_RECORD_STRUCTURE *)(UINTN)(VmcsDatabase);

  for (Index = 0; ; Index++) {
    if (Vmcs == VmcsDatabaseTable[Index].VmcsPhysPointer) {
      return &VmcsDatabaseTable[Index];
    }

    if (VmcsDatabaseTable[Index].Type == VMCS_RECORD_LAST) {
      return NULL;
    }
  }
}

/**

  This function process VMCS database request.

  @param VmcsDatabaseRequest VMCS database request
  @param VmcsDatabaseTable   VMCS database table

  @return STM_SUCCESS                     request processed
  @return ERROR_INVALID_PARAMETER         request error
  @return ERROR_STM_INVALID_VMCS_DATABASE VMCS database table error

**/
STM_STATUS
RequestVmcsDatabaseEntry (
  IN STM_VMCS_DATABASE_REQUEST          *VmcsDatabaseRequest,
  IN VMCS_RECORD_STRUCTURE              *VmcsDatabaseTable
  )
{
  UINTN                              Index;

  if (VmcsDatabaseTable == NULL) {
    return ERROR_STM_INVALID_VMCS_DATABASE;
  }

  if (VmcsDatabaseRequest->Reserved1 != 0) {
    return ERROR_INVALID_PARAMETER;
  }

  if (VmcsDatabaseRequest->AddOrRemove == STM_VMCS_DATABASE_REQUEST_REMOVE) {
    for (Index = 0; ; Index++) {
      if (VmcsDatabaseTable[Index].Type == VMCS_RECORD_OCCUPIED) {
        if (VmcsDatabaseTable[Index].VmcsPhysPointer == VmcsDatabaseRequest->VmcsPhysPointer) {
          VmcsDatabaseTable[Index].Type = VMCS_RECORD_EMPTY;
          return STM_SUCCESS;
        }
      }

      if (VmcsDatabaseTable[Index].Type == VMCS_RECORD_LAST) {
        return ERROR_STM_INVALID_VMCS_DATABASE;
      }
    }
  } else if (VmcsDatabaseRequest->AddOrRemove == STM_VMCS_DATABASE_REQUEST_ADD) {
    for (Index = 0; ; Index++) {
      if (VmcsDatabaseTable[Index].Type == VMCS_RECORD_EMPTY) {
        VmcsDatabaseTable[Index].VmcsPhysPointer   = VmcsDatabaseRequest->VmcsPhysPointer;
        VmcsDatabaseTable[Index].DomainType        = VmcsDatabaseRequest->DomainType;
        VmcsDatabaseTable[Index].XStatePolicy      = VmcsDatabaseRequest->XStatePolicy;
        VmcsDatabaseTable[Index].DegradationPolicy = VmcsDatabaseRequest->DegradationPolicy;
        VmcsDatabaseTable[Index].Type              = VMCS_RECORD_OCCUPIED;
        return STM_SUCCESS;
      }

      if (VmcsDatabaseTable[Index].Type == VMCS_RECORD_LAST) {
        return ERROR_STM_INVALID_VMCS_DATABASE;
      }
    }
  } else {
    return ERROR_INVALID_PARAMETER;
  }
}

/**

  This function dump VMCS database.

  @param VmcsDatabase VMCS database

**/
VOID
DumpVmcsRecord (
  IN UINT64                             VmcsDatabase
  )
{
  VMCS_RECORD_STRUCTURE              *VmcsDatabaseTable;
  UINTN                              Index;

  //
  // Should be physical address
  //
  if (VmcsDatabase == 0) {
    return ;
  }

  VmcsDatabaseTable = (VMCS_RECORD_STRUCTURE *)(UINTN)(VmcsDatabase);

  for (Index = 0; ; Index++) {
    if (VmcsDatabaseTable[Index].Type == VMCS_RECORD_EMPTY) {
      continue ;
    }

    if (VmcsDatabaseTable[Index].Type == VMCS_RECORD_LAST) {
      return ;
    }

    if (VmcsDatabaseTable[Index].Type != VMCS_RECORD_OCCUPIED) {
      // Something wrong
      DEBUG ((EFI_D_ERROR, "Invalid VMCS_RECORD %08x - %08x\n", Index, (UINTN)VmcsDatabaseTable[Index].Type));
      CpuDeadLoop ();
      return ;
    }

    DEBUG ((EFI_D_INFO, "VmcsRecord:\n"));
    DEBUG ((EFI_D_INFO, "  Index             : %08x\n", Index));
    DEBUG ((EFI_D_INFO, "  VmcsPhysPointer   : %016lx\n", VmcsDatabaseTable[Index].VmcsPhysPointer));
    DEBUG ((EFI_D_INFO, "  DomainType        : %08x\n", (UINTN)VmcsDatabaseTable[Index].DomainType));
    DEBUG ((EFI_D_INFO, "  XStatePolicy      : %08x\n", (UINTN)VmcsDatabaseTable[Index].XStatePolicy));
    DEBUG ((EFI_D_INFO, "  DegradationPolicy : %08x\n", (UINTN)VmcsDatabaseTable[Index].DegradationPolicy));
    DEBUG ((EFI_D_INFO, "  Type              : %08x\n", (UINTN)VmcsDatabaseTable[Index].Type));
  }
}
