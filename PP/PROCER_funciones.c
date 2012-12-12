#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //Contiene la funcion sleep
#include "../FuncionesPropias/manejo_archivos.h"
#include "../Estructuras/proceso.h"
#include "../Estructuras/manejo_listas.h"
#include "../Log/manejo_log.h"
#include "../Estructuras/manejo_pila_ejecucion.h"
#include "../Estructuras/manejo_mensajes.h"
#include "PROCER_funciones.h"
#include <semaphore.h>
#include <pthread.h>



//Variables globales

extern char *lpl;
extern int finQ;
extern unsigned int quantum_max;
extern char *espera_estandar;
extern int cant_iot_disponibles;
extern pthread_mutex_t mutexListaFinQuantum;
extern pthread_mutex_t mutexVarLPL;
extern pthread_mutex_t mutexVarFinQuantum;
extern pthread_mutex_t mutexListaBloqueados;
extern pthread_mutex_t mutexVarEsperaEstandar;
extern pthread_mutex_t mutexVarCantIOTDisponibles;


extern sem_t *sem_sts;
extern sem_t *sem_io;

//Listas globales
extern nodo_entrada_salida **listaBloqueados;
extern nodo_proceso **listaFinQuantum;

extern pthread_t id_hilo_procer;

int verificar_fin_ejecucion(proceso proceso,unsigned int cont_quantum,unsigned int cant_instrucciones){
	int fin=0;

	pthread_mutex_lock(&mutexVarLPL);
	if( strcmp(lpl,"RR") == 0){
		if( cont_quantum >= quantum_max ){

			pthread_mutex_lock(&mutexListaFinQuantum);
			agregar_proceso(listaFinQuantum,proceso);
			printf("Agregue el proceso %d a fin de quantum\n",proceso.pcb.pid);
			pthread_mutex_unlock(&mutexListaFinQuantum);
			sem_post(sem_sts);

			logx(proceso.pcb.pid,"PROCER",id_hilo_procer,"INFO","Se sobrepaso el quantum.");
			logx(proceso.pcb.pid,"PROCER",id_hilo_procer,"LSCH","Se agrego el proceso a la Lista de fin de quantum.");
			fin = -1;
		}
	}
	pthread_mutex_unlock(&mutexVarLPL);

	if( proceso.pcb.pc > cant_instrucciones){
		logx(proceso.pcb.pid,"PROCER",id_hilo_procer,"ERROR","Se sobrepaso el maximo de lineas del codigo del proceso.");
		fin = -1;
	}
	return fin;
}

unsigned int cant_lineas(const char *codigo){
	char *resto=(char *)malloc(strlen(codigo));
	memcpy(resto,codigo,strlen(codigo));
	unsigned int cant_lineas=0;

	while( resto != NULL){
			resto = strtok(resto,"\n");
			resto = strtok(NULL,"\0");
			cant_lineas++;
	}
	if( resto != NULL ){
		//free(resto);
	}
	return cant_lineas;
}

char * leer_instruccion(char *codigo,unsigned int pc){
	char *instruccion=NULL;
	char *linea;
	char *resto=(char *)malloc(strlen(codigo));
	memcpy(resto,codigo,strlen(codigo));
	unsigned int linea_actual=0;

	while( resto != NULL){
			linea = strtok(resto,"\n");
			resto = strtok(NULL,"\0");
			if( linea_actual == pc){
				instruccion = linea;
				break;
			}
			linea_actual++;

	}
	return instruccion;
}

