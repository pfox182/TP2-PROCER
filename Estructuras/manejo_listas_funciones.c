/*
 * manejo_listas_funciones.c
 *
 *  Created on: 12/11/2012
 *      Author: utnso
 */

#include <stdlib.h>
#include "manejo_listas_funciones.h"



void agregar_funcion(stack **lista_funciones,char *funcion,int numero_linea){

	stack *aux;

	//Creo el nodo a agregar
	stack *nueva_funcion;
	nueva_funcion=(stack *)malloc(sizeof(stack));
	nueva_funcion->linea= numero_linea;
	nueva_funcion->funcion= funcion;
	nueva_funcion->siguiente=NULL;

	if(*lista_funciones == NULL){//Si esta vacio, le asigno el nuevo
		*lista_funciones = nueva_funcion;
	}else{
		aux=*lista_funciones;
		while( aux->siguiente != NULL){//Recorro hasta el ultimo
			aux = aux->siguiente;
		}
		aux->siguiente =nueva_funcion;
	}
}




