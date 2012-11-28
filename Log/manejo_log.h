/*
 * manejo_log.h
 *
 *  Created on: 28/11/2012
 *      Author: utnso
 */

#ifndef MANEJO_LOG_H_
#define MANEJO_LOG_H_

#define SIZE_FECHA 11
#define SIZE_HORA 9

int logx(char *proceso,char *hilo, char *tipo, char *log);
char *cGetDate();
char *cGetTime();

#endif /* MANEJO_LOG_H_ */
