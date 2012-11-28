/*
 * manejo_semaforos.c
 *
 *  Created on: 26/11/2012
 *      Author: utnso
 */
#include <sys/sem.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#include "manejo_semaforos.h"

int esperar_semaforo(int id_semaforo, int posicion_en_array){
	struct sembuf Operacion;

	//	sem_num es el indice del semáforo en el array por el que queremos "pasar"
	//	sem_op es -1 para hacer que el proceso espere al semáforo.
	//	sem_flg son flags de operación. De momento nos vale un 0.

	Operacion.sem_num = posicion_en_array;
	Operacion.sem_op = -1;
	Operacion.sem_flg = 0;

	semop (id_semaforo, &Operacion, 1);

	return 0;
}

int liberar_semaforo(int id_semaforo, int posicion_en_array){
	struct sembuf Operacion;

	//	sem_num es el indice del semáforo en el array por el que queremos "pasar"
	//	El 1 indica que se levanta el semaforo.
	//	sem_flg son flags de operación. De momento nos vale un 0.

	Operacion.sem_num = posicion_en_array;
	Operacion.sem_op = 1;
	Operacion.sem_flg = 0;

	semop (id_semaforo, &Operacion, 1);

	return 0;
}

int inicializar_semaforos(int cant_semaforos_a_crear){
	key_t Clave;
	int id_semaforo;
	union semun arg;
	int i;

	// Igual que en cualquier recurso compartido (memoria compartida, semaforos
   // o colas) se obtien una clave a partir de un fichero existente cualquiera
   // y de un entero cualquiera. Todos los procesos que quieran compartir este
   // semaforo, deben usar el mismo fichero y el mismo entero.

	Clave = ftok ("/home/utnso/hola3", 0);
	if (Clave == (key_t)-1)
	{
		printf("No puedo conseguir clave de semáforo\n");
		exit(0);
	}

	// Se obtiene un array de semaforos (10 en este caso, aunque solo se usara
	// uno.
	// El IPC_CREAT indica que lo  cree si no lo está ya
	// el 0600 con permisos de lectura y escritura para el usuario que lance
	// los procesos. Es importante el 0 delante para que se interprete en
	// octal.
	//
	id_semaforo = semget (Clave, cant_semaforos_a_crear, 0600 | IPC_CREAT);
	if (id_semaforo == -1)
	{
		printf("No puedo crear semáforo\n");
		exit (0);
	}

	//	Se inicializa el semáforo con un valor conocido. Si lo ponemos a 0,
	//	es semáforo estará "rojo". Si lo ponemos a 1, estará "verde".
	//	El 0 de la función semctl es el índice del semáforo que queremos
	//	inicializar dentro del array de 10 que hemos pedido.
	//
	for( i=0;i<cant_semaforos_a_crear;i++){
		arg.val = 0;
		semctl (id_semaforo, i, SETVAL, &arg);
	}

	for( i=0;i<cant_semaforos_a_crear;i++){
		liberar_semaforo(id_semaforo,i);
	}
	//semctl(id_semaforo,cant_semaforos_a_crear,SETALL,1);//Inicializa todos los semaforos en 1

	return id_semaforo;
}

