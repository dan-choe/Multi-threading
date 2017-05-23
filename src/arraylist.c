
#include "arraylist.h"
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

/**
 * @visibility HIDDEN FROM USER
 * @return     true on success, false on failure
 */
static bool resize_al(arraylist_t* self) {
    bool ret = false;
    char* baseAddr = self->base;
    bool isNewAddress = false;
    char* previousAddr = self->base;

    // printf("resize baseAddr : %p \n", (void*)baseAddr);

    if(self->length == self->capacity) { // GROWING
        //void* growingAddr = realloc(baseAddr, ( (self->capacity*2) * self->item_size));
        void* growingAddr = calloc((self->capacity * 2), ((self->capacity*2) * self->item_size));
        if(growingAddr){

            if( growingAddr == baseAddr ){
                isNewAddress = true;
            }

            memcpy(growingAddr, baseAddr, ((self->capacity) * self->item_size) );
            self->base = growingAddr;
            ret = true;
            void* previousEndPtr = (char*) growingAddr + ( self->length * self->item_size) ;
            memset(previousEndPtr, 0, (int)( (self->capacity) * self->item_size) );
            self->capacity = self->capacity * 2;
        }
    }else if( self->length == ( (self->capacity/2) - 1) ) { // SHRINKING, Never shrink below INIT_SZ
        if(self->capacity/2 >= INIT_SZ) {
            //char* shrinkAddr = (char*) realloc(baseAddr, (size_t)((self->capacity/2) * self->item_size) );
            void* shrinkAddr = calloc((self->capacity / 2), ((self->capacity/2) * self->item_size));
            if(shrinkAddr!=NULL){
                if( shrinkAddr == baseAddr ){
                    isNewAddress = true;
                }
                memcpy(shrinkAddr, baseAddr, ((self->capacity/2) * self->item_size) );
                self->base = shrinkAddr;
                self->capacity = self->capacity / 2;
                ret = true;
            }
        }
    }
    if(!ret)
        errno = ENOMEM;

    if( isNewAddress && readcnt ==0 && writecnt == 1){
        //printf("\n\n\n $$$$$$$$$$$$$$$$$$$ NEW ADDRESS IS GOTTEN \n\n\n\n");
        previousAddr = previousAddr;
        //free(previousAddr);
    }

    return ret;
}

arraylist_t *new_al(size_t item_size) {
    // void *ret = NULL;
    arraylist_t* temp_arr = (arraylist_t*) malloc( sizeof(arraylist_t) );

    sem_init(&(temp_arr->semW), 0, 1);
    sem_init(&(temp_arr->semR), 0, 1);

    readcnt = 0;
    writecnt = 0;

    temp_arr->capacity    = INIT_SZ;                       // in terms of the number of items it can hold
    temp_arr->length      = 0;                               // the number of items currently inserted
    temp_arr->item_size   = item_size;
    temp_arr->base        = calloc(INIT_SZ, (INIT_SZ*item_size));
    temp_arr->runForeach = 0;
    pthread_mutex_init(&(temp_arr->mutex), NULL);
    pthread_mutex_init(&(temp_arr->mutex_f), NULL);
    pthread_mutex_init(&(temp_arr->mutex_d), NULL);

    //printf("                    INIT_SZ*item_size %d\n", (int)INIT_SZ * (int)item_size);
    if(!temp_arr) {
        errno = ENOMEM;
        return NULL;
    }
    return temp_arr;
}

