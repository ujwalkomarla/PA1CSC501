/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#define REALTIME 1

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
	if(0==numproc){
		optr= &proctab[currpid];
		getlast(rdytail);
		nptr = &proctab[0];
		nptr->pstate = PRCURR;		/* mark it currently running	*/
		//nptr->quantum = QUANTUM;
		#ifdef	RTCLOCK
		preempt = QUANTUM;		/* reset preemption counter	*/
		#endif
		epoch = QUANTUM;

	}else{
		if(schedClass == LINUXSCHED){
			

				////
			//Reduce quantum by used value.
				epoch-= ((optr= &proctab[currpid])->quantum - preempt);
				//kprintf("%s Process: %d Initial Quantum, %d Left Quantum(Preempt)\r\n",optr->pname,optr->quantum,preempt);
				optr->quantum = preempt;
				optr->goodness = (optr->quantum == 0)?0:(optr->tPriority + optr->quantum);
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
			
			if(EMPTY==(currpid = getlast(rdytail)) || 0>=epoch){
				register struct pentry *pptr;
				int i;
				epoch = 0;
				for(i=1;i<NPROC;i++){//// '1', To avoid counting NULL process quantum
					if((pptr = & proctab[i])->pstate != PRFREE){
						//epoch+ = pptr->pprio;
						pptr->goodness = (pptr->quantum <= 0)?pptr->pprio:(pptr->pprio + pptr->quantum);
						epoch+= pptr->quantum = (pptr->quantum <=0)?pptr->pprio:(pptr->pprio + (pptr->quantum)/2);
						pptr->tPriority = pptr->pprio;
						
					}
					if(pptr->pstate == PRREADY){
						// enqueue or insert
						insert(i,rdyhead,pptr->goodness);
					}
				}
				currpid = getlast(rdytail);
			}
			
			/* remove highest priority process at end of ready list */

			nptr = &proctab[ currpid ];
			nptr->pstate = PRCURR;		/* mark it currently running	*/
			#ifdef	RTCLOCK
			preempt = nptr->quantum;		/* reset preemption counter	*/
			#endif
		}else if(schedClass == MULTIQSCHED){
			//random
			//if random 70%
			if(rand()%100 < 70){
				//Real
				//ProcQueueInit(REALTIME);
			}else{
				//ProcQueueInit(!REALTIME);
			}
		}else{//DEFAULT
		
			/* no switch needed if current process priority higher than next*/
			
			if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
			   (lastkey(rdytail)<optr->pprio)) {
				return(OK);
			}
			
			/* force context switch */

			if (optr->pstate == PRCURR) {
				optr->pstate = PRREADY;
				insert(currpid,rdyhead,optr->pprio);
			}

			/* remove highest priority process at end of ready list */

			nptr = &proctab[ (currpid = getlast(rdytail)) ];
			nptr->pstate = PRCURR;		/* mark it currently running	*/
		#ifdef	RTCLOCK
			preempt = QUANTUM;		/* reset preemption counter	*/
		#endif
		}
	}
	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	/* The OLD process returns here when resumed. */
	return OK;
}