int ejecutar_instruccion(char * instruccion,proceso *proceso,seccion *seccion_ejecutandose){
	char *palabra;
	if(instruccion == NULL){
		logx(proceso->pcb.pid,"PROCER",id_hilo_procer,"ERROR","La instruccion es nula en 'ejecutar_instruccion'.");
		return -1;
	}
	char *resto=instruccion;

	while( resto != NULL){
		palabra = strtok(resto," ");
		resto = strtok(NULL,"\0");

		if( es_un_token_nulo(palabra) == 0){
			//No hacer nada
			break;
		}

		if( es_una_variable(palabra) == 0){//De la forma a=1 o a=b+c
			pthread_mutex_lock(&mutexVarLPL);
				if( strcmp(lpl,"SPN") == 0){
					pthread_mutex_unlock(&mutexVarLPL);
					proceso->instrucciones_spn++;
					proceso->es_instruccion_spn=1;
				}else{
					pthread_mutex_unlock(&mutexVarLPL);
				}
			if ( ejecutar_asignacion(palabra,(*proceso)) == 1){
				return 1;
			}
		}

		if( es_una_funcion(palabra) == 0){//De la forma f10()
			ejecutar_funcion(palabra,proceso);
		}

		if( es_un_salto(palabra) == 0){//De la forma snc o ssc
			if( strcmp(lpl,"SPN") == 0){
				pthread_mutex_unlock(&mutexVarLPL);
				proceso->instrucciones_spn++;
				proceso->es_instruccion_spn=1;
			}else{
				pthread_mutex_unlock(&mutexVarLPL);
			}
			ejecutar_salto(palabra,resto,(*proceso).pcb,seccion_ejecutandose);//Le tengo que pasar la instruccion ej: snc b inicio_for

			break;
		}

		if( es_un_imprimir(palabra) == 0){
			ejecutar_imprimir(resto,*proceso);
			return 1;
		}

		if(es_un_io(palabra) == 0){
			if( ejecutar_io(palabra,*proceso) != -1 ){//-1 => NO hay hilos de i/o disponibles
				return 1;//1 => Me fui a E/S
				break;
			}
		}

	}

	return 0;
}

int  es_un_token_nulo(char *palabra){
	if( strcmp("variables",palabra)==0 || strcmp("comienzo_programa",palabra)==0 || strcmp("",palabra)==0 ) {
		return 0;
	}

	if( palabra[strlen(palabra)-1] == ':'){//Si es una etiqueta
		return 0;
	}
	if( palabra[0] == '#'){
		return 0;
	}
	return -1;
}

int es_una_variable(char* palabra){
	int i,j;
	char letras[26];

	for (i = 0,j='a'; i < 26; i++,j++){letras[i] = j;}
	if( palabra[1] == '='){
		for(i=0;i<26;i++){
			if(letras[i] == palabra[0]){
				return 0;
			}
		}
	}
	return -1;
}

int es_una_funcion(char* palabra){

	if( strstr(palabra,"()") != NULL){
		return 0;
	}
	return -1;
}

int es_un_salto(char* palabra){
	if( strcmp(palabra,"snc")==0 || strcmp(palabra,"ssc")==0){
		return 0;
	}
	return -1;
}
int es_un_imprimir(char* palabra){
	if( strstr(palabra,"imprimir")!= NULL){
		return 0;
	}
	return -1;
}
int es_un_io(char* palabra){
	if( strstr(palabra,"io(")!= NULL){
		return 0;
	}
	return -1;
}


