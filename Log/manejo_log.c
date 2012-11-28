/*
 * manejo_log.c
 *
 *  Created on: 28/11/2012
 *      Author: utnso
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "../FuncionesPropias/manejo_archivos.h"
#include "manejo_log.h"


int logx(char *proceso,char *hilo, char *tipo, char *log){
	char *nombre_archivo="/home/utnso/pp.log";
	char *logeo=(char *)malloc(1024);
	char *fecha = cGetDate();
	char *hora=cGetTime();

	FILE * archivo;
	if ( (archivo=fopen (nombre_archivo, "a+")) == NULL){
		printf("Error al crear el archivo de log\n");
		return -1;
	}

	strcat(logeo,"[ ");
	strcat(logeo,fecha);
	strcat(logeo," ");
	strcat(logeo,hora);
	strcat(logeo," ] - [ ");
	strcat(logeo,proceso);
	strcat(logeo," ] [ ");
	strcat(logeo,hilo);
	strcat(logeo," ] [ ");
	strcat(logeo,tipo);
	strcat(logeo," ]: [ ");
	strcat(logeo,log);
	strcat(logeo," ]\n");

	fwrite(logeo,sizeof(char),strlen(logeo),archivo);

	fclose(archivo);

	return 0;
}

char *cGetDate() {
	time_t hora;
	struct tm *tiempo;
	char *fecha;

	hora = time(NULL);
	tiempo = localtime(&hora);

	fecha = (char *)malloc(sizeof(char)*SIZE_FECHA);
	if (fecha==NULL) {
		perror ("No hay memoria");
		return "";
	}

	sprintf (fecha,"%4d-%02d-%02d", tiempo->tm_year+1900,tiempo->tm_mon+1,tiempo->tm_mday);
	return fecha;
}

char *cGetTime() {
	time_t hora;
	struct tm *tiempo;
	hora = time(NULL);
	tiempo = localtime(&hora);
	char *tiempo_c;
	tiempo_c = (char *)malloc(sizeof(char)*SIZE_HORA);
	if (tiempo==NULL) {perror ("No hay memoria"); return "";} // verifico que pude obtener memoria 
	sprintf (tiempo_c,"%02d-%02d-%02d", tiempo->tm_hour, tiempo->tm_min, tiempo->tm_sec);
	return tiempo_c;
}

