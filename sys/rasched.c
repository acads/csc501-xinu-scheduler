/* PA1 -- rasched.c -- implements random scheduler */

#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lab1.h>

int         sched_algo; /* shceduling algorithm to use */
int         prev_sched_algo; /* curr scheduling algorithm being used */
int         qhead;      /* head of the random scheduling queues */
int         qtail;      /* tail of the random scheduling queues */
struct qent qh[NPQENT]; /* high priority queue: 99 thru 66 */
struct qent qm[NPQENT]; /* medium prority queue: 65 thru 33 */
struct qent ql[NPQENT]; /* low priority queue: 32 thru 0 */


/* Initializes the given random scheduler queue. */
void
initq_pa1(struct qent *q)
{
    uint32_t pid = 0;

    qhead = NPROC;
    qtail = NPROC + 1;
    q[qhead].qnext = qtail;
    q[qhead].qprev = EMPTY_PA1;
    q[qtail].qnext = EMPTY_PA1;
    q[qtail].qprev = qhead;

    for (pid = 0; pid < NPROC; ++pid) {
        q[pid].qprev = EMPTY_PA1;
        q[pid].qnext = EMPTY_PA1;
    }
}


/* Initializes all the three queues of random scheduler. */
void
init_queues_pa1(void)
{
    initq_pa1(qh);
    initq_pa1(qm);
    initq_pa1(ql);

    return;
}

/* Prints the contents of the given RS queue. */
void
printq_pa1(struct qent *q)
{
    int next = qhead;

    if (IS_Q_EMPTY(q)) {
        DTRACE("Empty\n");
        return;
    }

    while (q[q[next].qnext].qnext != EMPTY) {
        DTRACE("%d ", q[next].qnext);
        next = q[next].qnext;
    }
    DTRACE("\n");
}


/* Prints the contents of all RS queues. */
void
print_queue_pa1(void)
{
    DTRACE("\n");
    DTRACE("HQueue Details: ");
    printq_pa1(qh);

    DTRACE("MQueue Details: ");
    printq_pa1(qm);

    DTRACE("LQueue Details: ");
    printq_pa1(ql);
    DTRACE("\n");

    return;
}


/* Uses the previously used scheduler's ready queue to build 
 * RS ready queues. 
 */
void
rs_build_queue(void)
{
    int pid = 0;
    int head = 0;
    STATWORD ps;

    disable(ps);
    DTRACE_START;

    switch (prev_sched_algo) {
        case XINUSCHED:
            xs_print_queue();
            pid = q[rdytail].qprev;
            while (pid < NPROC) {
                enqueue_pid_pa1(pid);
                pid = q[pid].qprev;
            }
            xs_init_queue();
            break;

        case LINUXSCHED:
            for (pid = ls_rq[ls_rtail].qprev, head = ls_rhead; pid != head;
                    pid = ls_rq[pid].qprev) {
                enqueue_pid_pa1(pid);
            }
            ls_init_queue(ls_rq, ls_rhead, ls_rtail);
            break;

        default:
            break;
    }

    DTRACE_END;
    restore(ps);
    return;
}


/* Specifies which scheduler to be used. */
void
setschedclass(int sched_class)
{
    DTRACE_START;

    if (sched_algo == sched_class) {
        DTRACE("DBG_INFO: %d %s: current & requested scheduler are the same\n", currpid, __func__);
        DTRACE_END;
        return;
    }

    prev_sched_algo = sched_algo;
    if ((RANDOMSCHED != sched_class) && (LINUXSCHED != sched_class)) {
        sched_algo = XINUSCHED;
    } else {
        sched_algo = sched_class;
    }
    DTRACE("DBG_INFO: %d %s: sched_class set to %d\n", currpid, __func__, sched_algo);

    switch(sched_algo)
    {
        case RANDOMSCHED:
            rs_build_queue();
            break;

        case LINUXSCHED:
            ls_build_queue();
            break;

        default:
            xs_build_queue();
            break;
    }

    /* Use the new scheduler to schedule procs now. */
    resched();

    DTRACE_END;
    return;
}


/* Returns the current scheduler in use. */
int
getschedclass(void)
{
    DTRACE_START;
    DTRACE_END;
    return sched_algo;
}


/* Enqueues the given pid in the given RS queue. */
int
enqueue_pa1(struct qent *q, int pid)
{
    struct qent *tptr;
    struct qent *mptr;

    /* Never enqueue the nullproc. */
    if (NULLPROC == pid) {
        return pid;
    }

    tptr = &q[qtail];
    mptr = &q[pid];
    mptr->qnext = qtail;
    mptr->qprev = tptr->qprev;
    q[tptr->qprev].qnext = pid;
    tptr->qprev = pid;

    return pid;
}


