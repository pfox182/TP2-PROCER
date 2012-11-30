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
	}
	buffer[cant_caracteres]='\0';

	return buffer;
}

