#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "kthread.h"

#include "mutex.h"


//enum mutexstate { M_AVAILABLE , M_BUSY }; ///3.1 mutex
//
//// Long-term locks for processes
//struct kthread_mutex_t {
//    uint locked;       // Is the lock held?
//    struct spinlock lk; // spinlock protecting this sleep lock
//    enum mutexstate state;
//    int mid; //mutex_id
//    struct thread* mutex_thread[NTHREAD];
//    int locked_thread_id;
//    // For debugging:
//    int tid;           // thread holding lock
//};
//




struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

int nextpid = 1;
int nexttid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

static struct kthread_mutex_t mutex_arr[MAX_MUTEXES]; //3.1

//struct spinlock mutex_arr_lock;

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
int
cpuid() {
  return mycpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  int apicid, i;
  
  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");
  
  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc*
myproc(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}


//2.1 new
struct thread*
mythread(void) {
  struct cpu *c;
  struct thread *t;
  pushcli();
  c = mycpu();
  t = c->thread;
  popcli();
  return t;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
//  char *sp; 2.1

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == PUNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:
  p->state = PEMBRYO;
  p->pid = nextpid++;

  release(&ptable.lock);

  // Allocate kernel stack.
//  comment 2.1
//  if((p->kstack = kalloc()) == 0){
//    p->state = UNUSED;
//    return 0;
//  }
//  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
//  sp -= sizeof *p->tf;
//  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
//  sp -= 4;
//  *(uint*)sp = (uint)trapret;
//
//  sp -= sizeof *p->context;
//  p->context = (struct context*)sp;
//  memset(p->context, 0, sizeof *p->context);
//  p->context->eip = (uint)forkret;

  return p;
}



static struct thread*
alloctread(struct proc *p)
{
//  struct proc *p;
  char *sp;
  struct thread* t;

//  acquire(&ptable.lock); TODO
  int idx = 0;
  for (t = p->ttable; t < &p->ttable[NTHREAD]; t++) {
    if (t->state == TUNUSED) {
      goto found;
    }
    idx++;
  }
  return 0;

//  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
//    if(p->state == UNUSED)
//      goto found;

//  release(&ptable.lock);
//  return 0;

    found:
  t->state = TEMBRYO;
  t->tid = nexttid++;
  t->parent = p;

//  release(&ptable.lock);

  // Allocate kernel stack.
  if((t->kstack = kalloc()) == 0){
    t->state = TUNUSED;
    return 0;
  }
  sp = t->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *t->tf;
  t->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *t->context;
  t->context = (struct context*)sp;
  memset(t->context, 0, sizeof *t->context);
  t->context->eip = (uint)forkret;

  return t;
}




//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  struct thread* t;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();
  t = alloctread(p);
  
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(t->tf, 0, sizeof(*t->tf));
  t->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  t->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  t->tf->es = t->tf->ds;
  t->tf->ss = t->tf->ds;
  t->tf->eflags = FL_IF;
  t->tf->esp = PGSIZE;
  t->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);

//  p->state = RUNNABLE;
    p->state = PUSING;
    t->state = TRUNNABLE;

  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int  //todo need to protect