/* Enqueues the given pid in the app. RS queue. */
int
enqueue_pid_pa1(int pid)
{
    int tmp = 0;
    int pprio = 0;

    /* Never enqueue the nullproc. */
    if (NULLPROC == pid) {
        DTRACE("DBG_INFO: %s don't enqueue nullproc\n", __func__);
        return pid;
    }

    pprio = proctab[pid].pprio;
    if (IS_HIGH_PRIO(pprio)) {
        tmp = (enqueue_pa1(qh, pid));
        DTRACE("DBG_INFO: %s enq %d in qh\n", __func__, pid);
        return tmp;
    } else if (IS_MED_PRIO(pprio)) {
        tmp = (enqueue_pa1(qm, pid));
        DTRACE("DBG_INFO: %s enq %d in qm\n", __func__, pid);
        return tmp;
    } else {
        tmp = (enqueue_pa1(ql, pid));
        DTRACE("DBG_INFO: %s enq %d in ql\n", __func__, pid);
        return tmp;
    }
}


/* Dequeues a proc from the tail of the given RS queue. */
int
dequeue_pa1(struct qent *q)
{
    struct qent *mptr;
    int tmp;

    if (IS_Q_EMPTY(q)) {
        DTRACE("DBG_INFO: %s returns -1\n", __func__);
        return EMPTY_PA1;
    }

    tmp = q[qhead].qnext;
    mptr = &q[q[qhead].qnext];
    q[mptr->qprev].qnext = mptr->qnext;
    q[mptr->qnext].qprev = mptr->qprev;

    /* Reset the next & prev pointers of the item being dq'd */
    q[tmp].qnext = EMPTY_PA1;
    q[tmp].qprev = EMPTY_PA1;
    return tmp;
}


/* Dequeues the given pid from the app. RS queue. */
int
dequeue_pid_pa1(int pid)
{
    int ret_pid = EMPTY_PA1;
    DTRACE_START;

    /* Xinu scheduler does a dequeue on a pid. We need to search for the given
     * pid on all 3 queues and then dequeue it.
     */
    if (PID_IN_QUEUE(qh, pid)) {
        DTRACE("DBG_INFO: %s looking for pid %d in qh\n", __func__, pid);
        ret_pid = dequeue_pa1(qh);
        DTRACE("DBG_INFO: Deq %d from qh\n", ret_pid);
    } else if (PID_IN_QUEUE(qm, pid)) {
        DTRACE("DBG_INFO: %s looking for pid %d in qm\n", __func__, pid);
        ret_pid = dequeue_pa1(qm);
        DTRACE("ret suc\n");
        DTRACE("DBG_INFO: Deq %d from qm\n", ret_pid);
    } else if (PID_IN_QUEUE(ql, pid)) {
        DTRACE("DBG_INFO: %s looking for pid %d in ql\n", __func__, pid);
        ret_pid = dequeue_pa1(ql);
        DTRACE("ret suc\n");
        DTRACE("DBG_INFO: Deq %d from ql\n", ret_pid);
    } 
    DTRACE_END;
    return ret_pid;
}


/* Removes the given pid (if present) from the specified RS ready queue. */
int
rs_remove_pid_in_q(int pid, struct qent *qptr)
{
    DTRACE_START;
    if (FALSE == PID_IN_QUEUE(qptr, pid)) {
        DTRACE("DBG_INFO: %d %s: pid %d not present in given queue\n", currpid, __func__, pid);
        DTRACE_END;
        return EMPTY_PA1;
    }

    if ((EMPTY_PA1 != qptr[pid].qnext) && 
            (EMPTY_PA1 != qptr[pid].qprev)) {
        qptr[qptr[pid].qprev].qnext = qptr[pid].qnext;
        qptr[qptr[pid].qnext].qprev = qptr[pid].qprev;
        qptr[pid].qnext = qptr[pid].qprev = EMPTY_PA1;
        DTRACE("DBG_INFO: %d %s: pid %d removed from the given queue\n", currpid, __func__, pid);
        DTRACE_END;
        return pid;
    }
    DTRACE("DBG_INFO: %d %s: pid %d not present in given queue\n", currpid, __func__, pid);
    DTRACE_END;
    return EMPTY_PA1;
}

/* Removes the given pid from the app. RS ready queue, if pid is present. This
 * will be usually used to remove a suspended/terminated proc from the ready
 * queue.
 */
int
rs_remove_pid(int pid)
{
    int ret_pid = EMPTY_PA1;
    int pprio;
    DTRACE_START;

    pprio =  LS_GET_PPRIO(pid);
    if (IS_HIGH_PRIO(pprio)) {
        ret_pid = rs_remove_pid_in_q(pid, qh);
    } else if (IS_MED_PRIO(pprio)) {
        ret_pid = rs_remove_pid_in_q(pid, qm);
    } else {
        ret_pid = rs_remove_pid_in_q(pid, ql);
    }

    DTRACE_END;
    return ret_pid;
}

void
rasched_pa1(int pid)
{
    int pprio;

    pprio = proctab[pid].pprio;
    if ((pprio >= 0) && (pprio <= 32)) {
        pid = enqueue_pa1(ql, pid); 
    } else if ((pprio >= 33) && (pprio <= 62)) {
        pid = enqueue_pa1(qm, pid); 
    } else {
        pid = enqueue_pa1(qh, pid); 
    }

    return;
}


