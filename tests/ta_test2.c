#include <conf.h>
#include <kernel.h>
#include <q.h>

/* test2.c
 * This test program creates three processes, prA, prB, and prC, at
 * priority 30.  The main process has priority 20.
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

        kprintf("\n\nTEST2:\n");

	prA = create(prch,2000,30,"proc A",1,'A');
	prB = create(prch,2000,30,"proc B",1,'B');
	prC = create(prch,2000,30,"proc C",1,'C');

	resume(prC);
	resume(prB);
	resume(prA);


	while(count++ < LOOP) {
		kprintf("%c", 'D');
		for (i = 0; i< 1000000; i++);
	}
}