int ejecutar_funcion(char *nombre_funcion,proceso *proceso){
	//printf("Nombre de funcion es %s\n",nombre_funcion);
	nombre_funcion=strtok(nombre_funcion,"()");
	//printf("Nombre de funcion es %s\n",nombre_funcion);

	unsigned int posicion = buscar_inicio_de_funcion(nombre_funcion,(*proceso).pcb.codigo);
	//printf("Pase inicio funcion\n");
	//char *instruccion;
	char fin_funcion[1024];
	bzero(fin_funcion,1024);
	strcpy(fin_funcion,"fin_funcion ");
	strcat(fin_funcion,nombre_funcion);
	unsigned int *cont_funcion=(unsigned int *)malloc(sizeof(unsigned int));
	*cont_funcion=posicion;
	seccion aux;
	aux.nombre_seccion=(char *)malloc(1024);
	bzero(aux.nombre_seccion,1024);
	memcpy(aux.nombre_seccion,fin_funcion,strlen(fin_funcion));
	aux.contador_instruccion=cont_funcion;

	agregar_a_pila_ejecucion(aux,(*proceso).pila_ejecucion);

	return 0;
}
unsigned int buscar_inicio_de_funcion(char *nombre_funcion,char *codigo){
	unsigned int posicion=0;
	char *resto=(char *)malloc(strlen(codigo));
	memcpy(resto,codigo,strlen(codigo));
	char *linea;
	char comienzo_funcion[1024];
	strcpy(comienzo_funcion,"comienzo_funcion ");
	strcat(comienzo_funcion,nombre_funcion);
	while( resto != NULL ){
		linea=strtok(resto,"\n");
		resto=strtok(NULL,"\0");
		if( strcmp(linea,comienzo_funcion) == 0){
			posicion++;//Me muevo a donde esta la primera instruccion
			break;
		}
		posicion++;
	}
	//free(resto);

	return posicion;
}
int ejecutar_asignacion(char *palabra,proceso proceso){//ej: a+c;3
	char *log_text=(char *)malloc(256);


	int i,anterior;
	pcb pcb=proceso.pcb;
	char variable=palabra[0];


	bzero(log_text,256);
	sprintf(log_text,"El nombre de la variable es '%c', sacada de '%s'",variable,palabra);
	logx(pcb.pid,"PROCER",id_hilo_procer,"DEBUG",log_text);

	int valor_total=0;
	int valor_aux=0;
	int hice_un_io=0;
	char *numero;
	char se_espero='n';// n-> implica que no paso por ';' | s-> implica que si paso

	//Comprobamos que sea una asignacion
	if( palabra[1] != '='){
		bzero(log_text,256);
		sprintf(log_text,"El segundo caracter de la instruccion %s no es '='\n",palabra);
		logx(pcb.pid,"PROCER",id_hilo_procer,"ERROR",log_text);
		//if(log_text != NULL){free(log_text);}
		return -1;
	}

	//i=2 para saltear a la variable y al '='
	for(i=2;i<strlen(palabra);i++){//Compruebo que no se halla ejecutado antes una io()s
		//printf("Valor_total es %d\n",valor_total);
		//a=io(1,1)
		if( palabra[i] == 'i' && palabra[i+1] == 'o'){
			//printf("La palabra antes del ++ es %s\n",palabra);
			palabra++;
			palabra++;
			//printf("La palabra despues del ++ es %s\n",palabra);

			if ( ejecutar_io(palabra,proceso) == 0 ){
				valor_total=0;
				hice_un_io=1;
			}else{
				valor_total=1;
			}
			break;
		}

		if(es_un_caracter(palabra[i]) == 0 ){
			//printf("Entre en es_un_caracter con %c\n",palabra[i]);
			valor_aux=buscar_valor_de_variable(palabra[i],pcb);
			if( palabra[i-1] == '-' ){
				valor_total-=valor_aux;
			}else{
				valor_total+=valor_aux;
			}
		}

		if(es_un_numero(palabra[i]) == 0){
			//printf("Entre en es_un_numero\n");
			numero=extraer_numero(palabra,i);
			bzero(log_text,256);
			sprintf(log_text,"El numero extraido es %s ,de la linea '%s'\n",numero,palabra);
			logx(pcb.pid,"PROCER",id_hilo_procer,"DEBUG",log_text);
			if( numero != NULL){
				valor_aux=atoi(numero);
			}else{
				bzero(log_text,256);
				sprintf(log_text,"El numero extraido de %s es NULO\n",palabra);
				logx(pcb.pid,"PROCER",id_hilo_procer,"ERROR",log_text);
			}
			i+=(strlen(numero)-1);//avanzo la cantidad de caracteres del numero
			anterior=strlen(numero)<1?1:strlen(numero);
			if( palabra[i-anterior] == '-' ){
				valor_total-=valor_aux;
			}else{
				valor_total+=valor_aux;
			}
		}
		if( '+' == palabra[i] || '-' == palabra[i]){
			//No hacer nada
		}

		if( ';' == palabra[i]){
			se_espero='s';
			numero=extraer_numero(palabra,i+1);

			bzero(log_text,256);
			sprintf(log_text,"El numero extraido para ';' es %s ,para %s\n",numero,palabra);
			logx(pcb.pid,"PROCER",id_hilo_procer,"DEBUG",log_text);

			i+=(strlen(numero));//avanzo la cantidad de caracteres del numero
			sleep(atoi(numero));
			break;
		}
	}

	//printf("En ejecutar aignacion -> El valor a asignar a %c es %d\n",variable,valor_total);
	asignar_valor(variable,valor_total,pcb);
	if( se_espero == 'n'){//Solo se espera si no se espero en ';'

		pthread_mutex_lock(&mutexVarEsperaEstandar);
		sleep(atoi(espera_estandar));
		pthread_mutex_unlock(&mutexVarEsperaEstandar);
	}

	//if(log_text != NULL){free(log_text);}
	if (hice_un_io == 0){
		//printf("no Hice un IO\n");
		return 0;
	}else{
		//printf("Hice un IO\n");
		return 1;
	}

}
int asignar_valor(char variable,int valor,pcb pcb){
	char log_text[256];

	//printf("En asignar_valor -> El valor a asignar a %c es %d\n",variable,valor);
	data *datos=pcb.datos;
	int i;
	for(i=0;datos[i].variable;i++){
		if(datos[i].variable == variable){
			datos[i].valor=valor;
			bzero(log_text,256);
			sprintf(log_text,"Se asigno el valor %d a la variable '%c'\n",valor,datos[i].variable);
			logx(pcb.pid,"PROCER",id_hilo_procer,"DEBUG",log_text);
			//if( log_text != NULL){free(log_text);}
			return 0;
		}
	}
	bzero(log_text,256);
	sprintf(log_text,"No se encontro ninguna variable con el nombre '%c' para asignarle el valor %d\n",variable,valor);
	logx(pcb.pid,"PROCER",id_hilo_procer,"ERROR",log_text);
	//if( log_text != NULL){free(log_text);}
	return -1;
}

