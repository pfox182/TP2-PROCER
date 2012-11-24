/*
 * LTS_suspendido.c
 *
 *  Created on: 20/11/2012
 *      Author: utnso
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Estructuras/manejo_listas.h"
#include "../Estructuras/manejo_mensajes.h"
#include "../Estructuras/proceso.h"
#include <sys/socket.h>


//Listas Globales
extern nodo_proceso **listaProcesosReanudados;
extern nodo_proceso **listaProcesosSuspendidos;

//Variables Globales
extern int mpp;
extern int mpp_max;




void *LTS_suspendido(){
	int mmp_max;
	int mmp;
	mpp=mmp;
	mpp_max=mmp_max;

	proceso proceso;
	int i;
	char *respuestaReanudo=(char *)malloc(strlen("si"));
	char *numero=(char *)malloc(strlen("00000"));
	char *var=(char *)malloc(sizeof(char));

	char msjVariables[]="El estado del proceso suspendido es:\n";
	char msjReanudo[]="Desea reanudar el proceso.(si/no):";
	char msjMMP[]="No se pudo reanudar el proceso, se supero el nivel maximo de multiprogramacion(MMP)";

	//TODO:implementar semaforo
	proceso = sacar_proceso(listaProcesosSuspendidos);

	while( proceso.pcb.pid != -1 ){

		//Armo msjVariables para el proceso suspendido
		for( i=0;i<26;i++){

			//filtrar variables que no estan en el proceso
			var[0]=proceso.pcb.datos[i].variable;
			var[1]='\0';
			strcat(msjVariables,var);
			strcat(msjVariables,"= ");
			sprintf(numero,"%d",proceso.pcb.datos[i].valor);
			strcat(msjVariables,numero);
			//for(i=0;i<strlen(numero);i++){
				//msjVariables[strlen(msjVariables)]=numero[i];
			//}
			strcat(msjVariables,"\n");

		}

		//Envio mensaje variables proceso suspendido.
		enviar_mensaje(msjVariables,proceso.cliente_sock);

		//Envio mensaje reanudacion.
		enviar_mensaje(msjReanudo,proceso.cliente_sock);

		//Recibo la respuesta de msjReanudo
		recibir_mensaje(respuestaReanudo,proceso.cliente_sock);


		if ( (strstr(respuestaReanudo,"si")) != NULL ){
			if ( mmp < mmp_max ){
				//TODO: implementar semaforo
				agregar_proceso(listaProcesosReanudados,proceso);
			}else{
				//TODO: implementar semaforo
				agregar_proceso(listaProcesosSuspendidos,proceso);
				enviar_mensaje(msjMMP,proceso.cliente_sock);
			}

		}else{
			//TODO:implementar semaforo
			agregar_proceso(listaProcesosSuspendidos,proceso);

		}

		//TODO:implementar semaforo
		proceso = sacar_proceso(listaProcesosSuspendidos);

	}




	return 0;
}

