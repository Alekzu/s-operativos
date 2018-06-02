#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <time.h>


struct dogType{
    char nombre[32];
    char tipo[32];
    int32_t edad;
    char raza[16];
    int32_t estatura;
    float peso;
    char sexo;
    int next;
    int prev;
    int state;
//size 108 bytes
};

int hash(char name[32])
{
  long int numeric = strtol(name,NULL,36); //convierte el string a long int
  int key = numeric % 997; //obtiene el hash para ese numero
  if(key>999) key = 999;
  return key;
}
void iniciar(int *indice, int *pos){
  FILE *fp;
  //fopen("dataDogs.dat", "r"); //lee el archivo, si existe
  if(fopen("dataDogs.dat", "r") != NULL){ //si existe, carga en memoria el indice y las posiciones libres
    //fclose(fp);
    fp = fopen("dataDogs.dat", "r");
    if (fp == NULL){
      perror("Error de carga de archivo: ");
      return;
    }
    fseek(fp, 0, SEEK_SET);
    fread(indice, 1000*sizeof(int),1,fp);
    fseek(fp, 4000, SEEK_SET);
    fread(pos, 1000*sizeof(int),1,fp);
    fclose(fp);
    printf("archivo cargado\n");
  }
  else{     //si no, crea el archivo e inicializa los indices
    //fp = fopen("dataDogs.dat", "w"); //crea el archivo
    fp = fopen("dataDogs.dat", "a+"); //ecribir sin borrar
    if (fp == NULL){
      perror("Error de creacion archivo: ");
      return;
    }
    pos[0] = 8000; //establece el inicio de las estructuras
    pos[1] = 0;   //aqui se guardará el total de registros
    pos[2] = 0;   //aqui se lleva la cuenta de registros liberados
    //fseek(fp, 0, SEEK_SET);
    fwrite(indice, 1000*sizeof(int), 1,fp); //escribe el indice
    //fseek(fp, 4000, SEEK_SET);
    fwrite(pos, 1000*sizeof(int), 1,fp); //escribe pos
    fclose(fp);
    printf("archivo creado \n");
  }
}
void reload(int *indice, int *pos){
  FILE *fp;
  fp = fopen("dataDogs.dat", "r");
  if (fp == NULL){
    perror("Error reload: ");
    return;
  }
  //printf("archivo cargado\n");
  fseek(fp, 0, SEEK_SET);
  fread(indice, 1000*sizeof(int),1,fp);
  fseek(fp, 4000, SEEK_SET);
  fread(pos, 1000*sizeof(int),1,fp);
  fclose(fp);
}
void update(int *indi, int *pos){ //actualiza el archivo
  FILE *fp;
  fp = fopen("dataDogs.dat", "r+");
  if (fp == NULL){
    perror("Error de update: ");
    return;
  }
  fseek(fp, 0, SEEK_SET);
  fwrite(indi, 1000*sizeof(int), 1,fp); //actualiza el indice
  fseek(fp, 4000, SEEK_SET);
  fwrite(pos, 1000*sizeof(int), 1,fp); //actualiza el arreglo pos
  fclose(fp);
}
int poslibre(int *pos){ //encuentra una posicion libre en el arreglo
  int i, newpos;
    newpos = pos[0]; //pos[0] indica el final del archivo
    pos[0] = pos[0] + 108; //nueva posicion libre
    return newpos;
}

