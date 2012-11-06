/*
 * LTS.c
 *
 *  Created on: 26/10/2012
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024

//Prototipos de funcion
int server_socket(char *puerto);
int recvall(int client_fd,char *buffer,int *header,int flag);
void error(const char *msg);

//Variables globales
extern unsigned int mps,mpp,max_mps,max_mpp; //Se usa extern para indicar que son variables globales de otro archivo

void * LTS_funcion(void * var){

	char *puerto="4545";

	printf("Soy el hilo de LTS levantando el server.\n");
	server_socket(puerto);

	return 0;
}


/******* socket_server() *********************
 Crea un socket servidor y lo deja a la escucha
 de nuevas conecciones. Al detectar una peticion
 de coneccion de un cliente la acepta y la delega
 en un proceso hijo, el cual la adminstra con
 la funcion administrar_coneccion().
 ****************************************/
int server_socket(char *puerto)
{
	//Declaraciones para el select
		fd_set master;//Conjunto maestro de descriptores de ficheros
		fd_set read_fds;//Conjunto temporal de descriptores de fichero para select()
		int fdmax;//Numero maximo de descriptores de fichero
		int listener;//Descriptor de socket a la escucha
		int newfd;//Descriptor de nuevo socket a la escucha
		FD_ZERO(&master);//Borra los conjuntos maestro y temporal
		FD_ZERO(&read_fds);
		int i;//Representan descriptores en un for
		char *buffer=(char *)malloc(BUFFER_SIZE);
		int nbytes;

	 int header;
     int portno;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     int yes=1;

     if (puerto == NULL) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }

     listener = socket(AF_INET, SOCK_STREAM, 0);//Obtenemos el descriptor de fichero para el socket
     if (listener < 0){
        error("ERROR opening socket");
     }

     //Oviamos el mensaje "Address alredy in use"
     if( setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1){
    	 error("ERROR on setsockopt");
     }

     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(puerto);//Transformamos el char* a int

     //Creamos la estructura serv_addr
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;//INADDR_ANY indica la direccion ip de esta pc ( debe ir en network byte order )
     serv_addr.sin_port = htons(portno);//hton() -> convierte de Host byte order -> Network byte order )
     memset(&(serv_addr.sin_zero),'\0',8);

     if (bind(listener, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){//Enlazamos el descriptor con la ip y puerto
    	 error("ERROR on binding");
     }

     if(listen(listener,20) == -1){
    	 error("Error al escuchar");
     }

     //Añadir listener al conjunto maestro
     FD_SET(listener,&master);
     //Seguir la pista del descriptor de fichero mayor
     fdmax = listener;

     while (1) {
    	 read_fds = master;
    	 if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1){
    		 error("Error en el select");
    	 }

    	 //Explorar conexiones existentes en busca de datos a leer
    	 for(i=0;i<=fdmax;i++){
    		 if(FD_ISSET(i,&read_fds)){
    			 if( i == listener ){
    				 //Gestionar nuevas conexiones
    				 clilen = sizeof(cli_addr);
    				 if((newfd = accept(listener,(struct sockaddr *) &cli_addr, &clilen)) == -1){
    					 error("Error al aceptar conexion");
    				 }else{
    					 FD_SET(newfd,&master);//Añadir al conjunto maestro
    					 if( newfd > fdmax){//Actualizar el maximo
    						 	fdmax = newfd;
    					 }
    					 printf("Nueva coneccion desde en socket %d\n",newfd);
    				 }
    			 }else{

    				 //Se establecio la coneccion con un proceso PI

    				 //Validamos las variables mps y mpp
    				 	 //TODO:implementar validacion

    				 //Recibimos el header del PI
    				 if( (nbytes = recv(i,&header,sizeof(header),0)) <= 0){
    					 //Error o conexion cerrada por el cliente
						 if( nbytes == 0){
							 //Conexion cerrada
							 printf("El socket %d cerro la conexion\n",i);
						 }else{
							 error("Error al recibir datos del header");
						 }
						 close(i);
						 FD_CLR(i,&master);//Elimiar del conjunto maestro
    				 }

    				 printf("El header recibido es: %d \n",header);

    				 if ((nbytes = recvall(i,buffer,&header,0)) <= 0){
						 //Error o conexion cerrada por el cliente
						 if( nbytes == 0){
							 //Conexion cerrada
							 printf("El socket %d cerro la conexion\n",i);
						 }else{
							 error("Error al recibir datos del archivo");
						 }

						 printf("Se recibio:\n %s",buffer);

						 //Enviar confirmacion de que se recibio algo
						 char *msj="Recivi el mensaje";
						 if( (nbytes=send(i,msj,nbytes,19)) <= 0){
							 if(nbytes == 0){
								 printf("El cliente %i cerro la conexion y no se envio msj de confirmacion.\n",i);
							 }else{
								 error("Error al enviar confirmacion");
							 }
						 }

						 printf("Mensaje de confirmacion enviado");

						 close(i);
						 FD_CLR(i,&master);//Elimiar del conjunto maestro
					 }
    				 /*
    				 if ((nbytes = recv(i,buf,sizeof(buf),0)) <= 0){
    					 //Error o conexion cerrada por el cliente
    					 if( nbytes == 0){
    						 //Conexion cerrada
    						 printf("El socket %d cerro la conexion\n",i);
    					 }else{
    						 error("Error al recibir datos");
    					 }
    					 close(i);
    					 FD_CLR(i,&master);//Elimiar del conjunto maestro
    				 }else{
    					 //Tenemos datos de algun cliente
    					 for(j=0;j <= fdmax;j++){
    						 //Enviar a todos el mundo
    						 if(FD_ISSET(j,&master)){
    							 //Exepto al listener y a nosotros mismos
    							 if( j == i ){//Solo envio mensajes al que me hablo
    								 char *msj="Recivi el mensaje";
    								 if(send(j,msj,nbytes,19) == -1){
    									error("Error al enviar");
    								 }
    							 }
    						 }
    					 }
    				 }
    				 */
    			 }
    		 }
    	}
     }

     return 0; //Nunca se deberia llegar aca
}

int recvall(int client_fd,char *buffer,int *header,int flag){

	int total=0;//Los bytes que recibimos hasta ahora
	int bytes_left=*header;//Los bytes que faltan recibir
	int nbytes = 0;

	//Valido que halla suficiente espacio
	if( sizeof(buffer) < bytes_left){
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
/******* error() *********************
Imprime el stacktrace ante un error
 ****************************************/
void error(const char *msg)
{
    perror(msg);
    exit(1);
}


