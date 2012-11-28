/*
 * pp.c
 *
 *  Created on: 26/10/2012
 *      Author: utnso
 */
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/sem.h>
#include <semaphore.h>

/*
 * Headers propios
 */
#include "PP/IOT.h"
#include "PP/LTS.h"
#include "PP/PROCER.h"
#include "PP/STS.h"
#include "FuncionesPropias/manejo_archivos.h"
#include "Estructuras/proceso.h"
#include "Estructuras/colaConeccionesDemoradas.h"
#include "Log/manejo_log.h"
/*
 * Prototipos
 */
int comprobar_archivo_configuracion();
int cargar_archivo_configuracion();
void  SIGhandler(int sig);
//aux
int global_sts=0;
int global_procer=0;
int global_iot=0;
//SEMAFOROS *************************************
int semaforos;
pthread_mutex_t mutexListaNuevos = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexListaReanudados = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexListaSuspendidos = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexListaBloqueados = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexListaFinQuantum = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexListaFinIO = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexListaListos = PTHREAD_MUTEX_INITIALIZER;

	//Variables modificables en tiempo de ejcucion
pthread_mutex_t mutexVarMaxMMP = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexVarMaxMPS = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexVarMMP = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexVarMPS= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexVarLPL= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexVarLPN = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexVarLPR = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexVarFinQuantum = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexVarFinIO = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexVarSuspendido = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexVarEsperaEstandar = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexVarCantIOTDisponibles = PTHREAD_MUTEX_INITIALIZER;

//***************************

//Variables globales del archivo de configuracion
unsigned int max_mps=10; //Maximo de procesos en el sistema
unsigned int max_mmp=10; //Maximo valor de multiprogramacion
char *lpl; //Algoritmo de ordenamiento para lista de procesos listos.
unsigned int quantum_max=2;
char *puerto;
	//Prioridades de los algoritmos.
unsigned int lpn;
unsigned int lpr;
unsigned int finQ;
unsigned int finIO;
char *espera_estandar;
char *espera_estandar_io;
unsigned int cantidad_hilos_iot; //Valor de hilos IOT

//Variables globales propias
int suspendido=0;
unsigned int mps; //Procesos en el sistema
unsigned int mmp; //Valor de multiprogramacion
int cant_iot_disponibles;
unsigned int pid=0;

//Listas de procesos.
nodo_proceso **listaProcesosNuevos;
nodo_proceso **listaProcesosReanudados;
nodo_proceso **listaProcesosSuspendidos;
nodo_proceso **listaFinQuantum;
nodo_proceso **listaFinIO;
nodo_proceso **listaProcesosListos;
nodo_entrada_salida  **listaBloqueados;
coneccionesDemoradas **listaConeccionesDemoradas;


void  SIGhandler(int);


int main(int argc, char *argv[])
{
   int i;
   pthread_t LTS_hilo;//Declaracion del hilo de LTP ( Planificador a largo plazo )
   pthread_t STS_hilo;//Declaracion del hilo de LTP ( Planificador a largo plazo )
   pthread_t PROCER_hilo;//Declaracion del hilo de LTP ( Planificador a largo plazo )

   //Inicializar Semaforos
   pthread_mutex_init(&mutexListaNuevos,NULL);
   pthread_mutex_init(&mutexListaReanudados,NULL);
   pthread_mutex_init(&mutexListaSuspendidos,NULL);
   pthread_mutex_init(&mutexListaBloqueados,NULL);
   pthread_mutex_init(&mutexListaFinQuantum,NULL);
   pthread_mutex_init(&mutexListaFinIO,NULL);
   pthread_mutex_init(&mutexListaListos,NULL);

   pthread_mutex_init(&mutexVarMaxMMP,NULL);
   pthread_mutex_init(&mutexVarMaxMPS,NULL);
   pthread_mutex_init(&mutexVarMMP,NULL);
   pthread_mutex_init(&mutexVarMPS,NULL);
   pthread_mutex_init(&mutexVarLPL,NULL);
   pthread_mutex_init(&mutexVarLPN,NULL);
   pthread_mutex_init(&mutexVarLPR ,NULL);
   pthread_mutex_init(&mutexVarFinQuantum,NULL);
   pthread_mutex_init(&mutexVarFinIO,NULL);
   pthread_mutex_init(&mutexVarSuspendido,NULL);
   pthread_mutex_init(&mutexVarEsperaEstandar,NULL);
   pthread_mutex_init(&mutexVarCantIOTDisponibles,NULL);

   logx("PID=1","2","3","4");

   cargar_archivo_configuracion();
   printf("Cargue el archivo de configuracion\n");

   //CREACION DE LOS HILOS DEL PP.
   //TODO agregar validaciones a los hilos.
   //liberar_semaforo(semaforos,SEM_LISTA_NUEVOS);

   printf("Creando hilo de LTS\n");
   pthread_create(&(LTS_hilo), NULL, LTS_funcion, NULL); // Creamos el thread LTS

   printf("Creando hilo de STS\n");
   pthread_create(&(STS_hilo), NULL, STS_funcion, NULL); // Creamos el thread STS

   printf("Creando hilo de PROCER\n");
   pthread_create(&(PROCER_hilo), NULL, PROCER_funcion, NULL); // Creamos el thread PROCER


   //CREACION DE LOS HILOS IOT
   for (i=0; i < cantidad_hilos_iot; i++){
	   pthread_t IOT_hilo;
	   printf("Creando hilo de IOT numero:%d\n", i+1);
	   pthread_create(&(IOT_hilo), NULL, IOT_funcion, NULL); // Creamos el thread IOT
   }

   //CAPTURAR LA SEÑAL SIGUSR1.
   	   while (1){
   		   comprobar_archivo_configuracion();
   			signal(SIGUSR1, SIGhandler);
   		sleep(1);
   		}

   pthread_exit(NULL);// Última función que debe ejecutar el main() siempre
   return 0;
}

