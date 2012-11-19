/*
 * manejo_listas.c
 *
 *  Created on: 02/11/2012
 *      Author: utnso
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include "proceso.h"
#include "manejo_listas.h"



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