int buscar_valor_de_variable(char letra,pcb pcb){
	char *log_text=(char *)malloc(256);

	data *datos=pcb.datos;
	int i;
	for(i=0;datos[i].variable;i++){
		if(datos[i].variable == letra){
			bzero(log_text,256);
			sprintf(log_text,"El valor corrspondiente a la variable %c es %d\n",letra,datos[i].valor);
			logx(pcb.pid,"PROCER",id_hilo_procer,"DEBUG",log_text);
			//if( log_text != NULL){free(log_text);}
			return datos[i].valor;
		}
	}

	bzero(log_text,256);
	sprintf(log_text,"No se encontro ninguna variable con el nombre '%c' para extraer el valor\n",letra);
	logx(pcb.pid,"PROCER",id_hilo_procer,"ERROR",log_text);
	//if( log_text != NULL){free(log_text);}
	return -1;
}
char * extraer_numero(char *palabra,int posicion){//12+b o 12;
	int i=posicion;
	int j=0;
	char *numero=(char *)malloc(strlen(palabra));
	bzero(numero,sizeof(numero));

	if( es_un_numero(palabra[posicion]) == 0){
		while( es_un_delimitador(palabra[i]) != 0){
			numero[j]=palabra[i];
			i++;
			j++;
		}
	}else{
		return NULL;
	}
	return numero;
}
int es_un_caracter(char letra){
	int i,j;
	char letras[26];

	for (i = 0,j='a'; i < 26; i++,j++){letras[i] = j;}
	for(i=0;i<26;i++){
		if(letras[i] == letra){
			return 0;
		}
	}
	return -1;
}

int es_un_numero(char c){
	int i,j;
	char numeros[10];
	for (i = 0,j='0'; i < 10; i++,j++)
	{
		numeros[i]=j;
	}
	for(i=0;i<10;i++){
		if( c == numeros[i]){
			return 0;
		}
	}
	return -1;
}

int es_un_delimitador(char caracter){
	if(caracter == '+'){
		return 0;
	}
	if(caracter == '-'){
		return 0;
	}
	if(caracter == ';'){
		return 0;
	}
	if(caracter == '\0'){
		return 0;
	}
	if(caracter == '\n'){
		return 0;
	}
	return -1;
}

