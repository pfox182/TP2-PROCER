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

//Prototipos de funcion
int lista_esta_vacia_LTS_demorado();
int validar_mps_mmp_demorado(int cliente_sock);

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

	while(1){
		if(  lista_esta_vacia_LTS_demorado() != 0 ){

			int retorno;
			int socket_demorado;
			proceso proceso;

			if((socket_demorado=sacar_conexion_demorada(listaConeccionesDemoradas))>0){
				if( (retorno = validar_mps_mmp_demorado(socket_demorado)) == 0){
					enviar_mensaje("Enviame el codigo\n",socket_demorado);
					recibir_mensaje(&buffer_2,socket_demorado);
					recibir_mensaje(&prioridad,socket_demorado);
					proceso = crear_proceso(buffer_2,prioridad,socket_demorado);
//					if ( buffer_2 != NULL ){
//						free(buffer_2);
//					}

					pthread_mutex_lock(&mutexListaNuevos);
					agregar_proceso(listaProcesosNuevos,proceso);
					pthread_mutex_unlock(&mutexListaNuevos);

					pthread_mutex_lock(&mutexVarMPS);
					printf("Antes de que Incremente mps=%d\n",mps);
					mps++;
					printf("Incremente mps=%d\n",mps);
					pthread_mutex_unlock(&mutexVarMPS);

					pthread_mutex_lock(&mutexVarMMP);
					printf("Antes de que Incremente mmp=%d\n",mmp);
					mmp++;
					printf("Incremente mmp=%d\n",mmp);
					pthread_mutex_unlock(&mutexVarMMP);

				}else{
					if( retorno == 1){
						//printf("Volvi a encolar el socket demorado\n");
						encolar_primero(listaConeccionesDemoradas,socket_demorado);
					}else{
						printf("Se produjo un error al validar el mmp y mps.\n");
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

int validar_mps_mmp_demorado(int cliente_sock){

	pthread_mutex_lock(&mutexVarMaxMPS);
	pthread_mutex_lock(&mutexVarMaxMMP);
	pthread_mutex_lock(&mutexVarMPS);
	pthread_mutex_lock(&mutexVarMMP);
	if( mps >= max_mps || mmp >= max_mmp){//Si no entra al if => todo. ok
		pthread_mutex_unlock(&mutexVarMMP);
		pthread_mutex_unlock(&mutexVarMaxMMP);

		if( mps >= max_mps){
			pthread_mutex_unlock(&mutexVarMPS);
			pthread_mutex_unlock(&mutexVarMaxMPS);

			enviar_mensaje("Se sobrepaso el maximo de prosesos en el sistema(mps).\n",cliente_sock);
			printf("Sobrepaso de mps\n");
			close(cliente_sock);
			return 1;
		}else{
			pthread_mutex_unlock(&mutexVarMPS);
			pthread_mutex_unlock(&mutexVarMaxMPS);
			return 1;
		}

	}else{
		pthread_mutex_unlock(&mutexVarMPS);
		pthread_mutex_unlock(&mutexVarMMP);
		pthread_mutex_unlock(&mutexVarMaxMMP);
		pthread_mutex_unlock(&mutexVarMaxMPS);
	}




	printf("mps y mmp ok\n");
	return 0;
}


