/** @file

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _DUMP_H_
#define _DUMP_H_

#include <Base.h>

/**

  This function dump VMX capability MSR.

**/
VOID
DumpVmxCapabillityMsr (
  VOID
  );

/**

  This function dump VMCS all field.

**/
VOID
DumpVmcsAllField (
  VOID
  );

/**

  This function dump X86 register context.

  @param Reg X86 register context

**/
VOID
DumpRegContext (
  IN X86_REGISTER *Reg
  );

#endif