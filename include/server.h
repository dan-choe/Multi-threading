#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>

#include "debug.h"
#include "arraylist.h"
#include "foreach.h"
#include "const.h"

#endif


// pthread_mutex_t server_mutex;

typedef struct {
    char* username;
    int id;
}client_st;

void* login_threads (void* login_al) ;
int createThreads(arraylist_t* logn_self, int nthreads);
void *connection_handler(void *socket_desc);