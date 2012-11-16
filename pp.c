/*
 * pp.c
 *
 *  Created on: 26/10/2012
 *      Author: utnso
 */
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Headers propios
 */
#include "PP/LTS.h"
#include "PP/PROCER.h"
#include "PP/STS.h"
#include "FuncionesPropias/manejo_archivos.h"
#include "Estructuras/proceso.h"
/*
 * Prototipos
 */
int cargar_archivo_configuracion();

/*
 * Variables globales
 */
unsigned int mps=5; //Procesos en el sistema
unsigned int mpp=5; //Valor de multiprogramacion
unsigned int max_mps=10; //Maximo de procesos en el sistema
unsigned int max_mpp=10; //Maximo valor de multiprogramacion
unsigned int cantidad_hilos_iot=2; //Valor de hilos IOT
unsigned int pid=0;
char *lpl="FIFO"; //Algoritmo de ordenamiento para lista de procesos listos.
unsigned int quantum_max=2;
char *espera_estandar="2";
//Prioridades de los algoritmos.
unsigned int lpn;
unsigned int lpr;
unsigned int finQ;
unsigned int finIO;
//Listas de procesos.
listaProcesos *listaPN;
listaProcesos *listaPR; //¿Que son los procesos reanudados?
listaProcesos *listaFinQ;
listaProcesos *listaFinIO;
listaProcesos *listaPL;

// FUNCION QUE MANEJA LA SEÑAL DEL SIGUSR1 PI.
void sigusr1_handler (int numeroSenial){
	//TODO Suspender el proceso PROCER.
	printf("Capture la senial SIGUSR1");
	/* Se pone controlador por defecto para SIGUSR1 */
	signal (SIGUSR1, SIG_DFL);
}



int main(int argc, char *argv[])
{
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
	   pthread_create(&(IOT_hilo), NULL, PROCER_funcion, NULL); // Creamos el thread IOT
   }

   //CAPTURAR LA SEÑAL SIGUSR1.
   if (signal(SIGUSR1,sigusr1_handler) == SIG_ERR)
   {
	   perror("ERROR No se puede cambiar signal");
   }

   	/* Bucle infinito de espera.
   	 * pause() deja el proceso dormido hasta que llegue una señal. */
    while (1)
   		pause();

   pthread_exit(NULL);// Última función que debe ejecutar el main() siempre
   return 0;
}

int cargar_archivo_configuracion(){
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



