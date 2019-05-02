//
// Created by snirv@wincs.cs.bgu.ac.il on 4/30/19.
//

#include "types.h"
#include "user.h"
#include "kthread.h"
#include "tournament_tree.h"


 trnmnt_tree* trnmnt_tree_alloc(int depth){

     trnmnt_tree* tree = malloc(sizeof(trnmnt_tree));
     memset(tree, 0, sizeof(trnmnt_tree));

     tree->depth = depth;
     tree->mutex_num = (1 << depth) -1 ;
     tree->mutex_ids_arr = malloc(sizeof(int)*(tree->mutex_num));
     memset(tree->mutex_ids_arr, 0, sizeof(int)*(tree->mutex_num));

      for(int i = 0; i < tree->mutex_num; i++){
          int mutex_id = kthread_mutex_alloc();
          if(mutex_id >= 0){
              tree->mutex_ids_arr[i] = mutex_id;
          }
          else{
              return 0;
          }
      }
     return tree;
 }

int trnmnt_tree_dealloc(trnmnt_tree* tree){
    for(int i = 0; i < tree->mutex_num; i++) {
        if(kthread_mutex_dealloc(tree->mutex_ids_arr[i]) == -1){
            return -1;
        }
    }
    free(tree->mutex_ids_arr);
    free(tree);
    return 0;

}

int trnmnt_tree_acquire(trnmnt_tree* tree,int id){

   int tree_idx = id + ((1 << tree->depth) -1 );
    int father = (tree_idx-1) / 2;
    while (father >= 0){
       if( kthread_mutex_lock(tree->mutex_ids_arr[father]) == -1){
           return -1;
       }
        if(father == 0){
            break;
        }
        father = (father-1) / 2;
    }
    return 0;



}

int trnmnt_tree_release(trnmnt_tree* tree, int id ){
    int tree_idx = id + ((1 << tree->depth) -1 );
    return release_helper(tree,tree_idx);

}


int release_helper(trnmnt_tree* tree , int tree_idx){
    int father = (tree_idx -1) / 2 ;
    printf(1,"enter helpr, father is %d\n", father);
    if(father == 0){
        kthread_mutex_unlock(tree->mutex_ids_arr[father]);
        return 0;
    }
    else{
        int ret = release_helper(tree,father);
        if (ret == -1 || kthread_mutex_unlock(tree->mutex_ids_arr[father]) == -1 ){

            return -1;
        }
    }
    return 0;
}
