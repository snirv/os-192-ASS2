

#include "spinlock.h"
#include "mutex.h"



typedef struct trnmnt_tree {
    int depth;
    int mutex_num;
    int* mutex_ids_arr;
    int using_threads;
    int tree_mutex_id;

} trnmnt_tree;


trnmnt_tree* trnmnt_tree_alloc(int depth);
int trnmnt_tree_dealloc(trnmnt_tree* tree);
int trnmnt_tree_acquire(trnmnt_tree* tree,int id);
int trnmnt_tree_release(trnmnt_tree* tree, int id );
int release_helper(trnmnt_tree* tree , int tree_idx);