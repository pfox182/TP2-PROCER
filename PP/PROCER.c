#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //Contiene la funcion sleep
#include "../FuncionesPropias/manejo_archivos.h"
#include "../Estructuras/proceso.h"

#include "PROCER_funciones.h"

//Listas globales
extern nodo_proceso **listaProcesosListos;
extern nodo_proceso **listaTerminados;

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

	while( verificar_fin_ejecucion(proceso,cont_quantum,cant_instrucciones) != -1){
		printf("El PC es %d\n",proceso.pcb.pc);
		//Leemos la siguiente instruccion a ejecutar
		instruccion = leer_instruccion(proceso.pcb.codigo,proceso.pcb.pc);
		if( instruccion != NULL){
			if( strcmp(instruccion,"fin_programa") != 0){
				retorno = ejecutar_instruccion(instruccion,&proceso);
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
		proceso.pcb.pc++;
		cont_quantum++;
	}
	printf("El PC es %d\n",proceso.pcb.pc);
	return 0;
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
