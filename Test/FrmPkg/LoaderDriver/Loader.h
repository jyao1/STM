/** @file

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _LOADER_H_
#define _LOADER_H_

#define LOW_MEMORY_SIZE            0x4000     // 16K
#define HIGH_MEMORY_SIZE_COMMON    0x4000000  // 64M
#define HIGH_MEMORY_SIZE_PER_CPU   0x2000000  // 32M
#define EBDA_MEMORY_SIZE           0x1000     // 4K
#define EBDA_BASE_ADDRESS          0x40E

#endif
