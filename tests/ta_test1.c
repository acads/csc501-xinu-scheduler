#include <conf.h>
#include <kernel.h>
#include <q.h>

#define LOOP 100

/* test1.c
 * This test program creates three processes, prA, prB, and prC, at
 * priority 20.  The main process also has priority 20.
 */

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

	kprintf("\n\nTEST1:\n");
	resume(prA = create(prch,2000,20,"proc A",1,'A'));
	resume(prB = create(prch,2000,20,"proc B",1,'B'));
	resume(prC = create(prch,2000,20,"proc C",1,'C'));

	while (count++ < LOOP) {
		kprintf("%c", 'D');
		for (i = 0; i< 1000000; i++);
	}
}


