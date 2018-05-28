#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <inttypes.h>
#include <strings.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>// addr socket
#include <arpa/inet.h>
#define regSize sizeof(struct dogType)
#define TRANSFSIZE sizeof(struct transfer)
#define BACKLOG 8
#define PORT 3535

struct transfer{
  //estructura usada para intercambio de datos cliente-servidor
  int opcion;
  int reg;
  char nombre[32];
};

struct dogType{
  //estructura que contiene los datos de mascota
    char nombre[32];
    char tipo[32];
    int32_t edad;
    char raza[16];
    int32_t estatura;
    float peso;
    char sexo;
    int next; //registro siguiente
    int prev; //registro anterior
    int state; //define si está en uso o borrado
//size 108 bytes
};

void ingresar(void *ap){
  //recibe los datos de un nuevo registro
    struct dogType *dato;
    dato = ap;
    //struct dogType *archivo = malloc(sizeof(struct dogType));
    int key, actual, posicion, ctrl,fem,male;
    char sex[2], macho[] = "m",hembra[] = "h";
    ctrl = 0;
    if (dato == NULL)
    {
        return;
    }
    printf("\n ingrese nombre: ");
    scanf("%s", dato->nombre);
    printf("\n ingrese tipo: ");
    scanf(" %s", dato->tipo);
    printf("\n ingrese edad: ");
    scanf(" %i", &dato->edad);
    printf("\n ingrese raza: ");
    scanf(" %s", dato->raza);
    printf("\n ingrese estatura: ");
    scanf(" %i", &dato->estatura);
    printf("\n ingrese peso: ");
    scanf(" %f", &dato->peso);
    printf("\n ingrese sexo h o m: ");
    scanf(" %s", sex);
    while(ctrl == 0){ // comprobacion de h ó m
    	male = strcasecmp(macho, sex);
    	fem = strcasecmp(hembra, sex);
    	if(fem == 0 || male == 0){
    	  ctrl = 1;
        dato->sexo = sex[0];
    	}
    	else{
    	  printf("\n el sexo debe ser h ó m: ");
    	  scanf(" %c",sex);
    	}
    }
    dato->next = -1; //siguiente nodo nulo
    dato->prev = -1; //asignacion temporal

}
void enterCont(){ //pedir tecla para continuar
  char cont;
  printf("presione enter para continuar \n" );
  scanf(" %c", &cont);
}

void mostrar(void *ap){
  //recibe un apuntador al registro, y lo muestra en pantalla
  struct dogType *dato;
  dato = ap;

  printf("Nombre: %s \n", dato->nombre);
  printf("Tipo: %s \n", dato->tipo);
  printf("Edad: %i \n", dato->edad);
  printf("Raza: %s \n", dato->raza);
  printf("Estatura: %i \n", dato->estatura);
  printf("Peso: %f \n", dato->peso);
  printf("Sexo: %c \n", dato->sexo);
  printf("Next: %i \n", dato->next);
  printf("Prev: %i \n", dato->prev);
  printf("-------------------------------------------\n");

}

int main(){
  int salir, reg, opt, clientfd, r, resp, i, num;
  char term;
  struct dogType *tmp = malloc(regSize);
  struct transfer *info = malloc(TRANSFSIZE);
  struct sockaddr_in client;
  socklen_t tama= 0;
  salir = 1;

//conectar
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
      perror("conexion fallida:");
  exit(-1);
  }
  else printf("conexion realizada \n");


  do {
    printf("Veterinaria\n 1. Ingresar \n 2. Ver\n 3. Borrar\n 4. Buscar\n 5. Salir\n");
    scanf("%i",&opt);
    switch (opt) {
      case 1:
        printf("Ingreso de registro\n");
        info->opcion = opt;
        r = send(clientfd,info,TRANSFSIZE,0);//enviar el comando a realizar
        if(r == -1){
            perror("mal:");
        exit(-1);
        }
        ingresar(tmp); //recibe los datos de mascota
        r = send(clientfd,tmp,regSize,0); //envia los datos de mascota
        if(r == -1){
            perror("mal:");
        exit(-1);
        }
        r = recv(clientfd,&resp,4,0); //recibe confirmaciom
        if(r ==-1){
            perror("receive:");
        //exit(-1);
        }
        if(resp > 0)printf("Registro guardado con exito \n");
        else printf("Error guardando registro\n");
        enterCont();
      break;
      case 2:
        printf("Nº registro para ver: ");
        scanf(" %i", &reg);
        info->opcion = opt;
        info->reg = reg;
        r = send(clientfd,info,TRANSFSIZE,0);//enviar el comando a realizar
        if(r == -1){
            perror("mal:");
        exit(-1);
        }
        r = recv(clientfd,tmp,regSize,0); //recibe la estructura
        if(r ==-1){
            perror("receive:");
        //exit(-1);
        }
        //resp = strcasecmp(tmp->nombre,bnombre);
        if(tmp->next == 0){//comprueba si existe
          printf("Ingrese un numero de registro valido\n");
        }
        else{
          mostrar(tmp);
        }

        enterCont();
      break;
      case 3:
        info->opcion = opt;
        r = send(clientfd,info,TRANSFSIZE,0);//enviar el comando a realizar
        if(r == -1){
            perror("mal:");
        exit(-1);
        }
        r = recv(clientfd,&resp,4,0); //recibe numero de registros
        if(r ==-1){
            perror("receive:");
        //exit(-1);
        }
        printf("Numero de registros actual:%i\n", resp);
        printf("Ingrese el numero del registro a eliminar: ");
        if(scanf("%d%c", &num, &term) != 2 || term != '\n'){
          info->reg = 0;}
        else{
          info->reg = num;}
        //scanf(" %i", &info->reg);
        r = send(clientfd,info,TRANSFSIZE,0);//enviar el registro a borrar
        if(r == -1){
            perror("mal:");
        exit(-1);
        }
        r = recv(clientfd,&resp,4,0); //recibe confirmaciom
        if(r ==-1){
            perror("receive:");
        //exit(-1);
        }
        if(resp == 1){printf("Registro borrado con exito \n");}
        else if(resp == -1){printf("No existe el registro\n");}
        else{ printf("Ingrese un numero de registro valido\n");}
        enterCont();
      break;
      case 4:
        printf("Ingrese el nombre de la mascota: ");
        scanf(" %s", info->nombre);
        info->opcion = opt;
        r = send(clientfd,info,TRANSFSIZE,0);//enviar el comando a realizar
        if(r == -1){
            perror("mal:");
        exit(-1);
        }
        r = recv(clientfd,&resp,4,0); //recibe total
        if(r ==-1){
            perror("receive:");
        exit(-1);
        }
        if(resp == 0){
          printf("No se encontraron registros\n");
          enterCont();
          break;
        }
        for(i = 0; i < resp; i++){
          r = recv(clientfd,tmp,regSize,0); //recibe la estructura
          if(r ==-1){
            perror("receive:");
            exit(-1);
          }
          mostrar(tmp);
        }
        enterCont();
      break;
      case 5:
        //update(indice, pos);
        salir = 0;
      break;
      default:
        printf("Ingrese una opcion valida\n");
    }

  }while(salir != 0);

}
