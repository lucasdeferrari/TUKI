#ifndef UTILS_H_
#define UTILS_H_

#include<signal.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<commons/string.h>
#include<string.h>
#include<assert.h>

//MEMORIA
#define IP "127.0.0.1"
#define PUERTO "8002"

typedef enum
{
	MENSAJE,
	PAQUETE,
	CREATE_SEGMENT,
	DELETE_SEGMENT,
	COMPACTAR_MEMORIA,
	PROCESO_NUEVO,
	TABLA_SEGMENTOS,
	SIN_ESPACIO
}op_code_Kernel;

//client
typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code_Kernel codigo_operacion;
	t_buffer* buffer;
} t_paquete;

typedef struct
{
	int pid;
	t_list* segmentos;
}TablaDeSegmentos;

typedef struct {
    int idSegmentoMemoria;
    int idSegmentoKernel;
    size_t base;
    size_t desplazamiento;
} Segmento;

typedef struct {
    size_t base;
    size_t desplazamiento;
} HuecoLibre;

typedef struct {
    int cod_kernel;
    int cliente_fd;
} ClientKernelArgs;
extern t_log* logger;

void* recibir_buffer(int*, int);

int iniciar_servidor(void);
int esperar_cliente(int);
t_list* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);
char* recibir_handshake(int socket_cliente);
bool hayTablaSegmentosDe(int idProceso);

//client
int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
t_paquete* crear_paquete(void);
t_paquete* crear_super_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
t_paquete* crear_paquete_cod_operacion(int cod_operacion);
t_paquete* empaquetarTabla(int pid, t_list* cabeza, int cod_operacion);
void enviar_cod_operacion(char* mensaje, int socket_cliente, int cod_operacion);
#endif /* UTILS_H_ */
