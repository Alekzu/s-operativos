#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <strings.h>

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
void liberar(int reg, int *pos){
  int i;
  pos[2] = pos[2]+1; //nueva posicion disponible
  pos[1] = pos[1]-1; // un registro menos
  //if(pos[2]>=990){} rehash()}
  for(i = 9; i < 1000; i++){
    if (pos[i] == 0){ //buscar unindice libre en pos para ubicar la direccion
        pos[i] = reg; //añade direccion disponible
        return;
    }
  }
}
void update(int *indi, int *pos){ //escribe el indice y pos en archivo
  FILE *fp;
  fp = fopen("dataDogs.dat", "r+");
  fseek(fp, 0, SEEK_SET);
  fwrite(indi, 1000*sizeof(int), 1,fp); //actualiza el indice
  fseek(fp, 4000, SEEK_SET);
  fwrite(pos, 1000*sizeof(int), 1,fp); //actualiza el arreglo pos
  fclose(fp);
}

void ingresar(int *indi, int *pos){
  //recibe los datos deun nuevo registro, lo indexa y escribe en el archivo
    struct dogType *dato = malloc(sizeof(struct dogType));
    struct dogType *archivo = malloc(sizeof(struct dogType));
    int key, actual, posicion;

    if (dato == NULL | archivo == NULL)
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
    printf("\n ingrese sexo: ");
    scanf(" %c", &dato->sexo);
    dato->next = -1; //siguiente nodo nulo
    dato->prev = -1; //asignacion temporal

    FILE *fp;
    fp = fopen("dataDogs.dat", "r+");
    key = hash(dato->nombre); //encontrar el hash del nombre
    posicion = poslibre(pos);
    dato->state = key;
    if(posicion == 0){
      printf("error de posicion \n");
      return;
    }
    if (indi[key] == 0) // si no existe, inicia la lista
    {
       indi[key] = posicion;
       printf("nuevo registro nº: %i\n",posicion);
       fseek(fp, posicion, SEEK_SET); //ubica el indicador en la posicion libre
       fwrite(dato, sizeof(struct dogType), 1,fp); //escribir el dato en la posicion libre
       pos[1] =pos[1]+1; //+1 registros en total
       update(indi,pos); //actualiza
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
                fwrite(dato, sizeof(struct dogType), 1,fp); //escribir el dato en la posicion libre
                pos[1] =pos[1]+1; //+1 registros en total
                //printf("base: %i\n nuevo: %i anterior: %i",indi[key],posicion,siguiente);
                update(indi,pos);
                break;
            }
            // apuntar al siguiente
            actual = archivo->next;
        }
    }
    fclose(fp);
    free(dato);
    free(archivo);
}
void mostrar(void *ap){
  //recibe la direccion de memoria con un registro, y lo muestra en pantalla
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
void buscar(int *indi, int *pos, char bnombre[36]){
  //busca el registro por nombre y muestra todos los que existan
  int key, res, siguiente, num;
  num = 0;
  struct dogType *dato = malloc(sizeof(struct dogType));
  key = hash(bnombre); //obtiene el hash del nombre a buscar
  siguiente = indi[key]; //obtiene la direccion en indice usando el hash
  if(siguiente == 0){
    printf("No existe el registro\n");
    return;
  }
  FILE *fp;
  fp = fopen("dataDogs.dat", "r");
  if(fp == NULL){
    printf("Error abriendo el archivo\n");
    return;
  }

  while (siguiente != -1 && siguiente != 0)
  {
    fseek(fp, siguiente, SEEK_SET);  //se ubica en la direccion
    fread(dato, sizeof(struct dogType),1,fp); //lee el registro
    res = strcasecmp(dato->nombre,bnombre); //compara ignorando mayusculas
    //printf("pos: %i\n",siguiente);
    if(res == 0){  // si son iguales, muestra el registro
      printf("Registro nº: %i\n", siguiente);
      mostrar(dato);
      num++;  //control de registros con el mismo nombre
    }
    siguiente = dato->next; //se mueve al siguiente mienbro de la cadena enlazada
  }

  if(num == 0){ //no imprimio ningun resultado
    printf("No existe el registro\n");
  }
  fclose(fp);
}

void eliminar(int *indi, int *pos){
  int reg, key;
  FILE *fp;
  printf("Numero de registros actual:%i\n", pos[1]);
  printf("Ingrese el numero del registro a eliminar: ");
  scanf(" %i", &reg);
  if(reg<8000 || reg>pos[0]){  //verifica que el numero de registro exista
    printf("Ingrese un numero de registro valido \n");
    return;
  }
  struct dogType *dato = malloc(sizeof(struct dogType));
  struct dogType *ante = malloc(sizeof(struct dogType));
  struct dogType *sig = malloc(sizeof(struct dogType));
  fp = fopen("dataDogs.dat", "r");
  fseek(fp, reg, SEEK_SET); //apuntar a la direccion de registro
  fread(dato, sizeof(struct dogType),1,fp); //leer el registro
  key = hash(dato->nombre);
  if(dato->state == key){ //comprueba que registro no contiene basura
    if(dato->prev>=8000 && dato->next>=8000){ //comprueba si existe un registro anterior y uno siguiente
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
      liberar(reg, pos); //registro disponible para escritura
      update(indi,pos);
      printf("Registro borrado\n");
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
        liberar(reg, pos); //registro disponible para escritura
        update(indi,pos);
        printf("Registro borrado\n");
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
        liberar(reg, pos); //registro disponible para escritura
        update(indi,pos);
        printf("Registro borrado\n");
      }
      if(dato->next ==-1 && dato->prev == -1){//es el unico registro con ese indice
        indi[key] = 0;
        fp = fopen("dataDogs.dat", "r+");
        //borrar
        dato->state = -1; //para indicar registro vacio
        fseek(fp, reg, SEEK_SET);
        fwrite(sig, sizeof(struct dogType), 1,fp);
        fclose(fp);
        liberar(reg, pos); //registro disponible para escritura
        update(indi,pos);
        printf("Registro borrado\n");
      }
    }
  }
  else{
    printf("No existe el registro\n");
  }
  free(dato);
  free(ante);
  free(sig);
}

