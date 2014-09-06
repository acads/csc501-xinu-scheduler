#include <conf.h>
#include <kernel.h>
#include <q.h>

/* test3.c
 * This test program creates three processes, prA, prB, and prC, at
 * priority 10.  The main process has priority 20.
 *
 * The main routine then calls chprio to change the priorities of prA, prB
 * and prC to be 30, while it remains at priority 20.
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

        kprintf("\n\nTEST3:\n");

	resume(prA = create(prch,2000,10,"proc A",1,'A'));
	resume(prB = create(prch,2000,10,"proc B",1,'B'));
	resume(prC = create(prch,2000,10,"proc C",1,'C'));

	chprio(prA, 30);
	chprio(prB, 30);
	chprio(prC, 30);

	while (count++ < LOOP) {
		kprintf("%c", 'D');
		for (i=0; i< 1000000; i++);
	}
}


