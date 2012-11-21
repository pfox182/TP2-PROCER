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

void * PROCER_funcion(){
	//TODO:implementar semaforos
	proceso proceso=sacar_proceso(listaProcesosListos);

	unsigned int cant_instrucciones = cant_lineas(proceso.pcb.codigo);
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
				//TODO:implementar semaforos
				agregar_proceso(listaTerminados,proceso);
				break;
			}
		}
		proceso.pcb.pc++;
		cont_quantum++;
	}
	printf("El PC es %d\n",proceso.pcb.pc);
	return 0;
}
