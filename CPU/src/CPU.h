#ifndef CPU_H_
#define CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include "utils.h"
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <pthread.h>
#include <semaphore.h>


char* ip_memoria;
char* puerto_memoria;
sem_t semCPUServer;
sem_t semCPUClientMemoria;
t_contextoEjecucion* contextoPRUEBA;


void *serverCPU(void *ptr);
void* clientMemoria(void *ptr);
void iniciarHiloCliente();
void iniciarHiloServer();
pthread_t serverCPU_thread, client_Memoria;



void iterator(char* value);

t_log* iniciar_logger(void);
t_config* iniciar_config(void);
void leer_consola(t_log*);
void paquete(int);
void terminar_programa(int, t_log*, t_config*);
#endif /* CPU_H_ */

void liberarConexiones(int conexion, t_log* logger, t_config* config)
{
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion);

}
