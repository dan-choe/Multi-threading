#include "debug.h"
#include "arraylist.h"
#include "foreach.h"
#include "server.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>

struct sockaddr_in server , client, address;
pthread_mutex_t server_mutex;
int cnt, n_value;
arraylist_t* login_arrlist;
arraylist_t* user_arrlist;

int createThreads(arraylist_t* logn_self, int nthreads){//Will be used later for creating threads
    pthread_t * threadID = calloc(nthreads,sizeof(pthread_t));//TIDS FOR MAP THREADS ARE STORED HERE
    for(int i =0; i < nthreads;i++){
        // pthread_create(&(threadID[i]), NULL, connection_handler, NULL);
        // pthread_setname_np(threadID[i], final);//THE NAME IS SET HERE
    }
    for(int i =0; i < nthreads;i++){
        pthread_join(threadID[i], NULL);//THIS IS WHERE THE MAP THREADS JOIN
    }
    return 0;
}


void *connection_handler(void *socket)
{
	client_st* p = (client_st*) socket;
    int client_socket = p->id;

    char *message;
    char client_message[2000];
    int read_size;
    int read_ALOLA = 0;
    int read_IM = 0;
    int readline = 0;

    message = "I'm connection handler. I'll remove you from Login ArrayList when u loggined in, ";
    write(client_socket , message , strlen(message));

    message = "then I will attempt to log the user into the server using the ALOLA protocol \n";
    write(client_socket , message , strlen(message));

    message = "You have to type ALOLA! to login the server. \n";
    write(client_socket , message , strlen(message));

    while( (read_size = recv(client_socket , client_message , 2000 , 0)) > 0 )
    {
        client_message[read_size] = '\0';

        int result = strncmp(client_message, "ALOLA! \r\n", read_size);
        if(result == 0){
        	read_ALOLA = 1;
        	readline++;
        }

        int result_username = strncmp(client_message, "IM ", 3);
        if(result_username == 0){
        	read_IM = 1;
        	readline++;
        }

        read_IM = read_IM;

        result = strncmp(client_message, "\r\n", read_size);
        if(result == 0){
        	if(read_ALOLA == 1){
        		write(client_socket , "!ALOLA \r\n\r\n" , strlen("!ALOLA \r\n\r\n"));
        		puts("You are loggned in. I'll place u into the User ArrayList\n");
        		puts("and add the file descriptor to the communication thread's active set.\n");

        		remove_index_al(login_arrlist, client_socket);
        		insert_al(user_arrlist, p);

        		puts("I finished that you replace to User ArrayList.\n");
        		read_ALOLA = 0;
        	}
        }else{
        	//if(readline == 1)
        		//puts("You failed to login. I'll disconnet from the user.\n");
        }
    }

    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }

    free(socket);
    return 0;
}

void* login_threads (void* login_al) {
	//remove_index_al(login_al, 0); // Each of the login threads will remove an item from the Login ArrayList
	return NULL;

}

int main(int argc, char *argv[]){

	if(argc <= 1)
		return -1;

	cnt = 0;
	n_value = atoi(argv[1]);
	printf("n_value %d \n", n_value);

	pthread_mutex_init(&(server_mutex), NULL);

    int opt = 1;
    int m_socket, addrlen, new_socket;
    int act_select;
    int max_sd;
    struct sockaddr_in address;
    fd_set readfds;

    login_arrlist = new_al(sizeof(client_st));
    user_arrlist = new_al(sizeof(client_st));
	//createThreads(login_arrlist, n_value);

	//a single communication thread
	pthread_t* commThread = calloc(1,sizeof(pthread_t));
	pthread_create(commThread, NULL, login_threads, NULL);

    for (int i = 0; i < n_value; i++)
    {
    	client_st* client_socket;
    	client_socket = (client_st*) calloc(n_value, sizeof(struct client_t*));
        client_socket->id = 0;
        insert_al(login_arrlist, client_socket);
    }

    if( (m_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        perror("failed");
        exit(EXIT_FAILURE);
    }

    if( setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    if (bind(m_socket, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Listener on port %d \n", PORT);

    if (listen(m_socket, n_value) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    addrlen = sizeof(address);
    puts("Waiting for connections ...");

    while(1)
    {
        FD_ZERO(&readfds);
        FD_SET(m_socket, &readfds);
        max_sd = m_socket;

        client_st* p = calloc(1,login_arrlist->item_size);
        for (int i = 0 ; i < n_value ; i++)
        {
            p = get_index_al(login_arrlist,i);
            int item_id = p->id;

            if(item_id > 0)
                FD_SET( item_id , &readfds);

            if(item_id > max_sd)
                max_sd = item_id;
        }

        act_select = select( max_sd + 1 , &readfds , NULL , NULL , NULL);

        if ((act_select < 0) && (errno!=EINTR))
        	printf("select error");

        if (FD_ISSET(m_socket, &readfds))
        {
        	pthread_mutex_lock( &(server_mutex) );

		    while( (new_socket = accept(m_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) )
		    {
		        puts("Connection accepted");

		        pthread_t login_thread;

		        if(cnt < n_value){

					client_st* p = calloc(1,login_arrlist->item_size);
					client_st* ptr;

			        //socket to login arraylist
		            for (int i = 0; i < n_value; i++)
		            {
		            	p = get_index_al(login_arrlist,i);
		                if( (p->id) == 0 )
		                {
		                	ptr = (client_st*) login_arrlist->base + (int) i * login_arrlist->item_size;
		                    ptr->id = new_socket;
		                    break;
		                }
		            }

		            if( pthread_create( &login_thread , NULL ,  connection_handler , (void*) ptr) < 0)
			        {
			            perror("failed to create thread");
			            return 1;
			        }
			        cnt++;

		        	pthread_join( login_thread , NULL);
		        	puts("Handler assigned");
		        }
		    }

		    if (new_socket < 0)
		    {
		        perror("accept failed");
		        return 1;
		    }

		    pthread_mutex_unlock( &(server_mutex) );
        }
    }

    pthread_join(*commThread , NULL);

    return 0;
}