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
#include "Estructuras/manejo_semaforos.h"

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
/*
 * Prototipos
 */
int cargar_archivo_configuracion();
void  SIGhandler(int sig);
//aux
int global_sts=0;
int global_procer=0;
int global_iot=0;
//SEMAFOROS *************************************
int semaforos;
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
	//aux
	cant_iot_disponibles=1;
   int i;
   pthread_t LTS_hilo;//Declaracion del hilo de LTP ( Planificador a largo plazo )
   pthread_t STS_hilo;//Declaracion del hilo de LTP ( Planificador a largo plazo )
   pthread_t PROCER_hilo;//Declaracion del hilo de LTP ( Planificador a largo plazo )

   semaforos=inicializar_semaforos(1);

   cargar_archivo_configuracion();

   //CREACION DE LOS HILOS DEL PP.
   //TODO agregar validaciones a los hilos.

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
   			signal(SIGUSR1, SIGhandler);
   		sleep(1);
   		}

   pthread_exit(NULL);// Última función que debe ejecutar el main() siempre
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
		if( strstr(linea,"mpp")){
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
		if( strstr(linea,"espera_estandar")){
			valor = strtok(linea," ");
			valor = strtok(NULL,";");
			if( valor != NULL ){
				espera_estandar=valor;
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
}

