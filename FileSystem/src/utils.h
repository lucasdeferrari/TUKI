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
#include <commons/collections/queue.h>
#include<string.h>
#include<assert.h>
#include <math.h>

//FS
//#define IP "127.0.0.1"
//#define PUERTO "8003"

typedef enum
{
	MENSAJE,
	PAQUETE,
	F_OPEN,
	F_READ,
	F_WRITE,
	F_TRUNCATE,
	CREAR_ARCHIVO
}op_code;

//client
typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

typedef struct Bloque
{
	size_t tamanio;
	char* bytes;
} bloque;


// EL puntero indirecto es un bloque en realidad... pero todavía no entiendo que tiene un bloque... no se como tipearlo
typedef struct{
	int cod_memoria;
	char* registro;
	int direccionFisica;
	int tamanio;
} ClientMemoriaArgs;

typedef struct{
	int clientefd;
	char* instruccion;
	char* nombreArchivo;
	int direccionFisica;
	int cantBytes;
	int punteroArchivo;
} t_instruccion;

extern t_log* logger;

void* recibir_buffer(int*, int);

int iniciar_servidor(void);
int esperar_cliente(int);
t_list* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);
t_paquete* crear_paquete_cod_operacion(int);
char* recibir_handshake(int);


//client
int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
void enviar_mensaje_cod_operacion(char* mensaje, int socket_cliente, int cod_operacion);
t_paquete* crear_paquete(void);
t_paquete* crear_super_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);


#endif /* UTILS_H_ */
