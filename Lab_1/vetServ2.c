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
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <netdb.h>
#include <netinet/in.h>// addr socket
#define regSize sizeof(struct dogType)
#define TRANSFSIZE sizeof(struct transfer)
#define BACKLOG 8
#define SEMINIT 1

sem_t *semaforo;

static volatile int keepRunning = 1; //para ctrl + c

void intHandler(int dummy) {
    keepRunning = 0;
}

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
//funciones
int hash(char name[32])
//recibe el nombre, lo vuelve valor numerico y calcula el hash
{
  long int numeric = strtol(name,NULL,36); //convierte el string a long int
  int key = numeric % 997; //obtiene el hash para ese numero
  return key;
}

void iniciar(int *indice, int *pos){ //crea el archivo .dat o lo lee si existe
  FILE *fp;
  fopen("dataDogs.dat", "r"); //lee el archivo, si existe
  if(fopen("dataDogs.dat", "r") != NULL){ //si existe, carga en memoria el indice y las posiciones libres
    fp = fopen("dataDogs.dat", "r");
    printf("archivo cargado\n");
    fseek(fp, 0, SEEK_SET);
    fread(indice, 1000*sizeof(int),1,fp);
    fseek(fp, 4000, SEEK_SET);
    fread(pos, 1000*sizeof(int),1,fp);
    fclose(fp);
  }
  else{     //si no existe, crea el archivo e inicializa los indices
    printf("archivo creado \n");
    fp = fopen("dataDogs.dat", "w"); //crea el archivo
    fp = fopen("dataDogs.dat", "r+"); //ecribir sin borrar
    pos[0] = 8000; //establece el inicio de las estructuras
    pos[1] = 0;   //aqui se guardará el total de registros
    pos[2] = 0;   //aqui se lleva la cuenta de registros liberados
    fseek(fp, 0, SEEK_SET);
    fwrite(indice, 1000*sizeof(int), 1,fp); //escribe el indice en archivo
    fseek(fp, 4000, SEEK_SET);
    fwrite(pos, 1000*sizeof(int), 1,fp); //escribe pos en archivo
    fclose(fp);
  }
}

void reload(int *indice, int *pos){
  FILE *fp;
  fp = fopen("dataDogs.dat", "r");
  //printf("archivo cargado\n");
  fseek(fp, 0, SEEK_SET);
  fread(indice, 1000*sizeof(int),1,fp);
  fseek(fp, 4000, SEEK_SET);
  fread(pos, 1000*sizeof(int),1,fp);
  fclose(fp);
}

int poslibre(int *pos){
  //encuentra una posicion libre en el arreglo para escribir un nuevo registro
  int i, newpos;
  if(pos[2] != 0) { //comprueba que existan posiciones libres
    for(i = 9; i < 1000; i++){ //recorre el arreglo hasta encontrar una posicion libre
      if (pos[i] != 0){
          newpos = pos[i];
          pos[i] = 0;    //marca la posicion como usada
          pos[2] = pos[2] - 1; //menos una posicion sin usar
          return newpos;
      }
    }
  }
  else{ //si no hay posiciones libres, escribe al final del archivo
    newpos = pos[0]; //pos[0] indica el final del archivo
    pos[0] = pos[0] + 108; //final del archivo actualizado
    return newpos;
  }
}

void update(int *indi, int *pos){ //escribe el indice y pos en archivo
  FILE *fp;
  fp = fopen("dataDogs.dat", "r+");
  if (fp == NULL){
    perror("Error de archivo: ");
    return;
  }
  fseek(fp, 0, SEEK_SET);
  fwrite(indi, 1000*sizeof(int), 1,fp); //actualiza el indice
  fseek(fp, 4000, SEEK_SET);
  fwrite(pos, 1000*sizeof(int), 1,fp); //actualiza el arreglo pos
  fclose(fp);
}

