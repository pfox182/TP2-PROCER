/*
 * manejo_mensaje.h
 *
 *  Created on: 23/11/2012
 *      Author: utnso
 */

#ifndef MANEJO_MENSAJE_H_
#define MANEJO_MENSAJE_H_

int recibir_mensaje(char **mensaje,int socket_client);
int enviar_mensaje(char* mensaje,int socket_client);
int recvall(int client_fd,char *buffer,int *header,int flag);
int sendall(int sockfd,char *buffer, int *len);

#endif /* MANEJO_MENSAJE_H_ */
