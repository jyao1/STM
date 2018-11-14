/** @file

Exception Handler

This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmRuntime.h"
#include "PeStm.h"
#include "PeLoadVm.h"

extern UINT64 EndTimeStamp;
extern UINT32 save_Inter_PeVm(UINT32 CpuIndex);
extern UINT32 PostPeVmProc(UINT32 rc, UINT32 CpuIndex, UINT32 mode);
extern PE_VM_DATA PeVmData[];

// define this here for now
#define INTERRUPT_VECTOR_NMI 2
#define INTERRUPT_VECTOR_PF  14
#define INTERRUPT_VECTOR_GPF 13

unsigned int StmVmPeNmiExCount = 0;

void PeExceptionHandler( IN UINT32 CpuIndex)
{

	//CpuDeadLoop();
	VM_EXIT_INFO_INTERRUPTION IntInfo;
	UINT32 IntErr;
	UINT64 address;
	UINT32 VmType;

	EndTimeStamp = AsmReadTsc();
	IntInfo.Uint32 = VmRead32(VMCS_32_RO_VMEXIT_INTERRUPTION_INFO_INDEX);
	IntErr  = VmRead32(VMCS_32_RO_VMEXIT_INTERRUPTION_ERROR_CODE_INDEX);
	StmVmPeNmiExCount++;   // free up the waiting smi processor

	DEBUG((EFI_D_ERROR, "%ld PeExceptionHandler VmexitInterruptionInfo: 0x%x  INTERRUPTION_ERROR_CODE 0x%x\n", CpuIndex, IntInfo.Uint32, IntErr));

	if(IntInfo.Bits.Valid == 1)
	{

		switch(IntInfo.Bits.Vector)
		{
		case INTERRUPT_VECTOR_NMI:
			{

				DEBUG((EFI_D_ERROR, "%ld PeExceptionHandler - received NMI because SMI detected\n", CpuIndex));
				save_Inter_PeVm(CpuIndex);
				break;
			}
		case INTERRUPT_VECTOR_GPF:
			{
				// General Protection Fault- kill the PE/VM
				//DEBUG((EFI_D_ERROR, "%ld - PE/VM terminated because of an exception %x\n", CpuIndex, IntInfo.Uint32));

				DEBUG((EFI_D_ERROR, "%ld PeExceptionHandler - PE/VM General Protection Fault @ 0x%04lx:0x%016llx Address: 0x%016llx Info: 0x%lx\n",
					CpuIndex,
					VmRead16 (VMCS_16_GUEST_CS_INDEX),               
					VmReadN(VMCS_N_GUEST_RIP_INDEX),
					VmReadN(VMCS_N_RO_EXIT_QUALIFICATION_INDEX),
					IntInfo.Uint32));

				//DumpVmcsAllField();

				PostPeVmProc(PE_VM_GP_FAULT, CpuIndex, PRESERVE_VM);
				break;
			}

		case INTERRUPT_VECTOR_PF:
			{
				// General Protection Fault- kill the PE/VM
				//DEBUG((EFI_D_ERROR, "%ld - PE/VM terminated because of an exception %x\n", CpuIndex, IntInfo.Uint32));


				address = VmReadN(VMCS_N_RO_EXIT_QUALIFICATION_INDEX);

				DEBUG((EFI_D_ERROR, "%ld PeExceptionHandler - PE/VM Page Fault @ 0x%04lx:0x%016llx Address: 0x%016llx Info: 0x%lx\n",
					CpuIndex,
					VmRead16 (VMCS_16_GUEST_CS_INDEX),               
					VmReadN(VMCS_N_GUEST_RIP_INDEX),
					address,
					IntInfo.Uint32));

				DEBUG((EFI_D_ERROR, "     INTERRUPTION_ERROR_CODE: 0x%x\n",
					IntErr));
				if(IntErr & 0x00000001)
					DEBUG((EFI_D_ERROR, "Page-level protection violation\n"));
				else
					DEBUG((EFI_D_ERROR, "Non-present page\n"));
				if(IntErr & 0x00000002)
					DEBUG((EFI_D_ERROR, "Write-access attempted\n"));
				else
					DEBUG((EFI_D_ERROR, "read-access attempted\n"));
				if(IntErr & 0x00000004)
					DEBUG((EFI_D_ERROR, "USER mode\n"));
				else
					DEBUG((EFI_D_ERROR, "Supervisor mode\n"));
				if(IntErr & 0x00000008)
					DEBUG((EFI_D_ERROR, "Reserved bit set in paging structures\n"));
				if(IntErr & 0x00000010)
					DEBUG((EFI_D_ERROR, "Instruction FETCH\n"));
				else
					DEBUG((EFI_D_ERROR, "Data access\n"));

				// if the PT is 64-bit then dump the table for diagnostic purposes

#define Level4 0x0000FF8000000000ULL
#define Level3 0x0000007FC0000000ULL      
#define Level2 0x000000003FE00000ULL
#define Level1 0x00000000001FF000ULL
#define PAGING_4K_ADDRESS_MASK_64 0x000FFFFFFFFFF000ull

				if (sizeof(UINTN) == sizeof(UINT64)) 
				{
					UINTN                             PageTable;
					UINTN                             Pml4Index;
					UINTN							  PdpteIndex;
					UINTN							  PdeIndex;
					UINTN							  PteIndex;
					UINTN                            *Pde;
					UINTN                            *Pdpte;
					UINTN                            *Pml4;
					UINTN							 *Pte;
					UINTN                             BaseAddress;
					UINTN                             EndAddress;
					UINTN                             PhysBase;
					UINTN							  PdpteV;
					UINTN						      PdeV;
					UINTN							  PteV;
					UINTN							  Offset;

					PageTable = VmReadN (VMCS_N_GUEST_CR3_INDEX);
					//Pml4 = (UINT64 *)PageTable;

					Pml4Index = (address & Level4) >> 39;
					PdpteIndex = (UINTN)(address & Level3) >> 30;
					PdeIndex = (address & Level2) >> 21;
					PteIndex = (address & Level1) >> 12;

					DEBUG((EFI_D_ERROR, "Pagetable Chain causing the error\n"));
					DEBUG((EFI_D_ERROR, "    Pagetable Address (CR3): 0x%llx\n", PageTable));

					VmType = mHostContextCommon.HostContextPerCpu[CpuIndex].GuestVmType;
					BaseAddress = (UINTN)PeVmData[VmType].UserModule.AddressSpaceStart;
					EndAddress  = BaseAddress + PeVmData[VmType].UserModule.AddressSpaceSize - 1;
					PhysBase = (UINT64)PeVmData[VmType].SmmBuffer;

					if(PageTable < BaseAddress || PageTable > (EndAddress - 4096))
					{
						DEBUG((EFI_D_ERROR, "    CR3 points out of VM region\n"));
						goto endpf;
					}

					// find out where the page table is located in the VM memory

					Offset = PageTable - BaseAddress;
					Pml4 = (UINTN *) (PhysBase + Offset);

					DEBUG((EFI_D_ERROR, "    Pml4[0x%x]: 0x%llx\n", Pml4Index, Pml4[Pml4Index]));

					PdpteV = Pml4[Pml4Index] & PAGING_4K_ADDRESS_MASK_64;

					if(PdpteV < BaseAddress || PdpteV > (EndAddress - 4096))
					{
						DEBUG((EFI_D_ERROR, "    Pdpte points out of VM region\n"));
						goto endpf;
					}

					Offset = PdpteV - BaseAddress;
					Pdpte = (UINTN *)(PhysBase + Offset);

					DEBUG((EFI_D_ERROR, "    Pdpte[0x%x]: 0x%llx\n", PdpteIndex, Pdpte[PdpteIndex]));

					PdeV = Pdpte[PdpteIndex] & PAGING_4K_ADDRESS_MASK_64;

					if(PdeV < BaseAddress || PdeV > (EndAddress - 4096))
					{
						DEBUG((EFI_D_ERROR, "    Pde points out of VM region\n"));
						goto endpf;
					}

					if(Pdpte[PdpteIndex] & IA32_PG_PS)
					{
						DEBUG((EFI_D_ERROR, "    Pdpte Index: [0x%x]\n", PdpteIndex));
						DEBUG((EFI_D_ERROR, "    Pte   Index: [0x%x]\n", PteIndex));
						goto endpf;
					}

					Offset = PdeV - BaseAddress;
					Pde = (UINTN *) (PhysBase + Offset);

					DEBUG((EFI_D_ERROR, "    Pde[0x%x]: 0x%llx\n", PdeIndex, Pde[PdeIndex]));

					PteV = Pde[PdeIndex] & PAGING_4K_ADDRESS_MASK_64;

					if(PteV < BaseAddress || PteV > (EndAddress - 4096))
					{
						DEBUG((EFI_D_ERROR, "    Pte points out of VM region\n"));
						goto endpf;
					}

					if(Pde[PdeIndex] & IA32_PG_PS)
					{
						DEBUG((EFI_D_ERROR, "    Pte Index: [0x%x]\n", PteIndex));
						goto endpf;
					}

					Offset = PteV - BaseAddress;
					Pte = (UINTN *) (PhysBase + Offset);

					DEBUG((EFI_D_ERROR, "    Pte[0x%x]: 0x%llx\n", PteIndex, Pte[PteIndex]));
				}
				//DumpVmcsAllField();
endpf:
				PostPeVmProc(PE_VM_PAGE_FAULT, CpuIndex, PRESERVE_VM);
				break;
			}
		default:
			{
				DEBUG((EFI_D_ERROR, "%ld PeExceptionHandler - PE/VM Unhandled Exception @ 0x%04lx:0x%016llx Address: 0x%016llx Info: 0x%lx\n",
					CpuIndex,
					VmRead16 (VMCS_16_GUEST_CS_INDEX),               
					VmReadN(VMCS_N_GUEST_RIP_INDEX),
					VmReadN(VMCS_N_RO_EXIT_QUALIFICATION_INDEX),
					IntInfo.Uint32));

				//DumpVmcsAllField();

				PostPeVmProc(PE_VM_GP_FAULT, CpuIndex, PRESERVE_VM);
			}
		}
		// should not get here

		DEBUG((EFI_D_ERROR, "%ld PeExceptionHandler - CpuDeadLoop\n", CpuIndex));
		CpuDeadLoop();
		return;
	}

	DEBUG((EFI_D_ERROR, "%ld PeExceptionHandler - Warning Info Valid bits not equal to 1 @ 0x%04lx:0x%016llx Address: 0x%016llx Info: 0x%lx\n",
		CpuIndex,
		VmRead16 (VMCS_16_GUEST_CS_INDEX),               
		VmReadN(VMCS_N_GUEST_RIP_INDEX),
		VmReadN(VMCS_N_RO_EXIT_QUALIFICATION_INDEX),
		IntInfo.Uint32));
	return;
}
