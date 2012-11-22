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
#include "../Estructuras/manejo_listas.h"
#include "../Estructuras/colaConeccionesDemoradas.h"
#include "../Estructuras/manejo_listas_funciones.h"

#define BUFFER_SIZE 1024

//aux
void mostrar_funciones(stack *pila);
extern int global_sts;
//Prototipos de funcion
int server_socket(char *puerto);
proceso crear_proceso(char *buffer);
data* cargar_datos(char *buffer);
void error(const char *msg);
stack* sacar_funciones(char *buffer);
int notifica_sobrepaso_mps(int cliente_sock);
int notificar_demora_mpp(int cliente_sock);
int administrar_conexion(int cliente_sock,fd_set *master,char *buffer);
int validar_mps_mpp(int cliente_sock);
int recibir_codigo(int cliente_sock,fd_set *master,char *buffer);
int recvall(int client_fd,char *buffer,int *header,int flag);
//Variables globales
extern unsigned int mps,mpp,max_mps,max_mpp; //Se usa extern para indicar que son variables globales de otro archivo
extern unsigned int pid;

//Listas
extern nodo_proceso **listaProcesosNuevos;
extern nodo_proceso **listaProcesosReanudados;
extern nodo_proceso **listaProcesosSuspendidos;
extern coneccionesDemoradas **listaConeccionesDemoradas;
extern nodo_proceso **listaTerminados;


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
#include "../Estructuras/proceso.h"
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
    	    	  	 administrar_conexion(cliente_sock,&master,buffer);
    			 }
    		 }
    	}
     }

     return 0; //Nunca se deberia llegar aca
}

/******* administrar_conexion() *********************
 Realiza todos los procesos de validacion de mpp y mps, llama
 a lo procedimintos para crear el proceso y realiza las
 encolaciones necesarias.
 ****************************************/
int administrar_conexion(int cliente_sock,fd_set *master,char *buffer){
	proceso proceso;

	printf("Entre a administrar conexion\n");

	int socket_demorado=sacar_conexion_demorada(listaConeccionesDemoradas);
	printf("Pase la obtencio de socker demorado, que es %d\n",socket_demorado);
	if( socket_demorado > 0 && validar_mps_mpp(socket_demorado)==0){
		printf("Entre a socket demorado\n");
		if(recibir_codigo(socket_demorado,master,buffer) == 0){
			printf("Recibi el codigo del socket demorado\n");
			proceso = crear_proceso(buffer);
			//TODO:implementar semaforos
			agregar_proceso(listaProcesosNuevos,proceso);
			 //TODO: IMPLEMENTAR SEMAFOROS
			 mps++;
			 mpp++;
		}
	}else{
		if( socket_demorado >0 ){
			printf("Volvi a encolar el socket demorado\n");
			encolar_primero(listaConeccionesDemoradas,socket_demorado);
		}
	}

	printf("Pase la parte de socket demorado\n");

	 if(validar_mps_mpp(cliente_sock)==0 ){
		 if(recibir_codigo(cliente_sock,master,buffer) == 0){
		 printf("Recibi el codigo del proceso nuevo, que es: %s\n",buffer);
		 //Creamos el proceso
		 proceso = crear_proceso(buffer);
		 //TODO:implementar semaforos
		 agregar_proceso(listaProcesosNuevos,proceso);
		 global_sts=1;

		 //TODO: IMPLEMENTAR SEMAFOROS
		 mps++;
		 mpp++;

		 printf("El proceso creado fue:\n");
		 	printf("\tPID:%d\n",proceso.pcb.pid);
		 	printf("\tPC:%d\n",proceso.pcb.pc);
		 	printf("\tDatos:\n");
		 	int i;
		 	for( i=0;i<26;i++){
		 		printf("\t variable: %c valor:%d\n",proceso.pcb.datos[i].variable,proceso.pcb.datos[i].valor);
		 	}
		 	mostrar_funciones(proceso.pcb.pila);
		 printf("\tPrioridad:%d\n",proceso.prioridad);


		 close(cliente_sock);
		 FD_CLR(cliente_sock,&(*master));//Elimiar del conjunto maestro
	 }
	 }
	 return 0;
}
int validar_mps_mpp(int cliente_sock){
	//TODO:implementar semaforos
	if( mps >= max_mps || mpp >= max_mpp){//Si no entra al if => todo. ok
		if( mps >= max_mps){
			//notifica_sobrepaso_mps(cliente_sock);
			printf("Sobrepaso de mps\n");
			close(cliente_sock);
			return -1;
		}else{
			//notificar_demora_mpp(cliente_sock);
			printf("Sobrepaso de mpp\n");
			encolar_solicitud(listaConeccionesDemoradas,cliente_sock);
			return -1;
		}
	}
	printf("mps y mpp ok\n");
	return 0;
}
int recibir_codigo(int cliente_sock,fd_set *master,char *buffer){
	int nbytes;
	int header;

	printf("Entre en recibir codigo\n");
	//Recibimos el header del PI
	 if( (nbytes = recv(cliente_sock,&header,sizeof(header),0)) <= 0){
		 //Error o conexion cerrada por el cliente
		 if( nbytes == 0){
			 //Conexion cerrada
			 printf("El socket %d cerro la conexion\n",cliente_sock);
			 return -1;
		 }else{
			 error("Error al recibir datos del header");
			 return -1;
		 }
		 //close(cliente_sock);
		 //FD_CLR(cliente_sock,&(*master));//Elimiar del conjunto maestro
	 }

	 printf("El header recibido es: %d \n",header);

	 if (( recvall(cliente_sock,buffer,&header,0)) == -1){
		 error("Error al recibir datos del archivo");
		 return -1;
	 }

	 //Enviar confirmacion de que se recibio algo
	 char *msj="Recivi el mensaje";
	 if( (nbytes=send(cliente_sock,msj,strlen(msj),0)) <= 0){
		 if(nbytes == 0){
			 printf("El cliente %i cerro la conexion y no se envio msj de confirmacion.\n",cliente_sock);
			 return -1;
		 }else{
			 error("Error al enviar confirmacion");
			 return -1;
		 }
	 }
	 return 0;
}

