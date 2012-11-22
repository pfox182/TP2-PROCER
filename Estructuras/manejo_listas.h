/*
 * manejo_listas.h
 *
 *  Created on: 02/11/2012
 *      Author: utnso
 */

#ifndef MANEJO_LISTAS_H_
#define MANEJO_LISTAS_H_

#include "proceso.h"

void agregar_proceso(nodo_proceso **lista_procesos,proceso proceso);
proceso sacar_proceso(nodo_proceso **proceso);
void liberar_lista_de_procesos(nodo_proceso **proceso);
void agregar_entrada_salida(nodo_entrada_salida **lista_bloqueados,instruccion_io instruccion_io);
void agregar_primero_entrada_salida(nodo_entrada_salida **lista_bloqueados,instruccion_io instruccion_io);
instruccion_io sacar_entrada_salida(nodo_entrada_salida **lista_bloqueados);
void agregar_lista_de_procesos(nodo_proceso **listaProcesosListos, nodo_proceso **listaAAgregar,int prioridad);
void mostrar_lista(nodo_proceso **lista);

#endif /* MANEJO_LISTAS_H_ */
