/* setschedclass.c - setschedclass */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * setschedclass  --  get the current scheduling algorithm
 *------------------------------------------------------------------------
 */
SYSCALL setschedclass(int valSchedClass)
{
	schedClass = valSchedClass;
	return(OK);
}
