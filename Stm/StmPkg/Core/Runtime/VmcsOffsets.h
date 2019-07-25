/** @file

Mostly used to dynamically generate the VMCS Offsets for XHIM

add license stuff
*/

#ifndef _VMCSOFFSETS_H_
#define _VMCSOFFSETS_H_

#include "Library/Vmx.h"

typedef struct VmcsFieldOffset
{
	UINT32 FieldEncoding;
	UINT32 FieldOffset;
} VMCSFIELDOFFSET;

// initialize all fields to zero

typedef struct VmcsFieldPrint
{
	UINT64 FieldEncoding;
    char * FieldPrint;
} VMCSFIELDPRINT;


#endif
