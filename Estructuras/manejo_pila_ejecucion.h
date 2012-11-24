/*
 * manejo_pila_ejecucion.h
 *
 *  Created on: 24/11/2012
 *      Author: utnso
 */

#ifndef MANEJO_PILA_EJECUCION_H_
#define MANEJO_PILA_EJECUCION_H_

#include "proceso.h"

seccion crear_seccion(char *nombre,unsigned int *contador);
seccion sacar_primera_seccion(pila_ejecucion **pila);
void agregar_a_pila_ejecucion(seccion n_seccion,pila_ejecucion **pila_ejecuccion);


#endif /* MANEJO_PILA_EJECUCION_H_ */
