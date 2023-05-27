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
#include<string.h>
#include<assert.h>

//CPU
#define IP "127.0.0.1"
#define PUERTO "8001"

typedef enum
{
	MENSAJE,
	PAQUETE,
	CONTEXTO
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

//  CONTEXTO
typedef struct infoTablaSegmentos {
    int id;
    char* direccionBase; //VER TIPO
    int tamanio;
} t_infoTablaSegmentos;

typedef struct nodoTablaSegmentos {
	t_infoTablaSegmentos info_tablaSegmentos;
    struct nodoTablaSegmentos* sgte;
} t_nodoTablaSegmentos;

//UTILIZAMOS UN STRUCT PARA LOS REGISTROS EN VEZ DE UN VECTOR
//DEBEMOS ASUMIR QUE LOS REGISTROS SON DE 4,8,16 BYTES, O TENEMOS QUE LIMITAR CON char[4],char[8],char[16] ??
typedef struct registrosCPU {
	char* AX;
	char* BX;
	char* CX;
	char* DX;
	char* EAX;
	char* EBX;
	char* ECX;
	char* EDX;
	char* RAX;
	char* RBX;
	char* RCX;
	char* RDX;
} t_registrosCPU;

typedef struct {
	int instruccion_length;
	char* instruccion;
	char* recursoSolicitado;
	int tiempoBloqueado;
	t_list* listaInstrucciones;
	int programCounter; // numero de la siguiente instrucción a ejecutar
	t_registrosCPU registrosCpu;
	t_nodoTablaSegmentos* tablaSegmentos;// direccion base = char*?
} t_contextoEjecucion;


extern t_log* logger;

void* recibir_buffer(int*, int);

int iniciar_servidor(void);
int esperar_cliente(int);
t_list* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);
t_contextoEjecucion* recibir_contexto(int );

//client
int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
t_paquete* crear_paquete(void);
t_paquete* crear_super_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);

#endif /* UTILS_H_ */
