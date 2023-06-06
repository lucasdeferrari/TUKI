#ifndef FILE_SYSTEM_H_
#define FILE_SYSTEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include "utils.h"
#include <commons/string.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include <readline/readline.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>


char* ip_memoria;
char* puerto_memoria;
sem_t semFileSystemServer;
sem_t semFileSystemClientMemoria;



void *serverFileSystem(void *ptr);
void* clientMemoria(void *ptr);
void iniciarHiloCliente();
void iniciarHiloServer();
pthread_t serverFileSystem_thread, client_Memoria;


void iterator(char* value);

t_log* iniciar_logger(void);
t_config* iniciar_config(void);
void leer_consola(t_log*);
void paquete(int);
void terminar_programa(int, t_log*, t_config*);
#endif /* FILE_SYSTEM_H_ */

void liberarConexiones(int conexion, t_log* logger, t_config* config)
{
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion);

}
