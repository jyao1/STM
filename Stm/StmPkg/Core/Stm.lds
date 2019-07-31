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
	/* HARDWARE_STM_HEADER */
    	. = 0x0;
	.StmHeaderRevision : {LONG(0)}
	.MonitorFeatures   : {LONG(1)}
	.GdtrLimit	   : {LONG(0x47)}
	.GdtrBaseOffset    : {LONG(0x1000)}
	.CsSelector	   : {LONG(0x38)}
	.EipOffset         : {LONG(_ModuleEntryPoint)}
	.EspOffset         : {LONG(0)}
	.Cr3Offset         : {LONG(0)}

	/* SOFTWARE_STM_HEADER */

	.StmSpecVerMajor ALIGN(2048) : {BYTE(1)}
	.StmSpecVerMinor   	     : {BYTE(0)}
	.Reserved                    : {SHORT(0)}
	.StaticImageSize             : {LONG(0)}
	.PerProcDynamicMemorySize    : {LONG(0x8000)}
	.AdditionalDynamicMemorySize : {LONG(0x246000)}

	.StmFeatures	   : {LONG(0)}
	.NumberOfRevIDs    : {LONG(0)}
	.StmSmmRevIDs0     : {LONG(0)}
	. = ALIGN(0x1000);
	
	/* GDT Entriess */
	.LimitLow0 	   : {SHORT(0)}
	.BaseLow0	   : {SHORT(0)}
        .BaseMid0          : {BYTE(0)}
	.Attribute0        : {BYTE(0)}
        .LimitHi0          : {BYTE(0)}
        .BaseHi0           : {BYTE(0)}      

	.LimitLo1          : {SHORT(0)}
        .BaseLow1          : {SHORT(0)}
        .BaseMid1          : {BYTE(0)}
        .Attribute1        : {BYTE(0)}
        .LimitHi1          : {BYTE(0)}
        .BaseHi1           : {BYTE(0)}      

	.LimitLow2         : {SHORT(0xFFFF)}
        .BaseLow2          : {SHORT(0)}
        .BaseMid2          : {BYTE(0)}
        .Attribute2        : {BYTE(0x9B)}
        .LimitHi2          : {BYTE(0xCF)}
        .BaseHi2           : {BYTE(0)}      

	.LimitLow3         : {SHORT(0xFFFF)}
        .BaseLow3          : {SHORT(0)}
        .BaseMid3          : {BYTE(0)}
        .Attribute3        : {BYTE(0x93)}
        .LimitHi3          : {BYTE(0xCF)}
        .BaseHi3           : {BYTE(0)}      

	.LimitLow4         : {SHORT(0)}
        .BaseLow4          : {SHORT(0)}
        .BaseMid4          : {BYTE(0)}
        .Attribute4        : {BYTE(0)}
        .LimitHi4          : {BYTE(0)}
        .BaseHi4           : {BYTE(0)}      

	.LimitLow5         : {SHORT(104)}
        .BaseLow5          : {SHORT(0)}
        .BaseMid5          : {BYTE(0)}
        .Attribute5        : {BYTE(0x89)}
        .LimitHi5          : {BYTE(0x80)}
        .BaseHi5           : {BYTE(0)}      

	.LimitLowr6        : {SHORT(0)}
        .BaseLow6          : {SHORT(0)}
        .BaseMid6          : {BYTE(0)}
        .Attribute6        : {BYTE(0)}
        .LimitHi6          : {BYTE(0)}
        .BaseHi6           : {BYTE(0)}      

	.LimitLow7         : {SHORT(0xFFFF)}
        .BaseLow7          : {SHORT(0)}
        .BaseMid7          : {BYTE(0)}
        .Attribute7        : {BYTE(0x9B)}
        .LimitHi7          : {BYTE(0xAF)}
        .BaseHi7           : {BYTE(0)}      

	.LimitLow8         : {SHORT(0xFFFF)}
        .BaseLow8          : {SHORT(0)}
        .BaseMid8          : {BYTE(0)}
        .Attribute8        : {BYTE(0x93)}
        .LimitHi8          : {BYTE(0xCF)}
        .BaseHi8           : {BYTE(0)}      

	
    . = ALIGN(0x1000);
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