int ingresar(int *indi, int *pos, void *ap){
  //recibe los datos de un nuevo registro, lo indexa y escribe en el archivo
    struct dogType *dato;
    dato = ap;
    struct dogType *archivo = malloc(sizeof(struct dogType));
    int key, actual, posicion, resp, r;
    if (dato == NULL | archivo == NULL)
    {
        return 0;
    }
    reload(indi,pos);

    FILE *fp;
    fp = fopen("dataDogs.dat", "r+");
    if (fp == NULL){
      perror("Error de archivo: ");
      return 0;
    }
    key = hash(dato->nombre); //encontrar el hash del nombre
    posicion = pos[0]; //pos[0] indica el final del archivo
    //pos[0] = pos[0] + regSize; //nuevo final del archivo
    dato->state = key;
    if(posicion == 0){
      printf("error de posicion \n");
      return 0;
    }
    if (indi[key] == 0) // si no existe uno con ese hash, inicia la lista
    {
       indi[key] = posicion;
       printf("nuevo registro nº: %i\n",posicion);
       fseek(fp, posicion, SEEK_SET); //ubica el indicador en la posicion libre
       r = fwrite(dato, sizeof(struct dogType), 1,fp); //escribir el dato en la posicion libre
       if(r == 0){
         printf("error de escritura \n");
         return 0;
       }
       pos[1] =pos[1]+1; //+1 registros en total
       pos[0] = pos[0] + regSize; //nuevo final del archivo
       fclose(fp);
       update(indi,pos); //actualiza
       printf("Registro guardado con exito \n");
       resp = posicion;
    }
    else
    {
        actual = indi[key]; //primero en la lista del hash

        while (actual != 0) //recorre la lista enlazada
        {   //printf("next: %i\n",actual);
            fp = fopen("dataDogs.dat", "r");
            fseek(fp, actual, SEEK_SET); //apuntar a la direccion siguiente en la lista
            fread(archivo, sizeof(struct dogType),1,fp); //leer el registro
            // insertar si es la cola
            if (archivo->next == -1)
            {
                archivo->next = posicion; //ubicar el nuevo nodo en la cola
                dato->prev = actual; //el nuevo nodo apunta al anterior en prev
                fp = fopen("dataDogs.dat", "r+");
                fseek(fp, actual, SEEK_SET);
                fwrite(archivo, sizeof(struct dogType), 1,fp); //reescribir el anterior con el nuevo next
                fseek(fp, posicion, SEEK_SET); //ubica el indicador en la posicion libre
                r = fwrite(dato, sizeof(struct dogType), 1,fp); //escribir el dato en la posicion libre
                if(r == 0){
                  printf("error de escritura \n");
                  return 0;
                }
                pos[1] =pos[1]+1; //+1 registros en total
                pos[0] = pos[0] + regSize; //nuevo final del archivo
                fclose(fp);
                update(indi,pos);
                printf("Registro guardado con exito \n");
                resp = posicion;
                break;
            }
            // apuntar al siguiente
            actual = archivo->next;
        }
    }
    if(actual == 0){fclose(fp);}
    //free(dato);
    free(archivo);
    return resp;
}

void *buscar(int *indi, int *pos, char bnombre[36], int *total){
  //busca el registro por nombre y guarda los que coincidan con el nombre
  //en un arreglo de estructuras
  int key, res, siguiente, num, i, salir;
  salir = 1;
  num = 0;
  struct dogType *dato = malloc(sizeof(struct dogType));
  struct dogType *lista;
  reload(indi,pos);
  key = hash(bnombre); //obtiene el hash del nombre a buscar
  siguiente = indi[key]; //obtiene la direccion en indice usando el hash
  if(siguiente == 0){
    printf("No existe el registro\n");
    return NULL;
  }
  FILE *fp;
  fp = fopen("dataDogs.dat", "r");
  if(fp == NULL){
    printf("Error abriendo el archivo\n");
    return NULL;
  }
  lista = malloc(regSize*100);//temporal
  i = 0;

  while (siguiente != -1 && siguiente != 0)
  {
    fseek(fp, siguiente, SEEK_SET);  //se ubica en la direccion
    fread(dato, sizeof(struct dogType),1,fp); //lee el registro
    res = strcasecmp(dato->nombre,bnombre); //compara ignorando mayusculas
    if(res == 0){  // si son iguales, muestra el registro
      //printf("uno: %i \n",dato->state);
      //printf("while siguiente: %i \n",siguiente);
      lista[i] = *dato;
      num++;  //control de registros con el mismo nombre
      i++;
    }
    siguiente = dato->next; //se mueve al siguiente mienbro de la cadena enlazada
  }
  //printf("%i\n",num);
  *total = num;
  if(num == 0){ //no encontro registro que coincidan
    printf("No se encontro registro\n");
    return NULL;
  }

  fclose(fp);
  return lista;
}
void moverUlt(int *indi, int *pos, int reg){
  //mueve el ultimo registro a la posicion del registro borrado y arregla el hash
  FILE *fp;
  int dir;
  struct dogType *anterior = malloc(regSize);
  struct dogType *ultimo = malloc(regSize);
  fp = fopen("dataDogs.dat", "r");
  dir = pos[0]-regSize;
  fseek(fp, dir, SEEK_SET);
  fread(ultimo, sizeof(struct dogType),1,fp);
  if(ultimo->prev >= 8000){//existe un registro previo, arreglar la cadena
    fp = fopen("dataDogs.dat", "r+");
    fseek(fp, ultimo->prev, SEEK_SET);
    fread(anterior, sizeof(struct dogType),1,fp);
    anterior->next = reg; //apuntar a la nueva ubicacion del registro
    fseek(fp, ultimo->prev, SEEK_SET);
    fwrite(anterior, sizeof(struct dogType), 1,fp);
    fclose(fp);
  }
  //si es el primero o unico registro con ese hash, solo se debe copiar en
  //la posicion del registro borrado
  fp = fopen("dataDogs.dat", "r+");
  fseek(fp, reg, SEEK_SET);
  fwrite(ultimo, regSize, 1,fp);
  fclose(fp);
  pos[0] = pos[0]-regSize; //final del archivo un registro antes
  pos[1] = pos[1]-1; //ahora hay un registro menos
  update(indi,pos);
}
void newfile(int *indi, int *pos){
  //copia el archivo a uno nuevo y lo reemplaza
  FILE *fp;
  FILE *newfp;
  int i, dir, ini, fin;
  struct dogType *oldReg = malloc(regSize);
  fp = fopen("dataDogs.dat","a+");
  newfp = fopen("newDataDogs.dat","a+");
  //copiar el indice y pos en archivo nuevo
  fwrite(indi, 1000*sizeof(int), 1,newfp); //escribe el indice en archivo
  fwrite(pos, 1000*sizeof(int), 1,newfp); //escribe pos en archivo

  //copiar todos los registros
  for (i = 0; i <= pos[1]; i++ ){
    dir = 8000 + regSize*i;
    fseek(fp,dir,SEEK_SET);
    fread(oldReg, regSize,1,fp);//leer archivo antiguo
    //fseek(newfp,dir,SEEK_SET);
    fwrite(oldReg, regSize, 1,newfp); //escribir en el nuevo
  }
  free(oldReg);
  fclose(fp);
  fclose(newfp);
  remove("dataDogs.dat");
	rename("newDataDogs.dat" , "dataDogs.dat");
}