int comprobar_archivo_configuracion(){
	char *nombre_archivo="pp.conf";
	char *texto_del_archivo = leer_archivo(nombre_archivo);
	char *linea;
	char *valor;

	while( texto_del_archivo != NULL){
		linea = strtok(texto_del_archivo,"\n");
		texto_del_archivo = strtok(NULL,"\0");

		if( strstr(linea,"mps")){
			valor = strtok(linea," ");
			valor = strtok(NULL,";");
			if( valor != NULL && max_mps!=atoi(valor)){
				pthread_mutex_lock(&mutexVarMaxMPS);
				max_mps=atoi(valor);
				printf("Cambie el mps %d\n",max_mps);
				pthread_mutex_unlock(&mutexVarMaxMPS);
			}
		}
		if( strstr(linea,"mmp")){
			valor = strtok(linea," ");
			valor = strtok(NULL,";");
			if( valor != NULL && max_mmp!=atoi(valor)){
				pthread_mutex_lock(&mutexVarMaxMMP);
				max_mmp=atoi(valor);
				pthread_mutex_unlock(&mutexVarMaxMMP);
			}
		}

		if( strstr(linea,"lpl")){//Algoritmo de ordenamiento de lista de listos
			valor = strtok(linea," ");
			valor = strtok(NULL,";");
			if( valor != NULL && strcmp(lpl,valor)){
				pthread_mutex_lock(&mutexVarLPL);
				lpl=valor;
				pthread_mutex_unlock(&mutexVarLPL);
			}
		}
		if( strstr(linea,"lpn")){//Prioridad de un proceso NUEVO
			valor = strtok(linea," ");
			valor = strtok(NULL,";");
			if( valor != NULL && lpn!=atoi(valor)){
				pthread_mutex_lock(&mutexVarLPN);
				lpn=atoi(valor);
				pthread_mutex_unlock(&mutexVarLPN);
			}
		}
		if( strstr(linea,"lpr")){//Prioridad de un proceso REANUDADO
			valor = strtok(linea," ");
			valor = strtok(NULL,";");
			if( valor != NULL && lpr != atoi(valor)){
				pthread_mutex_lock(&mutexVarLPR);
				lpr=atoi(valor);
				pthread_mutex_unlock(&mutexVarLPR);
			}


		}
		if( strstr(linea,"finQ")){//Prioridad de un proceso de FIN QUANTUM
			valor = strtok(linea," ");
			valor = strtok(NULL,";");
			if( valor != NULL && finQ != atoi(valor)){
				pthread_mutex_lock(&mutexVarFinQuantum);
				finQ=atoi(valor);
				pthread_mutex_unlock(&mutexVarFinQuantum);
			}
		}
		if( strstr(linea,"finIO")){//Prioridad de un proceso de FIN E/S
			valor = strtok(linea," ");
			valor = strtok(NULL,";");
			if( valor != NULL && finQ != atoi(valor)){
				pthread_mutex_lock(&mutexVarFinIO);
				finIO=atoi(valor);
				pthread_mutex_unlock(&mutexVarFinIO);
			}
		}
		if( strstr(linea,"espera_estandar") && strstr(linea,"espera_estandar_io") == NULL){
			valor = strtok(linea," ");
			valor = strtok(NULL,";");
			if( valor != NULL && strcmp(espera_estandar,valor)!=0){
				pthread_mutex_lock(&mutexVarEsperaEstandar);
				espera_estandar=valor;
				pthread_mutex_unlock(&mutexVarEsperaEstandar);
			}
		}
	}

	return 0;
}

