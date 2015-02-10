//#include <time.h>
#include <conf.h>
#include <kernel.h>
//#include <proc.h>
//#include <q.h>
//#include <sleep.h>
//#include <sched.h>
#include <stdio.h>
//#include <math.h>
//#include "lab1.h"
#include <sched.h>
#define LOOP 50

int prA, prB, prC, prD, prE, prF;
int proc(char c);

int main() {
    int i;
    int count = 0;
    //char buf[8];
//    time_t t;
    for(i = 0; i < 10; i++){
        srand(i);
        printf("%d\n", rand()%100);
    }

    /* Linux like Scheduler */  
    kprintf("Sched Type is %d\n",getschedclass());
    //setschedclass(LINUXSCHED);
    setschedclass(MULTIQSCHED);
    kprintf("Sched Type is %d\n",getschedclass());
    //strtclk();
    resume(prA = create(proc, 2000, 5, "proc A", 1, 'A'));
    resume(prD = createReal(proc, 2000, 30, "proc D", 1, 'D'));
    resume(prB = create(proc, 2000, 50, "proc B", 1, 'B'));
    resume(prE = createReal(proc, 2000, 40, "proc E", 1, 'E'));
    resume(prC = create(proc, 2000, 90, "proc C", 1, 'C'));
    resume(prF = createReal(proc, 2000, 50, "proc F", 1, 'F'));
    while (count++ < LOOP) {
        kprintf("M");
        for (i = 0; i < 10000000; i++)
            ;
    }
    return 0;
}

int proc(char c) {
    int i;
    int count = 0;

    while (count++ < LOOP) {
        kprintf("%c", c);
        for (i = 0; i < 10000000; i++)
            ;
    }
    return 0;
}

