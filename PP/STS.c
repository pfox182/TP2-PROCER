/*
 * STS.c
 *
 *  Created on: 16/11/2012
 *      Author: utnso
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../Estructuras/proceso.h"

//Variables globales
extern char *lpl;

//Listas de procesos.
extern listaProcesos *listaPN;
extern listaProcesos *listaPR;//¿Que son los procesos reanudados?
extern listaProcesos *listaFinQ;
extern listaProcesos *listaFinIO;
extern listaProcesos *listaPL;

//Prioridades de los algoritmos.
extern unsigned int lpn;
extern unsigned int lpr;
extern unsigned int finQ;
extern unsigned int finIO;

void planificar(listaProcesos*);
listaProcesos* planificarPorFIFO(listaProcesos*);
void agregarListaListos(listaProcesos*);


void *STS_funcion (){
	unsigned int prioridad;

	//TODO: ¿Cada cuanto o dependiendo de q evento debo verificar las listas?.

	//Esto quizas se podria mejorar al ponerlo en una estructura y oredenarla para tener una secuencia de ejecución.
	for (prioridad = 1; prioridad < 5; ++prioridad) {
		if(prioridad == lpn){
			if (listaPN != NULL){
				planificar(listaPN);
			}
		}
		if (prioridad == lpr ){
			if (listaPR != NULL){
				planificar(listaPR);
			}
		}
		if (prioridad == finQ ){
			if (listaFinQ != NULL){
				planificar(listaFinQ);
			}
		}
		if (prioridad == finIO){
			if (listaFinIO != NULL){
				planificar(listaFinIO);
			}
		}
	}
}

void planificar(listaProcesos *listaAPlanificar){

	if ( strcmp(lpl,"FIFO") ) {
		agregarListaListos(planificarPorFIFO(listaAPlanificar));
	}
	if ( strcmp(lpl,"RR")) {
		  // TODO: Llamar a la funcion correspondiente a este algoritmo
	}
	if ( strcmp(lpl,"PRI") ) {
		// TODO: Llamar a la funcion correspondiente a este algoritmo
	}
	if ( strcmp(lpl,"SPN") ) {
		// TODO: Llamar a la funcion correspondiente a este algoritmo
	}
}

listaProcesos* planificarPorFIFO(listaProcesos *listaAPlanificar){
	return listaAPlanificar;
}

void agregarListaListos(listaProcesos *listaAAgregar){
	//TODO: Agregar al final de la lista de procesos listos.
}

