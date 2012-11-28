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
#include <pthread.h>
#include <semaphore.h>

#include "../Estructuras/proceso.h"
#include "../Estructuras/manejo_listas.h"
#include "../Estructuras/colaConeccionesDemoradas.h"
#include "../Estructuras/manejo_listas_funciones.h"
#include "../Estructuras/manejo_mensajes.h"
#include "LTS_suspendido.h"

//aux
void mostrar_funciones(stack *pila);

//Prototipos de funcion
int server_socket(char *port);
proceso crear_proceso(char *buffer,int socket);
data* cargar_datos(char *buffer);
void error(const char *msg);
stack* sacar_funciones(char *buffer);
int administrar_conexion(int cliente_sock,fd_set *master);
int validar_mps_mmp(int cliente_sock);


//Variables globales
extern unsigned int mps,mmp,max_mps,max_mmp; //Se usa extern para indicar que son variables globales de otro archivo
extern unsigned int pid;
extern char* puerto;
extern int lpn;
	//Semaforos
extern pthread_mutex_t mutexListaNuevos;
extern pthread_mutex_t mutexVarMaxMMP;
extern pthread_mutex_t mutexVarMaxMPS;
extern pthread_mutex_t mutexVarMMP;
extern pthread_mutex_t mutexVarMPS;
extern pthread_mutex_t mutexVarLPN;

//Listas
extern nodo_proceso **listaProcesosNuevos;
extern coneccionesDemoradas **listaConeccionesDemoradas;

void * LTS_funcion(void * var){
	pthread_t LTS_suspendido_hilo;

	printf("Creando hilo de LTS\n");


	pthread_create(&(LTS_suspendido_hilo), NULL, LTS_suspendido, NULL); // Creamos el thread LTS suspendido
	printf("Soy el hilo de LTS_suspendido.\n");

	server_socket(puerto);

	//pthread_exit(NULL);// Última función que debe ejecutar el main() siempre
	return 0;
}


/******* socket_server() *********************
 Crea un socket servidor y lo deja a la escucha
 de nuevas conecciones. Al detectar una peticion
 de coneccion de un cliente la acepta y la delega
 en un proceso hijo, el cual la adminstra con
 la funcion administrar_coneccion().
 ****************************************/
int server_socket(char *port)
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

     int portno;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     int yes=1;

     if (port == NULL) {
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
     portno = atoi(port);//Transformamos el char* a int
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
    	    	  	 administrar_conexion(cliente_sock,&master);
    			 }
    		 }
    	}
     }

     return 0; //Nunca se deberia llegar aca
}

/******* administrar_conexion() *********************
 Realiza todos los procesos de validacion de mmp y mps, llama
 a lo procedimintos para crear el proceso y realiza las
 encolaciones necesarias.
 ****************************************/
