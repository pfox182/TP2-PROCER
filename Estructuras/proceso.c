/*
 * proceso.c
 *
 *  Created on: 29/11/2012
 *      Author: utnso
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "../Estructuras/proceso.h"
#include "../Estructuras/manejo_listas.h"
#include "../Estructuras/colaConeccionesDemoradas.h"
#include "../Estructuras/manejo_listas_funciones.h"
#include "../Estructuras/manejo_mensajes.h"

//Prototipos de funcion
proceso crear_proceso(char *buffer,int socket);
data* cargar_datos(char *buffer);
void error(const char *msg);
stack* sacar_funciones(char *buffer);

//Variables globales
//TODO:semaforo pid
extern unsigned int pid;
extern int lpn;
	//Semaforos
extern pthread_mutex_t mutexVarLPN;


proceso crear_proceso(char *buffer,int socket){
	proceso proceso;
	pcb pcb;

	pcb.pid = ++pid;
	pcb.pc = 0;

	if( strlen(buffer) == 0){
		printf("El buffer en crear_proceso esta vacio\n");
	}

	printf("Pase la primera parte de crear proceso\n");
	pcb.codigo = (char *)malloc(strlen(buffer));
	bzero(pcb.codigo,strlen(buffer));
	memcpy(pcb.codigo,buffer,strlen(buffer));

	printf("Estoy por sacar funciones\n");
	pcb.pila= sacar_funciones(buffer);
	printf("Estoy por sacar datos\n");
	pcb.datos = cargar_datos(buffer);

	printf("Estoy por hacer bzero\n");
	bzero(buffer,strlen(buffer));

	proceso.pcb = pcb;

	pthread_mutex_lock(&mutexVarLPN);
	proceso.prioridad = lpn;
	pthread_mutex_unlock(&mutexVarLPN);

	proceso.prioridad_spn = 0;

	proceso.pila_ejecucion = (pila_ejecucion **)malloc(sizeof(pila_ejecucion));
	bzero(proceso.pila_ejecucion,sizeof(pila_ejecucion));
	proceso.cliente_sock = socket;

	printf("NO ROMPI AUN\n");
	printf("YA ROMPI AUN\n");
	return proceso;
}

data* cargar_datos(char *buffer){
	//Declaro variables

	data *puntero;
	data *datos=(data *)malloc(sizeof(data)*26);//Antes data datos[26];
	bzero(datos,sizeof(data)*26);
	int i=0;
	//char j;
	char *separacion;
	//int flag;
	char *resto=(char *)malloc(strlen(buffer));
	memcpy(resto,buffer,strlen(buffer));
	char *linea;

	//Inicializo el vector de variables
	/*
	for (i = 0,j='a'; i < 26; i++,j++)
	{
		datos[i].variable = j;
		datos[i].valor = -1;
	}
	*/
	while( resto != NULL){
		linea = strtok(resto,"\n");
		resto = strtok(NULL,"\0");

		if( strstr(linea,"variable") != NULL){
			//Lee la cadena y cargo en vector las variables existentes.
			separacion = strtok(linea," ");
			separacion = strtok(NULL,",");
			while( separacion != NULL )
			{
				//busco posicion de la variable en el vector
				datos[i].variable = *separacion;
				datos[i].valor = 0;

				separacion = strtok(NULL,",");
				i++;
			}
		}
	}

	puntero = &datos[0];
	datos=realloc(datos,sizeof(data)*i);

	if( resto != NULL ){
		free(resto);
	}

	return puntero;
}

stack* sacar_funciones(char *buffer){
	int numero_linea;
	char *funcion;
	char *resto=(char *)malloc(strlen(buffer));
	memcpy(resto,buffer,strlen(buffer));
	char *linea;
	stack **lista_funciones=(stack **)malloc(sizeof(stack));
	//stack *lista_aux=(stack *)malloc(sizeof(stack));
	//bzero(lista_aux,sizeof(stack));
	//memcpy(lista_funciones,lista_aux,sizeof(stack));
	*lista_funciones=NULL;

	numero_linea = 0;

	printf("Estoy por entrar al while\n");
	while( resto != NULL){
		printf("Estoy en el while =(\n");
		linea = strtok(resto,"\n");
		resto = strtok(NULL,"\0");
		numero_linea++;
		if( strstr(linea,"()") != NULL){
			funcion = strtok(linea,"()");
			agregar_funcion(lista_funciones,funcion,numero_linea);
		}

	}
	printf("Sali del while\n");

	if( resto != NULL ){
		free(resto);
	}

	return *lista_funciones;
}

/******* error() *********************
Imprime el stacktrace ante un error
 ****************************************/
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void mostrar_funciones(stack *pila){
	stack *aux=pila;
	//Muestro vector
	while( aux != NULL && aux->linea < 30 ){
		printf("Funcion %s , en la linea %d\n",aux->funcion,aux->linea);
		aux=aux->siguiente;
	}
}