growproc(int n)
{
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->sz;
  if(n > 0){
    if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->sz = sz;
  switchuvm(curproc,mythread());
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();
  struct thread *curthread = mythread();
  struct thread *nt;

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy process state from proc.
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
//    kfree(np->kstack);
//    np->kstack = 0;
    np->state = PUNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;

  if((nt = alloctread(np)) == 0){
    return -1;
  }
//  *np->tf = *curproc->tf;
    *nt->tf = *curthread->tf;
  // Clear %eax so that fork returns 0 in the child.
//  np->tf->eax = 0;
     nt->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
//  np->cwd = idup(curproc->cwd);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;

  acquire(&ptable.lock);

//  np->state = RUNNABLE;
    np->state = PUSING;
  if(nt->state == TZOMBIE){
    panic("fork\n");
  }
    nt->state = TRUNNABLE;

  release(&ptable.lock);

  return pid;
}


void
freethread(struct thread* t){
  kfree(t->kstack);
  t->kstack = 0;
  t->state= TUNUSED;
  t->tid = 0;
    //t->parent = 0;

}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
//void
//exit(void)
//{
//  struct proc *curproc = myproc();
//  struct thread* curthread = mythread();
//  struct thread* t;
//  struct proc *p;
//  int fd;
//
//  if(curproc == initproc)
//    panic("init exiting");
//
//
//  curproc->killed = 1; // 2.1 added
//
//  acquire(&ptable.lock);
//
//
//
//
//  curthread->state = TZOMBIE; //2.1
//
//    // check if curthread is last thread to exit 2.1
//  for (t = curproc->ttable; t < &curproc->ttable[NTHREAD]; t++) {
//    if ((t != curthread ) && ( (t->state != TZOMBIE ) && (t->state != TUNUSED) )){
//        //cprintf("exit sched 1 thread state:%d\n",curthread->state);
//      sched();
//      panic("zombie exit");
//    }
//  }
//    if(curthread->state != TZOMBIE){
//        panic("1\n");
//    }
//
//  release(&ptable.lock);
//
//  // Clean proc
//  // Close all open files.
//  for(fd = 0; fd < NOFILE; fd++){
//    if(curproc->ofile[fd]){
//      fileclose(curproc->ofile[fd]);
//      curproc->ofile[fd] = 0;
//    }
//  }
//
//  begin_op();
//  iput(curproc->cwd);
//  end_op();
//  curproc->cwd = 0;
//
//    if(curthread->state != TZOMBIE){
//        panic("3\n");
//    }
//
//  acquire(&ptable.lock);
//
//    // Parent might be sleeping in wait().
//    wakeup1(curproc->parent);
//
//    // Pass abandoned children to init.
//    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
//        if(p->parent == curproc){
//            p->parent = initproc;
//            if(p->state == PZOMBIE)
//                wakeup1(initproc);
//        }
//    }
//
//  // Jump into the scheduler, never to return.
//  curproc->state = PZOMBIE;
//  //cprintf("exit sched 2 thread state:%d\n",curthread->state);
//  sched();
//  panic("zombie exit");
//}


void
proc_exit(void){
    struct proc *curproc = myproc();
    struct proc *p;
    int fd;
//    acquire(&ptable.lock);
//    int cpu_id = mycpu()->apicid;
//    release(&ptable.lock);
    if(curproc == initproc)
        panic("init exiting");


    // Clean proc
    // Close all open files.
    for(fd = 0; fd < NOFILE; fd++){
        if(curproc->ofile[fd]){
            fileclose(curproc->ofile[fd]);
            curproc->ofile[fd] = 0;
        }
    }

//    cprintf("state: %d cpu id: %d\n",mythread()->state, cpu_id);
    begin_op();
    iput(curproc->cwd);
    end_op();
    curproc->cwd = 0;
//    cprintf("state: %d cpu id: %d\n",mythread()->state,cpu_id);


    acquire(&ptable.lock);

    // Parent might be sleeping in wait().
    wakeup1(curproc->parent);

    // Pass abandoned children to init.
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
        if(p->parent == curproc){
            p->parent = initproc;
            if(p->state == PZOMBIE)
                wakeup1(initproc);
        }
    }
//    cprintf("proc exit:\n");
//    cprintf("cpu id %d\n" , mycpu()->apicid);
//    cprintf("proc id %d\n" , curproc->pid);
//    cprintf("thread id %d\n" , mythread()->tid);
    // Jump into the scheduler, never to return.
    curproc->state = PZOMBIE;
    mythread()->state = TZOMBIE;
    sched();
    panic("zombie exit");
}

void
kthread_exit(void){

    struct proc *curproc = myproc();
    struct thread* curthread = mythread();
    struct thread* t;
//    cprintf("thread id %d killed %d\n",curthread->tid , curthread->killed);
    acquire(&ptable.lock);

    curthread->state = TZOMBIE; //2.1

    wakeup1(curthread);//2.2 added for join
//    cprintf("thread exit:\n");
//    cprintf("cpu id %d\n" , mycpu()->apicid);
//    cprintf("proc id %d\n" , curproc->pid);
//    cprintf("thread id %d\n" , curthread->tid);

    // check if cur thread is last thread to exit 2.1
    for (t = curproc->ttable; t < &curproc->ttable[NTHREAD]; t++) {
        if ((t != curthread ) && ( (t->state != TZOMBIE ) && (t->state != TUNUSED) )){
            sched();
            panic("zombie exit");
        }
    }

    release(&ptable.lock);
   // 2.1 all threads finished exit
   proc_exit();
}








