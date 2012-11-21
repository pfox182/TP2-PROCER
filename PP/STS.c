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
#include "../Estructuras/manejo_listas.h"

//Variables globales
extern char *lpl;

//Listas de procesos.
extern nodo_proceso **listaProcesosNuevos;
extern nodo_proceso **listaProcesosReanudados; //¿Que son los procesos reanudados?
extern nodo_proceso **listaFinQuantum;
extern nodo_proceso **listaFinIO;
extern nodo_proceso **listaProcesosListos;

//Prioridades de los algoritmos.
extern unsigned int lpn;
extern unsigned int lpr;
extern unsigned int finQ;
extern unsigned int finIO;


//Prototipos
void planificar(nodo_proceso**);
nodo_proceso** planificarPorFIFO(nodo_proceso**);

//AUX
int esperar_a_que_se_llene(nodo_proceso **lista);
extern int global;

void * STS_funcion (){
	unsigned int prioridad;

	if( esperar_a_que_se_llene(listaProcesosNuevos) == 0){
		printf("Sali de esperar esperar\n");
	//Esto quizas se podria mejorar al ponerlo en una estructura y oredenarla para tener una secuencia de ejecución.
	for (prioridad = 1; prioridad < 4; ++prioridad) {
		if(prioridad == lpn){
			if (listaProcesosNuevos != NULL){
				printf("Estoy agregando la lista de procesos nuevos a lista de lstos\n");
				agregar_lista_de_procesos(listaProcesosListos,listaProcesosNuevos,prioridad);
				//proceso proceso=sacar_proceso(listaProcesosNuevos);
				//agregar_proceso(listaProcesosListos,proceso);
				printf("Sali de agregar proceso\n");

			}
		}
		/*if (prioridad == lpr ){
			if (listaProcesosReanudados != NULL){
				agregar_lista_de_procesos(listaProcesosListos,listaProcesosReanudados,prioridad);
			}
		}
		if (prioridad == finQ ){
			if (listaFinQuantum != NULL){
				agregar_lista_de_procesos(listaProcesosListos,listaFinQuantum,prioridad);
			}
		}
		if (prioridad == finIO){
			if (listaFinIO != NULL){
				agregar_lista_de_procesos(listaProcesosListos,listaFinIO,prioridad);
			}
		}
		*/
	}
	printf("Estoy por planificar\n");
	proceso proceso=sacar_proceso(listaProcesosListos);
	printf("El proceso de la lista es: pid->%d , pc->%d\n",proceso.pcb.pid,proceso.pcb.pc);
	planificar(listaProcesosListos);
	printf("Sali de planificar\n");

	}
	return 0;
}


void planificar(nodo_proceso **listaAPlanificar){

	if ( strcmp(lpl,"FIFO") ) {
		planificarPorFIFO(listaAPlanificar);
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

nodo_proceso** planificarPorFIFO(nodo_proceso **listaAPlanificar){
	return listaAPlanificar;
}

int esperar_a_que_se_llene(nodo_proceso **lista){
	printf("Entre en esperar\n");
	while( global == 0 ){
		sleep(1);
	}
	return 0;
}
