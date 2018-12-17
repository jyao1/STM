#include "StmRuntime.h"
#include "PeStm.h"
#include "PeLoadVm.h"

extern PE_VM_DATA PeVmData[4];
extern PE_SMI_CONTROL PeSmiControl;

extern INT32 PeEptInit (EPT_POINTER *EptPointer);
extern void print_region_list(UINT32 PeType, UINT32 CpuIndex);
extern UINT32 SetupProtExecVm(UINT32 CpuIndex, UINT32 VM_Configuration, UINT32 mode, UINT32 PeType);
extern void LaunchPeVm(UINT32 PeType, UINT32 CpuIndex);
extern VOID EptDumpPageTable (IN EPT_POINTER *EptPointer );
static UINT32 setupModulepages(UINT32 PeType, UINT32 CpuIndex);

STM_STATUS AddPeVm(UINT32 CpuIndex, PE_MODULE_INFO * callerDataStructure, UINT32 PeType, UINT32 RunVm)
{
	// assume that the caller has aready moved the module_info data structure
	// into SMRAM and made sure that the MLE has not done something funny

	UINTN *sourceBuffer = (UINTN *)NULL;
	UINTN *destBuffer = (UINTN *) NULL;

	STM_STATUS rc;
	RETURN_STATUS rc1, rc2, rc3;               // used for return code
	UINT32 pageAlignedSize;
	UINT32 numModulePages;
	UINT_128   Data128;

	DEBUG((EFI_D_ERROR,"%ld AddPeVm - entered, PeType: %d\n", CpuIndex, PeType));
	PeVmData[PeType].PeVmState = PE_VM_ACTIVE;    // indicate we are here

	DEBUG((EFI_D_ERROR, "%ld AddPeVm - callerDataStructure location: 0x%08lx 0x%08lx\n", CpuIndex,  (UINT64) (((UINT64)callerDataStructure) >> 32), (UINT64)(callerDataStructure)));

	// pull information from the modules data structure

	PeVmData[PeType].UserModule.ModuleLoadAddress = callerDataStructure->ModuleLoadAddress;
	PeVmData[PeType].UserModule.ModuleSize = callerDataStructure->ModuleSize;
	PeVmData[PeType].UserModule.ModuleEntryPoint = callerDataStructure->ModuleEntryPoint;
	PeVmData[PeType].UserModule.AddressSpaceStart = callerDataStructure->AddressSpaceStart;
	PeVmData[PeType].UserModule.AddressSpaceSize = callerDataStructure->AddressSpaceSize;

	PeVmData[PeType].UserModule.VmConfig = callerDataStructure->VmConfig;
	PeVmData[PeType].UserModule.Cr3Load = callerDataStructure->Cr3Load;
	PeVmData[PeType].UserModule.SharedPage = callerDataStructure->SharedPage;
	PeVmData[PeType].UserModule.SharedPageSize = callerDataStructure->SharedPageSize;
	PeVmData[PeType].UserModule.Segment = callerDataStructure->Segment;
	PeVmData[PeType].UserModule.ModuleAddress = callerDataStructure->ModuleAddress;
	PeVmData[PeType].UserModule.ModuleDataSection = callerDataStructure->ModuleDataSection;
	PeVmData[PeType].UserModule.DoNotClearSize = callerDataStructure->DoNotClearSize;
	PeVmData[PeType].UserModule.RunCount = 0;

#if defined (MDE_CPU_X64)
	sourceBuffer = (UINTN *)((PeVmData[PeType].UserModule.ModuleAddress));
#else
	sourceBuffer = (UINTN *) ((UINT32) (PeVmData[PeType].UserModule.ModuleAddress) & 0x0FFFFFFFF);
	// note need to check for corner case where user passed 64 bit address to STM
#endif

	if (!IsGuestAddressValid ((UINTN)sourceBuffer, PeVmData[PeType].UserModule.ModuleSize, TRUE))
	{
		DEBUG((EFI_D_ERROR, "%ld AddPeVm - module_address: bad physical address %p\n", CpuIndex,(void *) sourceBuffer));

		PeVmData[PeType].PeVmState = PE_VM_AVAIL;  //  not there anymore
		return(PE_VM_BAD_PHYSICAL_ADDRESS);
	}

	DEBUG((EFI_D_ERROR, "%ld AddPeVm - ModuleSize: %ld (0x%08lx)\n", 
		CpuIndex,
		PeVmData[PeType].UserModule.ModuleSize, 
		(UINT64)PeVmData[PeType].UserModule.ModuleSize));
	DEBUG((EFI_D_ERROR, "%ld AddPeVm - module_entry_point:  0x%p\n", CpuIndex, (UINT64)PeVmData[PeType].UserModule.ModuleEntryPoint));
	DEBUG((EFI_D_ERROR, "%ld AddPeVm - ModuleLoadAddress: 0x%p\n", CpuIndex, (UINT64)PeVmData[PeType].UserModule.ModuleLoadAddress));
	DEBUG((EFI_D_ERROR, "%ld AddPeVm - AddressSpaceStart: 0x%p\n", CpuIndex, (UINT64)PeVmData[PeType].UserModule.AddressSpaceStart));
	DEBUG((EFI_D_ERROR, "%ld AddPeVm - AddressSpaceSize:  0x%p\n", CpuIndex, (UINT64) PeVmData[PeType].UserModule.AddressSpaceSize));
	DEBUG((EFI_D_ERROR, "%ld AddPeVm - DoNotClearSize: 0x%08lx\n", CpuIndex, (UINT64) PeVmData[PeType].UserModule.DoNotClearSize));

	DEBUG((EFI_D_ERROR, "%ld AddPeVm - CR3_LOAD:  0x%p\n", CpuIndex, (UINT64) PeVmData[PeType].UserModule.Cr3Load));
	DEBUG((EFI_D_ERROR, "%ld AddPeVm - VmConfig: 0x%08lx\n", CpuIndex, (UINT64) PeVmData[PeType].UserModule.VmConfig));
	DEBUG((EFI_D_ERROR, "%ld AddPeVm - shared page 0x%p\n", CpuIndex, (UINT64) PeVmData[PeType].UserModule.SharedPage));

	DEBUG((EFI_D_ERROR, "%ld AddPeVm - shared page size 0x%llx\n", CpuIndex, (UINT64) PeVmData[PeType].UserModule.SharedPageSize));
	DEBUG((EFI_D_ERROR, "%ld stmApaAddVM - segment page 0x%p\n", CpuIndex, (UINT64) PeVmData[PeType].UserModule.Segment));
	DEBUG((EFI_D_ERROR, "%ld AddPeVm - module_address 0x%p\n", CpuIndex, (UINT64) PeVmData[PeType].UserModule.ModuleAddress));
	DEBUG((EFI_D_ERROR, "%ld AddPeVm - ModuleDataSection 0x%p\n", CpuIndex, (UINT64) PeVmData[PeType].UserModule.ModuleDataSection));
	if(PeVmData[PeType].UserModule.SharedPage != 0)  //NULL not used because of 32bit/64bit diff
	{
		if (!IsGuestAddressValid ((UINTN)PeVmData[PeType].UserModule.SharedPage,
			(UINTN)PeVmData[PeType].UserModule.SharedPageSize,TRUE))
		{
			DEBUG((EFI_D_ERROR, "%ld AddPeVm - module_address: bad physical address %llx\n", CpuIndex,(void *) sourceBuffer));

			PeVmData[PeType].PeVmState = PE_VM_AVAIL;  //  not there anymore
			return(PE_VM_BAD_PHYSICAL_ADDRESS);
		}
	}

	// determine how large the module is
	// go to the module and determine its size from the header block

	// allocate the space needed for the requested space

	numModulePages = (PeVmData[PeType].UserModule.AddressSpaceSize + (PAGE_SIZE - 1)) >> 12; // calculate the number of pages in the module

	// allocate the space for the PE VM based upon the guest Address Space Size
	PeVmData[PeType].SmmBuffer = (UINTN *) AllocatePages(numModulePages);
	PeVmData[PeType].SmmBufferSize = numModulePages;  // AllocatePages does not retain size

	DEBUG((EFI_D_ERROR, "%ld AddPeVm - SmmBuffer at: %llx PeType: %d\n", CpuIndex, PeVmData[PeType].SmmBuffer, PeType));

	if(PeVmData[PeType].SmmBuffer == NULL)
	{
		DEBUG((EFI_D_ERROR, "%ld AddPeVm - failed to allocate module destination buffer\n", CpuIndex));
		FreePE_DataStructures(PeType);
		PeVmData[PeType].PeVmState = PE_VM_AVAIL;  //  not there anymore
		return(PE_SPACE_TOO_LARGE);
	}
	// move the module into the space

	// since we are allowing the user to have a larger memory than the module itself
	// we need to calculate where within SmmBuffer the module will be placed.
	// things to check for self protection  (will return an error to the user)
	//        ModuleLoadAddress < AddressSpaceStart
	//        ModuleLoadAddress + ModuleSize > AddressSpaceStart + AddressSpaceSize

	{
		if(PeVmData[PeType].UserModule.ModuleLoadAddress < PeVmData[PeType].UserModule.AddressSpaceStart)
		{
			DEBUG((EFI_D_ERROR, "%ld AddPeVm - Error: ModuleLoadAddress is lower than Address_Space_Start\n", CpuIndex));
			FreePE_DataStructures(PeType);
			PeVmData[PeType].PeVmState = PE_VM_AVAIL;  //  not there anymore
			return(PE_MODULE_ADDRESS_TOO_LOW);
		}

		// calculate the location within the allocated space to place the module
		destBuffer = PeVmData[PeType].SmmBuffer + PeVmData[PeType].UserModule.AddressSpaceStart - 
			PeVmData[PeType].UserModule.ModuleLoadAddress;
	}

	// make sure that the size of the module will fit into the allocated space

	pageAlignedSize = numModulePages << 12;
	{
		UINT64 Avail_Space = pageAlignedSize - (PeVmData[PeType].UserModule.ModuleLoadAddress - PeVmData[PeType].UserModule.AddressSpaceStart);
		if(Avail_Space < PeVmData[PeType].UserModule.ModuleSize)
		{
			DEBUG((EFI_D_ERROR, "%ld AddPeVm - Error: ModuleSize is too large to fit in the address space\n", CpuIndex));
			FreePE_DataStructures(PeType);
			PeVmData[PeType].PeVmState = PE_VM_AVAIL;  //  not there anymore
			return(PE_MODULE_TOO_LARGE);
		}
	}

	DEBUG((EFI_D_ERROR, "%ld AddPeVm - destBuffer: %llx sourceBuffer: %llx PeType %d\n", CpuIndex, (UINT64)destBuffer, (UINT64)sourceBuffer, PeType));
	{
		CopyMem (destBuffer, sourceBuffer, PeVmData[PeType].UserModule.ModuleSize);

		DEBUG((EFI_D_ERROR, "%ld AddPeVm - **loaded Module SMRAM location: 0x%016llx ModuleSize: 0x%08x contents: 0x%016llx\n", 
			CpuIndex, destBuffer, PeVmData[PeType].UserModule.ModuleSize, *destBuffer));
	}

	DEBUG((EFI_D_ERROR, "%ld AddPeVm - destBuffer: 0x%016llx  0x%016llx  0x%016llx  0x%016llx  0x%016llx\n",
		CpuIndex,
		*destBuffer,
		*(destBuffer + sizeof(UINT64)),
		*(destBuffer + 2*sizeof(UINT64)),
		*(destBuffer + 3*sizeof(UINT64)),
		*(destBuffer + 4*sizeof(UINT64))
		));

	/// create the EPT tables for the PE VM
	/// need to create this for Intel Ref c

	rc = PeEptInit(&mGuestContextCommonSmm[PeType].EptPointer);

	if (rc != STM_SUCCESS) {
		DEBUG((EFI_D_ERROR, "%ld AddPeVm - Unable to setup PE page tables\n", CpuIndex));
		FreePE_DataStructures(PeType);
		PeVmData[PeType].PeVmState = PE_VM_AVAIL;  //  not there anymore
		return(PE_NO_PT_SPACE);
	}

	// setup the permission for the shared page
	// map in our pages

	// setup the user module pages and address space

	rc1 = setupModulepages(PeType, CpuIndex);

	if(rc1 != STM_SUCCESS)
	{
		DEBUG((EFI_D_ERROR, "%ld AddPeVm - Unable to setup Module pages \n", CpuIndex));
		FreePE_DataStructures(PeType);
		PeVmData[PeType].PeVmState = PE_VM_AVAIL;  //  not there anymore
		return(PE_MODULE_MAP_FAILURE);
	}

	DEBUG((EFI_D_ERROR, "%ld AddPeVm - module pages sucessfully setup\n    now setting up shared page\n", CpuIndex));
	rc2 = EPTSetPageAttributeRange(
		mGuestContextCommonSmm[PeType].EptPointer.Uint64, 
		(UINTN) PeVmData[PeType].UserModule.SharedPage,
		(UINTN) PeVmData[PeType].UserModule.SharedPageSize, 
		(UINTN) PeVmData[PeType].UserModule.SharedPage,
		TRUE,       /*read*/
		TRUE,       /*write*/
		FALSE,       /*execute*/
		EptPageAttributeSet);

	if(rc2 != STM_SUCCESS) {
		DEBUG((EFI_D_ERROR, "%ld AddPeVm - Unable to map shared page into Prot Exec VM\n", CpuIndex));
		FreePE_DataStructures(PeType);
		PeVmData[PeType].PeVmState = PE_VM_AVAIL;  //  not there anymore
		return(PE_SHARED_MAP_FAILURE);
	}
	DEBUG((EFI_D_ERROR, "%ld AddPeVm - shared page sucessfully mapped - PeType: %d\n", CpuIndex, PeType));

	// set the permission and the PTEs for the r/o regions

	// setup the permission for the region list itself
	if(PeVmData[PeType].UserModule.Segment != NULL)
	{
		int counter;
		PE_REGION_LIST * rlist;

		if(!IsGuestAddressValid ((UINTN)PeVmData[PeType].UserModule.Segment,
			4096,TRUE))
		{
			DEBUG((EFI_D_ERROR, "%ld AddPeVm - region list: bad physical address: %p Size: %x\n", CpuIndex,
				PeVmData[PeType].UserModule.SharedPage, PeVmData[PeType].UserModule.SharedPageSize ));
			FreePE_DataStructures(PeType);
			
			PeVmData[PeType].PeVmState = PE_VM_AVAIL;  //  not there anymore
			return(PE_VM_BAD_PHYSICAL_ADDRESS);
		}

		// link the list into the PE VM's space
		DEBUG((EFI_D_ERROR, "%ld AddPeVm - mapping region list into PE VM Space - PeType %d\n", CpuIndex, PeType));

		rc3 = EPTSetPageAttributeRange(
			mGuestContextCommonSmm[PeType].EptPointer.Uint64, 
			(UINTN)PeVmData[PeType].UserModule.Segment, 
			4096,
			(UINTN)PeVmData[PeType].UserModule.Segment,          /* identity mapped */
			TRUE,       /* READ */
			FALSE,       /* write */
			FALSE,       /* Execute */
			EptPageAttributeSet);

		if (rc3 != STM_SUCCESS)
		{
			DEBUG((EFI_D_ERROR, "%ld AddPeVm - Couldn't Setup region list access\n", CpuIndex));
			FreePE_DataStructures(PeType);
			PeVmData[PeType].PeVmState = PE_VM_AVAIL;  //  not there anymore
			return(PE_REGION_LIST_SETUP_ERROR);
		}

		// now process the region list

		rlist = (PE_REGION_LIST *)PeVmData[PeType].UserModule.Segment;  // start the list

		// need to work in corner cases

		DEBUG((EFI_D_ERROR, "%ld AddPeVm - Setting up region list - PeType: %d\n", CpuIndex, PeType));
		for(counter = 0; counter < (4096/sizeof(PE_REGION_LIST)); counter++)
		{
			//!!! - Need to check to ensure the region is valid

			if((UINTN)rlist[counter].Address == 0)
				break;   // all done

			DEBUG((EFI_D_ERROR, "%ld AddPeVm - Setting up region %p size %016lx PeType: %d\n", CpuIndex, (UINTN) rlist[counter].Address, (UINTN) rlist[counter].Size, PeType));
			if(!IsGuestAddressValid((UINTN)rlist[counter].Address,(UINTN)rlist[counter].Size,TRUE))
			{
				DEBUG((EFI_D_ERROR, "%ld AddPeVm - region: bad physical address %p size %016lx\n", CpuIndex,
					(UINTN)rlist[counter].Address,(UINTN)rlist[counter].Size));
#ifdef PRODUCTION
				FreePE_DataStructures(PeType);
				PeVmData[PeType].PeVmState = PE_VM_AVAIL;  //  not there anymore
				return(PE_VM_BAD_PHYSICAL_ADDRESS);
#endif
				DEBUG((EFI_D_ERROR, "%ld AddPeVm - ***Warning*** bad region data - ignoring region entry in debug system\n", CpuIndex));
			}
			else
			{
				// now link it into the page table
				rc3 = EPTSetPageAttributeRange(
					mGuestContextCommonSmm[PeType].EptPointer.Uint64, 
					(UINTN)rlist[counter].Address, 
					(UINTN)rlist[counter].Size,
					(UINTN)rlist[counter].Address,          /* identity map */
					TRUE,       /* READ */
					FALSE,       /* write */
					FALSE,       /* Execute */
					EptPageAttributeSet);

				if (rc3 != STM_SUCCESS)
				{
					DEBUG((EFI_D_ERROR, "%ld AddPeVm - Couldn't Setup region list\n", CpuIndex));
					FreePE_DataStructures(PeType);
					PeVmData[PeType].PeVmState = PE_VM_AVAIL;  //  not there anymore
					return(PE_REGION_LIST_SETUP_ERROR);
				}
			}
		}
		print_region_list(PeType, CpuIndex);
	}
	else
	{
		DEBUG((EFI_D_ERROR, "%ld AddPeVm - No caller region requested\n", CpuIndex));
	}

	// allocate a page to be shared between the STM and the module

	// figure out a location in the guest physical address space for the page
	//
	// locate the shared STM/module page at the end of the guest physical address space

	PeVmData[PeType].UserModule.SharedStmPage =  PeVmData[PeType].UserModule.AddressSpaceStart +  PeVmData[PeType].UserModule.AddressSpaceSize;
	PeVmData[PeType].SharedPageStm =  (UINTN *) AllocatePages(1);  // one page for now

	DEBUG((EFI_D_ERROR, "%ld AddPeVm - ShareModuleStm at: %p\n", CpuIndex,  PeVmData[PeType].SharedPageStm));

	if(PeVmData[PeType].SharedPageStm == NULL)
	{
		DEBUG((EFI_D_ERROR,"%ld AddPeVm - failed to allocate module - STM shared page\n", CpuIndex));
		FreePE_DataStructures(PeType);
		PeVmData[PeType].PeVmState = PE_VM_AVAIL;  //  not there anymore
		return(PE_VM_PE_SETUP_ERROR);
	}
	// move the

	rc1 = EPTSetPageAttributeRange(
		mGuestContextCommonSmm[PeType].EptPointer.Uint64,   
		(UINTN)PeVmData[PeType].UserModule.SharedStmPage, 
		4096, 
		(UINTN) PeVmData[PeType].SharedPageStm,
		TRUE,     /* read */
		FALSE,     /* write */
		FALSE,    /* execute */
		EptPageAttributeSet);

	if(rc1 != STM_SUCCESS)
	{
		DEBUG((EFI_D_ERROR, "%d AddPeVm: could not setup module-stm shared page\n", CpuIndex));
	}
	DEBUG((EFI_D_ERROR, "%d AddPeVm: module-stm shared page setup\n", CpuIndex));
    //EptDumpPageTable (&mGuestContextCommonSmm[PeType].EptPointer);
	///link the PT with the allocated space

	/// somehow have the STM's PT mark these as R/O or invisable... (TODO)

	// check the hash, etc of the module (TBD stuff)

	Data128.Lo = mGuestContextCommonSmm[PeType].EptPointer.Uint64;
	Data128.Hi = 0;
	AsmInvEpt (INVEPT_TYPE_SINGLE_CONTEXT_INVALIDATION, &Data128);

	// (for now) start the VM...
	PeVmData[PeType].StartMode = PEVM_START_VMCALL;

	rc =  SetupProtExecVm(CpuIndex, PeVmData[PeType].UserModule.VmConfig, NEW_VM, PeType);

	if(rc != PE_SUCCESS)   // did we have a problem
	{
		DEBUG((EFI_D_ERROR, "%ld AddPeVm - Error in configuring PE VM\n", CpuIndex));
		FreePE_DataStructures(PeType);
		//setPEerrorCode(rc, StmVmm);    // tell the caller of the problem
		PeVmData[PeType].PeVmState = PE_VM_AVAIL;  //  not there anymore
		// StmVmm->NonSmiHandler = 0;     // no longer an PE VM
		AsmVmPtrLoad(&mGuestContextCommonSmi.GuestContextPerCpu[CpuIndex].Vmcs);

		/// at this point we should return to the MLE as per the Intel method...

		AsmVmClear(&mGuestContextCommonSmm[PeType].GuestContextPerCpu[0].Vmcs);
		mHostContextCommon.HostContextPerCpu[CpuIndex].GuestVmType = SMI_HANDLER;
		return(rc);
	}
	DEBUG((EFI_D_ERROR, "%ld AddPeVm - sucessfully completed - PeApicId: 0x%llx PeType: %d\n", CpuIndex, PeSmiControl.PeApicId, PeType));

	if(RunVm == 1)
	{
		PeVmData[PeType].StartMode = PEVM_START_VMCALL;
		LaunchPeVm(PeType, CpuIndex);  // launch the PE/VM

		// if we get to this point the PeVm has failed to launch so we need clean up the mess 
		// and return the error to the caller
		FreePE_DataStructures(PeType);
		DEBUG((EFI_D_ERROR, "%ld AddPeVm - VM/PE Launch Failure\n", CpuIndex));
		rc = PE_VMLAUNCH_ERROR;
		PeVmData[PeType].PeVmState = PE_VM_AVAIL;  //  not there anymore
	}
	else
	{
		DEBUG((EFI_D_ERROR, "%ld AddPeVm - VM not run per option\n", CpuIndex));
		rc = STM_SUCCESS;
		PeVmData[PeType].PeVmState = PE_VM_IDLE;  //  waiting for action
		PeSmiControl.PeExec = 0; // make sure
	}
	//setPEerrorCode(PE_VM_BAD_PHYSICAL_ADDRESS, StmVmm);

	AsmVmPtrLoad(&mGuestContextCommonSmi.GuestContextPerCpu[CpuIndex].Vmcs);

	/// at this point we should return to the MLE as per the Intel method...

	AsmVmClear(&mGuestContextCommonSmm[PeType].GuestContextPerCpu[0].Vmcs);

	mHostContextCommon.HostContextPerCpu[CpuIndex].GuestVmType = SMI_HANDLER;
	//PeSmiControl.PeExec = 1;         // when 1 PE_APIC_ID is executing a 
	return rc;
}

