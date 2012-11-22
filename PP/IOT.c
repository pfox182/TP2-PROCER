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
#include <sys/socket.h>

//aux
extern nodo_proceso **listaProcesosNuevos;
extern int global_iot;
int esperar_a_que_se_llene_iot(nodo_entrada_salida **lista);
void agregar_proceso_iot(nodo_proceso **lista_procesos,proceso proceso);

//Listas globales
extern nodo_proceso **listaFinIO;
extern nodo_entrada_salida  **listaBloqueados;

//Variables globales
extern int cant_iot_disponibles;
extern char *espera_estandar_io;

void * IOT_funcion(){
	int valor;

	esperar_a_que_se_llene_iot(listaBloqueados);
	printf("Sali de espera en IOT\n");
	//TODO: Productor-Consumidor implementar

	//TODO:implementar semaforos
	cant_iot_disponibles--;//IOT Ocupado

	instruccion_io instruccion;

	//TODO:implementar semaforos
	instruccion=sacar_entrada_salida(listaBloqueados);
	global_iot=0;
	if( strstr(instruccion.instruccion,"imprimir") != NULL ){
		if ( (valor = send(instruccion.proceso.cliente_sock,instruccion.mensaje,strlen(instruccion.mensaje),0)) == -1){
			printf("Error no se pudo enviar el mensaje imprimir en el hilo IOT\n");
		}
		printf("El valor del send es %d\n",valor);
		sleep(atoi(espera_estandar_io));
		//TODO:implementar semaforos
		agregar_proceso(listaFinIO,instruccion.proceso);
		mostrar_lista(listaFinIO);
		//TODO:implementar semaforos
		cant_iot_disponibles++;//Libero un IOT;
		//habilitar STS para que pueda retirar
	}else{
		sleep(atoi(instruccion.mensaje));
		//TODO:implementar semaforos
		printf("El proceso a agregar es PID:%d\n",instruccion.proceso.pcb.pid);
		agregar_proceso_iot(listaFinIO,instruccion.proceso);
		mostrar_lista(listaFinIO);
		//TODO:implementar semaforos
		cant_iot_disponibles++;//Libero un IOT
		//habilitar STS para que pueda retirar
	}

	return 0;
}


//AUXILIARES
int esperar_a_que_se_llene_iot(nodo_entrada_salida **lista){
	printf("Entre en esperar en IOT\n");
	while( global_iot == 0 ){
		sleep(1);
	}
	return 0;
}

void agregar_proceso_iot(nodo_proceso **lista_procesos,proceso proceso){
	printf("Entre en el agregar_proceso, el proceso que me llego es PID:%d\n",proceso.pcb.pid);
	nodo_proceso *aux;

	//Creo el nodo a agregar
	nodo_proceso *nuevo_proceso;
	nuevo_proceso=(nodo_proceso *)malloc(sizeof(nodo_proceso));
	nuevo_proceso->proceso=proceso;
	nuevo_proceso->sig=NULL;

	if(*lista_procesos == NULL){//Si esta vacio, le asigno el nuevo
		printf("La lista que me pasaron esta vacia\n");
		*lista_procesos = nuevo_proceso;
	}else{
		printf("La lista que me pasaron NO esta vacia\n");
		aux=*lista_procesos;
		while( aux->sig != NULL){//Recorro hasta el ultimo
			aux = aux->sig;
		}
		aux->sig =nuevo_proceso;
	}
	printf("Termine de agregar\n");
}




