/* PA1 -- implementation of ps-like command */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lab1.h>

/* Table to store and shuffle the process pointers. */
struct pentry *pproctabs[NPROC];

/* Field names to be printed. */
const char *ch_ps_fields[] = {
    "Name",
    "PID",
    "Priority",
    "Status",
    "Ticks",
    "StackSize"
};

/* Human readable process states. */
const char *ch_pstates[] = {
    "PRCURR",
    "PRFREE",
    "PRREADY",
    "PRRECV",
    "PRSLEEP",
    "PRUSUP",
    "PRWAIT",
    "PRTRECV"
};


/* Store the pointers of the current proctab in an array.
 * The pointers would then be shuffled based on the priority of the
 * process so as to not to disturb the existing proctab.
 */
void
init_pproctabs()
{
    uint32_t i = 0;

    for (i = 0; i < NPROC; ++i) {
        pproctabs[i] = &proctab[i];
    }
    return;
}


/* Uses insertion sort to sort the table of proctab pointers based on the
 * priority of the process. Higher priority process followed by lower.
 */
void
psort()
{
    int i = 0;
    int j = 0;
    uint32_t pprio = 0;

    init_pproctabs();
    
    /* Sort the local process table based on decending priority using
     * insertion sort
     */
    for (j = 1; j < NPROC; ++j) {
        pprio = pproctabs[j]->pprio;i = j - 1;
        while ((i >= 0) && (pproctabs[i]->pprio < pprio)) {
            pproctabs[i + 1] = pproctabs[i];
            i = i - 1;
        }
        pproctabs[i + 1] = &proctab[j];
    }
}


/* Prints the process information of currently active processes. The info
 * includes the PID, name, priority, stack size and CPU ticks used.
 */
void
ps()
{
    STATWORD ps;
    uint32_t pid = 0;
    struct pentry *ptab = NULL;

    disable(ps);

    kprintf("\n");
    kprintf("%16s", ch_ps_fields[EN_PS_PNAME]);
    kprintf("%9s", ch_ps_fields[EN_PS_PPID]);
    kprintf("%10s", ch_ps_fields[EN_PS_PPRIO]);
    kprintf("%12s", ch_ps_fields[EN_PS_PSTATE]);
    kprintf("%10s", ch_ps_fields[EN_PS_PTICKS]);
    DTRACE("%6s", "Ins");
    DTRACE("%6s", "Outs");
    kprintf("%12s", ch_ps_fields[EN_PS_PSTKSIZE]);
    kprintf("\n");

    psort();
    for (pid = 0; pid < NPROC; ++pid) {
        ptab = pproctabs[pid];
        if (PRFREE == ptab->pstate) {
            continue;
        }
        if (PRCURR == ptab->pstate) {
            /* Save the time spent until this point. */
            if (LINUXSCHED == sched_algo) {
                if (NULLPROC == ptab->pid) {
                    ptab->pticks += (QUANTUM - PA1_TICKS);
                } else {
                    ptab->pticks += (ptab->pquant - PA1_TICKS);
                }
            } else {
                ptab->pticks += (QUANTUM - PA1_TICKS);
            }
        }
        kprintf("%16s", ptab->pname);
        kprintf("%9u", ptab->pid);
        kprintf("%10u", ptab->pprio);
        kprintf("%12s", ch_pstates[ptab->pstate - 1]);
        kprintf("%10u", ptab->pticks);
        DTRACE("%6u", ptab->pins);
        DTRACE("%6u", ptab->pouts);
        kprintf("%12u", ptab->pstklen);
        kprintf("\n");
    }
    kprintf("\n");

    restore(ps);
}