STM_STATUS setupModulepages(UINT32 PeType, UINT32 CpuIndex)
{
	RETURN_STATUS rc1 = STM_SUCCESS;
	UINTN module_end, Address_end;
	BOOLEAN Write = FALSE;   // default for text region
	BOOLEAN ExecuteHeap = FALSE;
	UINTN module_address = PeVmData[PeType].UserModule.ModuleLoadAddress & ~OFFSET_BITMASK_IA32_4K;
	UINTN ModuleSize; //= (PeVmData[PeType].UserModule.ModuleSize +(module_address - PeVmData[PeType].UserModule.ModuleLoadAddress)+ PAGE_SIZE_4K - 1) & ~OFFSET_BITMASK_IA32E_4K;
	
	UINTN AddressSpaceStart = (UINTN) (PeVmData[PeType].UserModule.AddressSpaceStart & 0xFFFFFFFF);
	UINTN AddressSpaceSize = (PeVmData[PeType].UserModule.AddressSpaceSize + PAGE_SIZE_4K - 1) & ~OFFSET_BITMASK_IA32E_4K;
	UINTN StartEndBlock;

	PeVmData[PeType].UserModule.ModuleDataSection &= ~OFFSET_BITMASK_IA32_4K;

	if(PeVmData[PeType].UserModule.ModuleDataSection == 0)
	{
		ModuleSize = (PeVmData[PeType].UserModule.ModuleSize + (module_address - PeVmData[PeType].UserModule.ModuleLoadAddress)+ PAGE_SIZE_4K - 1) & ~OFFSET_BITMASK_IA32E_4K;
	}
	else
	{
		ModuleSize = (UINTN)(PeVmData[PeType].UserModule.ModuleDataSection & 0xFFFFFFFF) - module_address;
	}

	DEBUG((EFI_D_ERROR, "%ld setModulepages - entered AddressSpaceStart: %x AddressSpaceSize: %x\n", CpuIndex, AddressSpaceStart, AddressSpaceSize));
	if(module_address > AddressSpaceStart)
	{
		// we have some space between the start of the address space and the module

		PeVmData[PeType].UserModule.FrontDataRegionSize = module_address - AddressSpaceStart;

		DEBUG((EFI_D_ERROR, "%ld setModulepages - setting up address space before the PE module (AddressSpaceStart): %x\n", CpuIndex, AddressSpaceStart));

		rc1 = EPTSetPageAttributeRange(
			mGuestContextCommonSmm[PeType].EptPointer.Uint64, 
			AddressSpaceStart, 
			PeVmData[PeType].UserModule.FrontDataRegionSize,
			(UINTN) PeVmData[PeType].SmmBuffer,
			TRUE,  /* read */
			TRUE,  /* write */
			FALSE, /* execute */
			EptPageAttributeSet);

		if(rc1 != RETURN_SUCCESS)
		{
			DEBUG((EFI_D_ERROR, "%ld setModulepages - failed to setup area in front of module\n", CpuIndex));
			return 0xFFFFFFFF;  // generic stm error
		}   
	}
	else
	{
		DEBUG((EFI_D_ERROR, "%ld setModulepages - no data space in front of module\n", CpuIndex));
		PeVmData[PeType].UserModule.FrontDataRegionSize = 0;
		if(module_address < AddressSpaceStart)
		{
			DEBUG((EFI_D_ERROR, "%ld setModulepages: Module starts before address space starts\n", CpuIndex));
			return ERROR_STM_UNSPECIFIED;      // universal error for now
		}
	}

	// stuff in the middle

	DEBUG((EFI_D_ERROR, "%ld setModulepages: Setting up area in the middle (module_address): %x\n", CpuIndex, module_address));

	if((PeVmData[PeType].UserModule.VmConfig & PERM_VM_SET_TEXT_RW) == PERM_VM_SET_TEXT_RW)
	{
		Write = TRUE;
	}

	rc1 = EPTSetPageAttributeRange(
		mGuestContextCommonSmm[PeType].EptPointer.Uint64, 
		module_address, 
		ModuleSize,
		(UINTN)((UINTN)PeVmData[PeType].SmmBuffer + (UINTN)PeVmData[PeType].UserModule.FrontDataRegionSize),
		TRUE,       /* READ */
		TRUE,       /* write */
		TRUE,       /* Execute */
		EptPageAttributeSet);

	if(rc1 != RETURN_SUCCESS)
	{
		DEBUG((EFI_D_ERROR, "%d setModulepages - could not setup module area within address space\n", CpuIndex));
		return ERROR_STM_UNSPECIFIED;  // generic stm error
	}
	// stuff at the end

	module_end = module_address + ModuleSize;
	Address_end = AddressSpaceStart + AddressSpaceSize;

	if(PeVmData[PeType].UserModule.ModuleDataSection == 0)
	{
		DEBUG((EFI_D_ERROR, "%ld setModulepages - ModuleDataSection is NULL, calculating data section\n", CpuIndex));

		// user did not provide a data section - so we calculate it

		PeVmData[PeType].UserModule.ModuleDataSection = module_end;
	}

	if(PeVmData[PeType].UserModule.ModuleDataSection < Address_end)
	{
		PeVmData[PeType].UserModule.DataRegionSize = Address_end - module_end;
		StartEndBlock = (UINTN) (((UINTN) PeVmData[PeType].SmmBuffer) + (PeVmData[PeType].UserModule.ModuleDataSection - PeVmData[PeType].UserModule.AddressSpaceStart));
		DEBUG((EFI_D_ERROR, "%ld setModulepages - Setting up area at the end (module_end): %x\n", CpuIndex, module_end));
		DEBUG((EFI_D_ERROR, "%ld setModulepages - module_end: %llx DataRegionSize: %llx DataRegionStart: %llx SmmBuffer: %llx\n",
			CpuIndex,
			PeVmData[PeType].UserModule.ModuleDataSection,
			PeVmData[PeType].UserModule.DataRegionSize, 
			PeVmData[PeType].UserModule.ModuleDataSection, 
			PeVmData[PeType].SmmBuffer));

		PeVmData[PeType].UserModule.DataRegionSmmLoc = StartEndBlock;

		DEBUG((EFI_D_ERROR, "%ld setModulepages - StartEndBlock: %llx\n", CpuIndex, StartEndBlock));

		if((PeVmData[PeType].UserModule.VmConfig & PERM_VM_EXEC_HEAP) == PERM_VM_EXEC_HEAP)
		{
			ExecuteHeap = TRUE;
			DEBUG((EFI_D_ERROR, "%d setModulepages - Execute Heap set to TRUE\n"));
		}
		else
			ExecuteHeap = FALSE;

		/*DEBUG*/ DEBUG((EFI_D_ERROR, "%d setModulepages - Execute Heap set to TRUE for debug purposes\n"));
		ExecuteHeap = TRUE;

		rc1 = EPTSetPageAttributeRange(
			mGuestContextCommonSmm[PeType].EptPointer.Uint64, 
			(UINTN)PeVmData[PeType].UserModule.ModuleDataSection, 
			PeVmData[PeType].UserModule.DataRegionSize, 
			StartEndBlock,
			TRUE,     /* read */
			TRUE,     /* write */
			ExecuteHeap,    /* execute */
			EptPageAttributeSet);

		if(rc1 != RETURN_SUCCESS)
		{
			DEBUG((EFI_D_ERROR, "%d setModulepages - could not setup end of address space\n", CpuIndex));
			return ERROR_STM_UNSPECIFIED;
		}
		DEBUG((EFI_D_ERROR, "%d setModulepages - end address region setup\n", CpuIndex));
	}
	else
	{
		PeVmData[PeType].UserModule.DataRegionSize = 0;
		if(module_end == Address_end)
		{
			DEBUG((EFI_D_ERROR, "%d setModulepages - no space after end of module\n", CpuIndex));			
		}
		else
		{
			DEBUG((EFI_D_ERROR, "%d setModulepages - end of Module is beyond address space end\n", CpuIndex));
			rc1 = ERROR_STM_UNSPECIFIED;
		}
	}

	// verify the no data section clearing size

	if((UINTN)PeVmData[PeType].UserModule.DoNotClearSize > PeVmData[PeType].UserModule.DataRegionSize)
	{
		DEBUG((EFI_D_ERROR, "%d setModulepages - DoNotClearSize larger than DataRegionSize - Using DataRegion Size\n", CpuIndex));
		PeVmData[PeType].UserModule.DoNotClearSize = (UINT32)PeVmData[PeType].UserModule.DataRegionSize;
	}
	DEBUG((EFI_D_ERROR, "%d SetModulepages - DoNotClearSize is: 0x%08lx\n", 
		CpuIndex, PeVmData[PeType].UserModule.DoNotClearSize));
	
	//EptDumpPageTable (&mGuestContextCommonSmm[PeType].EptPointer);
	return (STM_STATUS) rc1;
}