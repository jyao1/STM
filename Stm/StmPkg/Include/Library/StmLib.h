/** @file
  STM library header file

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _STM_LIB_H_
#define _STM_LIB_H_

#include "Library/Vmx.h"
#include "Library/Smx.h"

// It must be 64-byte aligned
typedef struct {
  // IA32_FX_BUFFER
  UINT8  Buffer[512];
  // Header
  UINT8  Header[64];
  // Ext_Save_Area_2
  // Ext_Save_Area_3
  // ......
  // Ext_Save_Area_63
} IA32_X_BUFFER;

/**

  This function save XState.

  @param Mask   XState save mask
  @param Buffer XState buffer

**/
VOID
AsmXSave (
  IN  UINT64         Mask,
  OUT IA32_X_BUFFER  *Buffer
  );

/**

  This function restore XState.

  @param Mask   XState restore mask
  @param Buffer XState buffer

**/
VOID
AsmXRestore (
  IN UINT64         Mask,
  IN IA32_X_BUFFER  *Buffer
  );

/**

  This function get eXtended Control Register value.

  @param Index  XCR index

  @return XCR value

**/
UINT64
AsmXGetBv (
  IN UINT32  Index
  );

/**

  This function set eXtended Control Register value.

  @param Index  XCR index
  @param Value  XCR value

**/
VOID
AsmXSetBv (
  IN UINT32  Index,
  IN UINT64  Value
  );

/**

  This function LOCK test and set bit, and return orginal bit.

  @param BitIndex  Bit index
  @param Address   Bit string address

  @return Original bit value

**/
UINT32
AsmTestAndSet (
  IN UINT32  BitIndex,
  IN VOID    *Address
  );

/**

  This function LOCK test and reset (clear) bit, and return orginal bit.

  @param BitIndex  Bit index
  @param Address   Bit string address

  @return Original bit value

**/
UINT32
AsmTestAndReset (
  IN UINT32  BitIndex,
  IN VOID    *Address
  );

#endif
