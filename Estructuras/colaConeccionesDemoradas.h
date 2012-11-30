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

void encolar_solicitud(coneccionesDemoradas **listaConexionesDemoradas,int socket_client);
void encolar_primero(coneccionesDemoradas **listaConexionesDemoradas,int socket_client);
int sacar_conexion_demorada(coneccionesDemoradas **listaConnecionesDemoradas);
void mostrar_lista_conecciones(coneccionesDemoradas **listaProcesos);

#endif /* COLACONECCIONESDEMORADAS_H_ */
