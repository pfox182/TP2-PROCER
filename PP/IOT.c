/*
 * IOT.c
 *
 *  Created on: 20/11/2012
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //Contiene la funcion sleep
#include "../Estructuras/manejo_listas.h"
#include "../Estructuras/proceso.h"
#include "../Estructuras/manejo_mensajes.h"
#include <sys/socket.h>
#include <semaphore.h>
#include <pthread.h>


//PROTORIPOS
int las_listas_estan_vacias_iot();

//Listas globales
extern nodo_proceso **listaFinIO;
extern nodo_entrada_salida  **listaBloqueados;

//Variables globales
extern int cant_iot_disponibles;
extern char *espera_estandar_io;
extern pthread_mutex_t mutexListaBloqueados;
extern pthread_mutex_t mutexListaFinIO;
extern pthread_mutex_t mutexVarCantIOTDisponibles;

extern int finIO;

void * IOT_funcion(){

	instruccion_io instruccion;

	while(1){
			if ( las_listas_estan_vacias_iot() != 0 ){
				printf("Sali de espera en IOT\n");


				pthread_mutex_lock(&mutexVarCantIOTDisponibles);
				cant_iot_disponibles--;//IOT Ocupado
				pthread_mutex_unlock(&mutexVarCantIOTDisponibles);


				pthread_mutex_lock(&mutexListaBloqueados);
				instruccion=sacar_entrada_salida(listaBloqueados);
				pthread_mutex_unlock(&mutexListaBloqueados);

				if( strstr(instruccion.instruccion,"imprimir") != NULL ){

					enviar_mensaje(instruccion.mensaje,instruccion.proceso.cliente_sock);

					sleep(atoi(espera_estandar_io));

					instruccion.proceso.prioridad = finIO;

					pthread_mutex_lock(&mutexListaFinIO);
					agregar_proceso(listaFinIO,instruccion.proceso);
					pthread_mutex_unlock(&mutexListaFinIO);

					//mostrar_lista(listaFinIO);

				}else{

					sleep(atoi(instruccion.mensaje));


					instruccion.proceso.prioridad = finIO;

					pthread_mutex_lock(&mutexListaFinIO);
					agregar_proceso(listaFinIO,instruccion.proceso);
					pthread_mutex_unlock(&mutexListaFinIO);

					//mostrar_lista(listaFinIO);

				}

				pthread_mutex_lock(&mutexVarCantIOTDisponibles);
				cant_iot_disponibles++;//Libero un IOT
				pthread_mutex_unlock(&mutexVarCantIOTDisponibles);

		}else{
			sleep(1);
		}
	}
	return 0;
}

int las_listas_estan_vacias_iot(){
	if( *listaBloqueados == NULL ){
		return 0;
	}
	return 1;
}