int ejecutar_salto(char *tipo_de_salto,char *resto,pcb pcb,seccion *seccion_ejecutandose){
	char *log_text=(char *)malloc(256);

	//pthread_mutex_lock(&mutexVarCantInstruccionesEjecutadas);
	//cant_instrucciones_ejecutadas++;
    //pthread_mutex_unlock(&mutexVarCantInstruccionesEjecutadas);

	char variable,*etiqueta;
	int valor_de_variable,posicion_etiqueta;

	//Separo los elementos
	variable=resto[0];
	resto++;//Avanzo 2 espacios para saltear la variable y el espacio
	resto++;
	etiqueta=resto;
	bzero(log_text,256);
	sprintf(log_text,"Los valores extraidos del salto son: %s ,%c y %s \n",tipo_de_salto,variable,etiqueta);
	logx(pcb.pid,"PROCER",id_hilo_procer,"DEBUG",log_text);
	valor_de_variable=buscar_valor_de_variable(variable,pcb);

	if(strcmp(tipo_de_salto,"ssc")==0){
		if( valor_de_variable == 0){
			posicion_etiqueta=buscar_posicion_etiqueta(etiqueta,pcb.codigo);
			if( posicion_etiqueta != -1){
				*seccion_ejecutandose->contador_instruccion=posicion_etiqueta;
			}else{
				bzero(log_text,256);
				sprintf(log_text,"Error al calcular la posicion de la etiqueta %s, el resultado fue %d\n",etiqueta,posicion_etiqueta);
				logx(pcb.pid,"PROCER",id_hilo_procer,"ERROR",log_text);
				//if( log_text != NULL){free(log_text);}
				return -1;
			}

		}
	}else{
		if( valor_de_variable != 0){
			posicion_etiqueta=buscar_posicion_etiqueta(etiqueta,pcb.codigo);
			if( posicion_etiqueta != -1){
				*seccion_ejecutandose->contador_instruccion=posicion_etiqueta;
				bzero(log_text,256);
				sprintf(log_text,"El valor es distinto de 0, se deberia saltar a la linea %d, la proxima instruccion es %d\n",posicion_etiqueta,*seccion_ejecutandose->contador_instruccion);
				logx(pcb.pid,"PROCER",id_hilo_procer,"DEBUG",log_text);
			}else{
				bzero(log_text,256);
				sprintf(log_text,"Error al calcular la posicion de la etiqueta %s, el resultado fue %d\n",etiqueta,posicion_etiqueta);
				logx(pcb.pid,"PROCER",id_hilo_procer,"ERROR",log_text);
				//if( log_text != NULL){free(log_text);}
				return -1;
			}
		}
	}

	pthread_mutex_lock(&mutexVarEsperaEstandar);
	sleep(atoi(espera_estandar));
	pthread_mutex_unlock(&mutexVarEsperaEstandar);

	//if( log_text != NULL){free(log_text);}
	return 0;
}

int buscar_posicion_etiqueta(char *etiqueta,char *codigo){
	int posicion=0;
	char *linea;
	char *resto=(char *)malloc(strlen(codigo));
	memcpy(resto,codigo,strlen(codigo));

	while(resto!=NULL){
		linea=strtok(resto,"\n");
		resto=strtok(NULL,"\0");
		if(strstr(linea,etiqueta) != NULL){
			break;
		}
		posicion++;
	}

	//if( resto != NULL){free(resto);}
	return posicion < cant_lineas(codigo)? posicion:-1;
}

int ejecutar_imprimir(char *resto,proceso proceso){
	char *log_text=(char *)malloc(256);

	instruccion_io instruccion;
	char *variable;
	int espera=-1;
	int i;
	int valor=buscar_valor_de_variable(resto[0],proceso.pcb);
	char *numero=(char *)malloc(strlen("00000"));
	char *msj=(char *)malloc(strlen("IMPRIMIENDO VARIABLE a: 00000"));
	bzero(numero,strlen("00000"));
	bzero(msj,strlen("IMPRIMIENDO VARIABLE a: 00000"));

	if( resto[1] == ';'){
		variable=strtok(resto,";");
		resto=strtok(NULL,"\0");

		//resto -> ;123
		espera=atoi(resto);
	}else{
		variable=resto;
	}

	//printf("El tiempo ha esperar es %d\n",espera);
	strcpy(msj,"IMPRIMIENDO VARIABLE ");
	strcat(msj,variable);
	strcat(msj,": ");
	sprintf(numero,"%d",valor);

	for(i=0;i<strlen(numero);i++){
		msj[strlen(msj)]=numero[i];
	}

	bzero(log_text,256);
	sprintf(log_text,"La instruccion es un imprimir %s, el resultado sera %s",resto,msj);
	logx(proceso.pcb.pid,"PROCER",id_hilo_procer,"DEBUG",log_text);

	instruccion.proceso=proceso;
	instruccion.instruccion="imprimir";
	instruccion.mensaje=msj;
	instruccion.espera=espera;

	pthread_mutex_lock(&mutexListaBloqueados);
	agregar_entrada_salida(listaBloqueados,instruccion);
	pthread_mutex_unlock(&mutexListaBloqueados);
	sem_post(sem_io);
	logx(proceso.pcb.pid,"PROCER",id_hilo_procer,"LSCH","Se agrego el proceso a la lista de Bloqueados.");

	logx(proceso.pcb.pid,"PROCER",id_hilo_procer,"DEBUG","El proceso se fue a E/S.");

	//if( log_text != NULL){free(log_text);}

	return 0;
}

