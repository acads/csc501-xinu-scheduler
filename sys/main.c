#include <conf.h>
#include <kernel.h>
#include <q.h>
#include <stdio.h>
#include <math.h>
#include <proc.h>
#include <lab1.h>

#define LOOP    50

int prA, prB, prC;
int proc_a(), proc_b(), proc_c();
int proc(char c);
int proc_basic(char c);
volatile int a_cnt = 0;
volatile int b_cnt = 0;
volatile int c_cnt = 0;

int main() {
    int i;
    int count = 0;
    char buf[8];

    /* the first part */
    ps();

    /* the second part */
    setschedclass(LINUXSCHED);
    prA = create(proc_a, 2000, 30, "proc A", 1, 'A');
    prB = create(proc_b, 2000, 60, "proc B", 1, 'B');
    prC = create(proc_c, 2000, 90, "proc C", 1, 'C');
    resume(prA);
    resume(prB);
    resume(prC);
    setschedclass(RANDOMSCHED);
    sleep(10);
    ps();
    kill(prA);
    kill(prB);
    kill(prC);

    double total_cnt;
    double a_percent, b_percent, c_percent;
    total_cnt = a_cnt + b_cnt + c_cnt;
    a_percent = (double) a_cnt / total_cnt * 100;
    b_percent = (double) b_cnt / total_cnt * 100;
    c_percent = (double) c_cnt / total_cnt * 100;

    kprintf("\nTest RESULT: A = %d, B = %d, C = %d, (%d : %d : %d)\n",
                a_cnt, b_cnt, c_cnt, (int) a_percent, (int) b_percent,
                (int) c_percent);


    /* the third part */
    setschedclass(LINUXSCHED);
    resume(prA = create(proc, 2000, 5, "proc A", 1, 'A'));
    resume(prB = create(proc, 2000, 50, "proc B", 1, 'B'));
    resume(prC = create(proc, 2000, 90, "proc C", 1, 'C'));    
    sleep100(1);    
    setschedclass(RANDOMSCHED);
    suspend(prA);
    while (count++ < LOOP) {
        if (5 == count) {
        }
        kprintf("M");

        for (i = 0; i < 10000000; i++);
    }
    ps();
    sleep(33);
    resume(prA);
}

proc_basic(char c) {
    int count = 0;

    //kprintf("\nStart %c...\n", c);
    while (count++ < 1000) {
        kprintf("%c", c);
    }
}


proc(char c) {
    int i;
    int count = 0;

    while (count++ < LOOP) {
        if (prA == getpid()) {
            if (3 == count) {
                //DTRACE("\nA changed to 90 from %d\n", getprio(prA));
                //chprio(prA, 90);
           } else if (5 == count) {
                //DTRACE("\nA suspends B\n");
                //suspend(prB);
           } else if (6 == count) {
               //DTRACE("\nA goes to sleep\n");
               //sleep1000(9000);
           }
        } else if (prB == getpid()) {
            if (1 == count) {
                sleep(99);
                //DTRACE("\nB changed to 50 from %d\n", getprio(prB));
                //chprio(prB, 50);
            } else if (3 == count) {
                //DTRACE("\nC changed to 60 from %d\n", getprio(prB));
                //chprio(prB, 60);
            }
        } else if (prC == getpid()) { 
            if (3 == count) {
                //DTRACE("\nC changed to 10 from %d\n", getprio(prC));
                //chprio(prC, 10);

                //DTRACE("\nC suspends B\n");
                //suspend(prB);
            } else if (45 == count) {
                //DTRACE("\nC resumed B\n");
                //resume(prB);

                //DTRACE("\nC unsleeps A\n");
                //unsleep(prA);
                //ready(prA, RESCHYES);
            }
        }
        kprintf("%c", c);
        for (i = 0; i < 10000000; i++);
    }
    ps();
}

proc_a(c)
    char c; {
    int i;
    kprintf("Start... %c\n", c);
    b_cnt = 0;
    c_cnt = 0;

    while (1) {
        for (i = 0; i < 10000; i++)
            ;
        a_cnt++;
    }
}

proc_b(c)
    char c; {
    int i;
    a_cnt = 0;
    c_cnt = 0;

    kprintf("Start... %c\n", c);
    while (1) {
        for (i = 0; i < 10000; i++)
            ;
        b_cnt++;
    }
}

proc_c(c)
    char c; {
    int i;
    a_cnt = 0;
    b_cnt = 0;

    kprintf("Start... %c\n", c);
    while (1) {
        for (i = 0; i < 10000; i++)
            ;
        c_cnt++;
    }
}


