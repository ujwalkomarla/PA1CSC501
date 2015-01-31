/* getschedclass.c - getschedclass */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * getschedclass  --  get the current scheduling algorithm
 *------------------------------------------------------------------------
 */
SYSCALL getschedclass(void)
{
	return(schedClass);
}