int eliminar(int *indi, int *pos, int reg){
  //recibe un numero de registro y lo borra del archivo
  int key, resp;
  FILE *fp;

  if(reg<8000 || reg>pos[0]){  //verifica que el numero de registro exista
    //printf("\n Ingrese un numero de registro valido \n");
    return 0;
  }
  struct dogType *dato = malloc(sizeof(struct dogType));
  struct dogType *ante = malloc(sizeof(struct dogType));
  struct dogType *sig = malloc(sizeof(struct dogType));
  reload(indi, pos);
  fp = fopen("dataDogs.dat", "r");
  fseek(fp, reg, SEEK_SET); //apuntar a la direccion de registro
  fread(dato, sizeof(struct dogType),1,fp); //leer el registro
  key = hash(dato->nombre);
  if(dato->state == key){ //comprueba que registro no contiene basura
    if(dato->prev>=8000 && dato->next>=8000){ //existe un registro anterior y uno siguiente
      fp = fopen("dataDogs.dat", "r+");
      //actualizar el registro anterior
      fseek(fp, dato->prev, SEEK_SET);
      fread(ante, sizeof(struct dogType),1,fp);
      ante->next = dato->next;  //enlaza el anterior al siguiente
      fseek(fp, dato->prev, SEEK_SET);
      fwrite(ante, sizeof(struct dogType), 1,fp); //reescribir registro anterior
      //actualizar el registro siguiente
      fseek(fp, dato->next, SEEK_SET);
      fread(sig, sizeof(struct dogType),1,fp);
      sig->prev = dato->prev;  //enlaza el siguiente al anterior
      fseek(fp, dato->next, SEEK_SET);
      fwrite(sig, sizeof(struct dogType), 1,fp); //reescribir registro siguiente
      //borrar el registro
      dato->state = -1; //para indicar registro vacio
      fseek(fp, reg, SEEK_SET);
      fwrite(sig, sizeof(struct dogType), 1,fp);
      fclose(fp);
      moverUlt(indi,pos,reg); //llama a la funcion que mueve el ultimo registro
      newfile(indi,pos); //llama a la funcion que crea el archivo actualizado
      //liberar(reg, pos); //registro disponible para escritura
      //update(indi,pos);
      //printf("Registro borrado\n");
      resp = 1;
    }
    else{
      if(dato->prev>=8000){ //quiere decir que es el ultimo
        fp = fopen("dataDogs.dat", "r+");
        //prev
        fseek(fp, dato->prev, SEEK_SET);
        fread(ante, sizeof(struct dogType),1,fp);
        ante->next = -1;  //el anterior ahora sera el ultimo
        fseek(fp, dato->prev, SEEK_SET);
        fwrite(ante, sizeof(struct dogType), 1,fp); //reescribir registro anterior
        //borrar
        dato->state = -1; //para indicar registro vacio
        fseek(fp, reg, SEEK_SET);
        fwrite(sig, sizeof(struct dogType), 1,fp);
        fclose(fp);
        moverUlt(indi,pos,reg); //llama a la funcion que mueve el ultimo registro
        newfile(indi,pos); //llama a la funcion que crea el archivo actualizado
        //liberar(reg, pos); //registro disponible para escritura
        //update(indi,pos);
        //printf("Registro borrado\n");
        resp = 1;
      }
      if(dato->next>=8000){ //quiere decir que es el primero del indice
        indi[key] = dato->next; //nuevo inicio para indice del hashtable
        fp = fopen("dataDogs.dat", "r+");
        //next
        fseek(fp, dato->next, SEEK_SET);
        fread(sig, sizeof(struct dogType),1,fp);
        sig->prev = -1;  //ahora sera el primero
        fseek(fp, dato->next, SEEK_SET);
        fwrite(sig, sizeof(struct dogType), 1,fp); //reescribir registro siguiente
        //borrar
        dato->state = -1; //para indicar registro vacio
        fseek(fp, reg, SEEK_SET);
        fwrite(sig, sizeof(struct dogType), 1,fp);
        fclose(fp);
        moverUlt(indi,pos,reg); //llama a la funcion que mueve el ultimo registro
        newfile(indi,pos); //llama a la funcion que crea el archivo actualizado
        //liberar(reg, pos); //registro disponible para escritura
        //update(indi,pos);
        //printf("Registro borrado\n");
        resp = 1;
      }
      if(dato->next ==-1 && dato->prev == -1){//es el unico registro con ese indice
        indi[key] = 0;
        fp = fopen("dataDogs.dat", "r+");
        //borrar
        dato->state = -1; //para indicar registro vacio
        fseek(fp, reg, SEEK_SET);
        fwrite(sig, sizeof(struct dogType), 1,fp);
        fclose(fp);
        moverUlt(indi,pos,reg); //llama a la funcion que mueve el ultimo registro
        newfile(indi,pos); //llama a la funcion que crea el archivo actualizado
        //liberar(reg, pos); //registro disponible para escritura
        //update(indi,pos);
        //printf("Registro borrado\n");
        resp = 1;
      }
    }
  }
  else{
    resp = -1;//printf("No existe el registro\n");
  }
  free(dato);
  free(ante);
  free(sig);
  return resp;
}

