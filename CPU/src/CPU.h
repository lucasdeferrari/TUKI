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
#include <unistd.h>

char* ip_memoria;
char* puerto_memoria;
sem_t semCPUServer;
sem_t semCPUClientMemoria;
t_contextoEjecucion* contexto;
int server_fd;
int retardo_instruccion;
int tam_max_segmento;

void *serverCPU(void *ptr);
void* clientMemoria(void *arg);
void iniciarHiloClienteMemoria(int,char*,int);
//void* clientMemoria(t_infoClienteMemoria*);
//void iniciarHiloClienteMemoria(t_infoClienteMemoria*);
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
void serializarContexto(int);
t_list* tablaSegmentosActualizada(t_list*);
int tamanioRegistro(char* );
char* contenidoRegistro(char* );

int MMU(int ,int);
void mov_in_tp(char* , int );
void mov_out_tp(int, char*);
void createSeg_tp(int , int);
void deleteSeg_tp(int);
void fopen_tp(char*);
void fclose_tp(char*);
void fseek_tp(char*, int);
void fread_tp(char*, int, int);
void fwrite_tp(char*, int, int);
void ftruncate_tp(char*, int);

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


void sleep_ms(unsigned int milliseconds) {
    usleep(milliseconds * 1000);
}