void
exit(void)
{
    struct proc *curproc = myproc();
    curproc->killed = 1; // 2.1 added
    kthread_exit();
    panic("zombie exit");
}


// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
//  struct thread* curthread = mythread();
  struct thread* t;


  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;
//        cprintf("p state is: %d\n",p->state);
      if(p->state == PZOMBIE){
        // Found one.
        pid = p->pid;
        //assume all the thread's states are zombie/unused after exit // 2.1 added
        for (t = p->ttable; t < &p->ttable[NTHREAD]; t++) {
          if(t->state != TZOMBIE && t->state != TUNUSED){
            panic("zombie proc with non zombie/Unused thread");
          }
            if(t->state == TZOMBIE){
                freethread(t);
            }

        }
//        kfree(p->kstack);
//        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = PUNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
//       cprintf("sleep proc name %s:\n",curproc->name);
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep

  }
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  struct thread* t;
  c->proc = 0;
  
  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != PUSING)
        continue;

      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.

      for (t = p->ttable; t < &p->ttable[NTHREAD]; t++) {
        if (t->state != TRUNNABLE){
//            if(t->tid ==4 )cprintf("name: %s , state %d thread id:%d\n",p->name,t->state,t->tid);
          continue;
        }
//        cprintf("name: %s , state %d thread id:%d\n",p->name,p->state,t->tid);
        c->thread = t;
        c->proc = p;
        switchuvm(p,t);
        //p->state = RUNNING;
        t->state = TRUNNING;

        swtch(&(c->scheduler), t->context);
        switchkvm();

        // Process is done running for now.
        // It should have changed its p->state before coming back.
        c->proc = 0;
        c->thread = 0;
      }
    }
    release(&ptable.lock);

  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  //struct proc *p = myproc();
  struct thread *t = mythread();

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(t->state == TRUNNING) {
    panic("sched running");
  }
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&t->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
//  myproc()->state = RUNNABLE;
  if(mythread()->state == TZOMBIE){
    panic("yield");
  }
  mythread()->state = TRUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  //struct proc *p = myproc();

  struct thread* t = mythread();

  if(t == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
//  p->chan = chan;
//  p->state = SLEEPING;

  t->chan = chan;
  t->state = TSLEEPING;


  sched();

  // Tidy up.
//  p->chan = 0;

  t->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;
  struct thread* t;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    //if (p->state == SLEEPING && p->chan == chan)
    if (p->state == PUSING){
      for (t = p->ttable; t < &p->ttable[NTHREAD]; t++) {
        if (t->state == TSLEEPING && t->chan == chan){
          t->state = TRUNNABLE;
        }

      }
    }
//      p->state = RUNNABLE;
  }
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;
//  struct thread* t;
  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
//      for (t = p->ttable; t < &p->ttable[NTHREAD]; t++) { //TODO see if necessary
//        t->killed = 1;
//      }
      // Wake process from sleep if necessary.
//      if(p->state == SLEEPING) // 2.1
//        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [PUNUSED]    "proc_unused",
  [PEMBRYO]    "proc_embryo",
  //[SLEEPING]  "sleep ",
  [PUSING]  "proc_using",
  //[RUNNING]   "run   ",
  [PZOMBIE]    "proc_zombie"
  };
//  int i;
  struct proc *p;
  char *state;
//  uint pc[10];

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == PUNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
//    if(p->state == SLEEPING){
//      getcallerpcs((uint*)p->context->ebp+2, pc);
//      for(i=0; i<10 && pc[i] != 0; i++)
//        cprintf(" %p", pc[i]);
//    }
    cprintf("\n");
  }
}

int
kthread_join(int tid) { //added 2.2
    struct proc *curproc = myproc();
    struct thread* curthread = mythread();
    struct thread* t;
    if(curthread->tid == tid){
        cprintf("thread run join for itself!!");
        return -1;
    }

    acquire(&ptable.lock);

    for (t = curproc->ttable; t < &curproc->ttable[NTHREAD]; t++) {
        if(t == curthread || t->tid != tid){
            continue;
        }
        if (t->state == TUNUSED){
            release(&ptable.lock);
            return -1;
        }
        if (t->state == TZOMBIE){
            freethread(t); //TODO what we should do if 2 threads are joining for the same thread
            release(&ptable.lock);
            return 0;
        }
        else {
            sleep(t, &ptable.lock);
            return 0;
        }
    }

    release(&ptable.lock);
    return -1;

}


