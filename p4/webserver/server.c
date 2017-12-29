#include "cs537.h"
#include "server_impl.h"
#include <stdio.h>
//
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//
//pthread_cond_t empty;// condition signal
//pthread_cond_t fill; // condition signal
//pthread_mutex_t mutex;// mutex
//pthread_t *t;
//int numtrd;
void getargs(int *port, int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    *port = atoi(argv[1]);
//    numtrd = atoi(argv[2]);
//    printf("argc: %d\n",argc);
}


int main(int argc, char *argv[]) {
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;

    getargs(&port, argc, argv);
//    printf("%d %d %d",atoi(argv[1]),atoi(argv[2]),atoi(argv[3]));
    server_init(argc, argv);

    listenfd = Open_listenfd(port);
//    for(int i=0;i<numtrd;i++){
//      pthread_join(t[i],NULL);
//      printf("joined a thread\n");
//    }
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

        server_dispatch(connfd);
    }
}






