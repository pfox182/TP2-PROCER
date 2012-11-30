/*
 * PROCER_funciones.h
 *
 *  Created on: 13/11/2012
 *      Author: utnso
 */

#ifndef PROCER_FUNCIONES_H_
#define PROCER_FUNCIONES_H_

//aux
#include "../Estructuras/manejo_listas.h"
#include "../Estructuras/manejo_listas_funciones.h"

//CONSTANTES
#define BLOQUEANTE 1
#define NO_BLOQUEANTE 0

//Funiones internas de PROCER.c
int verificar_fin_ejecucion(proceso proceso,unsigned int cont_quantum,unsigned int cant_instrucciones);
unsigned int cant_lineas(const char *codigo);
char * leer_instruccion(char *codigo,unsigned int pc);
int ejecutar_instruccion(char * instruccion,proceso *proceso,seccion *seccion_ejecutandose);
int  es_un_token_nulo(char *palabra);
int es_una_variable(char* palabra);
int es_una_funcion(char* palabra);
int es_un_salto(char* palabra);
int es_un_imprimir(char* palabra);
int es_un_io(char* palabra);
int ejecutar_funcion(char *nombre_funcion,proceso *proceso);
unsigned int buscar_inicio_de_funcion(char *nombre_funcion,char *codigo);
int ejecutar_asignacion(char *palabra,pcb pcb);
int asignar_valor(char variable,int valor,pcb pcb);
char * extraer_numero(char *palabran,int posicion);
int buscar_valor_de_variable(char letra,pcb pcb);
int es_un_caracter(char c);
int es_un_numero(char c);
int es_un_delimitador(char caracter);
int ejecutar_salto(char *tipo_de_salto,char *resto,pcb pcb,seccion *seccion_ejecutandose);
int buscar_posicion_etiqueta(char *etiqueta,char *codigo);
int ejecutar_imprimir(char *resto,proceso proceso);
int ejecutar_io(char *palabra,proceso proceso);

#endif /* PROCER_FUNCIONES_H_ */
