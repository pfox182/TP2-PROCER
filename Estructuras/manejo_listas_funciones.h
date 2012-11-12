/*
 * manejo_listas_funciones.h
 *
 *  Created on: 12/11/2012
 *      Author: utnso
 */


#ifndef MANEJO_LISTAS_FUNCIONES_H_
#define MANEJO_LISTAS_FUNCIONES_H_

struct stack{
	int linea;
	char *funcion;
	struct stack *siguiente;
} typedef stack;

void agregar_funcion(stack **lista_funciones,char* funcion,int numero_linea);


#endif /* MANEJO_LISTAS_FUNCIONES_H_ */
