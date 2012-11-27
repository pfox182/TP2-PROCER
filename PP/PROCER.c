#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h> //Contiene la funcion sleep
#include "../FuncionesPropias/manejo_archivos.h"
#include "../Estructuras/proceso.h"
#include "../Estructuras/manejo_pila_ejecucion.h"
#include "../Estructuras/manejo_mensajes.h"
#include "../Estructuras/manejo_semaforos.h"

#include "PROCER_funciones.h"

//Prototipos
int enviar_proceso_terminado(proceso proceso);
int las_listas_estan_vacias_procer();

//Variables globales pp.c
extern unsigned int mmp;
extern unsigned int mps;
extern int suspendido;
extern int semaforos;

//Listas globales
extern nodo_proceso **listaProcesosListos;
extern nodo_proceso **listaProcesosSuspendidos;

//AUX
void mostrar_datos(data *datos);


void * PROCER_funcion(){

	while(1){
		if ( las_listas_estan_vacias_procer() != 0 ){


			esperar_semaforo(semaforos,SEM_LISTA_LISTOS);
			proceso proceso=sacar_proceso(listaProcesosListos);
			liberar_semaforo(semaforos,SEM_LISTA_LISTOS);

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
				   printf("Agregue el proceso %d a Suspendidos\n",proceso.pcb.pid);
				   esperar_semaforo(semaforos,SEM_LISTA_SUSPENDIDOS);
				   agregar_proceso(listaProcesosSuspendidos,proceso);
				   liberar_semaforo(semaforos,SEM_LISTA_SUSPENDIDOS);
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

							esperar_semaforo(semaforos,SEM_VAR_MMP);
							--mmp;
							liberar_semaforo(semaforos,SEM_VAR_MMP);

							esperar_semaforo(semaforos,SEM_VAR_MPS);
							--mps;
							liberar_semaforo(semaforos,SEM_VAR_MPS);

							mostrar_datos(proceso.pcb.datos);
							enviar_proceso_terminado(proceso);
							break;
						}
					}
					bzero(instruccion,strlen(instruccion));
				}
			}
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
