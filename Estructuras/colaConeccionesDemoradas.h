/*
 * colaConeccionesDemoradas.h
 *
 *  Created on: 09/11/2012
 *      Author: utnso
 */

#ifndef COLACONECCIONESDEMORADAS_H_
#define COLACONECCIONESDEMORADAS_H_

struct coneccionesDemoradas {
	int socket_cliente;
	struct coneccionesDemoradas *siguiente;
} typedef coneccionesDemoradas;

void encolar_solicitud(int cliente_sock,coneccionesDemoradas *coneccionesDemoradas);

#endif /* COLACONECCIONESDEMORADAS_H_ */