int ejecutar_io(char *palabra,proceso proceso){

	//printf("La intruccion es io: %s.\n",palabra);
	char *log_text=(char *)malloc(256);

	instruccion_io instruccion;

	char *numero=strtok(palabra,"(");
	numero=strtok(NULL,",");
	char *tipo=strtok(NULL,")");
	palabra=strtok(NULL,"\0");

	//io(1,1);
	if( palabra != NULL && *palabra == ';'){
		numero=strtok(palabra,";");
	}

	instruccion.proceso=proceso;
	instruccion.instruccion="io";
	instruccion.mensaje=numero;//El mensaje tiene el tiempo de espera
	instruccion.espera=atoi(numero);

	if( atoi(tipo) == BLOQUEANTE){
		proceso.pcb.pc++;

		bzero(log_text,256);
		sprintf(log_text,"Se recibio una instruccion de io bloqueante de %s segundos.",numero);
		logx(proceso.pcb.pid,"PROCER",id_hilo_procer,"DEBUG",log_text);

		pthread_mutex_lock(&mutexListaBloqueados);
		agregar_entrada_salida(listaBloqueados,instruccion);
		pthread_mutex_unlock(&mutexListaBloqueados);
		sem_post(sem_io);
		logx(proceso.pcb.pid,"PROCER",id_hilo_procer,"LSCH","Se agrego el proceso a la lista de Bloqueados.");

		logx(proceso.pcb.pid,"PROCER",id_hilo_procer,"DEBUG","El proceso se fue a E/S.");
		return 0;
	}else{
		bzero(log_text,256);
		sprintf(log_text,"Se recibio una instruccion de io no bloqueante de %s segundos.",numero);
		logx(proceso.pcb.pid,"PROCER",id_hilo_procer,"DEBUG",log_text);

		pthread_mutex_lock(&mutexVarCantIOTDisponibles);
		if( cant_iot_disponibles > 0){
			pthread_mutex_unlock(&mutexVarCantIOTDisponibles);
			proceso.pcb.pc++;
			logx(proceso.pcb.pid,"PROCER",id_hilo_procer,"DEBUG","La instruccion de io no bloqueante encontro un hilo de IOT disponible.");

			//agregar_primero_entrada_salida(listaBloqueados,instruccion);
			pthread_mutex_lock(&mutexListaBloqueados);
			agregar_entrada_salida(listaBloqueados,instruccion);
			pthread_mutex_unlock(&mutexListaBloqueados);
			sem_post(sem_io);

			logx(proceso.pcb.pid,"PROCER",id_hilo_procer,"LSCH","Se agrego el proceso a la lista de Bloqueados.");
			logx(proceso.pcb.pid,"PROCER",id_hilo_procer,"DEBUG","El proceso se fue a E/S.");
		}else{
			pthread_mutex_unlock(&mutexVarCantIOTDisponibles);
			enviar_mensaje("Error todos los hilos de IOT estan ocupados.",proceso.cliente_sock);
			logx(proceso.pcb.pid,"PROCER",id_hilo_procer,"ERROR","Error todos los hilos de IOT estan ocupados.");
			//if( log_text != NULL){free(log_text);}
			//printf("Esta ocupado el hilo de IOT\n");
			return -1;
		}


	}

//	if( log_text != NULL){free(log_text);}
	return 0;
}
