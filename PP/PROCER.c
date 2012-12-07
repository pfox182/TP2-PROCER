#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h> //Contiene la funcion sleep
#include <semaphore.h>
#include <pthread.h>

#include "../FuncionesPropias/manejo_archivos.h"
#include "../Estructuras/proceso.h"
#include "../Estructuras/manejo_pila_ejecucion.h"
#include "../Estructuras/manejo_mensajes.h"
#include "../Log/manejo_log.h"

#include "PROCER_funciones.h"

//Prototipos
int tengo_que_contar_quantum(char* instruccion);
int es_un_token_nulo(char *palabra);
int enviar_proceso_terminado(proceso proceso);
int liberar_proceso(proceso *proceso);
int las_listas_estan_vacias_procer();
int cantidad_nodos(nodo_proceso **listaAPlanificar);

//Variables globales pp.c
int cant_instrucciones_ejecutadas;
extern unsigned int mmp;
extern unsigned int mps;
extern int suspendido;
	//Semaforos
extern pthread_mutex_t mutexListaListos;
extern pthread_mutex_t mutexListaSuspendidos;
extern pthread_mutex_t mutexVarSuspendido;
extern pthread_mutex_t mutexVarMMP;
extern pthread_mutex_t mutexVarMPS;
extern pthread_mutex_t mutexVarCantInstruccionesEjecutadas;

//Listas globales
extern nodo_proceso **listaProcesosListos;
extern nodo_proceso **listaProcesosSuspendidos;

//AUX
void mostrar_datos(data *datos);

pthread_t id_hilo_procer;

void * PROCER_funcion(){
	id_hilo_procer=pthread_self();

	while(1){
		if ( las_listas_estan_vacias_procer() != 0 ){
			pthread_mutex_lock(&mutexVarCantInstruccionesEjecutadas);
			cant_instrucciones_ejecutadas=0;
			pthread_mutex_unlock(&mutexVarCantInstruccionesEjecutadas);

			printf("PROCER - Antes de sacar de listos hay %d procesos\n",cantidad_nodos(listaProcesosListos));
			mostrar_lista(listaProcesosListos);

			pthread_mutex_lock(&mutexListaListos);
			proceso proceso=sacar_proceso(listaProcesosListos);
			pthread_mutex_unlock(&mutexListaListos);

			printf("Saque el proceso %d de listos\n",proceso.pcb.pid);



			//printf("Se saco el proceso PID:%d de listos\n",proceso.pcb.pid);//TODO:BORRAR

			logx(proceso.pcb.pid,"PROCER",id_hilo_procer,"LSCH","Se saco el proceso de la lista de Listos.");

			unsigned int cant_instrucciones = cant_lineas(proceso.pcb.codigo);
			char *instruccion;
			unsigned int cont_quantum = 0;
			int retorno;
			seccion seccion_a_ejecutar;


			while( verificar_fin_ejecucion(proceso,cont_quantum,cant_instrucciones) != -1){

			   if( proceso.pcb.pc == 0 ){//Es la 1° vez que ejecuta
				   agregar_a_pila_ejecucion(crear_seccion("fin_programa",&proceso.pcb.pc ),proceso.pila_ejecucion);
			   }
			   if( suspendido == 1){
				   pthread_mutex_lock(&mutexVarSuspendido);

				   suspendido = 0;
				   pthread_mutex_lock(&mutexVarMMP);
				   mmp--;
				   pthread_mutex_unlock(&mutexVarMMP);

				   logx(proceso.pcb.pid,"PROCER",id_hilo_procer,"INFO","Se suspendio el proceso.");
				   pthread_mutex_lock(&mutexListaSuspendidos);
				   agregar_proceso(listaProcesosSuspendidos,proceso);
				   pthread_mutex_unlock(&mutexListaSuspendidos);
				   logx(proceso.pcb.pid,"PROCER",id_hilo_procer,"LSCH","Se agrego el proceso a la lista de Suspendidos.");

				   pthread_mutex_unlock(&mutexVarSuspendido);
				   break;
			   }else{//No se suspendio la ejecucion

				   seccion_a_ejecutar=sacar_primera_seccion(proceso.pila_ejecucion);
				   //printf("La instruccion extraida de la pila es %s\n",seccion_a_ejecutar.nombre_seccion);
				   if( strcmp(seccion_a_ejecutar.nombre_seccion,"") == 0){
					   logx(proceso.pcb.pid,"PROCER",id_hilo_procer,"ERROR","Error al sacar la seccion a ejecutar, es nula.");
					   break;
				   }
					//Leemos la siguiente instruccion a ejecutar
					instruccion = leer_instruccion(proceso.pcb.codigo,*seccion_a_ejecutar.contador_instruccion);

					if( instruccion != NULL){
						//Calculo la proxima instruccion a leer
						++(*seccion_a_ejecutar.contador_instruccion);
						if( tengo_que_contar_quantum(instruccion) == 0){
							cont_quantum++;
						}

						if( strcmp(instruccion,seccion_a_ejecutar.nombre_seccion) != 0){//No es el fin de la seccion a ejecutar
							agregar_a_pila_ejecucion(seccion_a_ejecutar,proceso.pila_ejecucion);
						}

						if( strcmp(instruccion,"fin_programa") != 0){
							retorno = ejecutar_instruccion(instruccion,&proceso,&seccion_a_ejecutar);
							if( retorno == -1){
								char *error="Error al ejecutar instruccion: ";
								strcat(error,instruccion);
								logx(proceso.pcb.pid,"PROCER",id_hilo_procer,"ERROR",error);
							}
							if( retorno == 1){//Quiere decir que se ejecuto una entrada/salida
								logx(proceso.pcb.pid,"PROCER",id_hilo_procer,"INFO","Se fue a E/S.");
								break;
							}
						}else{
							logx(proceso.pcb.pid,"PROCER",id_hilo_procer,"INFO","Finalizo la ejecucion");

							pthread_mutex_lock(&mutexVarMMP);
							--mmp;
							pthread_mutex_unlock(&mutexVarMMP);
							pthread_mutex_lock(&mutexVarMPS);
							--mps;
							pthread_mutex_unlock(&mutexVarMPS);

							//mostrar_datos(proceso.pcb.datos);
							if ( enviar_proceso_terminado(proceso) == 0){
								logx(proceso.pcb.pid,"PROCER",id_hilo_procer,"INFO","Se enviao el estado del proceso al PI.");
							}else{
								logx(proceso.pcb.pid,"PROCER",id_hilo_procer,"ERROR","Error al enviar el estado del proceso al PI.");
							}
							liberar_proceso(&proceso);
							logx(proceso.pcb.pid,"PROCER",id_hilo_procer,"INFO","Se libero la memoria del proceso.");
							break;
						}
					}
					bzero(instruccion,strlen(instruccion));
				}
			}
			cont_quantum=0;
		}else{
			sleep(1);
		}
	}
	return 0;
}
int tengo_que_contar_quantum(char* instruccion){
	char *palabra;
	char *resto=(char *)malloc(strlen(instruccion));
	bzero(resto,strlen(instruccion));
	memcpy(resto,instruccion,strlen(instruccion));

	while( resto != NULL){
		palabra = strtok(resto," ");
		resto = strtok(NULL,"\0");

		if( es_un_token_nulo(palabra)){
			return 1;
		}
	}

	return 0;
}

