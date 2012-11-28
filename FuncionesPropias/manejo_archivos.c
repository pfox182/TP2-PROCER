/*
 * manejo_archivos.c
 *
 *  Created on: 09/11/2012
 *      Author: utnso
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char * leer_archivo(char *nombre_archivo){
	FILE *archivo;
	char *buffer=(char *)malloc(1024);//TODO: malloc por el tamaño del archivo
	char caracter;
	int cant_caracteres = 0;

	archivo=fopen (nombre_archivo, "r");
	if( archivo == NULL){
		printf("Error de apertura del archivo\n");
	}


	while(feof(archivo) == 0){
		caracter = fgetc(archivo);
		buffer[cant_caracteres]=caracter;
		cant_caracteres++;
		//fscanf(archivo,"%s", *buffer) != EOF //Otra forma de leer
	}
	buffer[cant_caracteres]='\0';
	fclose(archivo);
	return buffer;
}

int crear_archivo(char *nombre){
	FILE * archivo;
	if ( (archivo=fopen (nombre, "w+")) == NULL){
		return -1;
	}
	return 0;
}

int existe_el_archivo(char *nombre){

	return 0;//Existe
}

