/*
 * PROCER_funciones.c
 *
 *  Created on: 13/11/2012
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //Contiene la funcion sleep
#include "../FuncionesPropias/manejo_archivos.h"
#include "../Estructuras/proceso.h"

#include "PROCER_funciones.h"

extern char *lpl;
extern unsigned int quantum_max;
extern char *espera_estandar;

int verificar_fin_ejecucion(pcb pcb,unsigned int cont_quantum,unsigned int cant_instrucciones){
	int fin=0;
	if( strcmp(lpl,"RR") == 0){
		if( cont_quantum >= quantum_max ){
			printf("Se sobrepaso el quantum\n");
			fin = -1;
		}
	}
	if( pcb.pc > cant_instrucciones){
		printf("Se sobrepaso el pc-d>%d cant_inst->%d\n",pcb.pc,cant_instrucciones);
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
	free(resto);
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

int ejecutar_instruccion(char * instruccion,pcb *pcb){
	char *palabra;
	char *resto=instruccion;

	while( resto != NULL){
		palabra = strtok(resto," ");
		resto = strtok(NULL,"\0");

		if( es_un_token_nulo(palabra) == 0){
			//No hacer nada
			break;
		}

		if( es_una_variable(palabra) == 0){//De la forma a=1 o a=b+c
			ejecutar_asignacion(palabra,*pcb);
		}

		if( es_una_funcion(palabra) == 0){//De la forma f10()
			//ejecutar_funcion(palabra,pcb);
		}

	}

	return 0;
}

int  es_un_token_nulo(char *palabra){
	if( strcmp("variables",palabra)==0 || strcmp("comienzo_programa",palabra)==0 || strcmp("",palabra)==0){
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

	if( palabra[0] == 'f' && (es_un_numero(palabra[1]) == 0)){
		return 0;
	}
	return -1;
}
int ejecutar_funcion(char *nombre_funcion,pcb pcb){
	unsigned int posicion = buscar_inicio_de_funcion(nombre_funcion,pcb.codigo);
	char *instruccion;
	char *fin_funcion="fin_funcion ";
	strcat(fin_funcion,nombre_funcion);

	//TODO:Se debe contar como 1 quantum por toda la funcion o 1 quantum x cada instruccion??
	//TODO:Que asa si se suspende el programa aca
	instruccion = leer_instruccion(pcb.codigo,posicion);
	while( strcmp(instruccion,fin_funcion) != 0 ){
		if( instruccion != NULL ){
			if( ejecutar_instruccion(instruccion,&pcb) == -1){
				printf("Error al ejecutar la instruccion:%s ,de la funcion %s\n",instruccion,nombre_funcion);
				return -1;
			}
		}else{
			printf("La instruccion leida de la funcion %s en la linea %d es nula\n",nombre_funcion,posicion);
		}
		posicion++;
	}

	return 0;
}
unsigned int buscar_inicio_de_funcion(char *nombre_funcion,char *codigo){
	unsigned int posicion=0;
	char *resto=(char *)malloc(sizeof(codigo));
	char *linea;

	while( resto != NULL ){
		linea=strtok(resto,"\n");
		resto=strtok(NULL,"\0");

		if( strstr(linea,nombre_funcion) != NULL ){
			posicion++;//Me muevo a donde esta la primera instruccion
			break;
		}
		posicion++;
	}


	free(resto);
	return posicion;
}
int ejecutar_asignacion(char *palabra,pcb pcb){//ej: a+c;3
	int i;
	char variable=palabra[0];
	printf("El nombre de la variable es '%c', sacada de '%s'\n",variable,palabra);
	int valor_total=0;
	int valor_aux=0;
	char *numero;
	char se_espero='n';// n-> implica que no paso por ';' | s-> implica que si paso

	//Comprobamos que sea una asignacion
	if( palabra[1] != '='){
		printf("El segundo caracter de la palabra no es '='\n");
		return -1;
	}

	//i=2 para saltear a la variable y al '='
	for(i=2;i<strlen(palabra);i++){
		if(es_un_caracter(palabra[i]) == 0){
			valor_aux=buscar_valor_de_variable(palabra[i],pcb.datos);
			if( palabra[i-1] == '-' ){
				valor_total-=valor_aux;
			}else{
				valor_total+=valor_aux;
			}
		}

		if(es_un_numero(palabra[i]) == 0){
			numero=extraer_numero(palabra,i);
			printf("El numero extraido es %s\n",numero);
			if( numero != NULL){
				valor_aux=atoi(numero);
			}else{
				printf("El numero extraido de %s es NULO\n",palabra);
			}
			i+=(strlen(numero)-1);//avanzo la cantidad de caracteres del numero
			if( palabra[i-1] == '-' ){
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
			printf("El numero extraido para ';' es %s\n",numero);
			i+=(strlen(numero));//avanzo la cantidad de caracteres del numero
			sleep(atoi(numero));
		}
	}

	asignar_valor(variable,valor_total,pcb.datos);
	if( se_espero == 'n'){
		sleep(atoi(espera_estandar));
	}
	return 0;

}
int asignar_valor(char variable,int valor,data *datos){
	int i;
	for(i=0;i<26;i++){
		if(datos[i].variable == variable){
			datos[i].valor=valor;
			return 0;
		}
	}
	printf("No se encontro ninguna variable con el nombre '%c' para asignarle el valor %d\n",variable,valor);
	return -1;
}

int buscar_valor_de_variable(char letra,data *datos){
	int i;
	for(i=0;i<26;i++){
		if(datos[i].variable == letra){
			printf("El valor corrspondiente a la variable %c es %d\n",letra,datos[i].valor);
			return datos[i].valor;
		}
	}

	printf("No se encontro ninguna variable con el nombre '%c' para extraer el valor\n",letra);
	return -1;
}
char * extraer_numero(char *palabra,int posicion){
	int i,j;
	char *numero=(char *)malloc(strlen(palabra));

	if( es_un_numero(palabra[posicion]) == 0){
		for(j=posicion,i=0;es_un_delimitador(palabra[j])!=0;i++,j++){
			numero[i]=palabra[j];
		}
	}else{
		printf("El primer caracter '%c'de %s, no es un numero\n",palabra[posicion],palabra);
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
	return -1;
}
