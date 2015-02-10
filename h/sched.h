#ifndef _SCHED_CLASSES_
#define _SCHED_CLASSES_
#define LINUXSCHED 1
#define MULTIQSCHED 2 
#endif
void setschedclass(int valSchedClass);
int getschedclass(void);
