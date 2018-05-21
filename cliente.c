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
#include <netdb.h>
#include <netinet/in.h>// addr socket
#include <arpa/inet.h>
#define BACKLOG 8
#define MSGSIZE 32
#define PORT 3535
#define MAXLEN 32
#define INFOSIZE sizeof(struct info)

struct info{
  int control;
  int a;
  int b;
  char mensaje[32];
};
int main(){
    int clientfd, r, a,b,dato, ctrl;
    char msg[32];
    struct info *datos = malloc(INFOSIZE);
    struct sockaddr_in client;
    socklen_t tama= 0;
    dato = 3;

    clientfd = socket(AF_INET, SOCK_STREAM,0);
    if(clientfd ==-1){
        perror("socket:");
    //exit(-1);
    }
    client.sin_family = AF_INET;
    client.sin_port = htons(PORT);//indianism converter (memoryarchitecture intel/arm)
    client.sin_addr.s_addr = inet_addr("127.0.0.1");
    bzero(client.sin_zero,8);//llenar 8 bit con ceros

    tama = sizeof(struct sockaddr_in);
    r = connect(clientfd,(struct sockaddr*)&client,tama);
    if(r ==-1){
        perror("connect:");
    exit(-1);
    }
    else printf("conexion realizada \n");
    ctrl = 3;
    while(ctrl != 0){
      printf("ingrese opcion: ");
      scanf("%i",&ctrl);
      datos->control = ctrl;

      if(ctrl == 1){
        printf("ingrese numeros a sumar\n");
        scanf("%i %i",&datos->a,&datos->b);
        printf("...\n");
        r = send(clientfd,datos,INFOSIZE,0);
        if(r == -1){
            perror("mal:");
        exit(-1);
        }

        r = recv(clientfd,&dato,4,0);
        if(r ==-1){
            perror("receive:");
        //exit(-1);
        }
        printf("Suma: %i \n",dato);
      }
      else{
        r = send(clientfd,datos,INFOSIZE,0);
        if(r == -1){
            perror("mal:");
        exit(-1);
        }
        r = recv(clientfd,datos,INFOSIZE,0);
        if(r ==-1){
            perror("receive:");
        //exit(-1);
        }
        printf("%s\n",datos->mensaje);
      }
    }
    free(datos);
    close(clientfd);
}
