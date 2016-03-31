/** @file
  STM platform SMM resource

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _STM_PLATFORM_RESOURCE_H_
#define _STM_PLATFORM_RESOURCE_H_

#define MASK0   0
#define MASK64  0xFFFFFFFFFFFFFFFFull

//
// LPC
//
#define LPC_BUS                 0
#define LPC_DEVICE              31
#define LPC_FUNCTION            0
#define R_ACPI_PM_BASE          0x40
#define ACPI_PM_BASE_MASK       0xFFF8

//
// MSRs
//
#define IA32_APIC_BASE_MSR_INDEX            0x1B
#define SMRR_PHYSBASE_MSR                   0x1F2
#define SMRR_PHYSMASK_MSR                   0x1F3 

/**

  Add resources to BIOS resource database.

**/
VOID
AddResourcesCmd (
  VOID
  );

#endif
