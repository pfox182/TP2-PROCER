/*
 * proceso.h
 *
 *  Created on: 02/11/2012
 *      Author: utnso
 */

#ifndef PROCESO_H_
#define PROCESO_H_

#define CANT_LETRAS 26

struct data{
	char variable;
	int valor;
} typedef data;

struct stack{
	int linea;
	char *funcion;
	struct stack *siguiente;
} typedef stack;

struct pcb{
	unsigned int pid;//Process ID
	unsigned int pc;//Program counter
	data *datos;
	stack *pila;
	char *codigo;
}typedef pcb;

struct proceso{
	pcb pcb;
	int prioridad;
}typedef proceso;
#endif /* PROCESO_H_ */
