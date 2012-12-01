/*
 * manejo_listas.c
 *
 *  Created on: 02/11/2012
 *      Author: utnso
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "proceso.h"
#include "manejo_listas.h"
#include "../Log/manejo_log.h"



void agregar_proceso(nodo_proceso **lista_procesos,proceso proceso){

	nodo_proceso *aux;

	//Creo el nodo a agregar
	nodo_proceso *nuevo_proceso;
	nuevo_proceso=(nodo_proceso *)malloc(sizeof(nodo_proceso));
	nuevo_proceso->proceso=proceso;
	nuevo_proceso->sig=NULL;

	if(*lista_procesos == NULL){//Si esta vacio, le asigno el nuevo
		*lista_procesos = nuevo_proceso;
	}else{
		aux=*lista_procesos;
		while( aux->sig != NULL){//Recorro hasta el ultimo
			aux = aux->sig;
		}
		aux->sig =nuevo_proceso;
	}
}

proceso sacar_proceso(nodo_proceso **lista_procesos){
	nodo_proceso *primero;
	proceso proceso;

	if( *lista_procesos != NULL){
		primero = *lista_procesos;

		*lista_procesos = primero->sig;

		proceso = primero->proceso;

		free(primero);
	}

	return proceso;
}

void liberar_lista_de_procesos(nodo_proceso **proceso){
	nodo_proceso *auxSig,*aux;

	aux = *proceso;
	while( aux != NULL){
		auxSig = aux->sig;

		//Liberamos la memoria con free()
		free(aux);
		aux = auxSig;
	}

	*proceso = NULL;
}

void agregar_entrada_salida(nodo_entrada_salida **lista_bloqueados,instruccion_io instruccion_io){

	nodo_entrada_salida *aux;

	//Creo el nodo a agregar
	nodo_entrada_salida *nueva_entrada_salida;
	nueva_entrada_salida=(nodo_entrada_salida *)malloc(sizeof(nodo_entrada_salida));
	nueva_entrada_salida->instruccion=instruccion_io;
	nueva_entrada_salida->sig=NULL;

	if(*lista_bloqueados == NULL){//Si esta vacio, le asigno el nuevo
		*lista_bloqueados = nueva_entrada_salida;
	}else{
		aux=*lista_bloqueados;
		while( aux->sig != NULL){//Recorro hasta el ultimo
			aux = aux->sig;
		}
		aux->sig =nueva_entrada_salida;
	}
}

void agregar_primero_entrada_salida(nodo_entrada_salida **lista_bloqueados,instruccion_io instruccion_io){

	nodo_entrada_salida *aux;

	//Creo el nodo a agregar
	nodo_entrada_salida *nueva_entrada_salida;
	nueva_entrada_salida=(nodo_entrada_salida *)malloc(sizeof(nodo_entrada_salida));
	nueva_entrada_salida->instruccion=instruccion_io;
	nueva_entrada_salida->sig=NULL;

	if(*lista_bloqueados == NULL){//Si esta vacio, le asigno el nuevo
		*lista_bloqueados = nueva_entrada_salida;
	}else{
		aux=*lista_bloqueados;
		*lista_bloqueados=nueva_entrada_salida;
		nueva_entrada_salida->sig=aux;
	}
}

instruccion_io sacar_entrada_salida(nodo_entrada_salida **lista_bloqueados){
	nodo_entrada_salida *primero;
	instruccion_io instruccion;

	if( *lista_bloqueados != NULL){
		primero = *lista_bloqueados;

		*lista_bloqueados = primero->sig;

		instruccion = primero->instruccion;

		free(primero);
	}

	return instruccion;
}

void agregar_lista_de_procesos(nodo_proceso **listaProcesos, nodo_proceso **listaAgregar){
	proceso proceso;
	nodo_proceso **listaAux = listaAgregar;

	//TODO:Implementar logs para los cambios de listas
	while( *listaAux != NULL){
		proceso = sacar_proceso(listaAux);
		agregar_proceso(listaProcesos,proceso);
	}
}

void mostrar_lista(nodo_proceso **listaProcesos){
	nodo_proceso **lista=(nodo_proceso **)malloc(sizeof(listaProcesos));
	memcpy(lista,listaProcesos,sizeof(listaProcesos));

	while( *lista != NULL){
		//printf("El PID del proceso es: %d\n",(*lista)->proceso.pcb.pid);
		(*lista)=(*lista)->sig;
	}
	free(lista);

}

void agregar_lista_de_procesos_log(nodo_proceso **listaProcesos, nodo_proceso **listaAgregar,char *listaOrigen,char *listaDestino,int prioridad_FIFO_RR){
	proceso proceso;
	nodo_proceso **listaAux = listaAgregar;
	char *mensaje = (char*)malloc(1024);
	char *pidProceso = (char*)malloc(1024);
	bzero(mensaje,1024);

	while( *listaAux != NULL){
		proceso = sacar_proceso(listaAux);
		prioridad_FIFO_RR++;
		proceso.prioridad_FIFO_RR=prioridad_FIFO_RR;
		agregar_proceso(listaProcesos,proceso);

		//Armo el mensaje a loguear.
		strcat(mensaje,"Se paso el proceso ");
		sprintf(pidProceso,"%d",proceso.pcb.pid);
		strcat(mensaje,pidProceso);
		strcat(mensaje," de ");
		strcat(mensaje,listaOrigen);
		strcat(mensaje," a ");
		strcat(mensaje,listaDestino);
		strcat(mensaje,".\n");

		logx(proceso.pcb.pid,"STS",pthread_self(),"LSCH",mensaje);
	}
	free(mensaje);
}
