//
// Created by snirv@wincs.cs.bgu.ac.il on 02/05/2019
//
#include "types.h"
#include "user.h"
#define CREATE_THREAD_NUM 3
#define JOIN_TEST_THREAD_NUM 4
#define MUTEX_TEST_THREAD_NUM 2
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

    int mutex_test(){
        int pids[MUTEX_TEST_THREAD_NUM];

        void *mutex_thread_stack_1 = ((char *) malloc(STACK_SIZE * sizeof(char))) + STACK_SIZE;
        void *mutex_thread_stack_2 = ((char *) malloc(STACK_SIZE * sizeof(char))) + STACK_SIZE;



        void (*mutex_threads_stacks[])(void) =
                {mutex_thread_stack_1,
                 mutex_thread_stack_2};

        
        void mutex_thread_start_func_1(){ 
        
        int mid; 
        int result; 
        mid = kthread_mutex_alloc(); 
        if(mid == -1){ 
            printf(1,"mutex allocated unsuccessfully\n"); 
        } 
        result = kthread_mutex_lock(mid); 
        if(result < 0){  
            printf(1,"mutex locked unsuccessfully\n"); 
        } 
        result = kthread_mutex_unlock(mid); 
        if(result < 0){ 
            printf(1,"mutex unlocked unsuccessfully\n"); 
        } 
        result = kthread_mutex_dealloc(mid); 
        if(result == 0){} 
        else if(result == -1){ 
            printf(1,"mutex deallocated unsuccessfully\n"); 
        } 
        else{ 
            printf(1,"unkown return code from mutex dealloc\n"); 
        } 
        kthread_exit(); 
        }   

        void mutex_thread_start_func_2(){ 
            int mid; 
            int result; 
            mid = kthread_mutex_alloc(); 
            if(mid == -1){ 
                printf(1,"mutex allocated unsuccessfully\n"); 
            } 
            result = kthread_mutex_lock(mid); 
            if(result < 0){  
                printf(1,"mutex locked unsuccessfully\n"); 
            } 
            result = kthread_mutex_unlock(mid); 
            if(result < 0){ 
                printf(1,"mutex unlocked unsuccessfully\n"); 
            } 
            result = kthread_mutex_dealloc(mid); 
            if(result == 0){} 
            else if(result == -1){ 
                printf(1,"mutex deallocated unsuccessfully\n"); 
            } 
            else{ 
                printf(1,"unkown return code from mutex dealloc\n"); 
            } 
            kthread_exit(); 
        }


        void (*mutex_thread_start_func[])(void) =
                {mutex_thread_start_func_1,
                 mutex_thread_start_func_2};

        for(int i = 0;i < MUTEX_TEST_THREAD_NUM;i++){
            pids[i] = kthread_create(mutex_thread_start_func[i], mutex_threads_stacks[i]);
        }


        for(int i = 0;i < MUTEX_TEST_THREAD_NUM;i++) {
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


    int tournament_test(){
        int result;
        trnmnt_tree* tree = trnmnt_tree_alloc(2); 
      if(tree == 0){ 
        printf(1,"2 trnmnt_tree allocated unsuccessfully\n"); 
        return -1;
    } 

    result = trnmnt_tree_acquire(tree, 0); 
    if(result < 0){  
        printf(1,"3 trnmnt_tree locked unsuccessfully\n");
        return -1; 
    } 

    result = trnmnt_tree_release(tree, 0); 
    if(result < 0){ 
        printf(1,"3 trnmnt_tree unlocked unsuccessfully\n"); 
         return -1;
    } 

    result = trnmnt_tree_acquire(tree, 1); 
    if(result < 0){  
        printf(1,"4 trnmnt_tree locked unsuccessfully\n"); 
         return -1;
    } 

    result = trnmnt_tree_release(tree, 1); 
    if(result < 0){ 
        printf(1,"4 trnmnt_tree unlocked unsuccessfully\n"); 
         return -1;
    } 

    result = trnmnt_tree_acquire(tree, 2); 
    if(result < 0){  
        printf(1,"5 trnmnt_tree locked unsuccessfully\n"); 
         return -1;
    } 

    result = trnmnt_tree_release(tree, 2); 
    if(result < 0){ 
        printf(1,"5 trnmnt_tree unlocked unsuccessfully\n"); 
         return -1;
    } 

    result = trnmnt_tree_acquire(tree, 3); 
    if(result < 0){  
        printf(1,"6 trnmnt_tree locked unsuccessfully\n"); 
         return -1;
    } 

    result = trnmnt_tree_release(tree, 3); 
    if(result < 0){ 
        printf(1,"6 trnmnt_tree unlocked unsuccessfully\n"); 
         return -1;
    } 

    result = trnmnt_tree_dealloc(tree); 
    if(result == 0){ 
        return 0;
        }
    else if(result == -1){ 
        printf(1,"12 trnmnt_tree deallocated unsuccessfully\n"); 
        return -1;
    } 
    else{ 
        printf(1,"2 unkown return code from trnmnt_tree_dealloc\n"); 
    } 

    }




    //  int tournament_test(){
    //         int result;
    //         trnmnt_tree* tree = trnmnt_tree_alloc(2); 
    //     if(tree == 0){ 
    //         printf(1,"2 trnmnt_tree allocated unsuccessfully\n"); 
    //         return -1;
    //         } 

    //     result = trnmnt_tree_acquire(tree, 0); 
    //     if(result < 0){  
    //         printf(1,"3 trnmnt_tree locked unsuccessfully\n");
    //         return -1; 
    //         } 

    //     result = trnmnt_tree_acquire(tree, 2); 
    //     if(result < 0){  
    //         printf(1,"5 trnmnt_tree locked unsuccessfully\n"); 
    //         return -1;
    //         } 

    //     result = trnmnt_tree_acquire(tree, 1); 
    //     if(result < 0){  
    //         printf(1,"4 trnmnt_tree locked unsuccessfully\n"); 
    //         return -1;
    //         } 

    //     result = trnmnt_tree_release(tree, 0); 
    //     if(result < 0){ 
    //         printf(1,"3 trnmnt_tree unlocked unsuccessfully\n"); 
    //         return -1;
    //         } 
        
    //     result = trnmnt_tree_acquire(tree, 3); 
    //     if(result < 0){  
    //         printf(1,"6 trnmnt_tree locked unsuccessfully\n"); 
    //         return -1;
    //         } 

    //     result = trnmnt_tree_release(tree, 1); 
    //     if(result < 0){ 
    //         printf(1,"4 trnmnt_tree unlocked unsuccessfully\n"); 
    //         return -1;
    //         } 


    //     result = trnmnt_tree_release(tree, 2); 
    //     if(result < 0){ 
    //         printf(1,"5 trnmnt_tree unlocked unsuccessfully\n"); 
    //         return -1;
    //         } 

    //     result = trnmnt_tree_release(tree, 3); 
    //     if(result < 0){ 
    //         printf(1,"6 trnmnt_tree unlocked unsuccessfully\n"); 
    //         return -1;
    //         } 

    //         result = trnmnt_tree_dealloc(tree); 
    //         if(result == 0){ 
    //         return 0;
    //         }
    //         else{ 
    //         printf(1,"12 trnmnt_tree deallocated unsuccessfully\n"); 
    //         return -1;
    //         } 
     
    //     return 0;
    // }




    int main(int argc, char *argv[]){
        if(create_exit_thread_test()== -1){
            printf(1,"thread creation failed\n");
        }
        printf(1,"thread creation test success\n");
        
        if(join_test()== -1){
            printf(1,"thread join failed\n");
        }
            printf(1,"thread join test success\n");
        
        if(mutex_test() == -1){
            printf(1,"mutex test failed\n");
        }
        printf(1,"mutex test suceess\n");
        
         if(tournament_test() == -1){
            printf(1,"trnmnt tree test failed\n");
        }
         printf(1,"trnmnt tree test success\n");

        printf(1,"sanity success!\n");
        exit();
    }