void llenar(int *indi, int *pos){
    struct dogType *dato = malloc(sizeof(struct dogType));
    struct dogType *archivo = malloc(sizeof(struct dogType));
    if (dato == NULL || archivo == NULL)
    {
      printf("error de memoria");
        return;
    }
    int key, actual, posicion, i, r;
    reload(indi,pos);
    //generar
    srand(time(NULL));
      char names[2000][32];
      char tmp[32];
    FILE *fnom,*fp;
      fnom = fopen("nombresMascotas.txt","r");
      int cn = 0;
      while(fscanf(fnom,"%s",names[cn])!=EOF)
          cn++;

      fclose(fnom);
      printf("names loaded \n");
      r = 0;
      fp = fopen("dataDogs.dat", "r+");
      if (fp == NULL){
        perror("0.Error de archivo: ");
        return;
      }
      for (i = 0 ; i < 1000000; i++)
      {
          r = rand()%cn;
          if(r>1999||r<0)r = 1500;
          strcpy(dato->nombre,names[r]);
          //int test = hash(reg->nombre);
          //printf("nombre:%s\n", names[r]);
          //printf("hash:%i\n", test);
          strcpy(dato->tipo, "Perro");
          dato->edad = rand()%12;
          dato->sexo = ( (rand()%2) ? 'M' : 'F');
          float w = (rand()% 600) + 1;
          w /= 10.0;
          dato->peso = w;
          dato->estatura = (rand() % 120) + 10;
          int ind = rand() % 7;
          switch (ind)
          {
              case 0:
                  strcpy(dato->raza,"Salchicha");
                  break;
              case 1:
                  strcpy(dato->raza,"Shitzu");
                  break;
              case 2:
                  strcpy(dato->raza,"Pitbull");
                  break;
              case 3:
                  strcpy(dato->raza,"Criollo");
                  break;
              case 4:
                  strcpy(dato->raza,"PastorAleman");
                  break;
              case 5:
                  strcpy(dato->raza,"Golden");
                  break;
              case 6:
                  strcpy(dato->raza,"Pointer");
                  break;
              default:
                  strcpy(dato->raza,"Unknown");
          }


          //escribir


          dato->next = -1; //siguiente nodo nulo
          dato->prev = -1; //asignacion temporal

          //FILE *fp;
          /*fp = fopen("dataDogs.dat", "r+");
          if (fp == NULL){
            perror("0.Error de archivo: ");
            return;
          }*/
          key = hash(dato->nombre); //encontrar el hash del nombre
          //if(key > 999) continue;
          //printf("nombre:%s\n",dato->nombre);
          //printf("hash:%i\n", key);
          posicion = poslibre(pos);
          //printf("posicion:%i\n", posicion);
          dato->state = key;
          if(posicion <= 0){
            printf("error de posicion \n");
            return;
          }
          if (indi[key] == 0) // si no existe, inicia la lista
          {  //printf("nuevo");
             indi[key] = posicion;
             //printf("1- ");
             /*fp = fopen("dataDogs.dat", "r+");
             if (fp == NULL){
               perror("0.Error de archivo: ");
               return;
             }*/
             fseek(fp, posicion, SEEK_SET); //ubica el indicador en la posicion libre
             //printf("2- ");
             fwrite(dato, sizeof(struct dogType), 1,fp); //escribir el dato en la posicion libre
             //printf("3- ");
             pos[1] =pos[1]+1; //+1 registros en total
             //printf("4- ");
             //fclose(fp);
             //printf("nuevo registro nº: %i\n",posicion);
             //update(indi,pos); //actualiza
          }
          else
          {   //printf("existe \n");
              actual = indi[key]; //primero en la lista del hash

              while (actual > 8000) //recorre la lista enlazada
              {   //printf("next: %i\n",actual);
                  /*fp = fopen("dataDogs.dat", "r");
                  if (fp == NULL){
                    perror("1.Error de archivo: ");
                    return;
                  }*/
                  fseek(fp, actual, SEEK_SET); //apuntar a la direccion siguiente en la lista
                  fread(archivo, sizeof(struct dogType),1,fp); //leer el registro
                  //fclose(fp);
                  // insertar si es la cola
                  if (archivo->next == -1)
                  {
                      //printf("escribe: %i \n",posicion);
                      archivo->next = posicion; //ubicar el nuevo nodo en la cola
                      dato->prev = actual; //el nuevo nodo apunta al anterior en prev
                      /*fp = fopen("dataDogs.dat", "r+");
                      if (fp == NULL){
                        perror("2.Error de archivo: ");
                        return;
                      }*/
                      fseek(fp, actual, SEEK_SET);
                      fwrite(archivo, sizeof(struct dogType), 1,fp); //reescribir el anterior con el nuevo next
                      fseek(fp, posicion, SEEK_SET); //ubica el indicador en la posicion libre
                      fwrite(dato, sizeof(struct dogType), 1,fp); //escribir el dato en la posicion libre
                      pos[1] = pos[1]+1; //+1 registros en total
                      //printf("base: %i\n nuevo: %i anterior: %i",indi[key],posicion,siguiente);
                      //fclose(fp);
                      //update(indi,pos);
                      //printf("nuevo registro en nº: %i\n",posicion);
                      break;
                  }
                  // apuntar al siguiente
                  actual = archivo->next;
              }
      }//printf("actual: %i \n",i);
  }

    fclose(fp);
    update(indi,pos);
    free(dato);
    free(archivo);
    printf("total: %i \n",i);
}

int main(void){
  int *indice, *pos, i;
  indice = malloc(1000 * sizeof(int)); //indice por nombre
  pos = malloc(1000 * sizeof(int));   //posiciones libres y otros
  iniciar(indice,pos);
	llenar(indice, pos);
  //update(indice,pos);
  free(indice);
  free(pos);
}
