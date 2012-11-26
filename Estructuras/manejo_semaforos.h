/*
 * manejo_semaforos.h
 *
 *  Created on: 26/11/2012
 *      Author: utnso
 */

#ifndef MANEJO_SEMAFOROS_H_
#define MANEJO_SEMAFOROS_H_

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
// La union ya está definida en sys/sem.h
#else
// Tenemos que definir la union
union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
	struct seminfo *__buf;
};
#endif

//DEFINES
#define SEM_LISTA_NUEVOS 0
#define SEM_LISTA_REANUDADOS 1
#define SEM_LISTA_SUSPENDIDOS 2
#define SEM_LISTA_FIN_QUANTUM 3
#define SEM_LISTA_FIN_IO 4
#define SEM_LISTA_LISTOS 5
#define SEM_LISTA_BLOQUEADOS 6

#define SEM_VAR_MPS 7
#define SEM_VAR_MMP 8
#define SEM_VAR_LPL 9
#define SEM_VAR_LPN 10
#define SEM_VAR_LPR 11
#define SEM_VAR_FIN_Q 12
#define SEM_VAR_FIN_IO 13
#define SEM_VAR_ESPERA_ESTANDAR 14
#define SEM_VAR_CANT_IOT_DISPONIBLES 15


//Prototipos
int esperar_semaforo(int id_semaforo, int posicion_en_array);
int liberar_semaforo(int id_semaforo, int posicion_en_array);
int inicializar_semaforos(int cant_semaforos_a_crear);
#endif /* MANEJO_SEMAFOROS_H_ */
