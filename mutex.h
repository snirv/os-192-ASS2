//
// Created by snirv@wincs.cs.bgu.ac.il on 4/30/19.
//


enum mutexstate { M_AVAILABLE , M_BUSY }; ///3.1 mutex

// Long-term locks for processes
struct kthread_mutex_t {
    uint locked;       // Is the lock held?
    struct spinlock lk; // spinlock protecting this sleep lock
    enum mutexstate state;
    int mid; //mutex_id
    struct thread* mutex_thread[NTHREAD];
    int locked_thread_id;
    // For debugging:
    int tid;           // thread holding lock
};

