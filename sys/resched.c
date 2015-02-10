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
	static int inSchedClass = !REALTIME;
	if(0==numproc){
		//kprintf("Null");
		optr= &proctab[currpid];
		optr->pstate = PRFREE;
		//getlast(rdytail);
		
		nptr = &proctab[currpid = 0];
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
				if (optr->pstate == PRCURR) {//Is this condition necessary?
					optr->pstate = PRREADY;
					//dequeue(currpid);
				}
			}else{
				/* no switch needed if current process goodness higher than next*/
				////pprio to goodness
				if ( ( optr->pstate == PRCURR) && (lastkey(rdytail)<optr->goodness)) 
					return(OK);
				/* force context switch */
				if (optr->pstate == PRCURR) {
					optr->pstate = PRREADY;
					//// 
					//pprio to goodness
					insert(currpid,rdyhead,optr->goodness);
				}
			}
			
			if(0>=(currpid = getlast(rdytail)) || 0>=epoch){//Is second condition a necessity? Instead a COUNT of ready process.
				register struct pentry *pptr;
				int i;
				//kprintf("e=%d",epoch);
				epoch = 0;
				(pptr = &proctab[0]) -> goodness = 0;
				epoch+= pptr->quantum = QUANTUM;
				(pptr->tPriority) = 0;
				insert(0,rdyhead,pptr->goodness);
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
				//kprintf("e=%d",epoch);
				currpid = getlast(rdytail);
			}
			
			/* remove highest priority process at end of ready list */

			nptr = &proctab[ currpid ];
			nptr->pstate = PRCURR;		/* mark it currently running	*/
			#ifdef	RTCLOCK
			preempt = nptr->quantum;		/* reset preemption counter	*/
			#endif
		}else if(schedClass == MULTIQSCHED){
			//Reduce quantum by used value.
			epoch-= ((optr= &proctab[currpid])->quantum - preempt);
			//kprintf("%s Process: %d Initial Quantum, %d Left Quantum(Preempt)\r\n",optr->pname,optr->quantum,preempt);
			if(REALTIME){
			optr->quantum -= (QUANTUM - preempt);
			}else{
			optr->quantum = preempt;
			}
			if(inSchedClass != REALTIME) optr->goodness = (optr->quantum == 0)?0:(optr->tPriority + optr->quantum);

			if(0>=preempt){
				/* force context switch */
				if (optr->pstate == PRCURR) {
					optr->pstate = PRREADY;
				}
				if(inSchedClass==REALTIME && optr->quantum > 0)  insert(currpid,rdyhead,1);
			
			}else{
				if(inSchedClass != REALTIME){
					if ( ( optr->pstate == PRCURR) && (lastkey(rdytail)<optr->goodness)) {
					////pprio to goodness
						return(OK);
					}

					/* force context switch */

					if (optr->pstate == PRCURR) {
						optr->pstate = PRREADY;
						//// 
						//pprio to goodness
						insert(currpid,rdyhead,optr->goodness);
					}
				}else{//realtime
					if (optr->pstate == PRCURR) return(OK);//i.e., The current process is still executable. If not, resched.
				}
			}
			if(0>=(currpid = getlast(rdytail)) || 0>=epoch){
				int visitedBothQueue = 0;
				do{
					if(rand()%100 < 70){
						inSchedClass=REALTIME;
						register struct pentry *pptr;
						int i;
						epoch = 0;
						epoch+= (pptr = &proctab[0]) -> quantum = QUANTUM;
						insert(0,rdyhead,0);
						for(i=1;i<NPROC;i++){//// '1', To avoid counting NULL process quantum
							if((pptr = & proctab[i])->pstate != PRFREE && pptr->realtime == REALTIME){
								//epoch+ = pptr->pprio;
								//pptr->goodness = (pptr->quantum <= 0)?pptr->pprio:(pptr->pprio + pptr->quantum);
								epoch+= pptr->quantum = QUANTUM*QUANTUM;
								//pptr->tPriority = pptr->pprio;
								
							}
							if(pptr->pstate == PRREADY && pptr->realtime == REALTIME){
								// enqueue or insert
								insert(i,rdyhead,1);
							}
						}
						visitedBothQueue|=1;
					}else{
						inSchedClass = !REALTIME;
						register struct pentry *pptr;
						int i;
						epoch = 0;
						(pptr = &proctab[0]) -> goodness = 0;
						epoch+= pptr->quantum = QUANTUM;
						(pptr->tPriority) = 0;
						insert(0,rdyhead,pptr->goodness);
						for(i=1;i<NPROC;i++){//// '1', To avoid counting NULL process quantum
							if((pptr = & proctab[i])->pstate != PRFREE && pptr->realtime == !REALTIME){
								//epoch+ = pptr->pprio;
								pptr->goodness = (pptr->quantum <= 0)?pptr->pprio:(pptr->pprio + pptr->quantum);
								epoch+= pptr->quantum = (pptr->quantum <=0)?pptr->pprio:(pptr->pprio + (pptr->quantum)/2);
								pptr->tPriority = pptr->pprio;
								
							}
							if(pptr->pstate == PRREADY && pptr->realtime == !REALTIME){
								// enqueue or insert
								insert(i,rdyhead,pptr->goodness);
							}
						}
						visitedBothQueue|=2;
					}				
					
					if((currpid = getlast(rdytail))>0){
						
						break;
					}
				}while(visitedBothQueue != 3);
				
				
			}
				nptr = &proctab[ currpid ];
				nptr->pstate = PRCURR;		/* mark it currently running	*/
				#ifdef	RTCLOCK
				preempt = nptr->quantum;
				if(inSchedClass == REALTIME) preempt = (nptr->quantum/QUANTUM > 1)?QUANTUM:nptr->quantum;//kprintf("p=%d",preempt);}
						/* reset preemption counter	*/
				#endif
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
	//kprintf("c=%d",currpid);
	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	/* The OLD process returns here when resumed. */
	return OK;
}