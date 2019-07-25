/*
 *     PCD Data file for the EDK stuff
 *
 */

#ifndef __PCDDATAG__
#define __PCDDATAG__

#ifdef __cplusplus
extern "C" {
#endif

#include <Base.h>
#include <Library/PcdLib.h>

extern const UINT32 _gPcd_FixedAtBuild_PcdMaximumUnicodeStringLength;
extern const UINT32 _gPcd_FixedAtBuild_PcdMaximumAsciiStringLength;
extern const UINT32 _gPcd_FixedAtBuild_PcdDebugPrintErrorLevel;
extern const UINT8 _gPcd_FixedAtBuild_PcdDebugPropertyMask;
extern const UINT8 _gPcd_FixedAtBuild_PcdDebugClearMemoryValue;
extern volatile  UINT64  _gPcd_BinaryPatch_PcdPciExpressBaseAddress;
extern const  UINT8  _gPcd_FixedAtBuild_PcdPerformanceLibraryPropertyMask;


#define _PCD_GET_MODE_32_PcdMaximumUnicodeStringLength  _gPcd_FixedAtBuild_PcdMaximumUnicodeStringLength

#define _PCD_GET_MODE_32_PcdMaximumAsciiStringLength  _gPcd_FixedAtBuild_PcdMaximumAsciiStringLength

#define _PCD_GET_MODE_32_PcdDebugPrintErrorLevel  _gPcd_FixedAtBuild_PcdDebugPrintErrorLevel
#define _PCD_GET_MODE_8_PcdDebugPropertyMask  _gPcd_FixedAtBuild_PcdDebugPropertyMask

#define _PCD_GET_MODE_8_PcdDebugClearMemoryValue  _gPcd_FixedAtBuild_PcdDebugClearMemoryValue

#define _PCD_GET_MODE_64_PcdPciExpressBaseAddress  _gPcd_BinaryPatch_PcdPciExpressBaseAddress

#define _PCD_GET_MODE_8_PcdPerformanceLibraryPropertyMask  _gPcd_FixedAtBuild_PcdPerformanceLibraryPropertyMask

#define _PCD_SET_MODE_64_PcdPciExpressBaseAddress(Value)  (_gPcd_BinaryPatch_PcdPciExpressBaseAddress = (Value))

#ifdef __cplusplus
}
#endif

#endif