/******* recvall() *********************
Se encarga de recibir una cantidad de bytes dados(header).
 ****************************************/
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

	pcb.codigo = (char *)malloc(strlen(buffer));
	memcpy(pcb.codigo,buffer,strlen(buffer));

	pcb.pila= sacar_funciones(buffer);
	pcb.datos = cargar_datos(buffer);

	free(buffer);

	proceso.pcb = pcb;
	proceso.prioridad = 0;
	return proceso;
}

data* cargar_datos(char *buffer){
	//Declaro variables

	data *puntero;
	data *datos=(data *)malloc(sizeof(data)*26);//Antes data datos[26];
	int i;
	char j;
	char *separacion;
	int flag;
	char *resto=(char *)malloc(strlen(buffer));
	memcpy(resto,buffer,strlen(buffer));
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

	resto=NULL;
	free(resto);

	return puntero;
}

stack* sacar_funciones(char *buffer){
	int numero_linea;
	char *funcion;
	char *resto=(char *)malloc(strlen(buffer));
	memcpy(resto,buffer,strlen(buffer));
	char *linea;
	stack **lista_funciones=(stack **)malloc(sizeof(stack));;

	numero_linea = 0;

	while( resto != NULL){

		linea = strtok(resto,"\n");
		resto = strtok(NULL,"\0");
		numero_linea++;
		if( strstr(linea,"()") != NULL){
			funcion = strtok(linea,"()");
			agregar_funcion(lista_funciones,funcion,numero_linea);
		}

	}

	resto=NULL;
	free(resto);
	return *lista_funciones;
}

int notifica_sobrepaso_mps(int cliente_sock){
	//TODO:IMPLEMENTAR
	return 0;
}

int notificar_demora_mpp(int cliente_sock){
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

void mostrar_funciones(stack *pila){
	//Muestro vector
	while( pila != NULL){
		printf("Funcion %s , en la linea %d\n",(*pila).funcion,(*pila).linea);
		pila=(*pila).siguiente;
	}
}
