/*
 * manejo_listas.h
 *
 *  Created on: 02/11/2012
 *      Author: utnso
 */

#ifndef MANEJO_LISTAS_H_
#define MANEJO_LISTAS_H_

#include "proceso.h"

//Estructuras
struct instruccion_io{
	proceso proceso;
	char *instruccion;//imprimir o io
	char *mensaje;
}typedef instruccion_io;

 struct nodo_proceso{
	proceso proceso;
	struct nodo_proceso *sig;
} typedef nodo_proceso;

struct nodo_entrada_salida{
	instruccion_io instruccion;
	struct nodo_entrada_salida *sig;
} typedef nodo_entrada_salida;

void agregar_proceso(nodo_proceso **lista_procesos,proceso proceso);
proceso sacar_proceso(nodo_proceso **proceso);
void liberar_lista_de_procesos(nodo_proceso **proceso);
void agregar_entrada_salida(nodo_entrada_salida **lista_bloqueados,instruccion_io instruccion_io);
void agregar_primero_entrada_salida(nodo_entrada_salida **lista_bloqueados,instruccion_io instruccion_io);
instruccion_io sacar_entrada_salida(nodo_entrada_salida **lista_bloqueados);

#endif /* MANEJO_LISTAS_H_ */
