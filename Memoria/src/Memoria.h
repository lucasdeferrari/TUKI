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

pthread_t client_Kernel, client_CPU, client_FS;

t_list * listaDeHuecosLibres;
t_list* tablasDeSegmento;
t_list * segmentos;

size_t base;

char* algoritmoAsignacion;
sem_t semMemoriaServer;
pthread_t serverMemoria_thread;
int tamanioSeg0, tamanioMemoria, retardoMemoria, retardoCompactacion;

void* espacioUsuario;
int asignarIdSegmento();
Segmento *crearSegmento0(size_t);
HuecoLibre* crearHuecoLibre(size_t tamanio, size_t base);

void* serverMemoria(void *ptr);
void iniciarHiloServer();
void iniciarHiloClienteKernel(int codigo_kernel,int cliente_fd);
void* clientKernel(void *arg);

void enviar_respuesta(int socket_cliente, char* quien_es);
char* recibir_buffer_mio(int socket_cliente);

void crearYDevolverProceso();

int crearSegmento(int );
int hayLugarParaCrearSegmento(size_t tamanio);
int hayLugarContiguoPara(size_t tamanio);
int hayTablaSegmentosDe(int idProceso);
TablaDeSegmentos* crearTablaSegmentosDe(int idProceso);
void agregarSegmentoATabla(Segmento *segmento, int idProceso);
size_t buscarLugarParaElSegmento(size_t tamanio);

size_t buscarPorFirst (size_t tamanio);
size_t buscarPorBest(size_t tamanio);
size_t buscarPorWorst(size_t tamanio);
void actualizarHuecosLibres(HuecoLibre *siguiente, size_t tamanio, int iteradorIndex);
size_t buscarSiguienteLugarOcupado(size_t base);

void eliminar_segmento(int id_proceso, int id_segmento);
void juntarHuecosContiguos();

void compactar_memoria();
int buscarIdMemoria(int idSegmentoMemoria);
bool comparador(void* elem1, void* elem2);

void eliminar_proceso(int idProceso);

void enviarTodasLasTablas(int cliente_fd);

char* crearValorLectura(char* array[], int longitud);

void iterator(char *value);
t_log* iniciar_logger(void);
t_config* iniciar_config(void);
void leer_consola(t_log*);
void paquete(int);
void terminar_programa(int, t_log*, t_config*);
#endif /* FILE_SYSTEM_H_ */

void sleep_ms(unsigned int milliseconds) {
    usleep(milliseconds * 1000);
}

void liberarConexiones(int conexion, t_log *logger, t_config *config) {
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion);

}
