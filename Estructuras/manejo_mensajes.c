/*
 * manejo_mensajes.c
 *
 *  Created on: 23/11/2012
 *      Author: utnso
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include "manejo_mensajes.h"


int enviar_mensaje(char* mensaje,int socket_client){

		//Armamos el paquete a enviar al server
		int header = strlen(mensaje);
		printf("Header:%d\n",header);
		//Enviamos el header para indicar el tamaño del paquete
		if( send(socket_client,&header,sizeof(header),0) == -1){
			printf("Error al enviar el header\n");
			return -1;
		}
		printf("Mensaje:%s\n",mensaje);
		//Enviamos el paquete
		if( sendall(socket_client,&(*mensaje),&header) == -1){
			printf("Error al enviar el paquete\n");
			return -1;
		}
		return 0;

}

int recibir_mensaje(char** mensaje,int socket_client){

	int nbytes;
	int header;

		//Recibimos el header del PI
		if( (nbytes = recv(socket_client,&header,sizeof(header),0)) <= 0){
			 //Error o conexion cerrada por el cliente
			 if( nbytes == 0){
				 //Conexion cerrada
				 printf("El socket %d cerro la conexion\n",socket_client);
				 return -1;
			 }else{
				 printf("Error al recibir datos del header");
				 return -1;
			 }
		}

		 printf("El header recibido es: %d \n",header);
		 //Seteamos el tamaño del mensaje
		 *mensaje=(char *)realloc(*mensaje,header);
		 bzero(*mensaje,header);
		 if (( recvall(socket_client,*mensaje,&header,0)) == -1){
			 printf("Error al recibir datos del archivo");
			 return -1;
		 }


	return 0;
}


int recvall(int client_fd,char *buffer,int *header,int flag){

	int total=0;//Los bytes que recibimos hasta ahora
	int bytes_left=*header;//Los bytes que faltan recibir
	int nbytes = 0;

	//Valido que halla suficiente espacio
	if( sizeof(buffer) < bytes_left){
		printf("Free - recvall\n");
		free(buffer);
		buffer=(char *)malloc(bytes_left+1);
	}

	while( total < *header){
		nbytes = recv(client_fd,buffer+total, bytes_left,flag);
		if( nbytes == -1){break;}
		total =+ nbytes;
		bytes_left =- nbytes;
	}

	*header = total;//Cantidad de paquetes recibidos en realidad

	return nbytes==-1?-1:0;
	return 0;
}




int sendall(int sockfd,char *buffer, int *len){
	int total=0;//Los bytes que enviamos hasta ahora
	int bytes_left=*len;//Los bytes que faltan enviar
	int n = 0;

	while( total < *len){
		n = send(sockfd,buffer+total, bytes_left,0);
		if( n == -1){break;}
		total =+ n;
		bytes_left =- n;
	}

	*len = total;//Cantidad de paquetes enviados en realidad

	return n==-1?-1:0;//Que copada esta sentencia XP
}