void enterCont(){ //pedir tecla para continuar
  char cont;
  printf("presione enter para continuar \n" );
  scanf(" %c", &cont);
}

void mos(int pos){
  //recibe una posicion de memoria, y muestra el registro en esa posicion
  struct dogType *dato = malloc(sizeof(struct dogType));
  FILE *fp;
  fp = fopen("dataDogs.dat", "r");
  fseek(fp, pos, SEEK_SET);
  fread(dato, sizeof(struct dogType),1,fp); //lee el registro
  mostrar(dato);
  fclose(fp);
  free(dato);
}

void main(){
  int *indice, *pos, salir, opt, reg;
  char bNombre[36];
  indice = malloc(1000 * sizeof(int)); //indice por nombre
  pos = malloc(1000 * sizeof(int));   //posiciones libres, total de registros y registros libres
  iniciar(indice, pos);
  salir = 1; //variable de control
  //menu
  do {
    printf("Veterinaria\n 1. Ingresar \n 2. Ver\n 3. Borrar\n 4. Buscar\n 5. Salir\n");
    scanf("%i",&opt);
    switch (opt) {
      case 1:
        printf("Ingreso de registro\n");
        ingresar(indice, pos);
        enterCont();
      break;
      case 2:
        printf("Nº registro para ver: ");
        scanf(" %i", &reg);
        mos(reg);
        enterCont();
      break;
      case 3:
        eliminar(indice, pos);
        enterCont();
      break;
      case 4:
        printf("Ingrese el nombre de la mascota: ");
        scanf(" %s", bNombre);
        buscar(indice, pos, bNombre);
        enterCont();
      break;
      case 5:
        //update(indice, pos);
        salir = 0;
      break;
      //default:
        //printf("Ingrese una opcion valida\n");
    }

  }while(salir != 0);

  free(indice);
  free(pos);

}
