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

//KERNEL
#define IP "127.0.0.1"
#define PUERTO "8000"

#define _POSIX_C_SOURCE 200809L
#define _BSD_SOURCE
#define _DEFAULT_SOURCE

typedef enum
{
	MENSAJE,
	PAQUETE
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

//typedef struct nodoInstrucciones {
//    char* info_instruccion;
//    struct nodoInstrucciones* sgte;
//} t_nodoInstrucciones;

t_list *listaInstrucciones;

typedef struct infoTablaSegmentos {
    int id;
    char* direccionBase; //VER TIPO
    int tamaño;
} t_infoTablaSegmentos;

typedef struct nodoTablaSegmentos {
	t_infoTablaSegmentos info_tablaSegmentos;
    struct nodoTablaSegmentos* sgte;
} t_nodoTablaSegmentos;


typedef struct infopcb {
    int pid;
    t_list* listaInstrucciones;
    int programCounter; // numero de la siguiente instrucción a ejecutar
    char* registrosCpu[11];// el CPU debe tener un 'mapa' y conocer que posición corresponde a cada registro
    t_nodoTablaSegmentos* tablaSegmentos;// direccion base = char*?
	float estimadoProxRafaga;
	int tiempoLlegadaReady;
	t_nodoArchivos* punterosArchivos;
} t_infopcb;


//EXIT, el kerner debe reconocer qué proceso se esta ejecutando en CPU para cdo reciba un mensaje de EXIT, saber que proceso finalizar
//El ready se ejecuta con fifo o hrrn según las instrucciones que cargue el usuario

///COLA NEW
typedef struct nodoNew {
    t_infopcb info_pcb;
    struct nodoNew* sgte;
} t_nodoNew;

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
t_paquete* crear_super_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);

#endif /* UTILS_H_ */