int cargar_archivo_configuracion(){
	char *nombre_archivo="pp.conf";
	char *texto_del_archivo = leer_archivo(nombre_archivo);
	char *linea;
	char *valor;

	listaConeccionesDemoradas=(coneccionesDemoradas **)malloc(sizeof(coneccionesDemoradas));
		bzero(listaConeccionesDemoradas,sizeof(coneccionesDemoradas));

	listaFinIO=(nodo_proceso **)malloc(sizeof(nodo_proceso));
	bzero(listaFinIO,sizeof(nodo_proceso));

	listaProcesosReanudados=(nodo_proceso **)malloc(sizeof(nodo_proceso));
	bzero(listaProcesosReanudados,sizeof(nodo_proceso));

	listaProcesosSuspendidos=(nodo_proceso **)malloc(sizeof(nodo_proceso));
	bzero(listaProcesosSuspendidos,sizeof(nodo_proceso));

	listaFinQuantum=(nodo_proceso **)malloc(sizeof(nodo_proceso));
	bzero(listaFinQuantum,sizeof(nodo_proceso));

	listaProcesosNuevos=(nodo_proceso **)malloc(sizeof(nodo_proceso));
	bzero(listaProcesosNuevos,sizeof(nodo_proceso));

	listaProcesosListos=(nodo_proceso **)malloc(sizeof(nodo_proceso));
	bzero(listaProcesosListos,sizeof(nodo_proceso));

	listaBloqueados=(nodo_entrada_salida **)malloc(sizeof(nodo_entrada_salida));
	bzero(listaBloqueados,sizeof(nodo_entrada_salida));

	while( texto_del_archivo != NULL){
		linea = strtok(texto_del_archivo,"\n");
		texto_del_archivo = strtok(NULL,"\0");

		if( strstr(linea,"mps")){
			valor = strtok(linea," ");
			valor = strtok(NULL,";");
			if( valor != NULL ){
				max_mps=atoi(valor);
				mps=0;
			}
		}
		if( strstr(linea,"mmp")){
			valor = strtok(linea," ");
			valor = strtok(NULL,";");
			if( valor != NULL ){
				max_mmp=atoi(valor);
				mmp=0;
			}
		}

		if( strstr(linea,"lpl")){//Algoritmo de ordenamiento de lista de listos
			valor = strtok(linea," ");
			valor = strtok(NULL,";");
			if( valor != NULL ){
				lpl=valor;
			}
		}
		if( strstr(linea,"quantum_max")){
			valor = strtok(linea," ");
			valor = strtok(NULL,";");
			if( valor != NULL ){
				quantum_max=atoi(valor);
			}
		}
		if( strstr(linea,"lpn")){//Prioridad de un proceso NUEVO
			valor = strtok(linea," ");
			valor = strtok(NULL,";");
			if( valor != NULL ){
				lpn=atoi(valor);
			}
		}
		if( strstr(linea,"lpr")){//Prioridad de un proceso REANUDADO
			valor = strtok(linea," ");
			valor = strtok(NULL,";");
			if( valor != NULL ){
				lpr=atoi(valor);
			}


		}
		if( strstr(linea,"finQ")){//Prioridad de un proceso de FIN QUANTUM
			valor = strtok(linea," ");
			valor = strtok(NULL,";");
			if( valor != NULL ){
				finQ=atoi(valor);
			}
		}
		if( strstr(linea,"finIO")){//Prioridad de un proceso de FIN E/S
			valor = strtok(linea," ");
			valor = strtok(NULL,";");
			if( valor != NULL ){
				finIO=atoi(valor);
			}
		}
		if( strstr(linea,"puerto")){//Puerto del server TCP/IP
			valor = strtok(linea," ");
			valor = strtok(NULL,";");
			if( valor != NULL ){
				puerto=valor;
			}
		}
		if( strstr(linea,"espera_estandar") && strstr(linea,"espera_estandar_io") == NULL){
			valor = strtok(linea," ");
			valor = strtok(NULL,";");
			if( valor != NULL ){
				espera_estandar=valor;
				printf("La espera estandar es %s\n",espera_estandar);
			}
		}
		if( strstr(linea,"espera_estandar_io")){
			valor = strtok(linea," ");
			valor = strtok(NULL,";");
			if( valor != NULL ){
				espera_estandar_io=valor;
			}
		}
		if( strstr(linea,"cantidad_hilos_iot")){
			valor = strtok(linea," ");
			valor = strtok(NULL,";");
			if( valor != NULL ){
				cantidad_hilos_iot=atoi(valor);
				cant_iot_disponibles=atoi(valor);
			}
		}

	}

	return 0;
}
// FUNCION QUE MANEJA LA SEÑAL DEL SIGUSR1 PI.
void  SIGhandler(int sig)
{
	suspendido = 1;
	printf("\nReceived a SIGUSR1.\n");
	printf("USR1, suspendido=%d",suspendido);
}

