/*
 * colaConeccionesDemoradas.c
 *
 *  Created on: 09/11/2012
 *      Author: utnso
 */

#include <stdlib.h>
#include "colaConeccionesDemoradas.h"

void encolar_solicitud(int cliente_sock,coneccionesDemoradas *connecionesDemoradas){

	coneccionesDemoradas *aux;

	coneccionesDemoradas *nueva_connecion_demorada;
	nueva_connecion_demorada=(coneccionesDemoradas *)malloc(sizeof(coneccionesDemoradas));
	nueva_connecion_demorada->socket_cliente=cliente_sock;
	nueva_connecion_demorada->siguiente=NULL;

	if(connecionesDemoradas == NULL){//Si esta vacio, le asigno el nuevo
		connecionesDemoradas = nueva_connecion_demorada;
	}else{
		aux=connecionesDemoradas;
		while( aux->siguiente != NULL){//Recorro hasta el ultimo
			aux = aux->siguiente;
		}
		aux->siguiente =nueva_connecion_demorada;
	}

}
