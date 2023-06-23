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


pthread_t client_Kernel;

t_list * listaDeHuecosLibres;
t_list* tablasDeSegmento;
t_list * segmentos;


size_t base;

char* algoritmoAsignacion;
sem_t semMemoriaServer;

int tamanioSeg0, tamanioMemoria;

void* espacioUsuario;

void* serverMemoria(void *ptr);
void iniciarHiloServer();
void iniciarHiloClienteKernel(int cod_kernel,int cliente_fd);
void* clientKernel(int cod_kernel, int cliente_fd);

void enviar_respuesta(int socket_cliente, char* quien_es);
pthread_t serverMemoria_thread;
int crearSegmento(int );
void informarKernelFaltaDeEspacio();
Segmento *crearSegmento0(size_t);
char* recibir_buffer_mio(int socket_cliente);
size_t buscarLugarParaElSegmento(size_t tamanio);
void agregarSegmentoATabla(Segmento *segmento, int idProceso);
void eliminar_segmento(int id_proceso, int id_segmento);
bool hayLugarParaCrearSegmento(size_t tamanio);
bool hayLugarContiguoPara(size_t tamanio);
void agregarSegmentoATabla(Segmento *segmento, int idProceso);
int asignarIdSegmento();
size_t buscarPorFirst (size_t tamanio);
size_t buscarPorBest(size_t tamanio);
size_t buscarPorWorst(size_t tamanio);
void crearYDevolverProceso();
TablaDeSegmentos* crearTablaSegmentosDe(int idProceso);
size_t buscarSiguienteLugarOcupado(size_t base);
void iterator(char *value);
void juntarHuecosContiguos(t_list* listaDeHuecosLibres);

bool segmentoEsElUltimo(Segmento* segmento, t_list* segmentos);

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
