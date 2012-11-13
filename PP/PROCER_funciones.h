/*
 * PROCER_funciones.h
 *
 *  Created on: 13/11/2012
 *      Author: utnso
 */

#ifndef PROCER_FUNCIONES_H_
#define PROCER_FUNCIONES_H_

//Funiones internas de PROCER.c
int verificar_fin_ejecucion(pcb pcb,unsigned int cont_quantum,unsigned int cant_instrucciones);
unsigned int cant_lineas(const char *codigo);
char * leer_instruccion(char *codigo,unsigned int pc);
int ejecutar_instruccion(char * instruccion,pcb *pcb);
int  es_un_token_nulo(char *palabra);
int es_una_variable(char* palabra);
int es_una_funcion(char* palabra);
int ejecutar_asignacion(char *palabra,pcb pcb);
char * extraer_numero(char *palabran,int posicion);
int asignar_valor(char variable,int valor,data *datos);
int buscar_valor_de_variable(char letra,data *datos);
int es_un_caracter(char c);
int es_un_numero(char c);
int es_un_delimitador(char caracter);

#endif /* PROCER_FUNCIONES_H_ */
