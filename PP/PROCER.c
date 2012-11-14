#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //Contiene la funcion sleep
#include "../FuncionesPropias/manejo_archivos.h"
#include "../Estructuras/proceso.h"

#include "PROCER_funciones.h"

//Variables globales
extern char *lpl;
extern unsigned int quantum_max;
extern char *espera_estandar;

void * PROCER_funcion(){
	//TODO:BORRAR {
		pcb pcb;
		pcb.pid=0;
		pcb.pc=0;
	//}

	//TODO:Saco el primer proceso de la lista de procesos listos

	unsigned int cant_instrucciones = cant_lineas(pcb.codigo);
	char *instruccion;
	unsigned int cont_quantum = 0;

	while( verificar_fin_ejecucion(pcb,cont_quantum,cant_instrucciones) != -1){
		//Leemos la siguiente instruccion a ejecutar
		instruccion = leer_instruccion(pcb.codigo,pcb.pc);
		if( instruccion != NULL){
			if( strcmp(instruccion,"fin_programa") != 0){
				if( ejecutar_instruccion(instruccion,&pcb) == -1){
					printf("Error al ejecutar instruccion:\n %s\n",instruccion);
				}
			}else{
				printf("Finalizo la ejecucion\n");
				break;
			}
		}
		pcb.pc++;
		cont_quantum++;
	}
	return 0;
}