int enviar_proceso_terminado(proceso proceso){
	int h,i;
	stack *aux;
	char template[]="------------------------------------------\n\n";
	char template1[]="ID=";
	char template2[]="PC=";
	char template3[]="\n- Estructura de codigo ----\n";
	char template4[]="-------------------------\n\n- Estructura de Datos ----\n";
	char template5[]="-------------------------\n\n- Estructura de Stack ----\n";

	char *respuestaReanudo=(char *)malloc(strlen("si"));
	bzero(respuestaReanudo,strlen("si"));
	char *numero=(char *)malloc(strlen("00000"));
	bzero(numero,strlen("00000"));
	char *var=(char *)malloc(sizeof(char));
	bzero(var,sizeof(char));
	char *id=(char *)malloc(strlen("00000"));
	bzero(id,strlen("00000"));
	char *pc=(char *)malloc(strlen("00000"));
	bzero(pc,strlen("00000"));
	char *funcion=(char *)malloc(256);
	bzero(funcion,256);
	char *msjVariables=(char *)malloc(1024);//mirar tamaño
	bzero(msjVariables,1024);


	strcpy(msjVariables,"El proceso ha finalizado:\n");


	strcat(msjVariables,template);

	//ID
	strcat(msjVariables,template1);
	sprintf(id,"%d",proceso.pcb.pid);
	strcat(msjVariables,id);
	strcat(msjVariables,"\n");
	//PC
	strcat(msjVariables,template2);
	sprintf(pc,"%d",proceso.pcb.pc);
	strcat(msjVariables,pc);
	strcat(msjVariables,"\n");
	//CODIGO
	strcat(msjVariables,template3);
	strcat(msjVariables,proceso.pcb.codigo);

	//VARIABLES
	strcat(msjVariables,template4);
	for( i=0;proceso.pcb.datos[i].variable;i++){

		//filtrar variables que no estan en el proceso
		var[0]=proceso.pcb.datos[i].variable;
		var[1]='\0';
		strcat(msjVariables,var);
		strcat(msjVariables,"=");
		sprintf(numero,"%d",proceso.pcb.datos[i].valor);
		strcat(msjVariables,numero);
		strcat(msjVariables,"\n");

	}

	//FUNCIONES
	h = 1;
	aux=proceso.pcb.pila;

	while ( aux != NULL ){
		if ( aux->linea <= proceso.pcb.pc ){
			if ( h == 1){
			strcat(msjVariables,template5);
			}
			sprintf(funcion,"%d",aux->linea);
			strcat(funcion,",");
			strcat(funcion,aux->funcion);
			strcat(msjVariables,funcion);
			strcat(msjVariables,"\n");
		}
		h = 0;
		aux = aux->siguiente;
	}

	//Envio mensaje variables proceso suspendido.
	if ( enviar_mensaje(msjVariables,proceso.cliente_sock) == -1 ){
		return -1;
	}

	//Libero Malloc
	free(respuestaReanudo);
	free(numero);
	free(var);
	free(id);
	free(pc);
	free(funcion);
	free(msjVariables);

	return 0;
}

int liberar_proceso(proceso *proceso){
	//printf("Free - liberar_proceso\n");
	//free(proceso->pcb.codigo);
	//printf("Libere el codigo\n");
	free(proceso->pcb.datos);
	//printf("Libere los datos\n");
	free(proceso->pcb.pila);
	//printf("Libere la pila\n");
	free(proceso->pila_ejecucion);
	//printf("Libere la pila de ejecucion\n");
	close(proceso->cliente_sock);
	//printf("Cerre la conexion\n");

	return 0;
}

int las_listas_estan_vacias_procer(){
	if( *listaProcesosListos == NULL ){
		return 0;
	}
	return 1;
}

//auxiliar
void mostrar_datos(data *datos){
	//Muestro vector
	int i;
	for (i = 0; datos[i].variable  ; i++)
	{
		printf("El valor de datos[%d] es var:%c valor:%d,\n",i,datos[i].variable,datos[i].valor);
	}
}
