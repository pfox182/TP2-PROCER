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
#include "../Estructuras/manejo_semaforos.h"
#include <sys/socket.h>


//PROTORIPOS
int las_listas_estan_vacias_iot();

//Listas globales
extern nodo_proceso **listaFinIO;
extern nodo_entrada_salida  **listaBloqueados;

//Variables globales
extern int semaforos;
extern int cant_iot_disponibles;
extern char *espera_estandar_io;

void * IOT_funcion(){
	while(1){
			if ( las_listas_estan_vacias_iot() != 0 ){
			printf("Sali de espera en IOT\n");
			//TODO: Productor-Consumidor implementar

			//TODO:implementar semaforos
			cant_iot_disponibles--;//IOT Ocupado

			instruccion_io instruccion;

			//TODO:implementar semaforos
			esperar_semaforo(semaforos,SEM_LISTA_BLOQUEADOS);
			instruccion=sacar_entrada_salida(listaBloqueados);
			liberar_semaforo(semaforos,SEM_LISTA_BLOQUEADOS);

			if( strstr(instruccion.instruccion,"imprimir") != NULL ){
				printf("El socket del cliente es %d\n",instruccion.proceso.cliente_sock);
				enviar_mensaje(instruccion.mensaje,instruccion.proceso.cliente_sock);
				sleep(atoi(espera_estandar_io));

				esperar_semaforo(semaforos,SEM_LISTA_FIN_IO);
				agregar_proceso(listaFinIO,instruccion.proceso);
				liberar_semaforo(semaforos,SEM_LISTA_FIN_IO);

				mostrar_lista(listaFinIO);
				//TODO:implementar semaforos
				cant_iot_disponibles++;//Libero un IOT;
			}else{
				sleep(atoi(instruccion.mensaje));
				//TODO:implementar semaforos
				printf("El proceso a agregar es PID:%d\n",instruccion.proceso.pcb.pid);

				esperar_semaforo(semaforos,SEM_LISTA_FIN_IO);
				agregar_proceso(listaFinIO,instruccion.proceso);
				liberar_semaforo(semaforos,SEM_LISTA_FIN_IO);

				mostrar_lista(listaFinIO);
				//TODO:implementar semaforos
				cant_iot_disponibles++;//Libero un IOT
			}

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








