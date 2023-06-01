#ifndef CPU_H_
#define CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include "utils.h"
#include <commons/string.h>
#include <commons/config.h>
#include <commons/string.h>
#include <readline/readline.h>
#include <pthread.h>
#include <semaphore.h>


char* ip_memoria;
char* puerto_memoria;
sem_t semCPUServer;
sem_t semCPUClientMemoria;
t_contextoEjecucion* contexto;


void *serverCPU(void *ptr);
void* clientMemoria(void *ptr);
void iniciarHiloCliente();
void iniciarHiloServer();
pthread_t serverCPU_thread, client_Memoria, client_Kernel;
void iniciarHiloClienteKernel();
void* clientKernel(void *ptr);

//Funciones instrucciones
void yield_tp();
void exit_tp();
void i_o_tp(int);
void wait_tp(char* );
void signal_tp(char* );
void set_tp(char* , char* );
void serializar();

int ejecutarFuncion(char*);
void iniciar_ejecucion();
void vaciarContexto();


void iterator(char*);

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
