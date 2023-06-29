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
#include<sys/time.h>

//KERNEL
#define IP "127.0.0.1"
#define PUERTO "8000"

#define _POSIX_C_SOURCE 200809L
#define _BSD_SOURCE
#define _DEFAULT_SOURCE

typedef enum
{
	MENSAJE,
	PAQUETE,
	CONTEXTO,
	INSTRUCCIONES
}op_code;


//client
typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	int cod_memoria;
	int pidProceso;
} ClientMemoriaArgs;

typedef struct
{
	int cod_fs;
    char* nombreArchivo;
    int posicionPuntero;
	int cantBytes;
	size_t direcFisica;
	size_t nuevoTamanioArchivo;
} ClientFSArgs;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

typedef struct infoTablaSegmentos {
    int id;
    size_t direccionBase;
    size_t tamanio;
} t_infoTablaSegmentos;

typedef struct infoTablaGlobalArchivos {
    char* nombreArchivo;
    t_queue* colaProcesosBloqueados;
} t_infoTablaGlobalArchivos;

typedef struct infoTablaArchivos {
    char* nombreArchivo;
    int posicionPuntero;
} t_infoTablaArchivos;

//DEBEMOS ASUMIR QUE LOS REGISTROS SON DE 4,8,16 BYTES, O TENEMOS QUE LIMITAR CON char[4],char[8],char[16] ??
typedef struct registrosCPU {
	char AX[5];
	char BX[5];
	char CX[5];
	char DX[5];
	char EAX[9];
	char EBX[9];
	char ECX[9];
	char EDX[9];
	char RAX[17];
	char RBX[17];
	char RCX[17];
	char RDX[17];
} t_registrosCPU;


typedef struct infopcb {
    int pid;
    t_list* listaInstrucciones;
    int programCounter;
    t_registrosCPU registrosCpu;
    t_list* recursosAsignados;
    t_list* tablaSegmentos;
	t_list* tablaArchivosAbiertos;

	//HRRN
	double rafaga;
	double estimadoProxRafaga;
	double estimadoAnterior;
	uint32_t empiezaAEjecutar;
	uint32_t terminaEjecutar;
	uint32_t entraEnColaReady;

	//recibimos de CPU + programCounter + registrosCPU
	char* ultimaInstruccion;
	int ultimaInstruccion_length;
	int tiempoBloqueado;
	int idSegmento;
	int tamanioSegmento; //size_t
	//recursos
	char* recursoALiberar;
	int recursoALiberar_length;
	char* recursoSolicitado;
	int recursoSolicitado_length;
	//archivos
	char* nombreArchivo;
	int nombreArchivo_length;
	int posicionArchivo;
	int cantBytesArchivo;
	int direcFisicaArchivo;
	int tamanioArchivo;
} t_infopcb;


///COLA
typedef struct nodoCola {
    t_infopcb* info_pcb;
    struct nodoCola* sgte;
} t_nodoCola;

typedef struct
{
	int instancias;
	char* recurso;
	t_queue* colaBloqueados;
} t_recursos;

//Funciones servidor
extern t_log* logger;

void* recibir_buffer(int*, int);

int iniciar_servidor(void);
int esperar_cliente(int);
t_list* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);
void enviar_mensaje_cod_operacion(char* , int , int );

//Funciones cliente
int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
t_paquete* crear_paquete(void);
t_paquete* crear_paquete_instrucciones(void);
t_paquete* crear_paquete_cod_operacion(int);
t_paquete* crear_super_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
uint32_t tomarTiempo();
// FUNCION PARA CALCULAR TIEMPO HRRN

#endif /* UTILS_H_ */
