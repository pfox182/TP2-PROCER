/*
 * colaConeccionesDemoradas.c
 *
 *  Created on: 09/11/2012
 *      Author: utnso
 */

#include <stdlib.h>
#include "colaConeccionesDemoradas.h"

void encolar_solicitud(coneccionesDemoradas **listaConexionesDemoradas,int socket_client){

	coneccionesDemoradas *aux;

	//Creo el nodo a agregar
	coneccionesDemoradas *nuevo_proceso;
	nuevo_proceso=(coneccionesDemoradas *)malloc(sizeof(coneccionesDemoradas));
	nuevo_proceso->socket_cliente=socket_client;
	nuevo_proceso->siguiente=NULL;

	if(*listaConexionesDemoradas == NULL){//Si esta vacio, le asigno el nuevo
		*listaConexionesDemoradas = nuevo_proceso;
	}else{
		aux=*listaConexionesDemoradas;
		while( aux->siguiente != NULL){//Recorro hasta el ultimo
			aux = aux->siguiente;
		}
		aux->siguiente =nuevo_proceso;
	}
}

void encolar_primero(coneccionesDemoradas **listaConexionesDemoradas,int socket_client){

	coneccionesDemoradas *aux;

	//Creo el nodo a agregar
	coneccionesDemoradas *nueva_coneccion;
	nueva_coneccion=(coneccionesDemoradas *)malloc(sizeof(coneccionesDemoradas));
	nueva_coneccion->socket_cliente=socket_client;
	nueva_coneccion->siguiente=NULL;

	if(*listaConexionesDemoradas == NULL){//Si esta vacio, le asigno el nuevo
		*listaConexionesDemoradas = nueva_coneccion;
	}else{
		aux=*listaConexionesDemoradas;
		*listaConexionesDemoradas=nueva_coneccion;
		nueva_coneccion->siguiente=aux;
	}
}

int sacar_conexion_demorada(coneccionesDemoradas **listaConexionesDemoradas){
	coneccionesDemoradas *primero;
	int socket_client=0;

	if( *listaConexionesDemoradas != NULL){
		primero = *listaConexionesDemoradas;

		*listaConexionesDemoradas = primero->siguiente;

		socket_client = primero->socket_cliente;
		free(primero);
	}

	return socket_client;
}