int
kthread_create(void (*start_func)(void), void* stack){
//    cprintf("start func!!!!!!!!!!!!!!!!!!111 %x \n", start_func);
//    cprintf("stack !!!!!!!!!!!!!!!!!!!!!!1 %x \n", stack);
    struct proc *curproc = myproc();
    struct thread* t;
    acquire(&ptable.lock);

    t = alloctread(curproc);
//    cprintf("proc name %s \n", curproc->name);
//    cprintf("thread id %d \n", t->tid);


    if (t == null){
        release(&ptable.lock);
        return -1;
    }
    t->state = TRUNNABLE;
    *t->tf = *mythread()->tf;
    t->tf->eip = (uint)start_func;
    t->tf->esp = (uint)stack;
    release(&ptable.lock);
    return t->tid;


}


int
kthread_mutex_alloc(){
//    acquire(&mutex_arr_lock);
    struct kthread_mutex_t* m ;
    int idx = 0;
    for (m = mutex_arr; m < &mutex_arr[MAX_MUTEXES]; m++) {
        acquire(&m->lk);
        if (m->state == M_AVAILABLE){
            m->state = M_BUSY;
            m->mid = idx;
            release(&m->lk);
//            release(&mutex_arr_lock);
            return m->mid;
        }
        idx++;
        release(&m->lk);
    }
//    release(&mutex_arr_lock);
    return -1;
}



int
kthread_mutex_dealloc(int mutex_id){

    struct kthread_mutex_t* m ;

//    acquire(&mutex_arr_lock);
    for (m = mutex_arr; m < &mutex_arr[MAX_MUTEXES]; m++) {
        if (m->mid == mutex_id){
            acquire(&m->lk);
            if (m->state == M_AVAILABLE){
                release(&m->lk);
//                release(&mutex_arr_lock);
                return 0;
            }
            else if (m->state == M_BUSY){
//                acquire(&m->lk);
                if(!m->locked){
                    m->state = M_AVAILABLE;
                    release(&m->lk);
//                    release(&mutex_arr_lock);
                    return 0;

                }
                else{
                    release(&m->lk);
//                    release(&mutex_arr_lock);
                    return -1;
                }
            }
            release(&m->lk);
        }
    }
//    release(&mutex_arr_lock);
    return -1;
}



int kthread_mutex_lock(int mutex_id){ //TODO
    struct kthread_mutex_t* m ;
//    acquire(&mutex_arr_lock);
    for (m = mutex_arr; m < &mutex_arr[MAX_MUTEXES]; m++) {
        acquire(&m->lk);
        if (m->mid == mutex_id && m->state ==  M_BUSY ) {
//            release(&mutex_arr_lock);
            while (m->locked) {
                sleep(m, &m->lk);
            }
            if(m->state == M_BUSY) {// check that didnt dealloc while sleeping
                m->locked = 1;
                m->tid = mythread()->tid;
                release(&m->lk);
                return 0;
            }
        }
        release(&m->lk);

    }
//    release(&mutex_arr_lock);
    return -1;
}




int kthread_mutex_unlock(int mutex_id){ //TODO
    struct kthread_mutex_t* m ;
//     acquire(&mutex_arr_lock);
    for (m = mutex_arr; m < &mutex_arr[MAX_MUTEXES]; m++) {
        acquire(&m->lk);
        if (m->mid == mutex_id && m->state ==  M_BUSY ) {
            m->locked = 0;
            m->tid = 0;
            wakeup(m);
            release(&m->lk);
            return 0;
        }
        release(&m->lk);
    }
//    release(&mutex_arr_lock);
    return -1;
}



//void
//acquiresleep(struct sleeplock *lk)
//{
//    acquire(&lk->lk);
//    while (lk->locked) {
//        sleep(lk, &lk->lk);
//    }
//    lk->locked = 1;
//    lk->pid = myproc()->pid;
//    release(&lk->lk);
//}
//
//void
//releasesleep(struct sleeplock *lk)
//{
//    acquire(&lk->lk);
//    lk->locked = 0;
//    lk->pid = 0;
//    wakeup(lk);
//    release(&lk->lk);
//}