size_t insert_al(arraylist_t *self, void* data) {
    size_t ret = UINT_MAX;

    // if(self->runForeach == 1){
    //     printf("trying inserting\n\n");
    //     //pthread_mutex_lock( &(self->mutex_f) );
    // }

    //printf("[insert_al] writer is trying to add an item.    readcnt: %d, writecnt: %d, self->length: %d \n", readcnt, writecnt, (int)self->length);
    pthread_mutex_lock( &(self->mutex) );
    sem_wait( &(self->semW) );
    writecnt++;
    //usleep(1 * 1000);
    //------------------------------------------ critical section
    //printf("[insert_al] writer is writting.    readcnt: %d, writecnt: %d, self->length: %d \n", readcnt, writecnt, (int)self->length);
    if( self->capacity == self->length )
        resize_al(self);

    char* ptrA = (char*) self->base;
    void* ptr  =  ptrA + (int)(self->length * self->item_size );
    memcpy( ptr, data, (int)self->item_size);
    ret = self->length;
    self->length = self->length + 1;
    //------------------------------------------ critical section
    //printf("[insert_al] writer left.    readcnt: %d, writecnt: %d, self->length: %d \n", readcnt, writecnt, (int)self->length);
    writecnt--;
    sem_post( &(self->semW) );
    pthread_mutex_unlock( &(self->mutex) );

    // if(self->runForeach == 1){
    //     printf("@@@ finished inserting\n\n");
    //     //pthread_mutex_unlock( &(self->mutex_f) );
    // }
    return ret;
}

// return the index of the item equal to data
size_t get_data_al(arraylist_t *self, void *data){
    size_t ret = 0;

    //printf("[get_data_al] reader is trying.    readcnt: %d, writecnt: %d, self->length: %d \n", readcnt, writecnt, (int)self->length);

    sem_wait( &(self->semR) );
    // <CRITICAL Section>
    readcnt++; // another pending reader
    if(readcnt == 1){
        sem_wait( &(self->semW) );
    }
    // <END CRITICAL Section>
    sem_post( &(self->semR) );


    //printf("[get_data_al] reader is reading.    readcnt: %d, writecnt: %d, self->length: %d \n", readcnt, writecnt, (int)self->length);

    //test_item_t* temp = (test_item_t*) data;

    //printf("\n\n @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ data: %d\n", temp->i);

    if( data == NULL ){   // return the first item in the arraylist
        sem_wait( &(self->semR) );
        readcnt--;
        if(readcnt == 0){ // first in
            sem_post( &(self->semW) );
        }
        sem_post( &(self->semR) );

        //printf("[get_data_al] data == NULL, reader is leaving.    readcnt: %d, writecnt: %d\n", readcnt, writecnt);
        return ret;
    }
    int result = get_indexId_sameData(self, data);

    //printf("\n\n @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ result: %d\n", result);

    if(result < -1) {
        errno = EINVAL;

        sem_wait( &(self->semR) );
        readcnt--;
        if(readcnt == 0){ // first in
            sem_post( &(self->semW) );
        }
        sem_post( &(self->semR) );

       // printf("[get_data_al] reader is leaving.    readcnt: %d, writecnt: %d\n", readcnt, writecnt);
        return UINT_MAX;
    }else{
        ret = result;

        sem_wait( &(self->semR) );
        readcnt--;
        if(readcnt == 0){ // first in
            sem_post( &(self->semW) );
        }
        sem_post( &(self->semR) );

       // printf("[get_data_al] reader is leaving.    readcnt: %d, writecnt: %d\n", readcnt, writecnt);
        return ret;
    }
}

int get_indexId_sameData(arraylist_t *self, void *data) {
    size_t ret = 0;
    char* ptrA = (char*) self->base;
    bool isFound = false;
    for(int i=0; i<self->length; i++) {
        void* ptr =  ptrA + (int)(i * self->item_size ) ;
        if( (memcmp(ptr, data, self->item_size)) == 0){ // compare_two_data(self->item_size, ptr, data)
            isFound = true;
            ret = i;
        }
    }
    if(isFound)
        return ret;
    return -1;
}

//printf("self->length : %d, index: %d\n", (int)self->length, (int)index);
//printf("%d\n", (int)self->length);

