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
#include <sys/socket.h>


//Listas globales
extern nodo_proceso **listaFinIO;
extern nodo_entrada_salida  **listaBloqueados;

//Variables globales
extern int cant_iot_disponibles;

void * IOT_funcion(){

	//TODO: Productor-Consumidor implementar

	//TODO:implementar semaforos
	cant_iot_disponibles--;//IOT Ocupado

	instruccion_io instruccion;

	//TODO:implementar semaforos
	instruccion=sacar_entrada_salida(listaBloqueados);

	if( strstr(instruccion.instruccion,"imprimir") != NULL ){
		if ( send(instruccion.proceso.cliente_sock,instruccion.mensaje,strlen(instruccion.mensaje),0) == -1){
			printf("Error no se pudo enviar el mensaje imprimir en el hilo IOT");
		}
		//TODO:implementar semaforos
		agregar_proceso(listaFinIO,instruccion.proceso);
		//TODO:implementar semaforos
		cant_iot_disponibles++;//Libero un IOT;
		//habilitar STS para que pueda retirar
	}
	else{
		sleep(atoi(instruccion.mensaje));
		//TODO:implementar semaforos
		agregar_proceso(listaFinIO,instruccion.proceso);
		//TODO:implementar semaforos
		cant_iot_disponibles++;//Libero un IOT
		//habilitar STS para que pueda retirar
	}

	return 0;
}







