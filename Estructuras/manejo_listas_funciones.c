/*
 * manejo_listas_funciones.c
 *
 *  Created on: 12/11/2012
 *      Author: utnso
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "manejo_listas_funciones.h"



void agregar_funcion(stack **lista_funciones,char *funcion,int numero_linea){

	stack *aux;

	//Creo el nodo a agregar
	stack *nueva_funcion;
	nueva_funcion=(stack *)malloc(sizeof(stack));
	nueva_funcion->linea= numero_linea;
	nueva_funcion->funcion= funcion;
	nueva_funcion->siguiente=NULL;

	printf("Pase las inicializaciones\n");
	if(*lista_funciones == NULL){//Si esta vacio, le asigno el nuevo
		printf("Entre con lista vacia\n");
		*lista_funciones = nueva_funcion;
	}else{
		aux=*lista_funciones;
		printf("Estoy por entrar al while de agregar_funcion\n");
		while( aux->siguiente != NULL){//Recorro hasta el ultimo

			printf("Estoy en el while de agregar_funcion con %s y %d\n",aux->funcion,aux->linea);
			sleep(1);
			aux = aux->siguiente;
		}
		printf("PSali del while de agregar_funcion\n");
		aux->siguiente =nueva_funcion;
	}
}




