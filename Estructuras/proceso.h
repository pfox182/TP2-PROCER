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

struct seccion{
	char *nombre_seccion;//programa,f1,f2,f3,etc
	unsigned int *contador_instruccion;//program_counter,function_counter
}typedef seccion;

struct pila_ejecucion{
	seccion seccion;
	struct pila_ejecucion *siguiente;
}typedef pila_ejecucion;

struct proceso{
	pcb pcb;
	int prioridad;
	int cliente_sock;
	pila_ejecucion **pila_ejecucion;
}typedef proceso;

struct nodo_proceso{
	proceso proceso;
	struct nodo_proceso *sig;
} typedef nodo_proceso;

struct instruccion_io{
	proceso proceso;
	char *instruccion;//imprimir o io
	char *mensaje;
}typedef instruccion_io;

struct nodo_entrada_salida{
	instruccion_io instruccion;
	struct nodo_entrada_salida *sig;
} typedef nodo_entrada_salida;


#endif /* PROCESO_H_ */


