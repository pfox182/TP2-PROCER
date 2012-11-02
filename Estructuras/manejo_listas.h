/*
 * manejo_listas.h
 *
 *  Created on: 02/11/2012
 *      Author: utnso
 */

#ifndef MANEJO_LISTAS_H_
#define MANEJO_LISTAS_H_

 struct nodo_proceso{
	pcb pcb;
	struct nodo_proceso *sig;
} typedef nodo_proceso;

void agregar_proceso(nodo_proceso **proceso,pcb pcb);
pcb sacar_proceso(nodo_proceso **proceso);
void liberar_lista_de_procesos(nodo_proceso **proceso);

#endif /* MANEJO_LISTAS_H_ */
