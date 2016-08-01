/** @file
  StmPlatform library header file

  Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _STM_PLATFORM_LIB_H_
#define _STM_PLATFORM_LIB_H_

/**
  This is STM platform hook to set MSR bitmap.
**/
VOID
EFIAPI
StmPlatformLibSetMsrBitmaps (
  VOID
  );

/**
  This is STM platform hook for some specific MSR read.

  @param  MsrIndex   MSR index
  @param  Data       MSR data

  @return TRUE  This MSR read is handled.
  @return FALSE This MSR read is NOT handled.
**/
BOOLEAN
EFIAPI
StmPlatformLibMsrRead (
  IN  UINT32     MsrIndex,
  OUT UINT64     *Data
  );

/**
  This is STM platform hook for some specific MSR write.

  @param  MsrIndex   MSR index
  @param  Data       MSR data

  @return TRUE  This MSR write is handled.
  @return FALSE This MSR write is NOT handled.
**/
BOOLEAN
EFIAPI
StmPlatformLibMsrWrite (
  IN UINT32     MsrIndex,
  IN UINT64     Data
  );

//
// External function
//

/**

  This function set MSR bitmap.

  @param MsrIndex MSR index
  @param MsrWrite TRUE means MsrWrite, FALSE means MsrRead

**/
VOID
EFIAPI
SetMsrBitmap (
  IN UINT32  MsrIndex,
  IN BOOLEAN MsrWrite
  );

#endif
