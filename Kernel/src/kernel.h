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
char* puerto_escucha;
char* algoritmo_planificacion;
char* estimacion_inicial;
char* hrrn_alfa;
char* grado_max_multiprogramación ;
//recursos ;
//instancias_recursos ;

//Inicializar punteros colas
t_nodoCola** frenteColaNew = NULL; // Puntero al frente de la cola
t_nodoCola** finColaNew = NULL; // Puntero al fin de la cola
t_nodoCola** frenteColaReady= NULL;
t_nodoCola** finColaReady = NULL;

int pid = 1; //Contador del PID de los PCB


//Semaforos e hilos
sem_t semKernelClientCPU, semKernelClientMemoria, semKernelClientFileSystem, semKernelServer,semReady;
pthread_t serverKernel_thread, client_CPU, client_FileSystem, client_Memoria,encolar_ready;

void *serverKernel(void *ptr);
void* clientCPU(void *ptr);
void* clientMemoria(void *ptr);
void* clientFileSystem(void *ptr);
void encolarReady();
void iniciarHiloClienteCPU();
void iniciarHiloClienteMemoria();
void iniciarHiloClienteFileSystem();
void iniciarHiloServer();


void armarPCB(t_list*);
void queue(t_nodoCola** , t_nodoCola** , t_infopcb);
t_infopcb unqueue(t_nodoCola** , t_nodoCola** );
void mostrarCola(t_nodoCola* );
int cantidadElementosCola(t_nodoCola*);

void iterator(char* value);
t_log* iniciar_logger(void);
t_config* iniciar_config(void);
void leer_consola(t_log*);
void paquete(int);
void terminar_programa(int, t_log*, t_config*);
char* recibir_handshake(int);

void liberarConexiones(int conexion, t_log* logger, t_config* config)
{
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion);

}

#endif /* KERNEL_H_ */
