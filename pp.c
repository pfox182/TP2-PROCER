/*
 * pp.c
 *
 *  Created on: 26/10/2012
 *      Author: utnso
 */

#include <pthread.h>
#include <stdio.h>
#include <stddef.h> //Contiene la constante NULL


void * foo(); // Función que ejecutará el thread

int main(int argc, char *argv[])
{
   pthread_t LTS_hilo;//Declaracion del hilo de LTP ( Planificador a largo plazo )

   printf("Creando hilo de LTS\n");
   pthread_create(&(LTS_hilo), NULL, foo, NULL); // Creamos el thread

   pthread_exit(NULL);// Última función que debe ejecutar el main() siempre
   return 0;
}

void * foo() // Función que ejecutará el thread
{
   printf("Soy el hilo de LTS\n");

   return NULL;
}
