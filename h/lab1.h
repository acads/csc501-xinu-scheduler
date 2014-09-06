#ifndef _LAB1_H_
#define _LAB1_H_

/* General stuff */
#define uint32_t        unsigned int
#define XINUSCHED       0   /* default Xinu scheduling algorithm */
#define RANDOMSCHED     1   /* random scheduling algorithm */
#define LINUXSCHED      2   /* Linux-like scheduling algorithm */
//#if 0
#define DTRACE          //kprintf
#define DTRACE_START    //kprintf
#define DTRACE_END      //kprintf
//#endif
#if 0
#define DTRACE          kprintf
#define DTRACE_START    kprintf("DBG_INFO: %d %s: start\n", currpid, __func__)
#define DTRACE_END      kprintf("DBG_INFO: %d %s: end\n", currpid, __func__)
#endif
#define DTRACE1         kprintf

extern void xs_print_queue(void);

/* PA1, task 1 - Implementation of ps-like command */
extern int      preempt;
extern unsigned long ctr1000;
extern const char *ch_pstates[];

typedef enum { 
    EN_PS_PNAME = 0,
    EN_PS_PPID,
    EN_PS_PPRIO,
    EN_PS_PSTATE,
    EN_PS_PTICKS,
    EN_PS_PSTKSIZE
} EN_PS_FIELDS; 

extern void ps(void);   /* list active processes */


/* PA1, task 1 - Implementation of random scheduling algorithm */
extern int      sched_algo;
extern int      prev_sched_algo;
extern struct   qent qh[];
extern struct   qent qm[];
extern struct   qent ql[];
extern int      qhead;
extern int      qtail;


#define NPQENT          NPROC + 2
#define EMPTY_PA1       -1
#define NULL_PA1        0
#define PA1_TICKS       preempt


#define IS_BAD_PRIO(prio)   ((prio < 0) || (prio > 99))    
#define IS_Q_EMPTY(Q)       (Q[qhead].qnext == qtail)
#define IS_HIGH_PRIO(PRIO)  ((PRIO <= 99) && (PRIO >= 66))
#define IS_MED_PRIO(PRIO)   ((PRIO <= 65) && (PRIO >= 33))
#define IS_LOW_PRIO(PRIO)   ((PRIO <= 32) && (PRIO >= 0))
#define RAND_FAVORS_HIGH(N) ((N <= 49) && (N >= 0))
#define RAND_FAVORS_MED(N)  ((N <= 79) && (N >= 50))
#define RAND_FAVORS_LOW(N)  ((N <= 99) && (N >= 80))
#define PID_IN_QUEUE(Q, PID)    ((Q[PID].qprev != EMPTY_PA1) && \
                                 (Q[PID].qnext != EMPTY_PA1))

extern void 
    setschedclass(int sched_class); /* set scheduling algo   */
extern int 
    getschedclass(void);            /* get scheduling algo   */

/* Random scheduler */
extern void 
    print_queue_pa1(void);
extern void 
    printq_pa1(struct qent *q);
extern void 
    init_queues_pa1(void);
extern void 
    initq_pa1(struct qent *q);
extern int  
    dequeue_pa1(struct qent *q);
extern int
    dequeue_pid_pa1(int pid);
extern int
    enqueue_pa1(struct qent *q, int pid);
extern int
    enqueue_pid_pa1(int pid);
extern void 
    rasched_pa1(int pid);
extern struct pentry* 
    rand_sched(struct pentry *optr);
extern int 
    is_rsched_queues_empty(void);
extern int
    rs_remove_pid(int pid);
extern int
    rs_remove_pid_in_q(int pid, struct qent *qptr);

/* Xinu scheduler */
extern void
    xs_build_queue(void);
extern struct pentry* 
    xinu_sched(struct pentry *optr);
extern void
    xs_init_queue(void);
extern void
    rs_build_queue(void);

/* PA1, task 2: Linux-like scheduling algorithm */
#define LS_MIN_PGOOD    -1
#define LS_MAX_PGOOD    MAXINT
#define LS_IS_Q_EMPTY(Q, HEAD, TAIL) (Q[HEAD].qnext == TAIL)
#define LS_ANY_RPROCS() (0 == ls_nready_proc)
#define LS_IS_VALID_PGOOD(PGOOD) (EMPTY_PA1 != PGOOD)
#define LS_GET_PID(PID) (proctab[PID].pid)
#define LS_GET_PNAME(PID) (proctab[PID].pname)
#define LS_GET_PPTR(PID) (&proctab[PID])
#define LS_GET_PPRIO(PID) (proctab[PID].pprio)
#define LS_GET_PGOOD(PID) (proctab[PID].pgood)
#define LS_GET_PQUANT(PID) (proctab[PID].pquant)
#define LS_GET_PSTATE(PID) (proctab[PID].pstate)
#define LS_GET_PCTR(PID) (proctab[PID].pctr)
#define LS_IS_ACTIVE_EPOCH() ls_active_epoch

extern int ls_change_pstate;
extern int ls_rhead;
extern int ls_rtail;
extern struct qent ls_rq[];

extern void 
    ls_print_queue(struct qent *qptr, int head, int tail);
extern void 
    ls_init_queue(struct qent *qptr, int head, int tail);
extern void 
    ls_reorder_queue(struct qent *qptr, int head, int tail);
extern int 
    ls_enqueue_pid(struct qent *q, int head, int tail, int pid, int pgood);
extern int 
    ls_remove_pid(int pid);
extern int 
    ls_dequeue_pid(struct qent *q, int head, int tail);
extern void 
    ls_build_queue(void);
extern void 
    ls_calc_pquant_pgood(struct qent *qptr, int head, int tail);
extern void 
    ls_calc_pgood(struct qent *qptr, int head, int tail);
extern void 
    ls_admit_new_procs(struct qent *qptr, int head, int tail);
extern int 
    ls_is_rqueue_empty(void);
extern void 
    ls_process_new_epoch(void);
extern struct pentry* 
    linux_sched(struct pentry *optr);

#endif /* _LAB1_H_ */
