/** @file

Setup a VM/PE

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

extern PE_VM_DATA PeVmData[4];   // right now support a max of 3 PE VM (VM 0 is the SMI_HANDLER)
extern int GetMultiProcessorState(UINT32 CpuIndex);

UINT32  SetupProtExecVm(UINT32 CpuIndex, UINT32 VM_Configuration, UINT32 mode, UINT32 PeType);
VOID InitPeGuestVmcs (IN UINT32 CpuIndex, IN UINT32 PeType, IN PE_GUEST_CONTEXT_PER_CPU   *Vmcs);

// modes:    NEW_VM - Create a new VM
//           RESTART_VM - restart a saved VM at the load point

UINT32 GetMPState;

UINT32  SetupProtExecVm(UINT32 CpuIndex, UINT32 VM_Configuration, UINT32 mode, UINT32 PeType) {
	UINT32 rc = PE_SUCCESS;
	UINT32 GCS_AR;
	UINT32 DS_AR;
	UINT32 SegLimit;

	UINTN CR0_config;
	UINTN CR4_config;

	UINT16 tr_access;
	UINT16 ldtr_access;
	UINT64 guest_efer = 0;
	UINT64 Data64;
	UINT16 cs_selector;
	UINT16 ds_selector;
	UINTN Rflags;

	DEBUG((EFI_D_ERROR, "%ld SetupProtExecVm - CR3_Index: %lx VmConfig: %lx mode: %x\n", 
		CpuIndex, PeType, VM_Configuration, mode));

	// make sure that we can jump over the calling instruction
	// StmVmm->VmexitInstructionLen = (UINT32)vmxRead(VM_EXIT_INSTRUCTION_LENGTH);
	// DEBUG((EFI_D_ERROR, " startSmiHandler2 GUEST_RIP %llx Ins Len %x\n", vmxRead(GUEST_RIP), StmVmm->VmexitInstructionLen));
	//DEBUG((EFI_D_ERROR, " startSmiHandler2 return dump %llx\n", *(UINT64 *) (vmxRead(GUEST_RIP)& 0xFFFFFFFFF)));  // should dump the calling location
	// do some sanity checks on some of the user specified parameters before attempting the setup

	// for now lets do this here - needs to be moved in a later version
	//GetRootVmxState(CpuIndex, (ROOT_VMX_STATE *) PeVmData[PeType].SharedPageStm);

	GetMPState = 0;  // initialize, assume we have no problems

	if(PeVmData[PeType].StartMode == PEVM_START_VMCALL)
	{
		// not necessary when we are started via smi

		// note: in the case that a hardware SMI gets there before this can fire of an SMI to get the
		// other processors state, we let SetupProtExecVm go ahead setup and start the VM.  The waiting NMI
		// will then shoot down the VM so that the hardware SMI can get handled
		// If this happens, we will set a flag and obtain the processor state once the VM is restarted 
		// after the SMI is handled

		if(GetMultiProcessorState(CpuIndex) ==  -1)
		{
			GetMPState = 1; // Indicate that we still need to get the processor state
		}
	}

	if(NEW_VM == mode)
	{

		if((CS_D | CS_L) == (VM_Configuration & (CS_D | CS_L))) // CS_D and CS_L cannot be set at the same time
		{
			FreePE_DataStructures(PeType);
			return(PE_VM_SETUP_ERROR_D_L);    // change to just telling the caller that it can't be done
		}

		if((CS_L & VM_Configuration) && !(SET_IA32E & VM_Configuration))
		{
			FreePE_DataStructures(PeType);
			return(PE_VM_SETUP_ERROR_IA32E_D);    // hange to just telling the caller that it can't be done
		}

		//populateSmmSaveState(StmVmm, VmexitQualification); // is this necessary?
		// set the vmcs pointer to at the smm montor (guest) vmcs
		// Allocate 4k aligned memory for VMCS

		if(mGuestContextCommonSmm[PeType].GuestContextPerCpu[0].Vmcs == 0L)
		{
			// memory has been released, so get some more

			mGuestContextCommonSmm[PeType].GuestContextPerCpu[0].Vmcs = (UINT64) AllocatePages(2);//GetVmcsSize() / PAGE_SIZE);

			if (0L == mGuestContextCommonSmm[PeType].GuestContextPerCpu[0].Vmcs)
			{
				DEBUG((EFI_D_ERROR, "%ld SetupProtExecVm - Failure allocating Prot execution VMCS memory\n", CpuIndex));
				FreePE_DataStructures(PeType);
				return(PE_VMCS_ALLOC_FAIL);    // change to just telling the caller that it can't be done
			}
			// Initialize the VMCS area to be all zeros - bad things happen otherwise
			// AllocatePages clears memor

			DEBUG((EFI_D_ERROR, "%ld SetupProtExecVm - Allocated and cleared VMCS memory\n", CpuIndex));
		}
		DEBUG((EFI_D_ERROR, "%ld SetupProtExecVm - VMCS region allocated at %llx\n", CpuIndex, mGuestContextCommonSmm[PeType].GuestContextPerCpu[0].Vmcs));

		// setup host and control vmcs here as we should only need to do this once
		// the guest state stuff will be always reset, so we do that stuff later

		// Write VMCS revision ID to VMCS memory

		*(UINT32 *)(UINTN)mGuestContextCommonSmm[PeType].GuestContextPerCpu[0].Vmcs = (UINT32)AsmReadMsr64 (IA32_VMX_BASIC_MSR_INDEX) & 0xFFFFFFFF;

		AsmVmPtrStore (&mGuestContextCommonSmi.GuestContextPerCpu[CpuIndex].Vmcs);
		AsmVmClear(&mGuestContextCommonSmi.GuestContextPerCpu[CpuIndex].Vmcs);
		Rflags = AsmVmClear(&mGuestContextCommonSmm[PeType].GuestContextPerCpu[0].Vmcs);
		if ((Rflags & (RFLAGS_CF | RFLAGS_ZF)) != 0) {
			DEBUG ((EFI_D_ERROR, "%ld SetupProtExecVm - ERROR: AsmVmClear - %016lx : %08x\n", 
				(UINTN)CpuIndex, mGuestContextCommonSmm[PeType].GuestContextPerCpu[0].Vmcs, Rflags));
			FreePE_DataStructures(PeType);
			return(PE_VMCS_ALLOC_FAIL);    // change to just telling the caller that it can't be done
		}
		Rflags = AsmVmPtrLoad(&mGuestContextCommonSmm[PeType].GuestContextPerCpu[0].Vmcs);   // make PE VMCS active
		if ((Rflags & (RFLAGS_CF | RFLAGS_ZF)) != 0) {
			DEBUG ((EFI_D_ERROR, "&ld SetupProtExecVm - ERROR: AsmVmPtrLoad - %016lx : %08x\n",
				(UINTN)CpuIndex, mGuestContextCommonSmm[PeType].GuestContextPerCpu[0].Vmcs, Rflags));
			FreePE_DataStructures(PeType);
			return(PE_VMCS_ALLOC_FAIL);    // change to just telling the caller that it can't be done
		}

		// Setup entry and exit controls

		// VMENTRY CONTROLS Setup

		//Data64 = AsmReadMsr64 (IA32_VMX_TRUE_ENTRY_CTLS_MSR_INDEX);
		Data64 = AsmReadMsr64 (IA32_VMX_ENTRY_CTLS_MSR_INDEX);

		PeVmData[PeType].GuestState.GuestContextPerCpu.VmEntryCtrls.Uint32 = (UINT32)Data64;
		//VmEntryCtrls.Bits.Ia32eGuest = mHostContextCommon.HostContextPerCpu[CpuIndex].TxtProcessorSmmDescriptor->SmmEntryState.Intel64Mode;
		PeVmData[PeType].GuestState.GuestContextPerCpu.VmEntryCtrls.Bits.EntryToSmm = 1;
		PeVmData[PeType].GuestState.GuestContextPerCpu.VmEntryCtrls.Bits.LoadIA32_EFER = 1;
		PeVmData[PeType].GuestState.GuestContextPerCpu.VmEntryCtrls.Bits.LoadDebugControls = 1;

		// Upon receiving control due to an SMI, the STM shall save the contents of the IA32_PERF_GLOBAL_CTRL MSR, disable any
		// enabled bits in the IA32_PERF_GLOBAL_CTRL MSR
		PeVmData[PeType].GuestState.GuestContextPerCpu.VmEntryCtrls.Bits.LoadIA32_PERF_GLOBAL_CTRL = 0;

		PeVmData[PeType].GuestState.GuestContextPerCpu.VmEntryCtrls.Uint32 &= (UINT32)RShiftU64 (Data64, 32);

		// VMEXIT CONTROLS SETUP

		//Data64 = AsmReadMsr64 (IA32_VMX_TRUE_EXIT_CTLS_MSR_INDEX);
		Data64 = AsmReadMsr64 (IA32_VMX_EXIT_CTLS_MSR_INDEX);
		PeVmData[PeType].GuestState.GuestContextPerCpu.VmExitCtrls.Uint32 = (UINT32)Data64;
		PeVmData[PeType].GuestState.GuestContextPerCpu.VmExitCtrls.Bits.Ia32eHost = (sizeof(UINT64) == sizeof(UINTN));
		// Upon receiving control due to an SMI, the STM shall save the contents of the IA32_PERF_GLOBAL_CTRL MSR, disable any
		// enabled bits in the IA32_PERF_GLOBAL_CTRL MSR
		PeVmData[PeType].GuestState.GuestContextPerCpu.VmExitCtrls.Bits.LoadIA32_PERF_GLOBAL_CTRL = 0;
		PeVmData[PeType].GuestState.GuestContextPerCpu.VmExitCtrls.Bits.SaveIA32_EFER = 1;
		PeVmData[PeType].GuestState.GuestContextPerCpu.VmExitCtrls.Bits.AcknowledgeInterrupt = 1;

		PeVmData[PeType].GuestState.GuestContextPerCpu.VmExitCtrls.Uint32 &= (UINT32)RShiftU64 (Data64, 32);

		if((VM_Configuration & SET_IA32E) && (PeVmData[PeType].GuestState.GuestContextPerCpu.VmExitCtrls.Bits.Ia32eHost == 1))
		{
			tr_access = 11;
			//PeVmData[PeType].GuestState.GuestContextPerCpu.VmExitCtrls.Bits.Ia32eHost = 1;
			PeVmData[PeType].GuestState.GuestContextPerCpu.VmEntryCtrls.Bits.Ia32eGuest = 1;
		}
		else

		{
			tr_access = 11;   // 32-bit busy TSS in non IA-32e mode and 64 bit busy TSS for IA-32e mode (3 = 16 bit tss)
			//PeVmData[PeType].GuestState.GuestContextPerCpu.VmExitCtrls.Bits.Ia32eHost =1;
			PeVmData[PeType].GuestState.GuestContextPerCpu.VmEntryCtrls.Bits.Ia32eGuest = 0;
			DEBUG((EFI_D_ERROR, "%ld SetupProtExecVm - WARNING - No IA32e Host\n", CpuIndex));
		}

		mGuestContextCommonSmm[PeType].IoBitmapA = 0;   // (UINT64)IoBitmapA;
		mGuestContextCommonSmm[PeType].IoBitmapB = 0;   //(UINT64)IoBitmapB;
		mGuestContextCommonSmm[PeType].MsrBitmap = 0;   //(UINT64)MsrBitmapReadLow;

#define CS_SEL      0x38
#define CODE_SEL    0x08
#define TR_SEL      0x68
#define DEF_BASE    0x00
#define DEF_LIMIT   0xFFFF
#define DS_ACCESS   0xC093

#define SEG_G        (1<<15)             // segment granularity
#define SEG_Present  (1<<7)              // segment present
#define SEG_CODEDATA (1<<4)              // segment code or data (zero means system)

		// set tr_access bits

		tr_access = tr_access | SEG_Present| SEG_G;
		ldtr_access = (2<<0)| SEG_Present | SEG_G;

		// setup the efer msr
		if(VM_Configuration & SET_IA32E)
		{
			// enable IA32E
			guest_efer |= IA32_EFER_MSR_MLE;
			if((VM_Configuration & CR0_PG) == CR0_PG)
			{
				guest_efer |= IA32_EFER_MSR_MLA;  // we are running unrestricted guest.. but need to test
				// intel manual says that LMA must mirror CRO_PG
			}
		}
		DEBUG((EFI_D_ERROR, "%ld SetupProtExecVm - guest_efer: 0x%llx\n", CpuIndex, guest_efer));
		// setup the code segment access right
		GCS_AR = (11<<0)|SEG_CODEDATA | SEG_Present;       // code, (execute, read/accessed), present, granularity
		DS_AR = (3<<0) | SEG_CODEDATA | SEG_Present;

		if(CS_L & VM_Configuration)
		{   // we are 64-bit mode
			GCS_AR |= CS_L | SEG_G;
			DS_AR  |= SEG_G | CS_D;
			SegLimit = 0xFFFFFFFF;
			DEBUG((EFI_D_ERROR, "%ld SetupProtExecVm - Setting 64 bit mode\n", CpuIndex));
			PeVmData[PeType].PeCpuInitMode = PEVM_INIT_64bit;
		}
		else
		{
			if(CS_D & VM_Configuration)
			{   // We are 32-bit mode
				GCS_AR |= CS_D | SEG_G;
				DS_AR |=  CS_D | SEG_G;
				SegLimit = 0xFFFFFFFF;
				DEBUG((EFI_D_ERROR, "%ld SetupProtExecVm - Setting 32 bit mode\n", CpuIndex));
				PeVmData[PeType].PeCpuInitMode = PEVM_INIT_32bit;
			}
			else  // we are 16-bit mode
			{
				SegLimit = 0xFFFF;
				DEBUG((EFI_D_ERROR, "%ld SetupProtExecVm - Setting 16 bit mode\n", CpuIndex));
				PeVmData[PeType].PeCpuInitMode = PEVM_INIT_16bit;
			}
		}

		DEBUG((EFI_D_ERROR, "%ld SetupProtExecVm - GCS_AR: 0x%llx SegLimit 0x%llx\n", CpuIndex, GCS_AR, SegLimit));

		// setup CR0 and CR4

		// add fudge factors here
		CR0_config = (UINTN)AsmReadMsr64 (IA32_VMX_CR0_FIXED0_MSR_INDEX);//CR0_WP ; // make sure that we set what is necessary
		//CR0_config = 0; // above not necessary in unrestriced guests...
		CR4_config = (UINTN)AsmReadMsr64 (IA32_VMX_CR4_FIXED0_MSR_INDEX);
		// clear and set bits demanded by processor mode
#define CR4_PCIDE (1u <<17)            // CpuDef.h does not have this
		if(VM_Configuration & SET_IA32E)
		{   
			CR0_config |= CR0_PG;        // has to be on in IA32E mode
			CR4_config |= CR4_PAE;       // has to be on in IA32E mode
		}
		else
		{
			CR0_config &= ~(CR0_PG|CR0_PE);  // turn these guy off 
			CR4_config &= ~CR4_PCIDE;        // must be turned off when IA32E is off
		}

		// set bits desired by user

		CR0_config |= (CR0_PG | CR0_PE) & VM_Configuration;

		// make sure the user does not shoot himself in the foot

		if(VM_Configuration & CR0_PG)
			CR0_config |= CR0_PE;

		if(VM_Configuration & SET_CR4_PAE)
			CR4_config |= CR4_PAE;
		CR4_config |= CR4_PSE & VM_Configuration;

		if(CR0_config & CR0_PE)  // are we using the segment registers as selectors
		{
			cs_selector = CS_SEL;
			ds_selector = CODE_SEL;
		}
		else
		{
			// real mode stuff set the segment registers to 0x000000

			cs_selector = 0;
			ds_selector = 0;
		}

		CR0_config &= (UINTN)AsmReadMsr64 (IA32_VMX_CR0_FIXED1_MSR_INDEX); // make sure that only these one bits can be set

		CR4_config &= (UINTN)AsmReadMsr64 (IA32_VMX_CR4_FIXED1_MSR_INDEX);

		DEBUG((EFI_D_ERROR, "%ld SetupProtExecVm - Setting GUEST_CR0: %llx GUEST_CR4: %llx\n", CpuIndex, CR0_config, CR4_config));  
		///

		//GuestRegionVmcs = PeVmData[CR3index].GuestRegionVmcs;

		PeVmData[PeType].GuestState.GuestContextPerCpu.CsSelector = cs_selector;
		PeVmData[PeType].GuestState.GuestContextPerCpu.CsBase     = DEF_BASE;
		PeVmData[PeType].GuestState.GuestContextPerCpu.CsLimit    = SegLimit;
		PeVmData[PeType].GuestState.GuestContextPerCpu.CsAccessRights = GCS_AR;      // defined by user input

		PeVmData[PeType].GuestState.GuestContextPerCpu.DsSelector = ds_selector;
		PeVmData[PeType].GuestState.GuestContextPerCpu.DsBase     = DEF_BASE;
		PeVmData[PeType].GuestState.GuestContextPerCpu.DsLimit    = SegLimit;
		PeVmData[PeType].GuestState.GuestContextPerCpu.DsAccessRights = DS_AR; 

		PeVmData[PeType].GuestState.GuestContextPerCpu.EsSelector = ds_selector;
		PeVmData[PeType].GuestState.GuestContextPerCpu.EsBase     = DEF_BASE;
		PeVmData[PeType].GuestState.GuestContextPerCpu.EsLimit    = SegLimit;
		PeVmData[PeType].GuestState.GuestContextPerCpu.EsAccessRights = DS_AR; 

		PeVmData[PeType].GuestState.GuestContextPerCpu.FsSelector = ds_selector;
		PeVmData[PeType].GuestState.GuestContextPerCpu.FsBase     = DEF_BASE;
		PeVmData[PeType].GuestState.GuestContextPerCpu.FsLimit    = SegLimit;
		PeVmData[PeType].GuestState.GuestContextPerCpu.FsAccessRights = DS_AR; 

		PeVmData[PeType].GuestState.GuestContextPerCpu.GsSelector = ds_selector;
		PeVmData[PeType].GuestState.GuestContextPerCpu.GsBase     = DEF_BASE;
		PeVmData[PeType].GuestState.GuestContextPerCpu.GsLimit    = SegLimit;
		PeVmData[PeType].GuestState.GuestContextPerCpu.GsAccessRights = DS_AR; 

		PeVmData[PeType].GuestState.GuestContextPerCpu.SsSelector = ds_selector;
		PeVmData[PeType].GuestState.GuestContextPerCpu.SsBase     = DEF_BASE;
		PeVmData[PeType].GuestState.GuestContextPerCpu.SsLimit    = SegLimit;
		PeVmData[PeType].GuestState.GuestContextPerCpu.SsAccessRights = DS_AR; 

		PeVmData[PeType].GuestState.GuestContextPerCpu.TrSelector = TR_SEL;
		PeVmData[PeType].GuestState.GuestContextPerCpu.TrBase     = DEF_BASE;
		PeVmData[PeType].GuestState.GuestContextPerCpu.TrLimit    = DEF_LIMIT;
		PeVmData[PeType].GuestState.GuestContextPerCpu.TrAccessRights = tr_access; 

		PeVmData[PeType].GuestState.GuestContextPerCpu.LdtrSelector     = TR_SEL;
		PeVmData[PeType].GuestState.GuestContextPerCpu.LdtrBase         = DEF_BASE;
		PeVmData[PeType].GuestState.GuestContextPerCpu.LdtrLimit = DEF_LIMIT;

		{
			PeVmData[PeType].GuestState.GuestContextPerCpu.GdtrLimit = DEF_LIMIT;
			PeVmData[PeType].GuestState.GuestContextPerCpu.IdtrLimit = DEF_LIMIT;
		}
		PeVmData[PeType].GuestState.GuestContextPerCpu.LdtrAccessRights = ldtr_access; 

		PeVmData[PeType].GuestState.GuestContextPerCpu.GdtrBase = DEF_BASE;
		//PeVmData[PeType].GuestState.GuestContextPerCpu.GdtrLimit = DEF_LIMIT;

#define DataSegType 0x3    // Segment type - data, read, write, accessed
#define CodeSegType 0xb    // Segment type - code, read, write, accessed
#define CodeDataDescriptorType (1<<4) // S - Descriptor Type: code or data
#define SegmentPresent (1<<7)         // P - segment is present in memory
#define SegmentAVL     (1<<12)        // AVL - Available for use by system software
#define Segment32bit   (1<<14)        // D/B - 1 = 32 bit segment
#define Granularity    (1<<15)        // G - Granularity (1 = 4096)


		if(PeVmData[PeType].PeCpuInitMode == PEVM_INIT_16bit)
		{
			// now setup the (big) real mode representation

		    UINT32 CodeAR32bit = CodeSegType|CodeDataDescriptorType|SegmentPresent|SegmentAVL|Segment32bit|Granularity;
		    UINT32 DataAR32bit = DataSegType|CodeDataDescriptorType|SegmentPresent|SegmentAVL|Segment32bit|Granularity;
			UINT32 Limit32bit = 0xFFFFF;
			UINT32 Base32bit  = 0;

			// if he is asking for big real mode (both code and data are 32-bit)
			// then CR0 and CR4 must be setup as if they are in real mode

			CR0_config = CR0_config & ~(CR0_PE | CR0_PG);  // set real mode
			CR4_config = 0;   // set real mode

			PeVmData[PeType].GuestState.GuestContextPerCpu.Rflags = 0x2;

			PeVmData[PeType].GuestState.GuestContextPerCpu.CsBase     =  Base32bit;
			PeVmData[PeType].GuestState.GuestContextPerCpu.CsLimit    = Limit32bit;
			PeVmData[PeType].GuestState.GuestContextPerCpu.CsAccessRights = CodeAR32bit;      // defined by user input

			PeVmData[PeType].GuestState.GuestContextPerCpu.DsBase     = Base32bit;
			PeVmData[PeType].GuestState.GuestContextPerCpu.DsLimit    = Limit32bit;
			PeVmData[PeType].GuestState.GuestContextPerCpu.DsAccessRights = DataAR32bit; 

			PeVmData[PeType].GuestState.GuestContextPerCpu.SsBase     = Base32bit;
			PeVmData[PeType].GuestState.GuestContextPerCpu.SsLimit    = Limit32bit;
			PeVmData[PeType].GuestState.GuestContextPerCpu.SsAccessRights = DataAR32bit; 
		}
	
		PeVmData[PeType].GuestState.GuestContextPerCpu.Rip = (UINTN)(PeVmData[PeType].UserModule.ModuleEntryPoint + PeVmData[PeType].UserModule.ModuleLoadAddress);  // module entry point;

		PeVmData[PeType].GuestState.GuestContextPerCpu.IdtrBase = DEF_BASE;
		//PeVmData[PeType].GuestState.GuestContextPerCpu.IdtrLimit = DEF_LIMIT;

		//PeVmData[PeType].GuestState.GuestContextPerCpu.Rip = (UINTN)(PeVmData[PeType].UserModule.ModuleEntryPoint + PeVmData[PeType].UserModule.ModuleLoadAddress);  // module entry point;
		PeVmData[PeType].GuestState.GuestContextPerCpu.Rsp = 0x0;
		//! \todo  Don't set up the guest stack - let him do it himself (so his stack isn't in MSEG)
		PeVmData[PeType].GuestState.GuestContextPerCpu.Rflags = 0x02; // bit1 defaults to 1
		mGuestContextCommonSmm[PeType].GuestContextPerCpu[0].Cr0 = CR0_config;

		mGuestContextCommonSmm[PeType].GuestContextPerCpu[0].Cr3 = (UINTN)PeVmData[PeType].UserModule.Cr3Load;
		mGuestContextCommonSmm[PeType].GuestContextPerCpu[0].Cr4 = CR4_config;

		PeVmData[PeType].GuestState.GuestContextPerCpu.ActivityState = GUEST_ACTIVITY_STATE_ACTIVE;
		PeVmData[PeType].GuestState.GuestContextPerCpu.InterruptibilityState.Uint32 = 0; 
		PeVmData[PeType].GuestState.GuestContextPerCpu.InterruptibilityState.Bits.BlockingBySmi = 1;  // We allow NMI to cause a VM exit

		PeVmData[PeType].GuestState.GuestContextPerCpu.VmcsLinkPointerFull = 0xFFFFFFFFFFFFFFFF;
		PeVmData[PeType].GuestState.GuestContextPerCpu.VmcsLinkPointerHigh = 0xFFFFFFFF;
		mGuestContextCommonSmm[PeType].GuestContextPerCpu[0].Efer = guest_efer;
	}
	else
	{
		// here we restart the PE VM
		//AsmVmClear(&StmVmm->SmmMonitorVmcsPtr ); // de-couple vmcs region (check on)
		AsmVmPtrLoad(&mGuestContextCommonSmm[PeType].GuestContextPerCpu[0].Vmcs);
	}

	// we always reinitialize the guest region upon every restart of the VM

	//GuestRegionVmcs = PeVmData[CR3index].GuestRegionVmcs;

	InitPeGuestVmcs( CpuIndex, PeType, &PeVmData[PeType].GuestState.GuestContextPerCpu);

	// make sure that page faults are turned off
	// Setup the page fault controls
	// also, allow for double fault exits
	{
		UINT32 ExceptionBitmap;
		UINT32 PageFaultErrorCodeMask = 0;  // inequality in these two means that bit 14 does not exit
		UINT32 PageFaultErrorCodeMatch = 0;

		if((PE_VM_EXCEPTION_HANDLING & VM_Configuration) & PE_VM_EXCEPTION_HANDLING)
		{
			ExceptionBitmap = (1<<14) | (1<<8);  // vmexit on page fault and double fault
		}
		else
		{
			ExceptionBitmap = 0xFFFFFFFF;       //  vmexit on any exception
		}

		VmWrite32 (VMCS_32_CONTROL_EXCEPTION_BITMAP_INDEX, ExceptionBitmap);
		VmWrite32 (VMCS_32_CONTROL_PAGE_FAULT_ERROR_CODE_MASK_INDEX, PageFaultErrorCodeMask);
		VmWrite32 (VMCS_32_CONTROL_PAGE_FAULT_ERROR_CODE_MATCH_INDEX, PageFaultErrorCodeMatch);
	}
	DEBUG((EFI_D_ERROR, "%ld SetupProtExecVm - Guest CS access rights %llx\n", CpuIndex, VmRead32(VMCS_32_GUEST_CS_ACCESS_RIGHT_INDEX)));
	return rc;
}