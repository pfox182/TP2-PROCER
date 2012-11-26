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
#include "../Estructuras/manejo_semaforos.h"
#include "../Estructuras/proceso.h"
#include <sys/socket.h>


//Listas Globales
extern nodo_proceso **listaProcesosReanudados;
extern nodo_proceso **listaProcesosSuspendidos;

//Variables Globales
extern unsigned int mmp;
extern unsigned int max_mmp;
extern int semaforos;

//Prototipos
int las_listas_estan_vacias_lts();


void *LTS_suspendido(){
	while(1){
		if ( las_listas_estan_vacias_lts() != 0 ){
			proceso proceso;
			int i;
			char *respuestaReanudo=(char *)malloc(strlen("si"));
			char *numero=(char *)malloc(strlen("00000"));
			char *var=(char *)malloc(sizeof(char));

			char *msjVariables=(char *)malloc(1024);//mirar tamaño
			strcpy(msjVariables,"El estado del proceso suspendido es:\n");
			char msjReanudo[]="Desea reanudar el proceso.(si/no):";
			char msjMMP[]="No se pudo reanudar el proceso, se supero el nivel maximo de multiprogramacion(MMP)";

			esperar_semaforo(semaforos,SEM_LISTA_SUSPENDIDOS);
			proceso = sacar_proceso(listaProcesosSuspendidos);
			liberar_semaforo(semaforos,SEM_LISTA_SUSPENDIDOS);

			while( proceso.pcb.pid != -1 ){

				//Armo msjVariables para el proceso suspendido
				for( i=0;i<26;i++){

					//filtrar variables que no estan en el proceso
					var[0]=proceso.pcb.datos[i].variable;
					var[1]='\0';
					strcat(msjVariables,var);
					strcat(msjVariables," = ");
					sprintf(numero,"%d",proceso.pcb.datos[i].valor);
					strcat(msjVariables,numero);
					strcat(msjVariables,"\n");

				}

				//Envio mensaje variables proceso suspendido.
				enviar_mensaje(msjVariables,proceso.cliente_sock);

				//Envio mensaje reanudacion.
				enviar_mensaje(msjReanudo,proceso.cliente_sock);

				//Recibo la respuesta de msjReanudo
				//recibir_mensaje(respuestaReanudo,proceso.cliente_sock);

				respuestaReanudo="si";
				if ( (strstr(respuestaReanudo,"si")) != NULL ){
					if ( mmp < max_mmp ){
						esperar_semaforo(semaforos,SEM_LISTA_REANUDADOS);
						agregar_proceso(listaProcesosReanudados,proceso);
						liberar_semaforo(semaforos,SEM_LISTA_REANUDADOS);
					}else{
						esperar_semaforo(semaforos,SEM_LISTA_SUSPENDIDOS);
						agregar_proceso(listaProcesosSuspendidos,proceso);
						liberar_semaforo(semaforos,SEM_LISTA_SUSPENDIDOS);
						enviar_mensaje(msjMMP,proceso.cliente_sock);
					}

				}else{
					esperar_semaforo(semaforos,SEM_LISTA_SUSPENDIDOS);
					agregar_proceso(listaProcesosSuspendidos,proceso);
					liberar_semaforo(semaforos,SEM_LISTA_SUSPENDIDOS);
				}
				esperar_semaforo(semaforos,SEM_LISTA_SUSPENDIDOS);
				proceso = sacar_proceso(listaProcesosSuspendidos);
				liberar_semaforo(semaforos,SEM_LISTA_SUSPENDIDOS);
			}
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

