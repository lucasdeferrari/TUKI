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
#include <math.h>


char* ip_memoria;
char* puerto_memoria;
sem_t semFileSystemServer;
sem_t semFileSystemClientMemoria;

t_list* listaFCB;
char* recibir_buffer_mio(int socket_cliente);
void abrir_archivo(char* nombreArchivo);
void crearArchivo(char* nombreArchivo);
void truncar_archivo(char* nombreArchivo, int tamanio);
void leerArchivo(char* nombreArchivo, int punteroArchivo, int cantBytesRead, int direcFisicaRead);
void escribirArchivo(char* nombreArchivo, int punteroArchivo, int cantBytesWrite, int direcFisicaWrite);
//t_infofcb* buscarFCB (char* nombreArchivo);
char* recibir_handshake(int);
int minimo(int, int);

void enviar_respuesta(int socket_cliente, char* quien_es);

void* serverFileSystem(void *ptr);
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
