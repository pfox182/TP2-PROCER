/*
 * LTS_demorado.c
 *
 *  Created on: 29/11/2012
 *      Author: utnso
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "../Estructuras/proceso.h"
#include "../Estructuras/manejo_mensajes.h"
#include "../Estructuras/manejo_listas.h"
#include "../Estructuras/colaConeccionesDemoradas.h"
#include "../Estructuras/manejo_listas_funciones.h"
#include "../Estructuras/manejo_mensajes.h"
#include "../Log/manejo_log.h"

//Prototipos de funcion
int lista_esta_vacia_LTS_demorado();
int validar_mmp_demorado(int cliente_sock);

//Variables globales
extern unsigned int mps,mmp,max_mps,max_mmp; //Se usa extern para indicar que son variables globales de otro archivo
	//Semaforos
extern pthread_mutex_t mutexListaNuevos;
extern pthread_mutex_t mutexVarMaxMMP;
extern pthread_mutex_t mutexVarMaxMPS;
extern pthread_mutex_t mutexVarMMP;
extern pthread_mutex_t mutexVarMPS;

extern coneccionesDemoradas **listaConeccionesDemoradas;
extern nodo_proceso **listaProcesosNuevos;

void * LTS_demorado(void * var){
	char *buffer_2=(char *)malloc(1);
	bzero(buffer_2,1);
	char *prioridad=(char *)malloc(1);
	bzero(prioridad,1);
	char *paso_mensaje=(char *)malloc(256);

	while(1){
		if(  lista_esta_vacia_LTS_demorado() != 0 ){

			pthread_t id_hilo=pthread_self();

			int retorno;
			int socket_demorado;
			proceso proceso;

			if((socket_demorado=sacar_conexion_demorada(listaConeccionesDemoradas))>0){
				pthread_mutex_lock(&mutexVarMMP);
				pthread_mutex_unlock(&mutexVarMMP);
				if( (retorno = validar_mmp_demorado(socket_demorado)) == 0){
					enviar_mensaje("Enviame el codigo\n",socket_demorado);
					recibir_mensaje(&buffer_2,socket_demorado);
					recibir_mensaje(&prioridad,socket_demorado);
					proceso = crear_proceso(buffer_2,prioridad,socket_demorado);

					bzero(paso_mensaje,256);
					sprintf(paso_mensaje,"Se creo el proceso con PID=%d\n",proceso.pcb.pid);
					enviar_mensaje(paso_mensaje,socket_demorado);

					logx(proceso.pcb.pid,"LTS_demorado",id_hilo,"INFO","El proceso ha sido creado.");

					char *log_text=(char *)malloc(127);
					sprintf(log_text,"La prioridad del proceso es %d.",proceso.prioridad);
					logx(proceso.pcb.pid,"LTS_demorado",id_hilo,"DEBUG",log_text);
//					if ( buffer_2 != NULL ){
//						free(buffer_2);
//					}

					pthread_mutex_lock(&mutexListaNuevos);
					agregar_proceso(listaProcesosNuevos,proceso);
					pthread_mutex_unlock(&mutexListaNuevos);
					logx(proceso.pcb.pid,"LTS_demorado",id_hilo,"LSCH","Agregue el proceso a la lista de Nuevos.");

					pthread_mutex_lock(&mutexVarMMP);
					mmp++;
					pthread_mutex_unlock(&mutexVarMMP);
					logx(proceso.pcb.pid,"LTS",id_hilo,"INFO","Se aumento el grado de multiprogramacion.");

				}else{

					 if( retorno == -2){
						logx(proceso.pcb.pid,"LTS",id_hilo,"ERROR","Se sobrepaso el maximo grado de multiprogramacion.");
						logx(proceso.pcb.pid,"LTS",id_hilo,"INFO","Se aumento el grado de multiprogramacion.");
					 }
				}

				//FD_CLR(socket_demorado,&(*master));
			}
			sleep(1);
		}else{
			sleep(1);
		}
	}

	return 0;
}

int lista_esta_vacia_LTS_demorado(){
	if( *listaConeccionesDemoradas == NULL ){
		return 0;
	}
	return 1;
}

int validar_mmp_demorado(int cliente_sock){

	pthread_mutex_lock(&mutexVarMaxMMP);
	pthread_mutex_lock(&mutexVarMMP);
	if( mmp >= max_mmp){//Si no entra al if => todo. ok
		pthread_mutex_unlock(&mutexVarMMP);
		pthread_mutex_unlock(&mutexVarMaxMMP);
		encolar_solicitud(listaConeccionesDemoradas,cliente_sock);

		return -2;

	}else{
		pthread_mutex_unlock(&mutexVarMMP);
		pthread_mutex_unlock(&mutexVarMaxMMP);
	}


	return 0;
}


