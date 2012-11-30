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
#include <pthread.h>
#include <semaphore.h>
#include "../Estructuras/proceso.h"
#include "../Estructuras/manejo_listas.h"

//Variables globales
double prioridad_anterior = 0;
extern char *lpl;
extern int cant_instrucciones_ejecutadas;
extern double alfa;

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

//Semaforos
extern pthread_mutex_t mutexListaNuevos;
extern pthread_mutex_t mutexListaListos;
extern pthread_mutex_t mutexListaReanudados;
extern pthread_mutex_t mutexListaFinQuantum;
extern pthread_mutex_t mutexListaFinIO;
extern pthread_mutex_t mutexVarAlfa;
extern pthread_mutex_t mutexVarCantInstruccionesEjecutadas;
extern pthread_mutex_t mutexVarLPN;

//Prototipos
void planificar(nodo_proceso**);
nodo_proceso** planificarPorFIFO(nodo_proceso**);
nodo_proceso** planificarPorRR(nodo_proceso **listaAPlanificar);
nodo_proceso* planificarPorPRI(nodo_proceso **listaAPlanificar);
nodo_proceso *planificarPorSPN(nodo_proceso **listaAPlanificar);
double calcular_prioridad_spn();
nodo_proceso *ordenaPorPrioridad(nodo_proceso *listaAPlanificar, int n);
nodo_proceso *ordenaPorPrioridadSPN(nodo_proceso *listaAPlanificar, int n);
int cantidad_nodos(nodo_proceso **listaAPlanificar);
int las_listas_estan_vacias_sts();
//AUX

void * STS_funcion (){
	unsigned int prioridad;

	while(1){
		if ( las_listas_estan_vacias_sts() != 0 ){

			//TODO:implementar los semaforos de las listas restantes
			//TODO: la prioridad maxima debe ser variable.
			for (prioridad = 1; prioridad < 5; ++prioridad) {
				pthread_mutex_lock(&mutexVarLPN);
				if(prioridad == lpn){
					if (listaProcesosNuevos != NULL){
						//TODO:Implementar semaforos
						printf("Entre en esperar en STS,con semaforo\n");

						pthread_mutex_lock(&mutexListaListos);
						pthread_mutex_lock(&mutexListaNuevos);
						agregar_lista_de_procesos(listaProcesosListos,listaProcesosNuevos,prioridad);
						pthread_mutex_unlock(&mutexListaNuevos);
						pthread_mutex_unlock(&mutexListaListos);
						printf("Agregue los procesos de NUEVOS STS\n");
					}
				}
				pthread_mutex_unlock(&mutexVarLPN);
				if (prioridad == lpr ){
					if (listaProcesosReanudados != NULL){
						pthread_mutex_lock(&mutexListaListos);
						pthread_mutex_lock(&mutexListaReanudados);
						agregar_lista_de_procesos(listaProcesosListos,listaProcesosReanudados,prioridad);
						pthread_mutex_unlock(&mutexListaReanudados);
						pthread_mutex_unlock(&mutexListaListos);
						printf("Agregue los procesos de REANUDADOS STS\n");
					}
				}

				if (prioridad == finQ ){
					if (listaFinQuantum != NULL){
						pthread_mutex_lock(&mutexListaListos);
						pthread_mutex_lock(&mutexListaFinQuantum);
						agregar_lista_de_procesos(listaProcesosListos,listaFinQuantum,prioridad);
						pthread_mutex_unlock(&mutexListaFinQuantum);
						pthread_mutex_unlock(&mutexListaListos);
						printf("Agregue los procesos de FIN DE QUANTUM STS\n");
					}
				}

				if (prioridad == finIO){
					if (listaFinIO != NULL){
						pthread_mutex_lock(&mutexListaListos);
						pthread_mutex_lock(&mutexListaFinIO);
						agregar_lista_de_procesos(listaProcesosListos,listaFinIO,prioridad);
						pthread_mutex_unlock(&mutexListaFinIO);
						pthread_mutex_unlock(&mutexListaListos);
					}
				}

			}

			printf("Estoy por planificar\n");
			pthread_mutex_lock(&mutexListaListos);
			//mostrar_lista(listaProcesosListos);
			planificar(listaProcesosListos);
			printf("Sali de planificar\n");
			//mostrar_lista(listaProcesosListos);
			pthread_mutex_unlock(&mutexListaListos);
			printf("Libere el semaforo de listos\n");

		}else{
			sleep(1);
		}
	}
	return 0;
}


void planificar(nodo_proceso **listaAPlanificar){

	if ( strcmp(lpl,"FIFO") == 0) {
		planificarPorFIFO(listaAPlanificar);
	}
	if ( strcmp(lpl,"RR") == 0) {
		planificarPorRR(listaAPlanificar);
	}
	if ( strcmp(lpl,"PRI") == 0) {
		planificarPorPRI(listaAPlanificar);
	}
	if ( strcmp(lpl,"SPN") == 0) {
		planificarPorSPN(listaAPlanificar);
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

nodo_proceso *planificarPorSPN(nodo_proceso **listaAPlanificar){
	nodo_proceso *listaAux = *listaAPlanificar;

	while( listaAux != NULL ){
		listaAux->proceso.prioridad_spn = calcular_prioridad_spn();
		listaAux = listaAux->sig;
	}

	return ordenaPorPrioridadSPN(*listaAPlanificar,cantidad_nodos(listaAPlanificar));
}
double calcular_prioridad_spn(){
	double prioridad;

	pthread_mutex_lock(&mutexVarAlfa);
	pthread_mutex_lock(&mutexVarCantInstruccionesEjecutadas);
	prioridad = prioridad_anterior * alfa + cant_instrucciones_ejecutadas * ( 1 - alfa);
	pthread_mutex_unlock(&mutexVarCantInstruccionesEjecutadas);
	pthread_mutex_unlock(&mutexVarAlfa);

	prioridad_anterior = prioridad;
	printf("La prioridad spn es:%f y el alfa es:%f\n",prioridad,alfa);
	return prioridad;
}

nodo_proceso *ordenaPorPrioridad(nodo_proceso *listaAPlanificar, int n) {
	nodo_proceso *aux;//=(nodo_proceso *)malloc(sizeof(nodo_proceso));
	nodo_proceso *siguiente;//=(nodo_proceso *)malloc(sizeof(nodo_proceso));
	nodo_proceso *anterior;//=(nodo_proceso *)malloc(sizeof(nodo_proceso));
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
	//free(aux);
	//free(siguiente);
	//->free(anterior);
	return listaAPlanificar;
}

nodo_proceso *ordenaPorPrioridadSPN(nodo_proceso *listaAPlanificar, int n) {
	nodo_proceso *aux;//=(nodo_proceso *)malloc(sizeof(nodo_proceso));
	nodo_proceso *siguiente;//=(nodo_proceso *)malloc(sizeof(nodo_proceso));
	nodo_proceso *anterior;//=(nodo_proceso *)malloc(sizeof(nodo_proceso));
	int j=1;
	int i;

	for(i=1;i<n;i++){
		aux = listaAPlanificar;
		anterior=NULL;
		j=1;
		while(j<=(n-i)){
			siguiente=aux->sig;
			if (aux->proceso.prioridad_spn > siguiente->proceso.prioridad_spn){
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
	//free(aux);
	//free(siguiente);
	//->free(anterior);
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

int las_listas_estan_vacias_sts(){
	if( *listaProcesosNuevos == NULL && *listaProcesosReanudados == NULL && *listaFinQuantum == NULL && *listaFinIO == NULL){
		return 0;
	}
	return 1;
}
