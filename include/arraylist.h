#ifndef ARRAYLIST_H
#define ARRAYLIST_H
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "const.h"
#include <semaphore.h>
#include <pthread.h>

/*
    _  _____ _____ _   _     ____  _     _____    ____  _____    _    ____
   / \|_   _|_   _| \ | |   |  _ \| |   |__  /   |  _ \| ____|  / \  |  _ \
  / _ \ | |   | | |  \| |   | |_) | |     / /    | |_) |  _|   / _ \ | | | |
 / ___ \| |   | | | |\  |   |  __/| |___ / /_    |  _ <| |___ / ___ \| |_| |
/_/   \_\_|   |_| |_| \_|   |_|   |_____/____|   |_| \_\_____/_/   \_\____/
*/
#define max_size 10



typedef struct{  // size_t 8 byte. void* 8 byte
    /* BEGIN: DO NOT MODIFY THE FIELDS BETWEEN THESE COMMENTS */
    size_t capacity;
    size_t length;
    size_t item_size;
    void* base;
    /* END: .. add locks, other fields BELOW THIS COMMENT if needed .. */
    int runForeach;
    pthread_mutex_t mutex;
    pthread_mutex_t mutex_f;
    pthread_mutex_t mutex_d;
    // void* mutex_result;
    sem_t semW;
	sem_t semR;
	sem_t semItem;

}arraylist_t;

int readcnt;
int writecnt;

// bool global_runForeach;

int sem_num;
//void setSemaphore();

arraylist_t *new_al(size_t item_size);


typedef struct {
    arraylist_t* self;
    void* data;
}args_self_data;



size_t insert_al(arraylist_t *self, void* data);
//void* insert_Thread(void *argss);



size_t get_data_al(arraylist_t *self, void* data);
void *get_index_al(arraylist_t *self, size_t index);

bool remove_data_al(arraylist_t *self, void *data);

//void* remove_data_thread(void* argss);


void *remove_index_al(arraylist_t *self, size_t index);

void delete_al(arraylist_t *self, void (*free_item_func)(void*));

int get_indexId_sameData(arraylist_t *self, void *data);

#endif