int mos(int *indi, int *pos, int reg, void *ap){
  //recibe una posicion de memoria, y muestra el registro en esa posicion
  int key, resp;
  struct dogType *dato;
  dato = ap;
  reload(indi,pos);
  if(reg<8000 || reg>pos[0]){  //verifica que el numero de registro exista
    //printf("Ingrese un numero de registro valido \n");
    dato->state = -1;
    return -1;
  }

  FILE *fp;
  fp = fopen("dataDogs.dat", "r");
  fseek(fp, reg, SEEK_SET);
  fread(dato, sizeof(struct dogType),1,fp); //lee el registro
  key = hash(dato->nombre);
  if(dato->state == key){ //comprueba que registro no contiene basura
	printf("Registro nº: %i \n",reg);
	//mostrar(dato);
  resp = 1;
  }
  else{
    //printf("No existe el registro \n");
    dato->state = -1;
    resp = -1;
  }
  fclose(fp);
  //free(dato);
  return resp;
}
void *conexion(void *socket_desc)
{
  int *indice, *pos, salir, opt, reg, errv, r,clientfd, ctrl, *total, i;
  clientfd = *(int*)socket_desc;
  char bNombre[36];
  struct dogType *tmp = malloc(sizeof(struct dogType));
  struct transfer *info = malloc(TRANSFSIZE);
  struct dogType *lista;
  indice = malloc(1000 * sizeof(int)); //indice por nombre
  pos = malloc(1000 * sizeof(int));   //posiciones libres, total de registros y registros libres
  total = malloc(sizeof(int));
  iniciar(indice, pos);
  salir = 1; //variable de control

  while(keepRunning){ //loop hasta que se presione ctrl + c
    r = recv(clientfd,info,TRANSFSIZE,0);
    if(r ==-1){
        perror("receive:");
    exit(-1);
    }
    opt = info->opcion;
    switch (opt) {
      case 1: //ingreso
        r = recv(clientfd,tmp,regSize,0);
        if(r ==-1){
            perror("receive:");
        exit(-1);
        }
        ctrl = ingresar(indice, pos, tmp);
        r = send(clientfd,&ctrl,4,0);
        if(r == -1){
            perror("mal:");
        exit(-1);
        }
      break;
      case 2: //mostrar
        ctrl =mos(pos,info->reg,tmp);
        if(ctrl != 1){ //el registro no existe
          tmp->next = 0;
        }
        r = send(clientfd,tmp,regSize,0);
        if(r == -1){
            perror("mal:");
        exit(-1);
        }
      break;
      case 3: //eliminar
        r = send(clientfd,&pos[1],4,0); //envia la cantidad de registros
        if(r == -1){
            perror("mal:");
        exit(-1);
        }
        r = recv(clientfd,info,TRANSFSIZE,0); //recibe el reg a eliminar
        if(r ==-1){
            perror("receive:");
        exit(-1);
        }
        printf("eliminar: %i\n",info->reg);
        ctrl = eliminar(indice, pos, info->reg);
        r = send(clientfd,&ctrl,4,0); //envia el resultado
        if(r == -1){
            perror("mal:");
        break;
        }
      break;
      case 4: //buscar
        //sem_wait(semaforo);
        lista = buscar(indice, pos, info->nombre, total);
        //sem_post(semaforo);
        if (lista == NULL)
        {
            *total = 0;
        }
        r = send(clientfd,total,4,0); //envia el total
        if(r == -1){
            perror("mal:");
        break;
        }
        if(*total > 0){
          for(i = 0; i < *total; i++){
            tmp = &lista[i];
            r = send(clientfd,tmp,regSize,0); //envia un registro
            if(r == -1){
                perror("mal:");
            break;
            }
          }
        }
        free(lista);
      break;
      case 5: //salir
        //update(indice, pos);
        salir = 0;
      break;
      default:
        printf("...\n");
    }

  }
  close(clientfd);
  free(tmp);
  free(total);
  free(lista);
  free(info);
  free(indice);
  free(pos);
}

