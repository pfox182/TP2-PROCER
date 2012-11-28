/*
 * manejo_listas_pila_ejecuccion.c
 *
 *  Created on: 24/11/2012
 *      Author: utnso
 */

#include "manejo_pila_ejecucion.h"
#include <stdlib.h>
#include <stdio.h>

seccion crear_seccion(char *nombre,unsigned int *contador){
	seccion n_seccion;
	n_seccion.nombre_seccion=nombre;
	n_seccion.contador_instruccion=contador;
	return n_seccion;
}
void agregar_a_pila_ejecucion(seccion n_seccion,pila_ejecucion **pila){
	pila_ejecucion *aux;

	//Creo el nodo a agregar
	pila_ejecucion *nueva_ejecucion;
	nueva_ejecucion=(pila_ejecucion *)malloc(sizeof(pila_ejecucion));
	nueva_ejecucion->seccion=n_seccion;
	nueva_ejecucion->siguiente=NULL;

	if(*pila == NULL){//Si esta vacio, le asigno el nuevo
		*pila = nueva_ejecucion;
	}else{
		aux=*pila;
		*pila=nueva_ejecucion;
		nueva_ejecucion->siguiente=aux;
	}
}

seccion sacar_primera_seccion(pila_ejecucion **pila){
	pila_ejecucion *primero;
	seccion seccion;

	if( *pila != NULL){
		primero = *pila;

		*pila = primero->siguiente;

		seccion = primero->seccion;

		//free(primero);
	}

	return seccion;
}
