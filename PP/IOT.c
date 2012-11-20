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
extern nodo_entrada_salida **lista_bloqueados;
extern nodo_proceso **lista_finio;

//Variables globales
extern int cant_iot_disponibles;

void * IOT_funcion(){

	//TODO: Productor-Consumidor implementar

	//SEMAFORO
	cant_iot_disponibles--;//IOT Ocupado

	instruccion_io instruccion;

	instruccion=sacar_entrada_salida(lista_bloqueados);

	if( strstr(instruccion.instruccion,"imprimir") != NULL ){
		if ( send(instruccion.proceso.cliente_sock,instruccion.mensaje,sizeof(instruccion.mensaje),0) == -1){
			printf("Error no se pudo enviar el mensaje imprimir en el hilo IOT");
		}
		agregar_proceso(lista_finio,instruccion.proceso);
		//SEMAFORO
		cant_iot_disponibles++;//Libero un IOT;
		//habilitar STS para que pueda retirar
	}
	else{
		sleep(atoi(instruccion.mensaje));
		agregar_proceso(lista_finio,instruccion.proceso);
		//SEMAFORO
		cant_iot_disponibles++;//Libero un IOT
		//habilitar STS para que pueda retirar
	}

	return 0;
}







