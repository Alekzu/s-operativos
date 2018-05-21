#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>// addr socket
#include <pthread.h>
#define BACKLOG 8
#define MSGSIZE 32
#define NUMTHREADS 8
#define INFOSIZE sizeof(struct info)

struct info{
  int control;
  int a;
  int b;
  char mensaje[32];
};

void *connection_handler(void *socket_desc)
{
    int r, sum, errv, ctrl;
    //Get the socket descriptor
    int clientfd = *(int*)socket_desc;
    struct info *datos = malloc(INFOSIZE);
    while(ctrl!=0){
      r = recv(clientfd,datos,INFOSIZE,0);
      if(r ==-1){
          perror("receive:");
      exit(-1);
      }
      if(datos->control == 1){
        //SEND
        sum = datos->a+datos->b;
        errv = send(clientfd,&sum,4,0);
        if(errv == -1){
            perror("mal:");
        exit(-1);
        }
      }
      else{
        strncpy(datos->mensaje,"wobalobadubdub",32);
        errv = send(clientfd,datos,INFOSIZE,0);
        if(errv == -1){
            perror("mal:");
        exit(-1);
        }
      }
      ctrl = datos->control;
    }
    free(datos);
    close(clientfd);
    //close(svfd);

}

/*
estructura del sockaddress
struct sockaddr_in{
    short sin_family; //protocolo ipv4
    unsigned short sin_port;
    struct in_addr sin_addr;
    char sin_zero[8];
};
struct in_addr{
    unsigned long s_addr;
};
*/
int main(){
    int svfd, errv, clientfd, sum, r, a, b, ctrl, client_sock;
    int sockfd, *new_sock;
    int option = 1;
    struct sockaddr_in server, cliente;
    socklen_t tamanio, tamac;
    pthread_t thread_id;
    //struct info *datos = malloc(INFOSIZE);
//socket
    svfd = socket(AF_INET,SOCK_STREAM,0); //
    setsockopt(svfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    if(svfd ==-1){
        perror("mal:");
    exit(-1);
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(3535);//indianism converter (memory architecture intel/arm)
    server.sin_addr.s_addr = INADDR_ANY; //para que escuche cualquier interfaz de red
    bzero(server.sin_zero,8);//llenar 8 bit con ceros
    tamanio = sizeof(struct sockaddr_in);

    errv = bind(svfd, (struct sockaddr*)&server, tamanio);
    if(errv == -1){
        perror("mal:");
    exit(-1);
    }
//LISTEN
    errv = listen(svfd,BACKLOG);
    if(errv == -1){
        perror("mal:");
    exit(-1);
    }
    tamac = sizeof(struct sockaddr_in); //si no se inicializa, da error
    //accept recibe la conexion con el socket del servidor,
    //guarda los datos del cliente (ip,etc)
    //crea un socket exclusivo para el cliente

    //multithread
    while(client_sock = accept(svfd, (struct sockaddr *)&cliente,&tamac))
        {
            puts("Connection accepted");
            pthread_t curr_thread;
            new_sock = malloc(sizeof(int));
            *new_sock = client_sock;

            if( pthread_create( &curr_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
            {
                perror("could not create thread");
                return 1;
            }

            //Now join the thread , so that we dont terminate before the thread
            //pthread_join( thread_id , NULL);
            puts("Handler assigned");
        }

        if (client_sock < 0)
        {
            perror("accept failed");
            return 1;
        }
    close(svfd);
    return 0;
  }


//RECEIVE

//sockfd = socket(AF_INET, SOCK_STREAM, 0);
