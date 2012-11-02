/*
 * manejo_listas.c
 *
 *  Created on: 02/11/2012
 *      Author: utnso
 *
 */
#include <stdlib.h>
#include "proceso.h"
#include "manejo_listas.h"



void agregar_proceso(nodo_proceso **proceso,pcb pcb){

	nodo_proceso *aux;

	//Creo el nodo a agregar
	nodo_proceso *nuevo_proceso;
	nuevo_proceso=(nodo_proceso *)malloc(sizeof(nodo_proceso));
	nuevo_proceso->pcb=pcb;
	nuevo_proceso->sig=NULL;

	if(*proceso == NULL){//Si esta vacio, le asigno el nuevo
		*proceso = nuevo_proceso;
	}else{
		aux=*proceso;
		while( aux->sig != NULL){//Recorro hasta el ultimo
			aux = aux->sig;
		}
		aux->sig =nuevo_proceso;
	}
}

pcb sacar_proceso(nodo_proceso **proceso){
	nodo_proceso *primero;
	pcb pcb;

	if( *proceso != NULL){
		primero = *proceso;

		*proceso = primero->sig;

		pcb = primero->pcb;

		free(primero);
	}

	return pcb;
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
