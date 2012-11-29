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
#include "../Estructuras/manejo_listas.h"
#include "../Estructuras/manejo_mensajes.h"
#include "../Estructuras/proceso.h"
#include <sys/socket.h>
#include <pthread.h>
#include <semaphore.h>



//Listas Globales
extern nodo_proceso **listaProcesosReanudados;
extern nodo_proceso **listaProcesosSuspendidos;

//Variables Globales
extern unsigned int mmp;
extern unsigned int max_mmp;
extern int lpr;
extern pthread_mutex_t mutexListaSuspendidos;
extern pthread_mutex_t mutexListaReanudados;

//Prototipos
int las_listas_estan_vacias_lts();


void *LTS_suspendido(){


	proceso proceso;
	stack *aux;
	int i;

	char template[]="------------------------------------------\n\n";
	char template1[]="ID=";
	char template2[]="PD=";
	char template3[]="\n- Estructura de codigo ----\n";
	char template4[]="-------------------------\n\n- Estructura de Datos ----\n";
	char template5[]="-------------------------\n\n- Estructura de Stack ----\n";
	char msjReanudo[]="Desea reanudar el proceso.(si/no):";
	char msjMMP[]="No se pudo reanudar el proceso, se supero el nivel maximo de multiprogramacion(MMP)";

	while(1){
		if ( las_listas_estan_vacias_lts() != 0 ){

			char *respuestaReanudo=(char *)malloc(strlen("si"));
			bzero(respuestaReanudo,strlen("si"));
			char *numero=(char *)malloc(strlen("00000"));
			bzero(numero,strlen("00000"));
			char *var=(char *)malloc(sizeof(char));
			bzero(var,sizeof(char));
			char *id=(char *)malloc(strlen("00000"));
			bzero(id,strlen("00000"));
			char *pc=(char *)malloc(strlen("00000"));
			bzero(pc,strlen("00000"));
			char *funcion=(char *)malloc(strlen("0000000000"));
			bzero(funcion,strlen("0000000000"));
			char *msjVariables=(char *)malloc(1024);//mirar tamaño
			bzero(msjVariables,sizeof(1024));


			strcpy(msjVariables,"El estado del proceso suspendido es:\n");


			pthread_mutex_lock(&mutexListaSuspendidos);
			proceso = sacar_proceso(listaProcesosSuspendidos);
			pthread_mutex_unlock(&mutexListaSuspendidos);


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
			for( i=0;i<26;i++){

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
			strcat(msjVariables,template5);
			aux=proceso.pcb.pila;

			while ( aux != NULL ){
				if ( aux->linea <= proceso.pcb.pc ){
					sprintf(funcion,"%d",aux->linea);
					strcat(funcion,",");
					strcat(funcion,aux->funcion);
					strcat(msjVariables,funcion);
					strcat(msjVariables,"\n");
				}
				aux = aux->siguiente;
			}


			//Mensaje reanudacion
			strcat(msjVariables,msjReanudo);

			//realloc(*msjVariables,strlen(msjVariables));

			//Envio mensaje con el estado proceso suspendido y pregunto si se reanuda.
			enviar_mensaje(msjVariables,proceso.cliente_sock);


			//Recibo la respuesta de msjReanudo
			recibir_mensaje(&respuestaReanudo,proceso.cliente_sock);

			//respuestaReanudo="no";
			if ( (strstr(respuestaReanudo,"si")) != NULL ){
				if ( mmp < max_mmp ){
					proceso.prioridad = lpr;
					pthread_mutex_lock(&mutexListaReanudados);
					agregar_proceso(listaProcesosReanudados,proceso);
					pthread_mutex_unlock(&mutexListaReanudados);
				}else{
					pthread_mutex_lock(&mutexListaSuspendidos);
					agregar_proceso(listaProcesosSuspendidos,proceso);
					pthread_mutex_unlock(&mutexListaSuspendidos);

					enviar_mensaje(msjMMP,proceso.cliente_sock);
				}

			}else{
				pthread_mutex_lock(&mutexListaSuspendidos);
				agregar_proceso(listaProcesosSuspendidos,proceso);
				pthread_mutex_unlock(&mutexListaSuspendidos);
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
	//TODO:Agregar las otras listas
	return 1;
}

