/** @file
  STM GCC linker scriptor
  
  NOTE: We need to make sure "virtual address" is same as "file pointer to raw data"
  in final PE/COFF image, so that we can let StmLoad load binrary to memory directly.

  Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

SECTIONS
{
    . = 0x0 + 0x280;
    . = ALIGN(0x20);
    .text      : {
      *(.text)
    }
    . = ALIGN(0x20);
    .data : {
      *(.data)
    }
    . = ALIGN(0x20);
    .rdata : {
      *(.rdata)
    }
    . = ALIGN(0x20);
    .bss : {
      *(.bss)
      *(COMMON)
      /* dummy */
      /* LONG (0x12345678) */
    }
    . = ALIGN(0x20);
    .edata : {
    }
    . = ALIGN(0x20);
    .reloc : {
    }
}
