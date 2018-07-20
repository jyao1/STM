
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