int administrar_conexion(int cliente_sock,fd_set *master){
	int retorno;
	char *buffer=(char *)malloc(1);
	proceso proceso;

	printf("Entre a administrar conexion\n");
	int socket_demorado;
	if((socket_demorado=sacar_conexion_demorada(listaConeccionesDemoradas))>0){
		printf("Pase la obtencio de socker demorado, que es %d\n",socket_demorado);
		if( (retorno = validar_mps_mmp(socket_demorado)) == 0){
			printf("El sokcet demorado es %d\n",socket_demorado);
			recibir_mensaje(&buffer,socket_demorado);
			printf("Recibi el codigo del socket demorado\n");
			proceso = crear_proceso(buffer,socket_demorado);

			pthread_mutex_lock(&mutexListaNuevos);
			agregar_proceso(listaProcesosNuevos,proceso);
			pthread_mutex_unlock(&mutexListaNuevos);



		}else{
			if( retorno == 1){
				printf("Volvi a encolar el socket demorado\n");
				encolar_primero(listaConeccionesDemoradas,socket_demorado);
			}else{
				printf("Se produjo un error al validar el mmp y mps.\n");
			}
		}
		FD_CLR(socket_demorado,&(*master));
	}

	// if( (retorno = validar_mps_mmp(cliente_sock)) ==0 ){
	if(recibir_mensaje(&buffer,cliente_sock) == 0){
		 printf("Recibi el codigo del proceso nuevo, que es: %s\n",buffer);
		 printf("El socket del cliente es %d\n",cliente_sock);
		 //Creamos el proceso
		proceso = crear_proceso(buffer,cliente_sock);
		printf("Sali de crear proceso, pid= %d\n",proceso.pcb.pid);
		if( (retorno = validar_mps_mmp(cliente_sock)) ==0 ){

			printf("Sali del Validar\n");

			pthread_mutex_lock(&mutexListaNuevos);
			agregar_proceso(listaProcesosNuevos,proceso);
			printf("Agregue el proceso nuevo %d\n",proceso.pcb.pid);
			pthread_mutex_unlock(&mutexListaNuevos);

			pthread_mutex_lock(&mutexVarMPS);
			mps++;
			pthread_mutex_unlock(&mutexVarMPS);

			pthread_mutex_lock(&mutexVarMMP);
			mmp++;
			pthread_mutex_unlock(&mutexVarMMP);

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

		 }else{
			 //bzero(buffer,strlen(buffer));
			 if( retorno == -1){
				printf("Se produjo un error al validar el mmp y mps.\n");
			}
		 }
	 }
	 FD_CLR(cliente_sock,&(*master));
	 return 0;
}
int validar_mps_mmp(int cliente_sock){

	//TODO: nose si hay que poner un semaforo tambien para MMP y MPS
	pthread_mutex_lock(&mutexVarMaxMPS);
	pthread_mutex_lock(&mutexVarMaxMMP);
	if( mps >= max_mps || mmp >= max_mmp){//Si no entra al if => todo. ok
		if( mps >= max_mps){
			enviar_mensaje("Se sobrepaso el maximo de prosesos en el sistema(mps).\n",cliente_sock);
			printf("Sobrepaso de mps\n");
			close(cliente_sock);
			return 1;
		}else{
			enviar_mensaje("Se sobrepaso el maximo de multiprogramacion(mmp), se encolara su solicitud.\n",cliente_sock);
			printf("Sobrepaso de mmp\n");
			encolar_solicitud(listaConeccionesDemoradas,cliente_sock);
			return 1;
		}
	}
	pthread_mutex_unlock(&mutexVarMaxMMP);
	pthread_mutex_unlock(&mutexVarMaxMPS);

	printf("mps y mmp ok\n");
	return 0;
}

proceso crear_proceso(char *buffer,int socket){
	proceso proceso;
	pcb pcb;

	pcb.pid = ++pid;
	pcb.pc = 0;

	if( strlen(buffer) == 0){
		printf("El buffer en crear_proceso esta vacio\n");
	}

	printf("Pase la primera parte de crear proceso\n");
	pcb.codigo = (char *)malloc(strlen(buffer));
	bzero(pcb.codigo,strlen(buffer));
	memcpy(pcb.codigo,buffer,strlen(buffer));

	printf("Estoy por sacar funciones\n");
	pcb.pila= sacar_funciones(buffer);
	printf("Estoy por sacar datos\n");
	pcb.datos = cargar_datos(buffer);

	printf("Estoy por hacer bzero\n");
	bzero(buffer,strlen(buffer));

	proceso.pcb = pcb;

	pthread_mutex_lock(&mutexVarLPN);
	proceso.prioridad = lpn;
	pthread_mutex_unlock(&mutexVarLPN);

	proceso.pila_ejecucion = (pila_ejecucion **)malloc(sizeof(pila_ejecucion));
	bzero(proceso.pila_ejecucion,sizeof(pila_ejecucion));
	proceso.cliente_sock = socket;

	free(buffer);
	return proceso;
}

data* cargar_datos(char *buffer){
	//Declaro variables

	data *puntero;
	data *datos=(data *)malloc(sizeof(data)*26);//Antes data datos[26];
	bzero(datos,sizeof(data)*26);
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
	stack **lista_funciones=(stack **)malloc(sizeof(stack));
	//stack *lista_aux=(stack *)malloc(sizeof(stack));
	//bzero(lista_aux,sizeof(stack));
	//memcpy(lista_funciones,lista_aux,sizeof(stack));
	*lista_funciones=NULL;

	numero_linea = 0;

	printf("Estoy por entrar al while\n");
	while( resto != NULL){
		printf("Estoy en el while =(\n");
		linea = strtok(resto,"\n");
		resto = strtok(NULL,"\0");
		numero_linea++;
		if( strstr(linea,"()") != NULL){
			funcion = strtok(linea,"()");
			agregar_funcion(lista_funciones,funcion,numero_linea);
		}

	}
	printf("Sali del while\n");

	resto=NULL;
	free(resto);

	return *lista_funciones;
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
	stack *aux=pila;
	//Muestro vector
	while( aux != NULL && aux->linea < 30 ){
		printf("Funcion %s , en la linea %d\n",aux->funcion,aux->linea);
		aux=aux->siguiente;
	}
}