void *get_index_al(arraylist_t *self, size_t index) {
    void *ret = NULL;
    void* ptr;
    void* copy_arr;
    char* ptrA = (char*) self->base;

    sem_wait( &(self->semR) );
    // <CRITICAL Section>
    readcnt++; // another pending reader
    if(readcnt == 1){
        sem_wait( &(self->semW) );
    }
    // <END CRITICAL Section>
    sem_post( &(self->semR) );

    if( index < 0){
        errno = EINVAL;

        sem_wait( &(self->semR) );
        readcnt--;
        if(readcnt == 0){ // first in
            sem_post( &(self->semW) );
        }
        sem_post( &(self->semR) );

        return ret;
    }
    else if( self->length <= index) {                                // If index exceeds number of items in the list,
        ptr =  ptrA + (int)(self->length * self->item_size ) ;      // copy and get the last item in the arraylist
        copy_arr = (void*) malloc( sizeof(self->item_size) );
        memcpy( copy_arr, ptr, (int)self->item_size);


        sem_wait( &(self->semR) );
        readcnt--;
        if(readcnt == 0){ // first in
            sem_post( &(self->semW) );
        }
        sem_post( &(self->semR) );


        return copy_arr;
    }else{                                                          // return a pointer to a copy of the item at self[index]
        ptr =  ptrA + (int)(index * self->item_size ) ;
        copy_arr = (void*) malloc( sizeof(self->item_size) );
        memcpy( copy_arr, ptr, (int)self->item_size);

        sem_wait( &(self->semR) );
        readcnt--;
        if(readcnt == 0){ // first in
            sem_post( &(self->semW) );
        }
        sem_post( &(self->semR) );

        return copy_arr;
    }

    sem_wait( &(self->semR) );
    readcnt--;
    if(readcnt == 0){ // first in
        sem_post( &(self->semW) );
    }
    sem_post( &(self->semR) );

    return ret;
}
bool remove_data_al(arraylist_t *self, void *data) {

    if(self->runForeach == 1){
        return false;
    }

    bool ret = false;
    char* baseAddr = (char*) self->base;

   // printf("[remove_data_al] writer is trying to remove.    readcnt: %d, writecnt: %d self->length: %d \n", readcnt, writecnt, (int)self->length);

    pthread_mutex_lock( &(self->mutex) );
    sem_wait( &(self->semW) );
    writecnt++;

    // --------------------------------------------------------------------------------------------
   // printf("[remove_data_al] writer is removing.    readcnt: %d, writecnt: %d self->length: %d \n", readcnt, writecnt, (int)self->length);
    if( data == NULL) {   // remove the first item in the arraylist
        void* ptr =  baseAddr + (int)(1 * self->item_size );
        memcpy(baseAddr, ptr, (int)( (self->length-1) * self->item_size) );
        ptr =  baseAddr + (int)( (self->length-1) * self->item_size);
        memset(ptr, '\0', self->item_size);
        self->length--;
        ret = true;
    }

    int result = get_indexId_sameData(self, data);
    if(result < 0) {
        //printf("[remove_data_al] writer left (result < 0).    readcnt: %d, writecnt: %d self->length: %d \n", readcnt, writecnt, (int)self->length);
        writecnt--;
        sem_post( &(self->semW) );
        pthread_mutex_unlock( &(self->mutex) );
        return ret;
    }else{
        char* targetItemAddr =  (char*)baseAddr + (int)( result * self->item_size ); // target data location
        char* nextTargetAddr =  (char*)baseAddr + (int)( (result+1) * self->item_size );
        int point = (int)self->length - (int) result - 1;
        memcpy(targetItemAddr, nextTargetAddr, (int)( point * (int)self->item_size) );
        nextTargetAddr =  (char*)baseAddr + (int)( (self->length-1) * self->item_size);
        memset(nextTargetAddr, '\0', self->item_size);

        self->length--;
        ret = true;
    }
    if(self->length == (self->capacity/2) - 1)
        resize_al(self);
    // --------------------------------------------------------------------------------------------
    //printf("[remove_data_al] writer left.    readcnt: %d, writecnt: %d self->length: %d \n", readcnt, writecnt, (int)self->length);
    writecnt--;
    sem_post( &(self->semW) );
    pthread_mutex_unlock( &(self->mutex) );
    return ret;
}

