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

#include "../Estructuras/proceso.h"

#define BUFFER_SIZE 1024

//Prototipos de funcion
int server_socket(char *puerto);
int recvall(int client_fd,char *buffer,int *header,int flag);
proceso crear_proceso(char *buffer);
data* cargar_datos(char *buffer);
void error(const char *msg);
stack* sacar_funciones(char *buffer);
int notifica_sobrepaso_mps(int cliente_sock);
int notificar_demora_mpp(int cliente_sock);
int encolar_solicitud(int cliente_sock);
int agregar_proceso_a_lista_nuevos(proceso proceso);

//Variables globales
extern unsigned int mps,mpp,max_mps,max_mpp; //Se usa extern para indicar que son variables globales de otro archivo
extern unsigned int pid;

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
		int cliente_sock;//Representan descriptores en un for
		char *buffer=(char *)malloc(BUFFER_SIZE);
		int nbytes;

	 int header;
     int portno;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     int yes=1;
     //proceso proceso;

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
    	 for(cliente_sock=0;cliente_sock<=fdmax;cliente_sock++){
    	      if(FD_ISSET(cliente_sock,&read_fds)){
    	    	  if( cliente_sock == listener ){
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

    				 //TODO:¿Fijarse si se reanudo algun proceso?

    				 //TODO:Antes de crear un nuevo proceso hay que fijarce que no halla otra coneccion demorada

    				 //Validamos las variables mps y mpp
    				 	 //TODO:implementar validacion
    	    	  	if( mps >= max_mps || mpp >= max_mpp){//Si no entra al if => todo. ok
						if( mps >= max_mps){
							//notifica_sobrepaso_mps(cliente_sock);
							close(cliente_sock);
						}else{
							//notificar_demora_mpp(cliente_sock);
							//encolar_solicitud(cliente_sock);
						}
    	    	  	}

					 //Recibimos el header del PI
					 if( (nbytes = recv(cliente_sock,&header,sizeof(header),0)) <= 0){
						 //Error o conexion cerrada por el cliente
						 if( nbytes == 0){
							 //Conexion cerrada
							 printf("El socket %d cerro la conexion\n",cliente_sock);
						 }else{
							 error("Error al recibir datos del header");
						 }
						 close(cliente_sock);
						 FD_CLR(cliente_sock,&master);//Elimiar del conjunto maestro
					 }

					 printf("El header recibido es: %d \n",header);

					 if ((nbytes = recvall(cliente_sock,buffer,&header,0)) <= 0){
						 //Error o conexion cerrada por el cliente
						 if( nbytes == 0){
							 //Conexion cerrada
							 printf("El socket %d cerro la conexion\n",cliente_sock);
						 }else{
							 error("Error al recibir datos del archivo");
						 }

						 printf("Se recibio:\n %s",buffer);

						 //Enviar confirmacion de que se recibio algo
						 char *msj="Recivi el mensaje";
						 if( (nbytes=send(cliente_sock,msj,nbytes,19)) <= 0){
							 if(nbytes == 0){
								 printf("El cliente %i cerro la conexion y no se envio msj de confirmacion.\n",cliente_sock);
							 }else{
								 error("Error al enviar confirmacion");
							 }
						 }

						 printf("Mensaje de confirmacion enviado");

						 //Creamos el proceso
						 //proceso = crear_proceso(buffer);

						 //TODO: IMPLEMENTAR SEMAFOROS PARA LA LISTA DE NUEVOS
						 //agregar_proceso_a_lista_nuevos(proceso);

						 //TODO: IMPLEMENTAR SEMAFOROS
						 mps++;
						 mpp++;

						 close(cliente_sock);
						 FD_CLR(cliente_sock,&master);//Elimiar del conjunto maestro
					 }

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

proceso crear_proceso(char *buffer){
	proceso proceso;
	pcb pcb;

	//TODO:IMPLEMENTAR SEMAFOROS
	pcb.pid = ++pid;
	pcb.pc = 0;

	pcb.codigo = (char *)malloc(sizeof(buffer));
	memcpy(pcb.codigo,buffer,sizeof(buffer));

	pcb.datos = cargar_datos(buffer);
	pcb.pila = sacar_funciones(buffer);

	free(buffer);

	proceso.pcb = pcb;
	proceso.prioridad = 0;
	return proceso;
}

data* cargar_datos(char *buffer){
	//Declaro variables

	data *puntero;
	data datos[26];
	int i;
	char j;
	char *separacion;
	int flag;
	char *resto=buffer;
	char *linea;

	//Inicializo el vector de variables

	for (i = 0,j='a'; i < 26; i++,j++)
	{
		datos[i].variable = j;
		datos[i].valor = -1;
	}

	while( resto != NULL){
		linea = strtok(resto,"\n");
		resto = strtok(NULL,"\0");

		if( strstr(linea,"variable") != NULL){
			//Lee la cadena y cargo en vector las variables existentes.
			separacion = strtok(linea," ");
			separacion = strtok(NULL,",");
			while( separacion != NULL )
			{
				//busco posicion de la variable en el vector
				i=0;
				flag=1;
				while(flag != 0)
				{
					if ( datos[i].variable == *separacion ){
						flag = 0;
					}
					else{
						i++;
					}
				}
				datos[i].valor = 0;
				separacion = strtok(NULL,",");
			}
		}
	}

	puntero = &datos[0];




	//Muestro vector
	for (i = 0; i < 26 ; i++)
	{
		printf("El valor de datos[%d] es var:%c valor:%d,\n",i,datos[i].variable,datos[i].valor);
	}

	return puntero;
}

stack* sacar_funciones(char *buffer){
	//int cantidad_de_funciones = 10;
	stack* pila = (stack *)malloc(sizeof(stack));
	/*
	 * while(funciones en buffer){
	 * 	pila = (stack *)realloc(sizeof(stack));
	 * 	pila.funcion = funcion;
	 * 	pila.linea = linea;
	 * }
	 */
	//TODO:IMPLEMENTAR
	return pila;
}

int agregar_proceso_a_lista_nuevos(proceso proceso){
	return 0;
}

int notifica_sobrepaso_mps(int cliente_sock){
	//TODO:IMPLEMENTAR
	return 0;
}

int notificar_demora_mpp(int cliente_sock){
	//TODO:IMPLEMENTAR
	return 0;
}

int encolar_solicitud(int cliente_sock){
	//TODO:IMPLEMENTAR
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
