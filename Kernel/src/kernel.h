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
#include <unistd.h>

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
int cantidadElementosSistema;
int cantidadElementosBloqueados;

//Manejo de FS
t_list* tablaGlobalArchivosAbiertos;


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
int pidProcesoNuevo;



//Semaforos e hilos
sem_t  semKernelClientMemoria, semPasarAExit;
pthread_t serverKernel_thread, client_CPU, client_FileSystem, client_Memoria, encolar_ready, interrupcion_IO;
pthread_mutex_t mutex_fd;

void* serverKernel(int);
void* clientCPU(void *);
void* clientMemoria(void *);
void* clientFileSystem(void *);
void* interrupcionIO(void *);
void encolarReady();
void iniciarHiloClienteCPU();
void iniciarHiloClienteMemoria(int);
void iniciarHiloClienteFileSystem(int , t_infopcb* );
int iniciarHiloServer(int);
void iniciarHiloIO();
void serializarContexto(int );
void calcularHRRN(t_infopcb*);
void manejar_recursos();
void encolar_ready_ejecucion(t_infopcb*);
void pasarAExit();
void inicializarRecursos();
void liberarRecursosAsignados();
void enviar_handshake_memoria();
void procedimiento_compactar();
int puntero(t_list* , char* );
int elArchivoEstaAbierto(char* );

void armarPCB(t_list*);
void queue(t_nodoCola** , t_nodoCola** , t_infopcb*);
t_infopcb* unqueue(t_nodoCola** , t_nodoCola** );
void mostrarCola(t_nodoCola* );
int cantidadElementosCola(t_nodoCola*);
void agregarElementoListaReady(t_nodoCola**, t_infopcb* );
void mostrarListaReady(t_list*);
int cantidadElementosListaReady(t_nodoCola*);
t_list* listaRecursos;
t_list* tablaSegmentosActualizada(t_list*);
void crearTablaSegmentos(int , t_list* );
void finalizarEncolar();

void iterator(char*);
t_log* iniciar_logger(void);
t_config* iniciar_config(void);
void leer_consola(t_log*);
void paquete(int);
void terminar_programa(int, t_log*, t_config*);
void recibir_contexto(int );
char* recibir_handshake(int );
t_paquete* empaquetarInstrucciones(t_list*);
t_paquete* empaquetarTabla(t_list* );

void liberarConexiones(int conexion, t_log* logger, t_config* config)
{
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion);

}
void sleep_ms(unsigned int milliseconds) {
    usleep(milliseconds * 1000);
}
#endif /* KERNEL_H_ */
