//
// Created by snirv@wincs.cs.bgu.ac.il on 02/05/2019
//
#include "types.h"
#include "user.h"
#define CREATE_THREAD_NUM 3
#define JOIN_TEST_THREAD_NUM 4
#define STACK_SIZE 500
#include "stat.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"
#include "tournament_tree.h"
#include "kthread.h"



    int create_exit_thread_test() {
        int kthreadCreateFlag = 0;

        void *threadStack_1 = ((char *) malloc(STACK_SIZE * sizeof(char))) + STACK_SIZE;
        void *threadStack_2 = ((char *) malloc(STACK_SIZE * sizeof(char))) + STACK_SIZE;
        void *threadStack_3 = ((char *) malloc(STACK_SIZE * sizeof(char))) + STACK_SIZE;


        void (*threads_stacks[])(void) =
                {threadStack_1,
                 threadStack_2,
                 threadStack_3};

        void thread_start_func_1() {
            sleep(10);
            kthread_exit();
        }
        void thread_start_func_2() {
            sleep(20);
            kthread_exit();
        }
        void thread_start_func_3() {
            sleep(30);
            kthread_exit();
        }

        void (*threads_starts_func[])(void) =
                {thread_start_func_1,
                 thread_start_func_2,
                 thread_start_func_3};

        for (int i = 0; i < CREATE_THREAD_NUM; i++) {
            kthreadCreateFlag = kthread_create(threads_starts_func[i], threads_stacks[i]);
            if (kthreadCreateFlag == -1) {
                return -1;
            }
        }
        return 0;
    }

    int join_test(){
        int pids[JOIN_TEST_THREAD_NUM];

        void *join_thread_stack_1 = ((char *) malloc(STACK_SIZE * sizeof(char))) + STACK_SIZE;
        void *join_thread_stack_2 = ((char *) malloc(STACK_SIZE * sizeof(char))) + STACK_SIZE;
        void *join_thread_stack_3 = ((char *) malloc(STACK_SIZE * sizeof(char))) + STACK_SIZE;
        void *join_thread_stack_4 = ((char *) malloc(STACK_SIZE * sizeof(char))) + STACK_SIZE;


        void (*join_threads_stacks[])(void) =
                {join_thread_stack_1,
                 join_thread_stack_2,
                 join_thread_stack_3,
                 join_thread_stack_4};

        void join_thread_start_func_1() {
            sleep(100);
            kthread_exit();
        }
        void join_thread_start_func_2() {
            sleep(200);
            kthread_exit();
        }
        void join_thread_start_func_3() {
            sleep(300);
            kthread_exit();
        }

        void join_thread_start_func_4() {
            sleep(400);
            kthread_exit();
        }

        void (*join_threads_starts_func[])(void) =
                {join_thread_start_func_1,
                 join_thread_start_func_2,
                 join_thread_start_func_3,
                 join_thread_start_func_4};

        for(int i = 0;i < JOIN_TEST_THREAD_NUM;i++){
            pids[i] = kthread_create(join_threads_starts_func[i], join_threads_stacks[i]);
            if(pids[i] == -1){
                return -1;
            }
        }

        for(int i = 0;i < JOIN_TEST_THREAD_NUM;i++) {
            int result = kthread_join(pids[i]);
            if (result == 0) {
                printf(1, "Finished joing thread %d\n", pids[i]);
            } else if (result == -1) {
                printf(1, "Error in joing thread %d\n", pids[i]);
                return -1;
            }
        }
        return 0;
    }


    int main(int argc, char *argv[]){
        if(create_exit_thread_test()== -1){
            printf(1,"thread creation failed");
        }

        if(join_test()== -1){
            printf(1,"thread creation failed");
        }




        exit();
    }





