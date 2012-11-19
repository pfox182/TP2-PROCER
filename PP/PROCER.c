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
extern char *espera_estandar_io;
extern int cant_iot;
extern int cant_iot_disponibles;
//TODO:declarar variables en pp.c

//Listas globales
extern nodo_entrada_salida **lista_bloqueados;
//TODO:declarar listas en pp.c

void * PROCER_funcion(){
	//TODO:BORRAR {sirve para que no de error, xq viene de una lista
		proceso proceso;
		char *buffer=leer_archivo("/home/utnso/hola");
		pcb pcb;
		pcb.pid=0;
		pcb.pc=0;
		char *codigo=leer_archivo("/home/utnso/hola");
		pcb.codigo=codigo;
		pcb.datos=cargar_datos(buffer);
	//pcb.pila=sacar_funciones(leer_archivo("/home/utnso/hola"));

	proceso.pcb=pcb;
	//}

	//TODO:Saco el primer proceso de la lista de procesos listos

	unsigned int cant_instrucciones = cant_lineas(pcb.codigo);
	char *instruccion;
	unsigned int cont_quantum = 0;
	int retorno;

	while( verificar_fin_ejecucion(proceso.pcb,cont_quantum,cant_instrucciones) != -1){
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
					proceso.pcb.pc++;
					break;
				}
			}else{
				printf("Finalizo la ejecucion\n");
				break;
			}
		}
		proceso.pcb.pc++;
		cont_quantum++;
	}
	printf("El PC es %d\n",proceso.pcb.pc);
	mostrar_lista(lista_bloqueados);
	mostrar_datos(proceso.pcb.datos);
	return 0;
}
