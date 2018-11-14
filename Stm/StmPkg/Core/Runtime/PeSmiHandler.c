/** @file

PE SMI handler - Used used for getting the other processor state and handling
SMI's during VM/PE build and execution

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

PE_SMI_CONTROL PeSmiControl;

extern void SignalPeVm(UINT32 CpuIndex);
extern int CheckAndGetState(UINT32 CpuIndex);
extern void CpuReadySync(UINT32 Index);
extern PE_VM_DATA PeVmData[4];   // right now support a max of 3 PE VM (VM 0 is the SMI_HANDLER)
extern int CheckTimerSTS(UINT32 Index);
extern void StopSwTimer(void);
extern void SetEndOfSmi(void);
extern void PrintVmxState(UINT32 CpuIndex, ROOT_VMX_STATE * RootState);

static UINT32 HandleTimer = 0;
static UINT32 HandleSmi = 0;

// additional VM/PE SMI handling 

static UINT32 retvalue = 0;

UINT32 PeSmiHandler(UINT32 CpuIndex)
{
	ROOT_VMX_STATE * RootState;
	UINT64 * NumProcessors;
	UINT32 PeType = PE_PERM;
	UINT32 CpuNum;

	InterlockedCompareExchange32(&PeSmiControl.PeSmiState, PESMINULL, PESMIHSMI);
		//DEBUG((EFI_D_ERROR, "%ld PeSmiHandler - CurrPeSmiState %ld\n", CpuIndex, PeSmiControl.PeSmiState));

	if(PeSmiControl.PeCpuIndex == (INT32)CpuIndex )  // when the pe/vm comes in...
	{
		//DEBUG((EFI_D_ERROR, "%ld PeSmiHandler - VM/PE responded to SMI, CurrPeSmiState %ld\n", CpuIndex, PeSmiControl.PeSmiState));
		InterlockedCompareExchange32(&PeSmiControl.PeSmiState, PESMIPNMI2, PESMINULL);
	}

	if(InterlockedCompareExchange32(&PeSmiControl.PeSmiState, PESMIPNMI, PESMIPNMI2) == PESMIPNMI)
		///PESMIPNMI == PeSmiControl.PeSmiState)
	{
		// eventually the VM/PE will be started (or at least built) and this will cause one of the processors
		// to send a NMI to the VM/PE processor causing it to drop out and process the SMI
		// when it does, all processors will exit this loop and process the SMI as usual

		SignalPeVm(CpuIndex);  // make sure that the PE/VM processes this SMI as well
		//PeSmiControl.PeSmiState = PESMINULL;

	}

	CpuReadySync(CpuIndex);     // everyone waits until processor 0 figures out what to do

	switch(PeSmiControl.PeSmiState)
	{
	case PESMIPSMI:

		// VM/PE sends a SMI to the other processors when it wants state information from other CPU's

		NumProcessors = (UINT64 *) PeVmData[PeType].SharedPageStm;
		RootState = (ROOT_VMX_STATE *) ((char *)NumProcessors + 64);//sizeof(*NumProcessors) + sizeof(*NumProcessors));

		// get the local processor state

		GetRootVmxState(CpuIndex, &RootState[CpuIndex]);

		//retvalue = 1;    // we did something
		CpuReadySync(CpuIndex);   // wait for everyone to finish
		if(CpuIndex == 0)
		{
			InterlockedCompareExchange32(&PeSmiControl.PeSmiState, PESMIPSMI, PESMINULL);   // reset the state
		}
		return 1;    // tell the SmiEventHandler that there is one less processor 

		break;

	case PESMIHSMI:

		if(PeSmiControl.PeCpuIndex == (INT32)CpuIndex )
		{	
			InterlockedCompareExchange32(&retvalue, 1, 0);   // make sure that this is zero

			if(InterlockedCompareExchange32(&PeSmiControl.PeWaitTimer, 1, 1) == 1)
			{
				if(CheckTimerSTS(CpuIndex) != 0)
				{
					//DEBUG((EFI_D_ERROR, "%ld CheckAndGetState - (PESMIHSMI) Processing VM/PE startup PeSmiState: %d\n", CpuIndex, PeSmiControl.PeSmiState));

					InterlockedCompareExchange32(&PeSmiControl.PeSmiState, PESMIHSMI, PESMIHTMR);

					NumProcessors = (UINT64 *) PeVmData[PeType].SharedPageStm;
					RootState = (ROOT_VMX_STATE *) ((char *)NumProcessors + 64);//sizeof(*NumProcessors) + sizeof(*NumProcessors));

					// get the local processor state

					GetRootVmxState(CpuIndex, &RootState[CpuIndex]);

					InterlockedCompareExchange32(&retvalue, 0, 1);  // we set to one to indicate we are not there

					// the VM/PE Cpu cleans up and runs
					//PeSmiControl.PeWaitTimer = 0;
					InterlockedCompareExchange32(&PeSmiControl.PeWaitTimer, 1, 0);
					//ClearSwTimerSTS();
					StopSwTimer();
					retvalue = 1;
					// start the VM/PE
					PeVmData[PeType].StartMode = PEVM_PRESTART_SMI; // starting from SMI
					CpuReadySync(CpuIndex);   // sync everyone ud
					SetEndOfSmi();    // make sure that the timer SMI has been cleared

					for(CpuNum = 0; CpuNum < mHostContextCommon.CpuNum; CpuNum++)
					{
						PrintVmxState(CpuNum, &RootState[CpuNum]);
					}

					if( mHostContextCommon.StmShutdown == 1)
					{
						// time to quit
						 StmTeardown(CpuIndex);
					}

					RunPermVM(CpuIndex);
					//PeVmData[PeType].StartMode = PEVM_START_SMI; // for consistency in error conditions
					//CpuDeadLoop ();
					//  should not get here...
					return retvalue;    // tell the SmiEventHandler that there is one less processor
				}
			}
		}

		CpuReadySync(CpuIndex);

		if(InterlockedCompareExchange32(&PeSmiControl.PeSmiState, PESMIHTMR, PESMIHTMR) == PESMIHTMR)
		{
			NumProcessors = (UINT64 *) PeVmData[PeType].SharedPageStm;
			RootState = (ROOT_VMX_STATE *) ((char *)NumProcessors + 64);//sizeof(*NumProcessors) + sizeof(*NumProcessors));

			GetRootVmxState(CpuIndex, &RootState[CpuIndex]);
			retvalue = 1;

			// we do  not reset the state here as the VM/PE will be processing
			// when it competes it should end with a PeSmiState pf PESMIPNMI (waiting for NMI)
		}
		else
		{
			if(CpuIndex == 0)
			{
				//PeSmiControl.PeSmiState = PESMINULL;
				InterlockedCompareExchange32(&PeSmiControl.PeSmiState, PESMIHSMI, PESMINULL); // one of these will work	
			}
			retvalue = 0;
		}
		return retvalue;

		break;

	case PESMINULL:

		return 0;    // process normally
		break;

	case PESMIPNMI:
	case PESMIHTMR:
		// at this point, if this is set, the VM/PE is in the startup process and
		// has set this so that at the next SMI, if it occurs while the VM/PE is active
		// the pssmihandler can shoot down the VM/PE

		// we have a one return because the VM/PE will stay in SMM

		//SetEndOfSmi();    // make sure that the timer SMI has been cleared

		return 1;
		break;

	default:

		DEBUG((EFI_D_ERROR, "%ld CheckAndGetState (default) ERROR incorrect PeSmiState: %ld, setting to PESMINULL (0)\n",
			CpuIndex, PeSmiControl.PeSmiState));
		PeSmiControl.PeSmiState = PESMINULL;
		return 0;
	}

}