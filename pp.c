/*
 * pp.c
 *
 *  Created on: 26/10/2012
 *      Author: utnso
 */

#include <pthread.h>
#include <stdio.h>
#include <stddef.h> //Contiene la constante NULL

/*
 * Headers propios
 */
#include "PP/LTS.h"


/*
 * Variables globales
 */
	unsigned int mps=10; //Maximo de procesos en el sistema
	unsigned int mpp=10; //Valor de multiprogramacion
	unsigned int cantidad_hilos_iot=2; //Valor de hilos IOT

// FUNCIONES QUE MANEJAN LOS HILOS DEL PP, luego se puede exportar a archivos STS y PROCER respectivamente.

void *STS_funcion (void *var){
	//TODO Funcionalidad STS.
	return 0;
}

void *PROCER_funcion (void *var){
	//TODO Funcionalidad PROCER.
	return 0;
}


int main(int argc, char *argv[])
{
   int i;
   pthread_t LTS_hilo;//Declaracion del hilo de LTP ( Planificador a largo plazo )
   pthread_t STS_hilo;//Declaracion del hilo de LTP ( Planificador a largo plazo )
   pthread_t PROCER_hilo;//Declaracion del hilo de LTP ( Planificador a largo plazo )


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

   pthread_exit(NULL);// Última función que debe ejecutar el main() siempre
   return 0;
}

