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
nodo_proceso* planificarPorPRI(nodo_proceso **listaAPlanificar);
nodo_proceso *ordenaPorPrioridad(nodo_proceso *listaAPlanificar, int n);
int cantidad_nodos(nodo_proceso **listaAPlanificar);
nodo_proceso** planificarPorRR(nodo_proceso **listaAPlanificar);

//AUX
int esperar_a_que_se_llene_sts(nodo_proceso **lista);
extern int global_sts;
extern int global_procer;

void * STS_funcion (){
	unsigned int prioridad;

	esperar_a_que_se_llene_sts(listaProcesosNuevos);
	printf("Sali de esperar esperar en STS\n");
	//TODO: la prioridad maxima debe ser variable.
	for (prioridad = 1; prioridad < 4; ++prioridad) {
		if(prioridad == lpn){
			if (listaProcesosNuevos != NULL){
				//TODO:Implementar semaforos
				agregar_lista_de_procesos(listaProcesosListos,listaProcesosNuevos,prioridad);
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
	mostrar_lista(listaProcesosListos);
	printf("Estoy por planificar\n");
	planificar(listaProcesosListos);
	mostrar_lista(listaProcesosListos);
	global_sts=0;
	global_procer=1;
	return 0;
}


void planificar(nodo_proceso **listaAPlanificar){

	if ( strcmp(lpl,"FIFO") ) {
		planificarPorFIFO(listaAPlanificar);
	}
	if ( strcmp(lpl,"RR")) {
		planificarPorRR(listaAPlanificar);
	}
	if ( strcmp(lpl,"PRI") ) {
		planificarPorPRI(listaAPlanificar);
	}
	if ( strcmp(lpl,"SPN") ) {
		// TODO: Llamar a la funcion correspondiente a este algoritmo
	}
}

nodo_proceso** planificarPorFIFO(nodo_proceso **listaAPlanificar){
	return listaAPlanificar;
}

nodo_proceso** planificarPorRR(nodo_proceso **listaAPlanificar){
	return listaAPlanificar;
}

nodo_proceso *planificarPorPRI(nodo_proceso **listaAPlanificar){
	return ordenaPorPrioridad(*listaAPlanificar,cantidad_nodos(listaAPlanificar));
}

nodo_proceso *ordenaPorPrioridad(nodo_proceso *listaAPlanificar, int n) {
	nodo_proceso *aux=(nodo_proceso *)malloc(sizeof(nodo_proceso));
	nodo_proceso *siguiente=(nodo_proceso *)malloc(sizeof(nodo_proceso));
	nodo_proceso *anterior=(nodo_proceso *)malloc(sizeof(nodo_proceso));
	int j=1;
	int i;

	for(i=1;i<n;i++){
		aux = listaAPlanificar;
		anterior=NULL;
		j=1;
		while(j<=(n-i)){
			siguiente=aux->sig;
			if (aux->proceso.prioridad > siguiente->proceso.prioridad){
				aux->sig = siguiente->sig;
				siguiente->sig =aux;
				if (anterior !=NULL){
					anterior->sig=siguiente;
					anterior=siguiente;
				} else {
					listaAPlanificar=siguiente;
					anterior = listaAPlanificar;
				}
				aux=anterior->sig;
			} else {
				anterior=aux;
				aux=siguiente;
			}
			j++;
		}
	}
	return listaAPlanificar;
}

int cantidad_nodos(nodo_proceso **listaAPlanificar){
	int cant_nodos=0;
	nodo_proceso **aux=(nodo_proceso **)malloc(sizeof(listaAPlanificar));
	memcpy(aux,listaAPlanificar,sizeof(listaAPlanificar));

	while (*aux != NULL){
		cant_nodos++;
		(*aux) =(*aux)->sig;
	}
	free(aux);
	return cant_nodos;
}

//Semaforo auxiliar
int esperar_a_que_se_llene_sts(nodo_proceso **lista){
	printf("Entre en esperar en STS\n");
	while( global_sts == 0 ){
		sleep(1);
	}
	return 0;
}