/* Returns TRUE iff all the 3 queues of RS are empty. */
int
is_rsched_queues_empty(void)
{
    if ((IS_Q_EMPTY(qh)) && (IS_Q_EMPTY(qm)) && (IS_Q_EMPTY(ql))) {
        return TRUE;
    }
    return FALSE;
}


/* Implements RS. */
struct pentry* 
rand_sched(struct pentry *optr)
{
    struct pentry *nptr;
    int pid = 0;
    uint32_t rand_num = 0;

    if (TRUE == is_rsched_queues_empty()) {
        if (PRCURR == optr->pstate) {
            /* No other process is eligible to run. Schedule
             * the current process again.
             */
            DTRACE("DBG_INFO: %s currproc %d is good to go\n", __func__, optr->pid);
            return NULL;
        } else {
            /* No processes to run at all. Run nullproc. */
            DTRACE("DBG_INFO: %s run nullproc\n", __func__);
            nptr = &proctab[NULLPROC];
            return nptr;
        }
    }

    while (1) {
        rand_num = rand() % 99;
        if (RAND_FAVORS_HIGH(rand_num)) {
            DTRACE("DBG_INFO: %s looking at qh\n", __func__);
            pid = dequeue_pa1(qh);
            DTRACE("DBG_INFO: %s deq %d from qh\n",  __func__, pid);
        } else if (RAND_FAVORS_MED(rand_num)) {
            DTRACE("DBG_INFO: %s looking at qm\n", __func__);
            pid = dequeue_pa1(qm);
            DTRACE("DBG_INFO: %s deq %d from qm\n", __func__, pid);
        } else {
            DTRACE("DBG_INFO: %s looking at ql\n", __func__);
            pid = dequeue_pa1(ql);
            DTRACE("DBG_INFO: %s deq %d from ql\n", __func__, pid);
        }
        if (EMPTY_PA1 != pid) {
            break;
        }
        DTRACE("DBG_INFO: %s selected empty proc.. trying again\n", __func__);
    }
    DTRACE("DBG_INFO: %s selected %d\n", __func__, pid);
    nptr = &proctab[pid];
    return nptr;
}


/* Uses the previous scheduler's ready queue to build Xinu scheduler's
 * ready queue.
 */
void
xs_build_queue(void)
{
    uint32_t i = 0;
    int head = 0;
    int pid = 0;
    struct qent *qptr = NULL;;
    struct qent *rq[3] = {qh, qm, ql};
    STATWORD ps;

    disable(ps);
    DTRACE_START;
    switch (prev_sched_algo) {
        case RANDOMSCHED:
            for (i = 0; i < 3; ++i) {
                qptr = rq[i];
                for (pid = 0; ((pid < NPROC) && (PID_IN_QUEUE(qptr, pid))); 
                        ++pid) {
                    insert(pid, rdyhead, LS_GET_PPRIO(pid));
                }
            }
            init_queues_pa1();
            break;

        case LINUXSCHED:
            insert(NULLPROC, rdyhead, LS_GET_PPRIO(NULLPROC));
            for (pid = ls_rq[ls_rtail].qprev, head = ls_rhead; pid != head;
                    pid = ls_rq[pid].qprev) {
                    insert(pid, rdyhead, LS_GET_PPRIO(pid));
            }
            xs_print_queue();
            ls_init_queue(ls_rq, ls_rhead, ls_rtail);
            break;

        default:
            break;
    }

    DTRACE_END;
    restore(ps);
    return;
}


/* Prints the contents of Xinu scheduler's ready queue. */
void
xs_print_queue(void)
{
    int pid;
    DTRACE_START;

    DTRACE("DBG_INFO: %d %s: ---XS rq info start---\n", currpid, __func__);
    DTRACE("DBG_INFO: %d %s: pid pname pstate pquant pctr pgood\n", currpid, __func__);
    for (pid = q[rdytail].qprev; pid < NPROC; pid = q[pid].qprev) { 
        DTRACE("DBG_INFO: %d %s: %d %s %s %d %d %d\n", currpid, __func__, proctab    [pid].pid, proctab[pid].pname, ch_pstates[LS_GET_PSTATE(pid) - 1], LS_GET_PQUANT(pid), LS_GET_PCTR(pid), LS_GET_PGOOD(pid));
    }
    DTRACE("DBG_INFO: %d %s: ---XS rq info end---\n", currpid, __func__);
    DTRACE_END;
    return;
}


/* Initializes Xinu scheduler's ready queue. */
void
xs_init_queue(void)
{
    rdyhead = NPROC;
    rdytail = NPROC + 1;

    q[rdyhead].qprev = EMPTY;
    q[rdyhead].qnext = rdytail;
    q[rdyhead].qkey = MININT;
    q[rdytail].qprev = rdyhead;
    q[rdytail].qnext = EMPTY;
    q[rdytail].qkey = MAXINT;

    return;
}

/* Implements Xinu scheduler. */
struct pentry*
xinu_sched(struct pentry *optr)
{
    struct pentry *nptr;

    if ((PRCURR == optr->pstate) &&
            (lastkey(rdytail) < optr->pprio)) {
        return NULL_PA1;
    }

    /* Remove highest priority process at end of ready list. */
    nptr = &proctab[getlast(rdytail)];
    return nptr;
}

