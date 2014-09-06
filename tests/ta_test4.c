#include <conf.h>
#include <kernel.h>
#include <q.h>

/* test4.c
 * This test program creates three processes, prA, prB, and prC, at
 * priorities 10, 9, and 10 respectively.  The main process has priority 20.
 *
 * The main routine then calls chprio to change the priority of prB to be 15,
 * while it changes its own priority to 5.
 */
#define LOOP 100
int prA, prB, prC;

prch(char c)
{
	int i, count=0;

	while(count++ < LOOP) {
		kprintf("%c", c);
		for (i = 0; i< 1000000; i++);
	}
}

main()
{
	int i, count=0;

        kprintf("\n\nTEST4:\n");

	resume(prA = create(prch,2000,10,"proc A",1,'A'));
	resume(prB = create(prch,2000, 9,"proc B",1,'B'));
	resume(prC = create(prch,2000,10,"proc C",1,'C'));


	chprio(prB,      15);
	chprio(getpid(),  5);

	while (count++ < LOOP) {
		kprintf("%c", 'D');
		for (i = 0; i< 1000000; i++);
	}
}


