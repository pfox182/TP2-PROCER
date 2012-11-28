#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h> //Contiene la funcion sleep
#include <semaphore.h>
#include <pthread.h>

#include "../FuncionesPropias/manejo_archivos.h"
#include "../Estructuras/proceso.h"
#include "../Estructuras/manejo_pila_ejecucion.h"
#include "../Estructuras/manejo_mensajes.h"

#include "PROCER_funciones.h"

//Prototipos
int enviar_proceso_terminado(proceso proceso);
int liberar_proceso(proceso *proceso);
int las_listas_estan_vacias_procer();

//Variables globales pp.c
extern unsigned int mmp;
extern unsigned int mps;
extern int suspendido;
extern pthread_mutex_t mutexListaListos;
extern pthread_mutex_t mutexListaSuspendidos;

//Listas globales
extern nodo_proceso **listaProcesosListos;
extern nodo_proceso **listaProcesosSuspendidos;

//AUX
void mostrar_datos(data *datos);


void * PROCER_funcion(){

	while(1){
		if ( las_listas_estan_vacias_procer() != 0 ){


			pthread_mutex_lock(&mutexListaListos);
			proceso proceso=sacar_proceso(listaProcesosListos);
			pthread_mutex_unlock(&mutexListaListos);

			printf("Se saco el proceso PID:%d de listos\n",proceso.pcb.pid);
			printf("El codigo es:\n%s\n",proceso.pcb.codigo);

			unsigned int cant_instrucciones = cant_lineas(proceso.pcb.codigo);
			printf("La cantidad de instrucciones son %d\n",cant_instrucciones);
			char *instruccion;
			unsigned int cont_quantum = 0;
			int retorno;
			seccion seccion_a_ejecutar;


			while( verificar_fin_ejecucion(proceso,cont_quantum,cant_instrucciones) != -1){

			   if( proceso.pcb.pc == 0 ){//Es la 1° vez que ejecuta
				   agregar_a_pila_ejecucion(crear_seccion("fin_programa",&proceso.pcb.pc ),proceso.pila_ejecucion);
				   printf("Se agrego por 1° vez la seccion PROGRAMA\n");
			   }
			   if( suspendido == 1){
				   suspendido = 0;
				   pthread_mutex_lock(&mutexListaSuspendidos);
				   agregar_proceso(listaProcesosSuspendidos,proceso);
				   pthread_mutex_unlock(&mutexListaSuspendidos);
				   printf("Agregue el proceso %d a Suspendidos\n",proceso.pcb.pid);
				   printf("Se suspendio un proceso, suspendido=%d\n",suspendido);
				   printf("suspendido=%d despues de meterle 0\n",suspendido);
				   break;
			   }else{//No se suspendio la ejecucion

				   seccion_a_ejecutar=sacar_primera_seccion(proceso.pila_ejecucion);
				   if( strcmp(seccion_a_ejecutar.nombre_seccion,"") == 0){
					   printf("Error al sacar la seccion a ejecutar, es igual a NULL\n");
					   break;
				   }
				   printf("Se extrajo { la seccion %s con contador=%d }\n",seccion_a_ejecutar.nombre_seccion,*seccion_a_ejecutar.contador_instruccion);

					printf("El PC es %d\n",proceso.pcb.pc);
					printf("Voy a leer la instruccion %d de la seccion %s\n",*seccion_a_ejecutar.contador_instruccion,seccion_a_ejecutar.nombre_seccion);
					//Leemos la siguiente instruccion a ejecutar
					instruccion = leer_instruccion(proceso.pcb.codigo,*seccion_a_ejecutar.contador_instruccion);
					if( instruccion != NULL){
						//Calculo la proxima instruccion a leer
						++(*seccion_a_ejecutar.contador_instruccion);
						cont_quantum++;

						if( strcmp(instruccion,seccion_a_ejecutar.nombre_seccion) != 0){//No es el fin de la seccion a ejecutar
							agregar_a_pila_ejecucion(seccion_a_ejecutar,proceso.pila_ejecucion);
							printf("Se volvio a agregar a la pila la seccion %s con contador=%d\n",seccion_a_ejecutar.nombre_seccion,*seccion_a_ejecutar.contador_instruccion);
						}

						if( strcmp(instruccion,"fin_programa") != 0){
							retorno = ejecutar_instruccion(instruccion,&proceso,&seccion_a_ejecutar);
							if( retorno == -1){
								printf("Error al ejecutar instruccion:\n %s\n",instruccion);
							}
							if( retorno == 1){//Quiere decir que se ejecuto una entrada/salida
								printf("Nos fuimos a entrada/salida\n");
								break;
							}
						}else{
							printf("Finalizo la ejecucion\n");

							//TODO impolementar semaforos
							--mmp;
							--mps;

							mostrar_datos(proceso.pcb.datos);
							enviar_proceso_terminado(proceso);
							//liberar_proceso(&proceso);
								printf("Free - liberar_proceso\n");
								//TODO:arreglar
								//free(proceso.pila_ejecucion);
								//printf("Libere la pila de ejecucion\n");
								free(proceso.pcb.codigo);
								printf("Libere el codigo\n");
								free(proceso.pcb.datos);
								printf("Libere los datos\n");
								//free(proceso.pcb.pila);
								//printf("Libere la pila\n");

								close(proceso.cliente_sock);
								printf("Cerre la conexion\n");
							break;
						}
					}
					bzero(instruccion,strlen(instruccion));
				}
			}
			cont_quantum=0;
		}else{
			sleep(1);
		}
	}
	return 0;
}

int enviar_proceso_terminado(proceso proceso){
	int i;
	char *numero=(char *)malloc(strlen("00000"));
	char *var=(char *)malloc(sizeof(char));
	char *msjVariables=(char *)malloc(1024);//mirar tamaño

	strcpy(msjVariables,"El proceso a finalizado:\n");

	for( i=0;i<26;i++){

		//filtrar variables que no estan en el proceso
		var[0]=proceso.pcb.datos[i].variable;
		var[1]='\0';
		strcat(msjVariables,var);
		strcat(msjVariables," = ");
		sprintf(numero,"%d",proceso.pcb.datos[i].valor);
		strcat(msjVariables,numero);
		strcat(msjVariables,"\n");

	}

	//Envio mensaje variables proceso suspendido.
	if ( enviar_mensaje(msjVariables,proceso.cliente_sock) == -1 ){
		return -1;
	}

	return 0;
}

int liberar_proceso(proceso *proceso){
	printf("Free - liberar_proceso\n");
	free(proceso->pcb.codigo);
	printf("Libere el codigo\n");
	free(proceso->pcb.datos);
	printf("Libere los datos\n");
	free(proceso->pcb.pila);
	printf("Libere la pila\n");
	free(proceso->pila_ejecucion);
	printf("Libere la pila de ejecucion\n");
	close(proceso->cliente_sock);
	printf("Cerre la conexion\n");

	return 0;
}

int las_listas_estan_vacias_procer(){
	if( *listaProcesosListos == NULL ){
		return 0;
	}
	return 1;
}

//auxiliar
void mostrar_datos(data *datos){
	//Muestro vector
	int i;
	for (i = 0; i < 26 ; i++)
	{
		printf("El valor de datos[%d] es var:%c valor:%d,\n",i,datos[i].variable,datos[i].valor);
	}
}