void *remove_index_al(arraylist_t *self, size_t index) {

    if(self->runForeach == 1){
        return NULL;
    }

    void *ret = NULL;
    char* baseAddr = (char*) self->base;



    //printf("[remove_index_al] writer is trying to remove.    readcnt: %d, writecnt: %d self->length: %d \n", readcnt, writecnt, (int)self->length);

    pthread_mutex_lock( &(self->mutex) );
    sem_wait( &(self->semR) );
    writecnt++;

    //printf("[remove_index_al] writer is removing.    readcnt: %d, writecnt: %d self->length: %d \n", readcnt, writecnt, (int)self->length);


    if( index < 0) {
        errno = EINVAL;
        //printf("[remove_data_al] writer left ( index < 0).    readcnt: %d, writecnt: %d self->length: %d \n", readcnt, writecnt, (int)self->length);
        writecnt--;
        sem_post( &(self->semR) );
        pthread_mutex_unlock( &(self->mutex) );
        return ret;
    }
    else if( index >= (self->length-1)) {   // copy and remove the last item in the arraylist
        char* ptr =  (char*) baseAddr + (int)((self->length-1) * self->item_size );
        //printf("\n\n####### malloc 1 #########\n\n");
        void* copy_arr = (void*) malloc( sizeof(self->item_size) );
        memcpy(copy_arr, ptr, (int)( self->item_size ) );
        int point = (int)self->length - (int) index - 1;
        ptr =  (char*)baseAddr + (int)( (point) * (int)self->item_size);
        memset(ptr, '\0', self->item_size);

        self->length--;
        ret = copy_arr;
    }else{
        char* targetItemAddr =  (char*) baseAddr + (int)( index * self->item_size );
        //printf("\n\n####### malloc 2 #########, index: %d, self->length: %d \n\n", (int)index, (int)self->length);
        void* copy_arr = (void*) malloc( sizeof(self->item_size) );
        memcpy(copy_arr, targetItemAddr, (int)( self->item_size ) );

        if( (int)self->length == 1 && index == 0 ){ //
            memset(baseAddr, '\0', (int)self->item_size);
        }
        else if( (int)self->length == 2 && index == 0 ){ //
            char* nextTargetAddr =  (char*)baseAddr + (int)( self->item_size ); // [1] - [1]
            memcpy(targetItemAddr, nextTargetAddr, (int)(self->item_size) );
            int point = (int)self->length - 1;
            targetItemAddr =  (char*)baseAddr + (int)( (int)(point) * (int)self->item_size);
            memset(targetItemAddr, '\0', (int)self->item_size);
        }
        else if( (int)self->length > 1 && index != 0 ){ //
            char* nextTargetAddr =  (char*)baseAddr + (int)( (index+1) * self->item_size );
            int point = (int)self->length - (int) index - 1;
            memcpy(targetItemAddr, nextTargetAddr, (int)( point * (int)self->item_size) );
            targetItemAddr =  (char*)baseAddr + (int)( (int)(self->length-1) * (int)self->item_size);
            memset(targetItemAddr, '\0', (int)self->item_size);
        }

        self->length--;
        ret = copy_arr;
    }

    if(self->length == (self->capacity/2) - 1)
        resize_al(self);

    //printf("[remove_index_al] writer left.    readcnt: %d, writecnt: %d self->length: %d \n", readcnt, writecnt, (int)self->length);
    writecnt--;
    sem_post( &(self->semR) );
    pthread_mutex_unlock( &(self->mutex) );
    return ret;
}

void delete_al(arraylist_t *self, void (*free_item_func)(void*)) {
    if(self->runForeach == 1){
        return;
    }
    pthread_mutex_lock( &(self->mutex_d) );
    // sem_wait( &(self->semR) );
    // writecnt++;

    if(free_item_func == NULL){
        pthread_mutex_unlock( &(self->mutex_d) );
        return;
    }

    if(self->capacity == 0){
        // writecnt--;
        // sem_post( &(self->semR) );
        pthread_mutex_unlock( &(self->mutex_d) );
        return;
    }else{
        if(free_item_func == NULL){
            // writecnt--;
            // sem_post( &(self->semR) );
            pthread_mutex_unlock( &(self->mutex_d) );
            return;
        }
        for(int i= self->length-1; i>0; i--){
            char* ptr = (char*)self->base + (int)((i) * self->item_size);
            free_item_func(ptr);
            self->length--;
        }
        free_item_func((char*)self->base);

        self->length = 0;
        self->capacity = 0;
        self->item_size = 0;
    }
    // writecnt--;
    // sem_post( &(self->semR) );
    pthread_mutex_unlock( &(self->mutex_d) );
    return;
}
