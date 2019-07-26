/** @file

Gather the hardware state to be passed to the VM/PE for analysis

This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "StmRuntime.h"
#include "PeStm.h"

#define memcpy CopyMem

extern PE_VM_DATA PeVmData[4];   // right now support a max of 3 PE VM (VM 0 is the SMI_HANDLER)
extern PE_SMI_CONTROL PeSmiControl;

extern void SendSmiToOtherProcessors(UINT32 CpuIndex);
extern VOID CpuReadySync(UINT32 Index);
extern void MapVmcs ();
extern UINT32 GetVmcsOffset( UINT32 field_encoding);

void SetupGetRootVmxState();
void PrintVmxState(UINT32 CpuIndex, ROOT_VMX_STATE * RootState);

static UINT32 SetupGetRootVmxStateDone = 0;

static UINT64 VMCS_N_HOST_CR0_OFFSET = 0;
static UINT64 VMCS_N_HOST_CR3_OFFSET = 0;
static UINT64 VMCS_N_HOST_CR4_OFFSET = 0;
static UINT64 VMCS_N_HOST_GDTR_BASE_OFFSET = 0;
static UINT64 VMCS_N_HOST_IDTR_BASE_OFFSET = 0;
static UINT64 VMCS_N_HOST_RSP_OFFSET = 0;
static UINT64 VMCS_N_HOST_RIP_OFFSET = 0;
static UINT64 VMCS_64_CONTROL_EPT_PTR_OFFSET = 0;
static UINT64 VMCS_N_GUEST_RIP_OFFSET = 0;

static UINT64 VMCS_N_GUEST_CR0_OFFSET = 0;
static UINT64 VMCS_N_GUEST_CR3_OFFSET = 0;
static UINT64 VMCS_N_GUEST_CR4_OFFSET = 0;
static UINT64 VMCS_N_GUEST_GDTR_BASE_OFFSET = 0;
static UINT64 VMCS_32_GUEST_GDTR_LIMIT_OFFSET = 0;
static UINT64 VMCS_N_GUEST_IDTR_BASE_OFFSET = 0;
static UINT64 VMCS_32_GUEST_LDTR_LIMIT_OFFSET = 0;
static UINT64 VMCS_N_GUEST_RSP_OFFSET = 0;
static UINT64 VMCS_64_CONTROL_EXECUTIVE_VMCS_PTR_OFFSET = 0;
static UINT64 VMCS_64_GUEST_VMCS_LINK_PTR_OFFSET = 0;
static UINT64 VMCS_OFFSET_READY = 0;

int GetMultiProcessorState(UINT32 CpuIndex)
{
	UINT32 PeType = PE_PERM;
	UINT64 * NumProcessors = (UINT64 *) PeVmData[PeType].SharedPageStm;
	ROOT_VMX_STATE * RootState;   // = (ROOT_VMX_STATE *) (NumProcessors + sizeof(*NumProcessors));
	UINT32 CpuNum;

	DEBUG((EFI_D_ERROR, "%ld GetMultiProcessorState - Started\n", CpuIndex));

	if(PeVmData[PeType].SharedPageStm == NULL)
	{
		DEBUG((EFI_D_ERROR, "%ld GetMultiProcessorState - SharedPageStm is NULL, not gathering state\n", CpuIndex));
		return -2;
	}
	// first clear out the data structures and set the number of processors

	RootState = (ROOT_VMX_STATE *) ((char *)NumProcessors + 64 );//sizeof(*NumProcessors) + sizeof(*NumProcessors));
	*NumProcessors = mHostContextCommon.CpuNum;  // number of CPUs

	ZeroMem ((VOID *)(UINTN) RootState, sizeof(ROOT_VMX_STATE) * mHostContextCommon.CpuNum);

	// make sure that the VMCS offsets are setup

	SetupGetRootVmxState();

	// send an SMI to the other processors

	if(InterlockedCompareExchange32(&PeSmiControl.PeSmiState, PESMINULL, PESMIPSMI) != PESMINULL) //&PeSmiControl.PeSmiState = 1;
	{
		DEBUG((EFI_D_ERROR, "%ld x - Aborting, SMI handler already there. PeSmiState %ld\n", CpuIndex, PeSmiControl.PeSmiState));
		return -1;                   // need to tell about smi handler is already there
	}

	SendSmiToOtherProcessors(CpuIndex);

	// wait for the other processors to sync up and decide what to do
	CpuReadySync(CpuIndex);

	// get the local processor state

	//CpuReadySync(CpuIndex);
	GetRootVmxState(CpuIndex, &RootState[CpuIndex]);

	// need to think about this --- without it this hangs, what in context of other processors
	//InterlockedCompareExchange32(&PeSmiControl.PeSmiState, PESMIPSMI, PESMINULL);//PeSmiControl.PeSmiState = 0;  // all done - may need to sync processors in the case of 
	// another SMI coming in

	CpuReadySync(CpuIndex);  // wait for everyone to finish the job - PeSmiHandler will set PeSmiState to 0
	                         // once everyone has synched up

	for(CpuNum = 0; CpuNum < mHostContextCommon.CpuNum; CpuNum++)
	{
		PrintVmxState(CpuNum, &RootState[CpuNum]);
	}

	
	DEBUG((EFI_D_ERROR, "%ld GetMultiProcessorState - Completed. PeSmiState: %ld\n", CpuIndex, PeSmiControl.PeSmiState));
	return 0;
}

#define MAXVMCSFLUSH 6
#define VmcsSizeInPages 1
void GetRootVmxState(UINT32 CpuIndex, ROOT_VMX_STATE * RootState)
{
	//UINT64 ExecutiveVMCS;
	UINT64 HostRootVMCS;
	UINT64 CurrentVMCSSave;
	UINT64 RootGuestCR0_M;
	UINT64 RootGuestCR3_M;
	UINT64 RootGuestCR4_M;
	UINT64 RootGuestGDTRBase_M;
	UINT64 RootGuestGDTRLimit_M;
	UINT64 RootGuestIDTRBase_M;
	UINT64 RootGuestIDTRLimit_M;
	UINT64 RootGuestRSP_M;
	UINT64 RootGuestRIP_M;
	UINT64 RootContExecVmcs_M;
	UINT64 RootContLinkVmcs_M;

	UINT32 FlushCount;
	UINT32 i;

	char * DummyVmcs[MAXVMCSFLUSH];
	UINT32 VmxRevId;

	RootState->Vmxon = mHostContextCommon.HostContextPerCpu[CpuIndex].Vmxon;
	//UINT32 ApicId = (UINT32) (get_apic_id() & 0xFF);
	RootState->LinkVMCS = VmRead64(VMCS_64_GUEST_VMCS_LINK_PTR_INDEX);
	RootState->ExecutiveVMCS = VmRead64(VMCS_64_CONTROL_EXECUTIVE_VMCS_PTR_INDEX); // get the executive VMCS

	//	DEBUG((EFI_D_ERROR, "%ld GetRootVmxState\n   VMXON: 0x%016llx\n   ExecutiveVMCS: 0x%016llx\n   LinkVMCS: 0x%016llx\n",
	//		CpuIndex,
	//		RootState->Vmxon,
	//		RootState->ExecutiveVMCS,
	//		RootState->LinkVMCS));

	RootState->RootGuestCR0  = VmReadN(VMCS_N_GUEST_CR0_INDEX);
	RootState->RootGuestCR3  = VmReadN(VMCS_N_GUEST_CR3_INDEX);
	RootState->RootGuestCR4  = VmReadN(VMCS_N_GUEST_CR4_INDEX);
	RootState->RootGuestGDTRBase = VmReadN(VMCS_N_GUEST_GDTR_BASE_INDEX);
	RootState->RootGuestGDTRLimit = VmRead32(VMCS_32_GUEST_GDTR_LIMIT_INDEX);
	RootState->RootGuestIDTRBase = VmReadN(VMCS_N_GUEST_IDTR_BASE_INDEX);
	RootState->RootGuestIDTRLimit = VmRead32(VMCS_32_GUEST_LDTR_LIMIT_INDEX);
	RootState->RootGuestRSP  = VmReadN(VMCS_N_GUEST_RSP_INDEX);
	RootState->RootGuestRIP  = VmReadN(VMCS_N_GUEST_RIP_INDEX);
	RootState->RootContEPT   = VmReadN(VMCS_64_CONTROL_EPT_PTR_INDEX);
	// test result

	// save the current working vmcs

	// find the vmcs that contains the root/host datastrucure
	// this this the host state information for the root VMCS on the host
	// it contains the information needed to proces the guest vmexit

	if(RootState->ExecutiveVMCS == RootState->Vmxon)  // ref: section 34.15.4.7
	{
		// we are in root operation, so our VMCS of interest is in the VNCS-Link field

		if(RootState->LinkVMCS != 0xFFFFFFFFFFFFFFFF)
		{
			RootState->VmcsType = 1;   // guest-VM being sericed by VMM
			HostRootVMCS = RootState->LinkVMCS;
			//HostRootVMCS = VmRead64(VMCS_64_GUEST_VMCS_LINK_PTR_INDEX);
			RootState->VmxState = VMX_STATE_ROOT;
			//DEBUG((EFI_D_ERROR, "%ld GetRootVmxState (%d): execVMCS is vmxon: 0x%016llx using VMCS_LINK_POINTER\n",
				//CpuIndex, RootState->VmcsType, HostRootVMCS));
		}
		else
		{
			HostRootVMCS = RootState->ExecutiveVMCS;
			RootState->VmcsType = 2;
			//HostRootVMCS = VmRead64(VMCS_64_GUEST_VMCS_LINK_PTR_INDEX);
			RootState->VmxState = VMX_STATE_ROOT;
			//DEBUG((EFI_D_ERROR, "%ld GetRootVmxState (%d): execVMCS is vmxon: But LinkVMCS is 0xFFFFFFFFFFFFFFF so no current Vmcs. Using Executive Vmcs: %llx\n",
			//	CpuIndex, RootState->VmcsType, HostRootVMCS));
		}
	}
	else
	{
		// in guest operation, so our VMCS of interest is in the executive-VMCS field

		RootState->VmcsType = 3;
		HostRootVMCS = RootState->ExecutiveVMCS;
		RootState->VmxState = VMX_STATE_GUEST;
		//DEBUG((EFI_D_ERROR, "%ld GetRootVmxState (%d): execVMCS is guest VMCS: 0x%016llx using Executive VMCS\n",
			//CpuIndex, RootState->VmcsType, HostRootVMCS));
	}

	AsmVmClear(&(CurrentVMCSSave));
	AsmVmPtrStore(&CurrentVMCSSave);
	RootGuestRIP_M = *(UINT64 *)((UINTN)CurrentVMCSSave + (UINTN)VMCS_N_GUEST_RIP_OFFSET);

VmcsFlushStart:
	FlushCount = 0;

	while((RootState->RootGuestRIP != RootGuestRIP_M) &&
		(FlushCount < MAXVMCSFLUSH))
	{
		// got here because the in-memory copy of the VMCS is different than 
		// what is in the processor - so we need to flush
		//DEBUG((EFI_D_ERROR, "%ld - GetRootState: RootGuestRIPMemory: 0x%016llx, Location: 0x%016llx\n", 
		//CpuIndex, RootGuestRIPMemory, ((UINTN)HostRootVMCS + (UINTN)VMCS_N_GUEST_RIP_OFFSET)));
		// first create a dummy VMCS
		VmxRevId = AsmReadMsr32(IA32_VMX_BASIC_MSR_INDEX);
		DummyVmcs[FlushCount] = (char *) AllocatePages(VmcsSizeInPages);

		if(DummyVmcs[FlushCount] == NULL)
		{
			// ran out of memory - release everything and start over
			// that way someone else hopefully gets a chance to complete
			DEBUG((EFI_D_ERROR, "%ld - GetRootState: ran out of memory - so free everything and restart - Flushcount: %d\n",
				CpuIndex, FlushCount));
			if(FlushCount == 0)
				goto VmcsFlushStart;

			for(i = 0; i < FlushCount; i++)
			{
				FreePages(DummyVmcs[i], VmcsSizeInPages);
			}
			goto VmcsFlushStart;
		}

		memcpy(DummyVmcs[FlushCount], &VmxRevId, 4);
		AsmVmPtrLoad((UINT64 *) &DummyVmcs[FlushCount]);
		RootGuestRIP_M = *(UINT64 *)((UINTN)CurrentVMCSSave + (UINTN)VMCS_N_GUEST_RIP_OFFSET);  // try again
		FlushCount++;
	}

	AsmVmPtrLoad(&CurrentVMCSSave);   // in any case, reload this and free the dummies if necessary

	if(FlushCount > 0)
	{
		DEBUG((EFI_D_ERROR, "%ld GetRootVmxState - Flush necessary to get VMCS in sync. Flushcount=%d\n", 
			CpuIndex, FlushCount));
		//DEBUG((EFI_D_ERROR, "%ld GetRootVmxState: after Flush: VMCS_N_GUEST_RIP_MEMORY: 0x%016llx (test) \n", CpuIndex, RootGuestRIPMemory));
		// release the buffers
		for(i = 0; i < FlushCount; i++)
		{
			FreePages(DummyVmcs[i], VmcsSizeInPages);
		}
	}
	//AsmVmPtrStore(&CurrentVMCSSave);
	//AsmVmClear(&(CurrentVMCSSave));
	//AsmVmPtrLoad(&HostRootVMCS);

	RootState->HostRootVMCS = HostRootVMCS;
	//DEBUG((EFI_D_ERROR, "%ld - GetRootVmxState:   HostRootVmcs 0x%016llx\n", CpuIndex, RootState->HostRootVMCS));

	RootGuestCR0_M  = *(UINT64 *)((UINTN)HostRootVMCS + (UINTN)VMCS_N_GUEST_CR0_OFFSET);
	RootGuestCR3_M  = *(UINT64 *)((UINTN)HostRootVMCS + (UINTN)VMCS_N_GUEST_CR3_OFFSET);
	RootGuestCR4_M  = *(UINT64 *)((UINTN)HostRootVMCS + (UINTN)VMCS_N_GUEST_CR4_OFFSET);
	RootGuestGDTRBase_M = *(UINT64 *)((UINTN)HostRootVMCS + (UINTN)VMCS_N_GUEST_GDTR_BASE_OFFSET);
	RootGuestGDTRLimit_M = (*(UINT64 *)((UINTN)HostRootVMCS + (UINTN)VMCS_32_GUEST_GDTR_LIMIT_OFFSET)) & 0x00000000FFFFFFFF;
	RootGuestIDTRBase_M = *(UINT64 *)((UINTN)HostRootVMCS + (UINTN)VMCS_N_GUEST_IDTR_BASE_OFFSET);
	RootGuestIDTRLimit_M = (*(UINT64 *)((UINTN)HostRootVMCS + (UINTN)VMCS_32_GUEST_LDTR_LIMIT_OFFSET)) & 0x00000000FFFFFFFF;
	RootGuestRSP_M  = *(UINT64 *)((UINTN)HostRootVMCS + (UINTN)VMCS_N_GUEST_RSP_OFFSET);
	RootGuestRIP_M = *(UINT64 *)((UINTN)HostRootVMCS + (UINTN)VMCS_N_GUEST_RIP_OFFSET);
	RootContExecVmcs_M = *(UINT64 *)((UINTN)HostRootVMCS + (UINTN)VMCS_64_CONTROL_EXECUTIVE_VMCS_PTR_OFFSET);
	RootContLinkVmcs_M = *(UINT64 *)((UINTN)HostRootVMCS + (UINTN)VMCS_64_GUEST_VMCS_LINK_PTR_OFFSET);

#ifdef VMCSDEBUGPRINT
	if(RootState->VmcsType !=2)  // only want active Vmcs
	{
		DEBUG((EFI_D_ERROR, "%ld GetRootVmxState (%d) HostRootVmcs 0x%016llx\n G_CR0 %llx\n G_CR3 %llx\n G_CR4 %llx\n G_GDTR %llx:%llx\n G_IDTR %llx:%llx\n G_RSP %llx\n G_RIP %llx\n", 
			CpuIndex,
			RootState->VmcsType, 
			RootState->HostRootVMCS,
			RootState->RootGuestCR0,
			RootState->RootGuestCR3,
			RootState->RootGuestCR4,
			RootState->RootGuestGDTRBase,
			RootState->RootGuestGDTRLimit,
			RootState->RootGuestIDTRBase,
			RootState->RootGuestIDTRLimit,
			RootState->RootGuestRSP,
			RootState->RootGuestRIP));

		DEBUG((EFI_D_ERROR, "%ld GetRootVmxState (%d) (control) HostRootVmcs 0x%016llx\n VMXON %llx\n ExecutiveVMCS %llx\n LinkVMCS %llx\n EPT %llx\n",
			CpuIndex,
			RootState->VmcsType,
			RootState->HostRootVMCS,
			RootState->Vmxon,
			RootState->ExecutiveVMCS,
			RootState->LinkVMCS,
			RootState->RootContEPT));

		DEBUG((EFI_D_ERROR, "%ld GetRootVmxState (%d) (memory) HostRootVmcs 0x%016llx\n G_CR0m %llx\n G_CR3m %llx\n G_CR4m %llx\n G_GDTRm %llx:%llx\n G_IDTRm %llx:%llx\n G_RSPm %llx\n G_RIPm %llx\n", 
			CpuIndex,
			//	"GetRootVmxState (memory)\n G_CR0m %llx\n G_CR3m %llx\n G_CR4m %llx\n G_GDTRm %llx:%llx\n G_IDTRm %llx:%llx\n G_RSPm %llx\n G_RIPm %llx\n C_ExecVMCSm %llx\n C_LinkVMCSm %llx\n",
			RootState->VmcsType,
			RootState->HostRootVMCS,
			RootGuestCR0_M,
			RootGuestCR3_M,
			RootGuestCR4_M,
			RootGuestGDTRBase_M,
			RootGuestGDTRLimit_M,
			RootGuestIDTRBase_M,
			RootGuestIDTRLimit_M,
			RootGuestRSP_M,
			RootGuestRIP_M));

		DEBUG((EFI_D_ERROR, "%ld GetRootVmxState (%d) (memory) HostRootVmcs 0x%016llx\n C_ExecVMCSm %llx\n C_LinkVMCSm %llx\n",
			CpuIndex,
			RootState->VmcsType,
			RootState->HostRootVMCS,
			RootContExecVmcs_M,
			RootContLinkVmcs_M));
	}
#endif
	//DEBUG((EFI_D_ERROR, "%ld GetRootVmxState: VMCS_N_GUEST_RIP_MEMORY: 0x%016llx VMCS_N_GUEST_RIP:  0x%016llx, Location: 0x%016llx (test) \n", 
	//	CpuIndex, RootGuestRIP_M, RootState->RootGuestRIP, ((UINTN)HostRootVMCS + (UINTN)VMCS_N_GUEST_RIP_OFFSET)));

	// need to save the root vmx host structures
#ifdef ZERO
	if(RootState->VmxState == VMX_STATE_ROOT)
	{
		// if root, these entries are meaningless, so clear them out
		RootState->RootHostCR0   = 0;
		RootState->RootHostCR3   = 0;
		RootState->RootHostCR4   = 0;
		RootState->RootHostGDTRBase  = 0;
		RootState->RootHostIDTRBase  = 0;
		RootState->RootHostRSP   = 0;
		RootState->RootHostRIP   = 0;
		RootState->RootHostEPT   = 0;
	}
	else
#endif
	{
		RootState->RootHostCR0   = *(UINT64 *)((UINTN)HostRootVMCS + (UINTN)VMCS_N_HOST_CR0_OFFSET);
		RootState->RootHostCR3   = *(UINT64 *)((UINTN)HostRootVMCS + (UINTN)VMCS_N_HOST_CR3_OFFSET);
		RootState->RootHostCR4   = *(UINT64 *)((UINTN)HostRootVMCS + (UINTN)VMCS_N_HOST_CR4_OFFSET);
		RootState->RootHostGDTRBase  = *(UINTN *)((UINTN)HostRootVMCS + (UINTN)VMCS_N_HOST_GDTR_BASE_OFFSET);
		RootState->RootHostIDTRBase  = *(UINTN *)((UINTN)HostRootVMCS + (UINTN)VMCS_N_HOST_IDTR_BASE_OFFSET);

		RootState->RootHostRSP   = *(UINTN*)((UINTN)HostRootVMCS + (UINTN)VMCS_N_HOST_RSP_OFFSET);
		RootState->RootHostRIP   = *(UINTN*)((UINTN)HostRootVMCS + (UINTN)VMCS_N_HOST_RIP_OFFSET);
		RootState->RootHostEPT   = *(UINTN*)((UINTN)HostRootVMCS + (UINTN)VMCS_64_CONTROL_EPT_PTR_OFFSET);
	}
	// Indicate to the master that we are all done

	RootState->valid = 1;  

	// restore the current working vmcs

	//AsmVmClear(&HostRootVMCS);
	//AsmVmPtrLoad(&CurrentVMCSSave); 
#ifdef VMCSDEBUGPRINT
	if(RootState->VmcsType != 2)
	{
		DEBUG((EFI_D_ERROR, "%ld GetRootVmxState (%d) \n H_CR0 %llx\n H_CR3 %llx\n H_CR4 %llx\n H_GDTR %llx\n H_IDTR %llx\n H_RSP %llx\n H_RIP %llx\n H_EPT %llx\n", 
			CpuIndex,
			RootState->VmcsType,
			RootState->RootHostCR0,
			RootState->RootHostCR3,
			RootState->RootHostCR4,
			RootState->RootHostGDTRBase,
			RootState->RootHostIDTRBase,
			RootState->RootHostRSP,
			RootState->RootHostRIP,
			RootState->RootHostEPT));
	}
#endif
}

// setups the offsets needed for accessing in memory the root vmcs state (the host part at least)

void SetupGetRootVmxState()
{
	if(SetupGetRootVmxStateDone == 1)
		return;   // already done, so move on

	MapVmcs();    // make sure we have a map

	if(VMCS_OFFSET_READY == 1)
		return;

	VMCS_N_HOST_CR0_OFFSET = GetVmcsOffset(  VMCS_N_HOST_CR0_INDEX);
	VMCS_N_HOST_CR3_OFFSET = GetVmcsOffset(  VMCS_N_HOST_CR3_INDEX);
	VMCS_N_HOST_CR4_OFFSET = GetVmcsOffset(  VMCS_N_HOST_CR4_INDEX);
	VMCS_N_HOST_GDTR_BASE_OFFSET = GetVmcsOffset(  VMCS_N_HOST_GDTR_BASE_INDEX);
	VMCS_N_HOST_IDTR_BASE_OFFSET = GetVmcsOffset(  VMCS_N_HOST_IDTR_BASE_INDEX);
	VMCS_N_HOST_RSP_OFFSET = GetVmcsOffset(  VMCS_N_HOST_RSP_INDEX);
	VMCS_N_HOST_RIP_OFFSET = GetVmcsOffset(  VMCS_N_HOST_RIP_INDEX);
	VMCS_64_CONTROL_EPT_PTR_OFFSET = GetVmcsOffset(  VMCS_64_CONTROL_EPT_PTR_INDEX);
	VMCS_N_GUEST_RIP_OFFSET = GetVmcsOffset(VMCS_N_GUEST_RIP_INDEX);

	VMCS_N_GUEST_CR0_OFFSET = GetVmcsOffset(VMCS_N_GUEST_CR0_INDEX);
	VMCS_N_GUEST_CR3_OFFSET = GetVmcsOffset(VMCS_N_GUEST_CR3_INDEX);
	VMCS_N_GUEST_CR4_OFFSET = GetVmcsOffset(VMCS_N_GUEST_CR4_INDEX);
	VMCS_N_GUEST_GDTR_BASE_OFFSET = GetVmcsOffset(VMCS_N_GUEST_GDTR_BASE_INDEX);
	VMCS_32_GUEST_GDTR_LIMIT_OFFSET = GetVmcsOffset(VMCS_32_GUEST_GDTR_LIMIT_INDEX);
	VMCS_N_GUEST_IDTR_BASE_OFFSET = GetVmcsOffset(VMCS_N_GUEST_IDTR_BASE_INDEX);
	VMCS_32_GUEST_LDTR_LIMIT_OFFSET = GetVmcsOffset(VMCS_32_GUEST_LDTR_LIMIT_INDEX);
	VMCS_N_GUEST_RSP_OFFSET = GetVmcsOffset(VMCS_N_GUEST_RSP_INDEX);
	VMCS_64_CONTROL_EXECUTIVE_VMCS_PTR_OFFSET = GetVmcsOffset(VMCS_64_CONTROL_EXECUTIVE_VMCS_PTR_INDEX);
	VMCS_64_GUEST_VMCS_LINK_PTR_OFFSET = GetVmcsOffset(VMCS_64_GUEST_VMCS_LINK_PTR_INDEX);
	// need to initialize the VMCS Offset table, if it has not already been done
	VMCS_OFFSET_READY = 1;
}

void PrintVmxState(UINT32 CpuIndex, ROOT_VMX_STATE * RootState)
{
		if(RootState->ExecutiveVMCS == RootState->Vmxon)  // ref: section 34.15.4.7
	{
		// we are in root operation, so our VMCS of interest is in the VNCS-Link field

		if(RootState->LinkVMCS != 0xFFFFFFFFFFFFFFFF)
		{
			DEBUG((EFI_D_ERROR, "%ld PrintVmxState (%d): execVMCS is vmxon: 0x%016llx using VMCS_LINK_POINTER\n",
				CpuIndex, RootState->VmcsType, RootState->LinkVMCS));
		}
		else
		{
			DEBUG((EFI_D_ERROR, "%ld PrintVmxState (%d): execVMCS is vmxon: But LinkVMCS is 0xFFFFFFFFFFFFFFF so no current Vmcs. Using Executive Vmcs: %llx\n",
				CpuIndex, RootState->VmcsType, RootState->ExecutiveVMCS));
		}
	}
	else
	{
		// in guest operation, so our VMCS of interest is in the executive-VMCS field

		DEBUG((EFI_D_ERROR, "%ld PrintVmxState (%d): execVMCS is guest VMCS: 0x%016llx using Executive VMCS\n",
			CpuIndex, RootState->VmcsType, RootState->ExecutiveVMCS));
	}

		if(RootState->VmcsType !=2)  // only want active Vmcs
	{
		DEBUG((EFI_D_ERROR, "%ld PrintVmxState (%d) HostRootVmcs 0x%016llx\n G_CR0 %llx\n G_CR3 %llx\n G_CR4 %llx\n G_GDTR %llx:%llx\n G_IDTR %llx:%llx\n G_RSP %llx\n G_RIP %llx\n", 
			CpuIndex,
			RootState->VmcsType, 
			RootState->HostRootVMCS,
			RootState->RootGuestCR0,
			RootState->RootGuestCR3,
			RootState->RootGuestCR4,
			RootState->RootGuestGDTRBase,
			RootState->RootGuestGDTRLimit,
			RootState->RootGuestIDTRBase,
			RootState->RootGuestIDTRLimit,
			RootState->RootGuestRSP,
			RootState->RootGuestRIP));

		DEBUG((EFI_D_ERROR, "%ld PrintVmxState (%d) (control) HostRootVmcs 0x%016llx\n VMXON %llx\n ExecutiveVMCS %llx\n LinkVMCS %llx\n EPT %llx\n",
			CpuIndex,
			RootState->VmcsType,
			RootState->HostRootVMCS,
			RootState->Vmxon,
			RootState->ExecutiveVMCS,
			RootState->LinkVMCS,
			RootState->RootContEPT));

		DEBUG((EFI_D_ERROR, "%ld PrintVmxState (%d) \n H_CR0 %llx\n H_CR3 %llx\n H_CR4 %llx\n H_GDTR %llx\n H_IDTR %llx\n H_RSP %llx\n H_RIP %llx\n H_EPT %llx\n", 
			CpuIndex,
			RootState->VmcsType,
			RootState->RootHostCR0,
			RootState->RootHostCR3,
			RootState->RootHostCR4,
			RootState->RootHostGDTRBase,
			RootState->RootHostIDTRBase,
			RootState->RootHostRSP,
			RootState->RootHostRIP,
			RootState->RootHostEPT));
	}
}


