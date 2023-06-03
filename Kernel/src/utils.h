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
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;


//Listas PCB
typedef struct nodoArchivos {
    char* info_archivos;//ver tipo de direccion
    struct nodoArchivos* sgte;
} t_nodoArchivos;

typedef struct infoTablaSegmentos {
    int id;
    char* direccionBase; //VER TIPO
    int tamanio;
} t_infoTablaSegmentos;

typedef struct nodoTablaSegmentos {
	t_infoTablaSegmentos info_tablaSegmentos;
    struct nodoTablaSegmentos* sgte;
} t_nodoTablaSegmentos;


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

//REVEER: LOS TIPOS DE DATOS DE LAS LISTAS (TENEMOS PUNTEROS A LISTAS, PODRÍAMOS TENER DIRECTAMENTE LAS LISTAS)
typedef struct infopcb {
    int pid;
    t_list* listaInstrucciones;
    int programCounter; // numero de la siguiente instrucción a ejecutar --> uint32_t
    t_registrosCPU registrosCpu;// el CPU debe tener un 'mapa' y conocer que posición corresponde a cada registro
    t_nodoTablaSegmentos* tablaSegmentos;// direccion base = char*?
	t_nodoArchivos* punterosArchivos;
	double rafaga;
	double estimadoProxRafaga;
	double estimadoAnterior;
	uint32_t empiezaAEjecutar;
	uint32_t terminaEjecutar;
	uint32_t entraEnColaReady;
	int tiempoBloqueado;
	char* recursoALiberar;
	int recursoALiberar_length;
	char* recursoSolicitado;
	int recursoSolicitado_length;
	char* ultimaInstruccion;
	int ultimaInstruccion_length;
} t_infopcb;


//typedef struct {
//	char* instruccion;
//	int instruccion_length;
//	char* recursoSolicitado;
//	int recursoSolicitado_length;
//	char* recursoALiberar;
//	int recursoALiberar_length;
//	int tiempoBloqueado;
//	int programCounter;
//	t_registrosCPU registrosCpu;
//	//t_nodoTablaSegmentos* tablaSegmentos;
//} t_contextoEjecucion;


//EXIT, el kerner debe reconocer qué proceso se esta ejecutando en CPU para cdo reciba un mensaje de EXIT, saber que proceso finalizar

///COLA
typedef struct nodoCola {
    t_infopcb* info_pcb;
    struct nodoCola* sgte;
} t_nodoCola;

typedef struct
{
	int instancias;
	char* recurso;
	t_nodoCola* frenteBloqueados;
	t_nodoCola* finBloqueados;
} t_recursos;

//Funciones servidor
extern t_log* logger;

void* recibir_buffer(int*, int);

int iniciar_servidor(void);
int esperar_cliente(int);
t_list* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);

//Funciones cliente
int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
t_paquete* crear_paquete(void);
t_paquete* crear_paquete_instrucciones(void);
t_paquete* crear_super_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
uint32_t tomarTiempo();
// FUNCION PARA CALCULAR TIEMPO HRRN

#endif /* UTILS_H_ */