int main(){
  int svfd, r,clientfd, ctrl, client_sock, i;
  int sockfd, *new_sock;
  int option = 1;

  //semaforo
  //semaforo = sem_open("semaforo1",O_CREAT, 0700, SEMINIT);

  //iniciar servidor
  struct sockaddr_in server, cliente;
  socklen_t tamanio, tamac;
//socket
  svfd = socket(AF_INET,SOCK_STREAM,0); // socket para servidor
  if(svfd ==-1){
      perror("mal:");
  exit(-1);
  }
  setsockopt(svfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)); //reutilizar el socket
  //datos del socket
  server.sin_family = AF_INET;
  server.sin_port = htons(3535);//indianism converter (memory architecture intel/arm)
  server.sin_addr.s_addr = INADDR_ANY; //para que escuche cualquier interfaz de red
  bzero(server.sin_zero,8);//llenar 8 bit con ceros
  tamanio = sizeof(struct sockaddr_in);
  //bind
  r = bind(svfd, (struct sockaddr*)&server, tamanio);
  if(r == -1){
      perror("mal:");
  exit(-1);
  }
//LISTEN
  r = listen(svfd,BACKLOG);
  if(r == -1){
      perror("mal:");
  exit(-1);
  }
  pthread_t t[20]; //added
  tamac = 0; //si no se inicializa, da error
  //accept recibe la conexion con el socket del servidor,
  //guarda los datos del cliente (ip,etc)
  //crea un socket exclusivo para el cliente

  //multithread
    /*while(client_sock = accept(svfd, (struct sockaddr *)&cliente,&tamac))
        {
            puts("Connection accepted");
            pthread_t curr_thread;
            new_sock = malloc(sizeof(int));
            *new_sock = client_sock;

            if( pthread_create( &curr_thread , NULL ,  conexion , (void*) new_sock) < 0)
            {
                perror("could not create thread");
                return 1;
            }

            //Now join the thread , so that we dont terminate before the thread
            //pthread_join( thread_id , NULL);
            puts("Handler assigned");
        }*/
    while(1)        //loop infinito
      {
       for(i=0;i<20;i++)      //can support 20 clients at a time
       {
        client_sock=accept(svfd,NULL,NULL);
        printf("Connected to client %d\n",client_sock);
        pthread_create(&t[i],NULL, conexion, (void *)&client_sock);
       }
      }

        if (client_sock < 0)
        {
            perror("accept failed");
            return 1;
        }
    close(svfd);
    //sem_close(semaforo);
    //sem_unlink("semaforo1");
    //close(clientfd);
    //close(svfd);

    return 0;
}
