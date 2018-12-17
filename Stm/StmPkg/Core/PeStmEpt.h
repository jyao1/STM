/** @file

PE EPT Header

This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/
#ifndef _PESTMEPT_H_
#define _PESTMEPT_H_

UINT32 PeMapRegionEpt(UINT32 VmType,
                      UINTN membase,
                      UINTN memsize,
                      UINTN physbase,
                      BOOLEAN isRead,
                      BOOLEAN isWrite,
                      BOOLEAN isExec,
                      UINT32 CpuIndex); 

#endif /* STMPEEPT_H_ */