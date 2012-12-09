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
#include "../Log/manejo_log.h"



//PROTORIPOS
//int las_listas_estan_vacias_iot();

//Listas globales
extern nodo_proceso **listaFinIO;
extern nodo_entrada_salida  **listaBloqueados;

//Variables globales
extern int cant_iot_disponibles;
extern char *espera_estandar_io;
extern pthread_mutex_t mutexListaBloqueados;
extern pthread_mutex_t mutexListaFinIO;
extern pthread_mutex_t mutexVarCantIOTDisponibles;

extern sem_t *sem_sts;
extern sem_t *sem_io;
extern int finIO;

void * IOT_funcion(){

	instruccion_io instruccion;

	pthread_t id_hilo=pthread_self();
	printf("Soy el hilo de IOT creandome, con id=%lu\n",id_hilo);

	while(1){
			sem_wait(sem_io);

				pthread_mutex_lock(&mutexListaBloqueados);
				instruccion=sacar_entrada_salida(listaBloqueados);
				pthread_mutex_unlock(&mutexListaBloqueados);
				logx(instruccion.proceso.pcb.pid,"IOT",id_hilo,"LSCH","Se saco el proceso de ListaBloqueados.");

				pthread_mutex_lock(&mutexVarCantIOTDisponibles);
				cant_iot_disponibles--;//IOT Ocupado
				pthread_mutex_unlock(&mutexVarCantIOTDisponibles);
				logx(instruccion.proceso.pcb.pid,"IOT",id_hilo,"DEBUG","Se ocupo un hilo IOT(cant_iot_disponibles).");




				if( strstr(instruccion.instruccion,"imprimir") != NULL ){
					logx(instruccion.proceso.pcb.pid,"LTS_suspendido",id_hilo,"INFO","La instruccion es un imprimir.");


					if (instruccion.espera == -1){
						sleep(atoi(espera_estandar_io));
						logx(instruccion.proceso.pcb.pid,"IOT",id_hilo,"INFO","Sleep imprimir espera_estandar_io");

					}else{
						sleep(instruccion.espera);
						logx(instruccion.proceso.pcb.pid,"IOT",id_hilo,"INFO","Sleep imprimir instruccion.espera");
					}

					char msj_seg[256];
					sprintf(msj_seg,"Estoy imprimiendo en el hilo %lu.",id_hilo);
					enviar_mensaje(msj_seg,instruccion.proceso.cliente_sock);

					enviar_mensaje(instruccion.mensaje,instruccion.proceso.cliente_sock);

					pthread_mutex_lock(&mutexListaFinIO);
					agregar_proceso(listaFinIO,instruccion.proceso);
					pthread_mutex_unlock(&mutexListaFinIO);
					sem_post(sem_sts);
					logx(instruccion.proceso.pcb.pid,"IOT",id_hilo,"LSCH","Se agrego el proceso a ListaFinIO.");



				}else{
					logx(instruccion.proceso.pcb.pid,"LTS_suspendido",id_hilo,"INFO","La instruccion es un io().");
					char msj_seg[256];
					sprintf(msj_seg,"Estoy esperando %d segundos, en el hilo %lu.",instruccion.espera,id_hilo);
					enviar_mensaje(msj_seg,instruccion.proceso.cliente_sock);
					sleep(instruccion.espera);
					logx(instruccion.proceso.pcb.pid,"IOT",id_hilo,"INFO","Sleep io() instruccion.espera.");


					pthread_mutex_lock(&mutexListaFinIO);
					agregar_proceso(listaFinIO,instruccion.proceso);
					pthread_mutex_unlock(&mutexListaFinIO);
					sem_post(sem_sts);
					logx(instruccion.proceso.pcb.pid,"IOT",id_hilo,"LSCH","Se agrego el proceso a ListaFinIO.");


				}

				pthread_mutex_lock(&mutexVarCantIOTDisponibles);
				cant_iot_disponibles++;//Libero un IOT
				pthread_mutex_unlock(&mutexVarCantIOTDisponibles);
				logx(instruccion.proceso.pcb.pid,"IOT",id_hilo,"DEBUG","Se libero un hilo IOT(cant_iot_disponibles).");

	}
	return 0;
}

//int las_listas_estan_vacias_iot(){
//	if( *listaBloqueados == NULL ){
//		return 0;
//	}
//	return 1;
//}
