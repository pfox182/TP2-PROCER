/*
 * LTS_suspendido.c
 *
 *  Created on: 20/11/2012
 *      Author: utnso
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>
#include <semaphore.h>
#include "../Estructuras/manejo_listas.h"
#include "../Estructuras/manejo_mensajes.h"
#include "../Estructuras/proceso.h"
#include "../Log/manejo_log.h"




//Listas Globales
extern nodo_proceso **listaProcesosReanudados;
extern nodo_proceso **listaProcesosSuspendidos;

//Variables Globales
extern unsigned int mmp;
extern unsigned int max_mmp;
extern int lpr;
extern pthread_mutex_t mutexListaSuspendidos;
extern pthread_mutex_t mutexListaReanudados;
extern pthread_mutex_t mutexVarMaxMMP;
extern pthread_mutex_t mutexVarMMP;

//Prototipos
int las_listas_estan_vacias_lts();


void *LTS_suspendido(){


	proceso proceso;
	stack *aux;
	int i;
	int h;

	char template[]="------------------------------------------\n\n";
	char template1[]="ID=";
	char template2[]="PC=";
	char template3[]="\n- Estructura de codigo ----\n";
	char template4[]="-------------------------\n\n- Estructura de Datos ----\n";
	char template5[]="-------------------------\n\n- Estructura de Stack ----\n";
	char msjReanudo[]="Desea reanudar el proceso.(si/no):";
	char msjMMP[]="No se pudo reanudar el proceso, se supero el nivel maximo de multiprogramacion(mmp)";

	pthread_t id_hilo=pthread_self();

	while(1){
		if ( las_listas_estan_vacias_lts() != 0 ){

			char *respuestaReanudo=(char *)malloc(strlen("si")+1);
			bzero(respuestaReanudo,strlen("si")+1);
			char *numero=(char *)malloc(strlen("00000")+1);
			bzero(numero,strlen("00000")+1);
			char *var=(char *)malloc(sizeof(char));
			bzero(var,sizeof(char));
			char *id=(char *)malloc(strlen("00000")+1);
			bzero(id,strlen("00000")+1);
			char *pc=(char *)malloc(strlen("00000")+1);
			bzero(pc,strlen("00000")+1);
			char *funcion=(char *)malloc(strlen("0000000000")+1);
			bzero(funcion,strlen("0000000000")+1);
			char *msjVariables=(char *)malloc(1024);//mirar tamaño
			bzero(msjVariables,sizeof(1024));


			strcpy(msjVariables,"El estado del proceso suspendido es:\n");



			pthread_mutex_lock(&mutexListaSuspendidos);
			proceso = sacar_proceso(listaProcesosSuspendidos);
			pthread_mutex_unlock(&mutexListaSuspendidos);
			logx(proceso.pcb.pid,"LTS_suspendido",id_hilo,"LSCH","Se saco el proceso de ListaSuspendidos.");

			strcat(msjVariables,template);

			//ID
			strcat(msjVariables,template1);
			sprintf(id,"%d",proceso.pcb.pid);
			strcat(msjVariables,id);
			strcat(msjVariables,"\n");
			//PC
			strcat(msjVariables,template2);
			sprintf(pc,"%d",proceso.pcb.pc);
			strcat(msjVariables,pc);
			strcat(msjVariables,"\n");
			//CODIGO
			strcat(msjVariables,template3);
			strcat(msjVariables,proceso.pcb.codigo);

			//VARIABLES
			strcat(msjVariables,template4);
			for( i=0;proceso.pcb.datos[i].variable;i++){

				//filtrar variables que no estan en el proceso
				var[0]=proceso.pcb.datos[i].variable;
				var[1]='\0';
				strcat(msjVariables,var);
				strcat(msjVariables,"=");
				sprintf(numero,"%d",proceso.pcb.datos[i].valor);
				strcat(msjVariables,numero);
				strcat(msjVariables,"\n");

			}

			//FUNCIONES
			h = 1;
			aux=proceso.pcb.pila;

			while ( aux != NULL ){
				if ( aux->linea <= proceso.pcb.pc ){
					if ( h == 1){
					strcat(msjVariables,template5);
					}
					sprintf(funcion,"%d",aux->linea);
					strcat(funcion,",");
					strcat(funcion,aux->funcion);
					strcat(msjVariables,funcion);
					strcat(msjVariables,"\n");
				}
				h = 0;
				aux = aux->siguiente;
			}


			//Mensaje reanudacion
			strcat(msjVariables,msjReanudo);

			msjVariables=realloc(msjVariables,strlen(msjVariables)+1);

			//Envio mensaje con el estado proceso suspendido y pregunto si se reanuda.
			enviar_mensaje(msjVariables,proceso.cliente_sock);
			logx(proceso.pcb.pid,"LTS_suspendido",id_hilo,"INFO","Se envio el estado del proceso suspendido y se pregunto si se queria reanudar.");


			//Recibo la respuesta de msjReanudo
			recibir_mensaje(&respuestaReanudo,proceso.cliente_sock);
			logx(proceso.pcb.pid,"LTS_suspendido",id_hilo,"INFO","Se recibio el mensaje de reanudacion.");


			if ( (strstr(respuestaReanudo,"si")) != NULL ){
				pthread_mutex_lock(&mutexVarMaxMMP);
				pthread_mutex_lock(&mutexVarMMP);
				if ( mmp < max_mmp ){
					pthread_mutex_unlock(&mutexVarMaxMMP);
					pthread_mutex_unlock(&mutexVarMMP);

					proceso.prioridad = lpr;
					logx(proceso.pcb.pid,"LTS_suspendido",id_hilo,"DEBUG","Se cambio la prioridad del proceso para agregarlo en ListaProcesosReanudados.");

					pthread_mutex_lock(&mutexListaReanudados);
					agregar_proceso(listaProcesosReanudados,proceso);
					pthread_mutex_unlock(&mutexListaReanudados);
					logx(proceso.pcb.pid,"LTS_suspendido",id_hilo,"LSCH","Se agrego el proceso a ListaProcesosReanudados.");

				}else{
					pthread_mutex_unlock(&mutexVarMaxMMP);
					pthread_mutex_unlock(&mutexVarMMP);

					pthread_mutex_lock(&mutexListaSuspendidos);
					agregar_proceso(listaProcesosSuspendidos,proceso);
					pthread_mutex_unlock(&mutexListaSuspendidos);
					logx(proceso.pcb.pid,"LTS_suspendido",id_hilo,"LSCH","Se agrego el proceso a ListaSuspendidos.");

					enviar_mensaje(msjMMP,proceso.cliente_sock);
					logx(proceso.pcb.pid,"LTS_suspendido",id_hilo,"INFO","Se envio el mensaje que se supero MMP.");
				}

			}else{
				pthread_mutex_lock(&mutexListaSuspendidos);
				agregar_proceso(listaProcesosSuspendidos,proceso);
				pthread_mutex_unlock(&mutexListaSuspendidos);
				logx(proceso.pcb.pid,"LTS_suspendido",id_hilo,"LSCH","Se agrego el proceso a ListaSuspendidos.");
			}

			//Libero Malloc
			free(respuestaReanudo);
			free(numero);
			free(var);
			free(id);
			free(pc);
			free(funcion);
			free(msjVariables);


		}else{
			sleep(1);
		}
	}
	return 0;
}

int las_listas_estan_vacias_lts(){
	if( *listaProcesosSuspendidos == NULL ){
		return 0;
	}
	return 1;
}
