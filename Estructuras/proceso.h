/*
 * proceso.h
 *
 *  Created on: 02/11/2012
 *      Author: utnso
 */

#ifndef PROCESO_H_
#define PROCESO_H_

struct data{
	char *variable;
	int valor;
	struct data *siguiente;
} typedef data;

struct stack{
	int linea;
	char *funcion;
	struct stack *siguiente;
} typedef stack;

struct code{
	char *codigo;
}typedef code;

struct pcb{
	unsigned int pid;//Process ID
	unsigned int pc;//Program counter
	data *datos;
	stack *pila;
	code codigo;
}typedef pcb;

#endif /* PROCESO_H_ */
