/*
 * IOT.c
 *
 *  Created on: 20/11/2012
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //Contiene la funcion sleep
#include "../Estructuras/manejo_listas.h"
#include "../Estructuras/proceso.h"
#include "../Estructuras/manejo_mensajes.h"
#include <sys/socket.h>

//aux
extern int global_iot;
int esperar_a_que_se_llene_iot(nodo_entrada_salida **lista);

//Listas globales
extern nodo_proceso **listaFinIO;
extern nodo_entrada_salida  **listaBloqueados;

//Variables globales
extern int cant_iot_disponibles;
extern char *espera_estandar_io;

void * IOT_funcion(){

	esperar_a_que_se_llene_iot(listaBloqueados);
	printf("Sali de espera en IOT\n");
	//TODO: Productor-Consumidor implementar

	//TODO:implementar semaforos
	cant_iot_disponibles--;//IOT Ocupado

	instruccion_io instruccion;

	//TODO:implementar semaforos
	instruccion=sacar_entrada_salida(listaBloqueados);
	global_iot=0;
	if( strstr(instruccion.instruccion,"imprimir") != NULL ){
		printf("El socket del cliente es %d\n",instruccion.proceso.cliente_sock);
		enviar_mensaje(instruccion.mensaje,instruccion.proceso.cliente_sock);
		sleep(atoi(espera_estandar_io));
		//TODO:implementar semaforos
		agregar_proceso(listaFinIO,instruccion.proceso);
		mostrar_lista(listaFinIO);
		//TODO:implementar semaforos
		cant_iot_disponibles++;//Libero un IOT;
		//habilitar STS para que pueda retirar
	}else{
		sleep(atoi(instruccion.mensaje));
		//TODO:implementar semaforos
		printf("El proceso a agregar es PID:%d\n",instruccion.proceso.pcb.pid);
		agregar_proceso(listaFinIO,instruccion.proceso);
		mostrar_lista(listaFinIO);
		//TODO:implementar semaforos
		cant_iot_disponibles++;//Libero un IOT
		//habilitar STS para que pueda retirar
	}

	return 0;
}


//AUXILIARES
int esperar_a_que_se_llene_iot(nodo_entrada_salida **lista){
	printf("Entre en esperar en IOT\n");
	while( global_iot == 0 ){
		sleep(1);
	}
	return 0;
}





