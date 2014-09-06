#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lab1.h>

#define LOOP 100

/* test1.c
 * This test program creates three processes, prA, prB, and prC, at
 * priority 20.  The main process also has priority 20.
 */

int prA, prB, prC, prD, prE, prF;

prch(char c)
{
    int i, count=0;

    while(count++ < LOOP) {
        kprintf("%c", c); 
        for (i = 0; i< 1000000; i++);
    }   
    kprintf("\nps for pid %d proc %s\n", currpid, proctab[currpid].pname);
    ps();
}

main()
{
    int i, count=0;

    kprintf("\n\nTEST1:\n");
    resume(prA = create(prch,2000,80,"proc A",1,'A'));
    resume(prC = create(prch,2000,5,"proc C",1,'C'));
    resume(prB = create(prch,2000,90,"proc B",1,'B'));
    resume(prF = create(prch,2000,60,"proc F",1,'F'));
    setschedclass(RANDOMSCHED);
    resume(prD = create(prch,2000,10,"proc D",1,'D'));
    resume(prE = create(prch,2000,40,"proc E",1,'E'));
    kprintf("\nps for pid %d proc %s\n", currpid, proctab[currpid].pname);
    ps();

    while (count++ < LOOP) {
        kprintf("%c", 'M');
        for (i = 0; i< 1000000; i++);
    }
    kprintf("\nps for pid %d proc %s\n", currpid, proctab[currpid].pname);
    ps();
}


