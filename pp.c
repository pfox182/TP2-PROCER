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
/*
 * Variables globales
 */
int suspendido=0;
unsigned int mps=5; //Procesos en el sistema
unsigned int mpp=5; //Valor de multiprogramacion
unsigned int max_mps=10; //Maximo de procesos en el sistema
unsigned int max_mpp=10; //Maximo valor de multiprogramacion
unsigned int cantidad_hilos_iot=1; //Valor de hilos IOT
unsigned int pid=0;
char *lpl="FIFO"; //Algoritmo de ordenamiento para lista de procesos listos.
unsigned int quantum_max=2;
char *espera_estandar="60";
char *espera_estandar_io="1";
int cant_iot_disponibles;

//Prioridades de los algoritmos.
unsigned int lpn=1;
unsigned int lpr=2;
unsigned int finQ=4;
unsigned int finIO=3;

//Listas de procesos.
nodo_proceso **listaProcesosNuevos;
nodo_proceso **listaProcesosReanudados;
nodo_proceso **listaProcesosSuspendidos;
nodo_proceso **listaFinQuantum;
nodo_proceso **listaFinIO;
nodo_proceso **listaProcesosListos;
nodo_entrada_salida  **listaBloqueados;
nodo_proceso **listaTerminados;
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


   cargar_archivo_configuracion();
   printf("mps=%d\n",mps);
   printf("mpp=%d\n",mpp);
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
	listaTerminados=(nodo_proceso **)malloc(sizeof(nodo_proceso));
	bzero(listaTerminados,sizeof(nodo_proceso));

	while( texto_del_archivo != NULL){
		linea = strtok(texto_del_archivo,"\n");
		texto_del_archivo = strtok(NULL,"\0");

		if( strstr(linea,"mps")){
			valor = strtok(linea," ");
			valor = strtok(NULL,";");
			if( valor != NULL ){
				mps=atoi(valor);
			}
		}
		if( strstr(linea,"mpp")){
			valor = strtok(linea," ");
			valor = strtok(NULL,";");
			if( valor != NULL ){
				mpp=atoi(valor);
			}
		}

		if( strstr(linea,"lpl")){
			valor = strtok(linea," ");
			valor = strtok(NULL,";");
			if( valor != NULL ){
				lpl=valor;
			}
		}

		// FUNCION QUE MANEJA LA SEÑAL DEL SIGUSR1 PI.
		void  SIGhandler(int sig)
		{
			suspendido = 1;
			printf("\nReceived a SIGUSR1.\n");
		}

		if( strstr(linea,"quantum_max")){
			valor = strtok(linea," ");
			valor = strtok(NULL,";");
			if( valor != NULL ){
				quantum_max=atoi(valor);
			}
		}

		if( strstr(linea,"espera_estandar")){
			valor = strtok(linea," ");
			valor = strtok(NULL,";");
			if( valor != NULL ){
				espera_estandar=valor;
			}
		}
	}

	return 0;
}


