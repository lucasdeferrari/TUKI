#ifndef MEMORIA_H_
#define MEMORIA_H_

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


typedef struct {
    int idSegmento;
    size_t base;
    size_t desplazamiento;
} Segmento;


sem_t semMemoriaServer;

void* serverMemoria(void *ptr);
void iniciarHiloServer();
void enviar_respuesta(int socket_cliente, char* quien_es);
pthread_t serverMemoria_thread;
int crearSegmento(int );
void informarKernelFaltaDeEspacio();
Segmento *crearSegmento0(size_t);
char* recibir_buffer_mio(int socket_cliente);

void iterator(char *value);

t_log* iniciar_logger(void);
t_config* iniciar_config(void);
void leer_consola(t_log*);
void paquete(int);
void terminar_programa(int, t_log*, t_config*);
#endif /* FILE_SYSTEM_H_ */

void liberarConexiones(int conexion, t_log *logger, t_config *config) {
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion);

}
