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
#include "../Log/manejo_log.h"
#include "../Estructuras/manejo_listas.h"
#include "../Estructuras/colaConeccionesDemoradas.h"
#include "../Estructuras/manejo_listas_funciones.h"
#include "../Estructuras/manejo_mensajes.h"
#include "LTS_suspendido.h"
#include "LTS_demorado.h"

//aux
//void mostrar_funciones(stack *pila);

//Prototipos de funcion
int server_socket(char *port);
void error(const char *msg);
int administrar_conexion(int cliente_sock,fd_set *master);
int validar_mps_mmp(int cliente_sock);


//Variables globales
extern unsigned int mps,mmp,max_mps,max_mmp; //Se usa extern para indicar que son variables globales de otro archivo
extern char* puerto;
	//Semaforos
extern pthread_mutex_t mutexListaNuevos;
extern pthread_mutex_t mutexListaDemorados;
extern pthread_mutex_t mutexVarMaxMMP;
extern pthread_mutex_t mutexVarMaxMPS;
extern pthread_mutex_t mutexVarMMP;
extern pthread_mutex_t mutexVarMPS;

extern sem_t *sem_sts;
extern sem_t *sem_lts_demorado;

//Listas
extern nodo_proceso **listaProcesosNuevos;
extern coneccionesDemoradas **listaConeccionesDemoradas;

void * LTS_funcion(void * var){
	pthread_t LTS_suspendido_hilo;
	pthread_t LTS_demorado_hilo;

	pthread_create(&(LTS_suspendido_hilo), NULL, LTS_suspendido, NULL); // Creamos el thread LTS suspendido
	pthread_create(&(LTS_demorado_hilo), NULL, LTS_demorado, NULL); // Creamos el thread LTS demorado

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
					  //printf("Nueva coneccion desde en socket %d\n",newfd);
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
	char *paso_mensaje=(char *)malloc(256);
	char *buffer=(char *)malloc(1024);
	bzero(buffer,1024);
	char *prioridad=(char *)malloc(64);
	bzero(prioridad,64);
	proceso proceso;
	pthread_t id_hilo=pthread_self();

	// if( (retorno = validar_mps_mmp(cliente_sock)) ==0 ){
	if(recibir_mensaje(&buffer,cliente_sock) == 0){
		if( recibir_mensaje(&prioridad,cliente_sock) == 0){
			if( (retorno = validar_mps_mmp(cliente_sock)) ==0 ){

				//Creamos el proceso
				printf("Me llego la prioridad: %s\n",prioridad);
				proceso = crear_proceso(buffer,prioridad,cliente_sock);
				bzero(paso_mensaje,256);
				sprintf(paso_mensaje,"Se creo el proceso con PID=%d\n",proceso.pcb.pid);
				enviar_mensaje(paso_mensaje,cliente_sock);


				logx(proceso.pcb.pid,"LTS",id_hilo,"INFO","El proceso ha sido creado.");
				char *log_text=(char *)malloc(127);
				sprintf(log_text,"La prioridad del proceso es %d.",proceso.prioridad);
				logx(proceso.pcb.pid,"LTS",id_hilo,"DEBUG",log_text);
				//if( log_text != NULL ){ free(log_text);}
	//			if ( buffer != NULL ){
	//				free(buffer);
	//			}
	//			if ( prioridad != NULL ){
	//				free(prioridad);
	//			}

				pthread_mutex_lock(&mutexListaNuevos);
				agregar_proceso(listaProcesosNuevos,proceso);
				pthread_mutex_unlock(&mutexListaNuevos);
				sem_post(sem_sts);
				logx(proceso.pcb.pid,"LTS",id_hilo,"LSCH","Agregue el proceso a la lista de Nuevos.");

				pthread_mutex_lock(&mutexVarMPS);
				mps++;
				pthread_mutex_unlock(&mutexVarMPS);
				logx(proceso.pcb.pid,"LTS",id_hilo,"INFO","Se aumento el grado de procesos en el sistema.");

				pthread_mutex_lock(&mutexVarMMP);
				mmp++;
				pthread_mutex_unlock(&mutexVarMMP);
				logx(proceso.pcb.pid,"LTS",id_hilo,"INFO","Se aumento el grado de multiprogramacion.");

			 }else{
				 if( retorno == -1){
					 logx(proceso.pcb.pid,"LTS",id_hilo,"ERROR","Se sobrepaso el maximo de proceso en el sistema.");
				}
				 if( retorno == -2){
					logx(proceso.pcb.pid,"LTS",id_hilo,"ERROR","Se sobrepaso el maximo grado de multiprogramacion.");
					pthread_mutex_lock(&mutexVarMPS);
					mps++;
					pthread_mutex_unlock(&mutexVarMPS);
					logx(proceso.pcb.pid,"LTS",id_hilo,"INFO","Se aumento el grado de procesos en el sistema.");
				}
			 }
		}
	 }
	 FD_CLR(cliente_sock,&(*master));
	 return 0;
}
int validar_mps_mmp(int cliente_sock){

	pthread_mutex_lock(&mutexVarMaxMPS);
	pthread_mutex_lock(&mutexVarMaxMMP);
	pthread_mutex_lock(&mutexVarMPS);
	pthread_mutex_lock(&mutexVarMMP);
	if( mps >= max_mps || mmp >= max_mmp){//Si no entra al if => todo. ok
		pthread_mutex_unlock(&mutexVarMMP);
		pthread_mutex_unlock(&mutexVarMaxMMP);

		if( mps >= max_mps){
			pthread_mutex_unlock(&mutexVarMPS);
			pthread_mutex_unlock(&mutexVarMaxMPS);

			enviar_mensaje("Se sobrepaso el maximo de prosesos en el sistema(mps).\n",cliente_sock);
			close(cliente_sock);
			return -1;
		}else{
			pthread_mutex_unlock(&mutexVarMPS);
			pthread_mutex_unlock(&mutexVarMaxMPS);
			enviar_mensaje("Se sobrepaso el maximo de multiprogramacion(mmp), se encolara su solicitud.\n",cliente_sock);

			pthread_mutex_lock(&mutexListaDemorados);
			encolar_solicitud(listaConeccionesDemoradas,cliente_sock);
			pthread_mutex_unlock(&mutexListaDemorados);

			sem_post(sem_lts_demorado);

			return -2;
		}

	}else{
		pthread_mutex_unlock(&mutexVarMPS);
		pthread_mutex_unlock(&mutexVarMMP);
		pthread_mutex_unlock(&mutexVarMaxMMP);
		pthread_mutex_unlock(&mutexVarMaxMPS);
	}

	return 0;
}

