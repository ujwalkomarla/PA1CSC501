/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>

unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int resched()
{
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */

	if(schedClass == LINUXSCHED){
		if(0==numproc){
			optr= &proctab[currpid];
			nptr = &proctab[getlast(rdytail)];
			nptr->pstate = PRCURR;		/* mark it currently running	*/
			//nptr->quantum = QUANTUM;
			#ifdef	RTCLOCK
			preempt = QUANTUM;		/* reset preemption counter	*/
			#endif
			epoch = QUANTUM;
		}else{
		
			////
			//Reduce quantum by used value.
				epoch-= ((optr= &proctab[currpid])->quantum - preempt);
				//kprintf("%s Process: %d Initial Quantum, %d Left Quantum(Preempt)\r\n",optr->pname,optr->quantum,preempt);
				optr->quantum = preempt;
				
			////
			
			if(0>=preempt){
				/* force context switch */
				if (optr->pstate == PRCURR) {
					//optr->pstate = PRREADY;//Be careful not to remove it. Read suspend() function code before doing anything here. Never ending recursion
					optr->pstate = PRREADY;
					dequeue(currpid);
					//// 
					//pprio to goodness
					//insert(currpid,rdyhead,optr->goodness);
					//suspend(currpid);
					////
					
				}
			
			}else{
				/* no switch needed if current process goodness higher than next*/

				if ( ( optr->pstate == PRCURR) &&
				////pprio to goodness
				   (lastkey(rdytail)<optr->goodness)) {
				////
					return(OK);
				}

				/* force context switch */

				if (optr->pstate == PRCURR) {
					optr->pstate = PRREADY;
					//// 
					//pprio to goodness
					insert(currpid,rdyhead,optr->goodness);
					//Reduce quantum by used value.
					//epoch-= optr->quantum - preempt;
					//optr->quantum =preempt;
					////
				}
			}
			
			if(0==(currpid = getlast(rdytail)) || 0>=epoch){
				ProcQueueInit();
			}
			
			/* remove highest priority process at end of ready list */

			nptr = &proctab[ currpid ];
			nptr->pstate = PRCURR;		/* mark it currently running	*/
			#ifdef	RTCLOCK
			preempt = nptr->quantum;		/* reset preemption counter	*/
			#endif
		}
		
		//kprintf("currpid : %d \r\n",currpid);
	}else{
	
	}
	
	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
	/* The OLD process returns here when resumed. */
	return OK;
}

LOCAL void ProcQueueInit(void){

	register struct pentry *pptr;
	int i;
	epoch = 0;
	for(i=0;i<NPROC;i++){//// '1', To avoid counting NULL process quantum
		if((pptr = & proctab[i])->pstate != PRFREE){
			//epoch+ = pptr->pprio;
			epoch+= pptr->quantum = (pptr->quantum==0)?pptr->pprio:(pptr->pprio + (pptr->quantum)/2);
			pptr->goodness = (pptr->quantum == 0)?0:(pptr->pprio + pptr->quantum);
			
		}
		if(pptr->pstate == PRREADY){
			// enqueue or insert
			insert(i,rdyhead,pptr->goodness);
		}
	}
	currpid = getlast(rdytail);
}