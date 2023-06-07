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
double estimacion_inicial;
int hrrn_alfa;
int grado_max_multiprogramación ;
int server_fd;
int cantidadElementosReady;

//VERIFICAR TIPO
//char ** recursos ;
//char ** instancias_recursos ;

//Manejo de recursos;

//Inicializar punteros colas
t_nodoCola** frenteColaNew = NULL; // Puntero al frente de la cola
t_nodoCola** finColaNew = NULL; // Puntero al fin de la cola

//SI UTILIZAMOS FIFO
t_nodoCola** frenteColaReady= NULL;
t_nodoCola** finColaReady = NULL;

//SI UTILIZAMOS HRRN
t_list* listaReady;

t_infopcb* estadoEnEjecucion;
int pid = 1; //Contador del PID de los PCB




//Semaforos e hilos
sem_t semKernelClientCPU, semKernelClientMemoria, semKernelClientFileSystem, semKernelServer, semReady;
pthread_t serverKernel_thread, client_CPU, client_FileSystem, client_Memoria, encolar_ready, interrupcion_IO;

void *serverKernel(void *ptr);
void* clientCPU(void *ptr);
void* clientMemoria(void *ptr);
void* clientFileSystem(void *ptr);
void* interrupcionIO(void *ptr);
void encolarReady();
void iniciarHiloClienteCPU();
void iniciarHiloClienteMemoria();
void iniciarHiloClienteFileSystem();
void iniciarHiloServer();
void iniciarHiloIO();
void serializarContexto(int );
void calcularHRRN(t_infopcb*);
void manejar_recursos();
void encolar_ready_ejecucion(t_infopcb*);
void pasarAExit(t_infopcb*);

void armarPCB(t_list*);
void queue(t_nodoCola** , t_nodoCola** , t_infopcb*);
t_infopcb* unqueue(t_nodoCola** , t_nodoCola** );
void mostrarCola(t_nodoCola* );
int cantidadElementosCola(t_nodoCola*);
void agregarElementoListaReady(t_nodoCola**, t_infopcb* );
void mostrarListaReady(t_list*);
int cantidadElementosListaReady(t_nodoCola*);
t_list* listaRecursos;

void iterator(char* value);
t_log* iniciar_logger(void);
t_config* iniciar_config(void);
void leer_consola(t_log*);
void paquete(int);
void terminar_programa(int, t_log*, t_config*);
void recibir_contexto(int );
char* recibir_handshake(int );
t_paquete* empaquetar(t_list*);

void liberarConexiones(int conexion, t_log* logger, t_config* config)
{
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion);

}

#endif /* KERNEL_H_ */
