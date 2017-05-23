#include "debug.h"
#include "arraylist.h"
#include "foreach.h"

static pthread_key_t key;
static pthread_once_t key_once = PTHREAD_ONCE_INIT;
static int* cnt;

static void make_key(){
    (void) pthread_key_create(&key, NULL);
}

void *foreach_init(arraylist_t *self){

    //printf("foreach \n\n");
    pthread_mutex_lock( &(self->mutex_f) );

    void *ret = NULL;

    cnt = malloc(sizeof(void*));
    (void) pthread_once(&key_once, make_key);
    (void) pthread_setspecific(key, cnt);

    arraylist_t* tempAL = (arraylist_t*) self;
    int length = (int)tempAL->length;
    int item_size = (int)tempAL->item_size;

    //printf("status : %d \n\n", tempAL->runForeach);

    if(length == 0){
        pthread_mutex_unlock( &(self->mutex_f) );
        return NULL; // This will skip the loop
    }

    tempAL->runForeach = 1;
    // global_runForeach = true;
    char* ptrBase = (char*) tempAL->base;
    void* copy_arr = (void*) malloc( item_size );
    memcpy( copy_arr, ptrBase, item_size );
    ret = copy_arr;

    pthread_mutex_unlock( &(self->mutex_f) );
    return ret; // Pointer to copy of the data of the FIRST (0th) item on success
}

// This function updates the current item in the list
// with any changes made during this iteration and
// moves to the next item in the list by returning a pointer
// to the copy of the next item.
// If data is NULL, do not update the item in the list.

// foreach_next will update the data in the list at the proper position.
void *foreach_next(arraylist_t *self, void* data){
    pthread_mutex_lock( &(self->mutex_f) );

    sem_wait( &(self->semR) );
    // <CRITICAL Section>
    readcnt++; // another pending reader
    if(readcnt == 1){
        sem_wait( &(self->semW) );
    }
    // <END CRITICAL Section>
    sem_post( &(self->semR) );

    void *ret = NULL;
    int length = (int)self->length;
    int item_size = (int)self->item_size;

    cnt = pthread_getspecific(key);

    //printf("current cnt :D : %d  \n\n", *cnt);
    if (cnt == NULL && *cnt == length) {
        //printf("#### current cnt : %d \n\n", *cnt);
        self->runForeach = 0;

        sem_wait( &(self->semR) );
        readcnt--;
        if(readcnt == 0){ // first in
            sem_post( &(self->semW) );
        }
        sem_post( &(self->semR) );

        pthread_mutex_unlock( &(self->mutex_f) );
        return ret;
    }

    //printf("status : %d \n\n", tempAL->runForeach);
    // printf("cnt: %d, length: %d \n\n", (int)*cnt, length);
    if(length == 0){
        self->runForeach = 0;

        sem_wait( &(self->semR) );
        readcnt--;
        if(readcnt == 0){ // first in
            sem_post( &(self->semW) );
        }
        sem_post( &(self->semR) );

        pthread_mutex_unlock( &(self->mutex_f) );
        return NULL;
    }

    char* ptrBase = (char*) self->base;
    void* ptr =  ptrBase + (int)( *cnt * item_size ); // current postion

    if(data!=NULL){ // updates the current item in the list
        memcpy( ptr, data, item_size );
    }

    // p = ptr;
    //printf("current updated num: %d, name : %s \n\n", p->num, p->name);

    if( *cnt < (length-1)){
        *cnt += 1;
        if( *cnt < length ){ // if(!(*(char*)ptr)){
            ret = get_index_al(self,*cnt);
            // ptr =  ptrBase + (int)( *cnt * item_size ); // next postion
            // void* copy_arr = (void*) malloc( item_size );
            // memcpy( copy_arr, ptr, item_size );
            // ret = copy_arr;
        }
    }
    else if( *cnt == (length-1) ){
        *cnt += 1;
        ret = get_index_al(self,*cnt);
        // ptr =  ptrBase + (int)( *cnt * item_size ); // next postion
        // void* copy_arr = (void*) malloc( item_size );
        // memcpy( copy_arr, ptr, item_size );
        //ret = copy_arr;
    }
    else if (*cnt == length || *cnt > length ) {
        self->runForeach = 0;

        sem_wait( &(self->semR) );
        readcnt--;
        if(readcnt == 0){ // first in
            sem_post( &(self->semW) );
        }
        sem_post( &(self->semR) );

        pthread_mutex_unlock( &(self->mutex_f) );
        return ret;
    }

    sem_wait( &(self->semR) );
    readcnt--;
    if(readcnt == 0){ // first in
        sem_post( &(self->semW) );
    }
    sem_post( &(self->semR) );


    pthread_mutex_unlock( &(self->mutex_f) );
    return ret;
}

size_t foreach_index(){
    size_t ret = 0;

    if( (cnt = pthread_getspecific(key)) == NULL){
        return UINT_MAX;
    }
    ret = *cnt;
    return ret;
}

bool foreach_break_f(){
    bool ret = true;
    free(pthread_getspecific(key));
    pthread_key_delete(key);
    return ret;
}

int apply(arraylist_t *self, int (*application)(void*)){
    int ret = 0;
    foreach(void*, value, self) {
        int result = (*application)(value);
        //printf("result: %d \n", result);
        if(result == -1){
            value = NULL;
        }
    }
    //printf("foreach ended. cnt: %d \n", *cnt);

    return ret;
}