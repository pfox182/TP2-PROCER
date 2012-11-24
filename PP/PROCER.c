#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h> //Contiene la funcion sleep
#include "../FuncionesPropias/manejo_archivos.h"
#include "../Estructuras/proceso.h"
#include "../Estructuras/manejo_pila_ejecucion.h"

#include "PROCER_funciones.h"

//Prototipos


//Variables globales PROCER
extern int suspendido;

//Listas globales
extern nodo_proceso **listaProcesosListos;
extern nodo_proceso **listaTerminados;
extern nodo_proceso **listaProcesosSuspendidos;

//AUX
extern int global_procer;
int esperar_a_que_se_llene_procer(nodo_proceso **lista);
void mostrar_datos(data *datos);


void * PROCER_funcion(){
	esperar_a_que_se_llene_procer(listaProcesosListos);
	printf("Sali de espera en PROCER\n");
	proceso proceso=sacar_proceso(listaProcesosListos);
	global_procer=0;
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
		   agregar_proceso(listaProcesosSuspendidos,proceso);
		   return 0; //TODO:hay que ver si va return
	   }

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
				mostrar_datos(proceso.pcb.datos);
				//TODO:implementar semaforos
				agregar_proceso(listaTerminados,proceso);
				mostrar_lista(listaTerminados);
				break;
			}
		}
		bzero(instruccion,strlen(instruccion));
	}
	printf("El PC es %d\n",proceso.pcb.pc);
	return 0;
}

// FUNCION QUE MANEJA LA SEÑAL DEL SIGUSR1 PI.
void  SIGhandler(int sig)
{
	suspendido = 1;
	printf("\nReceived a SIGUSR1.\n");
}
//Semaforo auxiliar
int esperar_a_que_se_llene_procer(nodo_proceso **lista){
	printf("Entre en esperar en PROCER\n");
	while( global_procer == 0 ){
		sleep(1);
	}
	return 0;
}
void mostrar_datos(data *datos){
	//Muestro vector
	int i;
	for (i = 0; i < 26 ; i++)
	{
		printf("El valor de datos[%d] es var:%c valor:%d,\n",i,datos[i].variable,datos[i].valor);
	}
}
