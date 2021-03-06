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
	double prioridad_spn;
	int instrucciones_spn;
	int es_instruccion_spn;
	int prioridad_FIFO_RR;
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
	int espera;
}typedef instruccion_io;

struct nodo_entrada_salida{
	instruccion_io instruccion;
	struct nodo_entrada_salida *sig;
} typedef nodo_entrada_salida;


//PROTOTIPOS
proceso crear_proceso(char *buffer,char *prioridad,int socket);
data* cargar_datos(char *buffer);
void error(const char *msg);
stack* sacar_funciones(char *buffer);

#endif /* PROCESO_H_ */


