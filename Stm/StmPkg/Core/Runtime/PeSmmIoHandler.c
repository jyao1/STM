/** @file

IO Handler

This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmRuntime.h"
#include "PeStm.h"
#include <Library/SerialPortLib.h>

#define NUMDEBLEN  200

extern SPIN_LOCK  mInternalDebugLock;  // have to make sure we do not step on debug statements
extern PE_VM_DATA PeVmData[4];   // right now support a max of 3 PE VM (VM 0 is the SMI_HANDLER)

extern UINTN
	TranslateEPTGuestToHost (
	IN UINT64      EptPointer,
	IN UINTN       Addr,
	OUT EPT_ENTRY  **EntryPtr  OPTIONAL
	);

void PeIoHandler( IN UINT32 CpuIndex)
{
	X86_REGISTER *Reg;
	UINT32       VmType;
	UINT32	     PortNumber;  // I/O port requested
	UINT64       GuestAddress;
	UINT64       GuestAddressEnd;
	UINT32       DataSize;

	UINTN PhysAddress;
	//UINTN PhysAddressParameterEnd;

	// for debugging a VM/PE debugging output can be sent through:
	//      RDX:    port -   0x3F8 or 0x3D8
	//      RCX:    number of bytes (size over NUMVMDEBUGLEN is truncated)
	//      DS:ESI  location in PE/VM where output is located
	//      use instruction OUTSB/OUTSW/OUTSD  0x6E or Ox6F
	//      note: do not use a loop with a rep statement (which is what is normally done)
	//
	//      all other attempts to use I/O ports will result in VM/PE termination

	//DEBUG((EFI_D_INFO, "%ld PeIoHandler - started\n", CpuIndex));

	VmType = mHostContextCommon.HostContextPerCpu[CpuIndex].GuestVmType;

	Reg = &mGuestContextCommonSmm[VmType].GuestContextPerCpu[0].Register;
	PortNumber = ReadUnaligned32((UINT32 *) &Reg->Rdx);

	if((PortNumber == 0x3D8)||(PortNumber == 0x3F8))
	{
		UINT64 AddressSpaceStart = PeVmData[VmType].UserModule.AddressSpaceStart;
		UINT64 AddressSpaceEnd = PeVmData[VmType].UserModule.AddressSpaceStart + PeVmData[VmType].UserModule.AddressSpaceSize;
		GuestAddress = ReadUnaligned64((UINT64 *) &Reg->Rsi); // assume that DS Base is zero
		DataSize = ReadUnaligned32((UINT32 *) &Reg->Rcx);
		//DEBUG((EFI_D_INFO, "%ld PeIoHandler - GuestAddress: 0x%016llx DataSize: 0x%016llx \n", CpuIndex, GuestAddress, DataSize));
		GuestAddressEnd = GuestAddress + DataSize;

		// make sure the GuestAddress fits in the block that is within
		// SMRAM

		if(GuestAddress < AddressSpaceStart ||
			GuestAddressEnd > AddressSpaceEnd)
		{
			DEBUG((EFI_D_INFO, "%ld PeIoHander - **ERROR** Requested serial output not within address space string: 0x%016llx:0x%016llx address range: 0x%016llx:0x%016llx\n",
				CpuIndex, GuestAddress, GuestAddressEnd, AddressSpaceStart, AddressSpaceEnd)); //
		}
		else
		{
			// address within bounds, then process it

			// find it within SMRAM
			PhysAddress = TranslateEPTGuestToHost(mGuestContextCommonSmm[VmType].EptPointer.Uint64, (UINTN)GuestAddress, 0L);
			//PhysAddressEnd = TranslateEPTGuestToHost(mGuestContextCommonSmm[VmType].EptPointer.Uint64, (UINTN)GuestAddress + GuestAddressEnd), 0L);

			if(DataSize > NUMDEBLEN)
			{
				DataSize = NUMDEBLEN;
			}

			AcquireSpinLock (&mInternalDebugLock); 
			SerialPortWrite ((UINT8 *)"(VM/PE) ", sizeof("(VM/PE) ") - 1);
			SerialPortWrite ((UINT8 *) PhysAddress, DataSize);
			ReleaseSpinLock (&mInternalDebugLock);
		}
	}
	else
	{
		DEBUG((EFI_D_ERROR, "%ld PeIoHandler - IO Port 0x%x not permitted\n", CpuIndex, PortNumber));
	}

	// need to bump the instruction counter to get past the I/O instruction

	VmWriteN (VMCS_N_GUEST_RIP_INDEX, VmReadN (VMCS_N_GUEST_RIP_INDEX) + VmRead32 (VMCS_32_RO_VMEXIT_INSTRUCTION_LENGTH_INDEX));
	return;  // all done
}
