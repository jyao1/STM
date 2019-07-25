/** @file

APIC Handler

This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmRuntime.h"
#include "PeStm.h"

extern PE_SMI_CONTROL PeSmiControl;


// function to signal to the PE VM that it needs to handle a SMI

#define ICR_LOW  0x300
#define ICR_HIGH 0x310
#define APIC_REG(offset) (*(UINTN *)(apicAddress + offset))

static UINTN apicAddress;

#define APIC_EN 1 << 11
#define APIC_EXTD 1 << 10

#define LOCAL_APIC_DISABLED 0
#define APIC_INVALID        APIC_EXTD

// apic modes
#define xAPIC_MODE          APIC_EN
#define x2APIC_MODE         APIC_EN|APIC_EXTD

#ifndef __writemsr
static __inline__ __attribute__((always_inline)) void __writemsr (UINT32 msr, UINT64 Value)
{
	__asm__ __volatile__ (
	  "wrmsr"
	  :
	  : "c" (msr), "A" (Value)
	  );
}
#endif

// from LocalApic.h

// lower half of Interrupt Command Register (ICR)

typedef union
{
	struct
	{
		UINT32	Vector:8;               // the vector number of the interrupt being sent
		UINT32	DeliveryMode:3;         // Specifies the type of IPI being sent
		UINT32  DestinationMode:1;      // 0: physical destination mode, 1: logical destination mode
		UINT32  DeliveryStatus:1;       // Indicates the IPI delivery status. Reserved in x2APIC mode
		UINT32  Reserved0:1;            // Reserved
		UINT32  Level:1;                // 0 for the INIT level de-assert delivery mode. Otherwise 1
		UINT32  TriggerMode:1;          // 0: edge, 1: level when using the INIT level de-assert delivery mode
		UINT32  Reserved1:2;            // Reserved
		UINT32  DestinationShorthand:2; // A shorthand notation to specify the destination of the message
		UINT32  Reserved2:12;           // Reserved
	} Bits;
	UINT32 Uint32;
}   LOCAL_APIC_ICR_LOW;

#define LOCAL_APIC_DELIVERY_MODE_SMI                        2
#define LOCAL_APIC_DESTINATION_SHORTHAND_ALL_EXCLUDING_SELF 3

void SignalPeVm(UINT32 CpuIndex)
{
	UINT32 low, high;
	UINT64 ApicMsr;
	//	UINT32 mine = 0;

#ifdef SHOWSMI
	if(CpuIndex == 0)
	{
		UINT16 pmbase = get_pmbase();
		DEBUG((EFI_D_ERROR, " %ld SignalPeVm ****SMI*** smi_en: %x smi_sts: %x\n", CpuIndex, IoRead32(pmbase + SMI_EN), IoRead32(pmbase + SMI_STS)));
	}
#endif

	if((PeSmiControl.PeExec == 1) && (InterlockedCompareExchange32(&PeSmiControl.PeNmiBreak, 0, 1) == 0))
	{
		ApicMsr = AsmReadMsr64 (IA32_APIC_BASE_MSR_INDEX);

		apicAddress = (UINTN)(ApicMsr & 0xffffff000);   // assume the default for now

		low = APIC_REG(ICR_LOW) & 0xFFF32000;
		high = APIC_REG(ICR_HIGH) & 0x00FFFFFF;

		//high |= (PE_APIC_ID << 24); // put the destination apic ID into the upper eight bits
		high = PeSmiControl.PeApicId << 24;
		//low  |= 0x4400;             // bits: (8-10) NMI, (14) asset trigger mode
		low =  0x400;

		switch (ApicMsr & (APIC_EN|APIC_EXTD))
		{
		case xAPIC_MODE:
			APIC_REG(ICR_HIGH) = high;   // write high before low
			APIC_REG(ICR_LOW)  = low;    // because writing to low triggers the IPI
			break;

		case x2APIC_MODE:
			// x2APIC uses MSR's
			__writemsr(0x800 + 0x30, low |((UINT64)PeSmiControl.PeApicId << 32));
			break;

		default:
			DEBUG((EFI_D_ERROR, " %ld SignalPeVm - APIC mode invalid or APIC disabled\n", CpuIndex)); 
		}   
		DEBUG((EFI_D_ERROR, "%ld SignalPeVm - Sent NMI to ApicId: %ld CpuIndex: %ld command: high: %08lx low: %08lx APIC_MSR: %p\n", 
			CpuIndex, 
			PeSmiControl.PeApicId,
			PeSmiControl.PeCpuIndex,
			high, 
			low,
			ApicMsr));
	}
#ifdef SMIVMPE
	else
	{
		if(PeSmiControl.PeExec == 1)
		{
			DEBUG((EFI_D_ERROR, " %ld - ***+++*** SMI present with PE/VM active\n", CpuIndex)); 
		}
	}
#endif

}

void SendSmiToOtherProcessors(UINT32 CpuIndex)
{
	LOCAL_APIC_ICR_LOW low;
	UINT32 high;
	UINTN highSave;
	UINT64 ApicMsr;

	ApicMsr = AsmReadMsr64 (IA32_APIC_BASE_MSR_INDEX);

	apicAddress = (UINTN)(ApicMsr & 0xffffff000);   // assume the default for now

	highSave = APIC_REG(ICR_HIGH);
	low.Uint32 =  0;
	low.Bits.DeliveryMode = LOCAL_APIC_DELIVERY_MODE_SMI;
	low.Bits.Level = 1;
	low.Bits.DestinationShorthand = LOCAL_APIC_DESTINATION_SHORTHAND_ALL_EXCLUDING_SELF;

	high = 0;    // nonspecific

	switch (ApicMsr & (APIC_EN|APIC_EXTD))
	{
	case xAPIC_MODE:
		APIC_REG(ICR_HIGH) = high;   // write high before lows
		APIC_REG(ICR_LOW)  = low.Uint32;    // because writing to low triggers the IPI
		break;

	case x2APIC_MODE:
		// x2APIC uses MSR's
		__writemsr(0x800 + 0x30, low.Uint32 |((UINT64)high << 32));
		break;

	default:
		DEBUG((EFI_D_ERROR, "%ld SendSmiToOtherProcessors - APIC mode invalid or APIC disabled\n", CpuIndex)); 
	}

	APIC_REG(ICR_HIGH) = highSave;  //restore high

	DEBUG((EFI_D_ERROR, "%ld SendSmiToOtherProcessors - Sent SMI to other processors command: high: 0x%08lx low: 0x%08lx APIC_MSR: 0x%p\n", 
		CpuIndex, 
		high, 
		low,
		ApicMsr));
}
