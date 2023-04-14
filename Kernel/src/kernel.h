#ifndef KERNEL_H_
#define KERNEL_H_

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

char* ip_cpu;
char* puerto_cpu;
char* ip_memoria;
char* puerto_memoria;
char* ip_filesystem;
char* puerto_filesystem;
sem_t semKernelServer;
sem_t semKernelClientCPU;
sem_t semKernelClientMemoria;
sem_t semKernelClientFileSystem;

void *serverKernel(void *ptr);
void* clientCPU(void *ptr);
void* clientMemoria(void *ptr);
void* clientFileSystem(void *ptr);
void iniciarHilosClienteCPU();
void iniciarHilosClienteMemoria();
void iniciarHilosClienteFileSystem();
void iniciarHiloServer();
pthread_t serverKernel_thread, client_CPU, client_FileSystem, client_Memoria;


void iterator(char* value);

t_log* iniciar_logger(void);
t_config* iniciar_config(void);
void leer_consola(t_log*);
void paquete(int);
void terminar_programa(int, t_log*, t_config*);
#endif /* KERNEL_H_ */

void liberarConexiones(int conexion, t_log* logger, t_config* config)
{
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion);

}
