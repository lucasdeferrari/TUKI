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

typedef struct nodoInstrucciones {
    char* info_instruccion;
    struct nodoInstrucciones* sgte;
} t_nodoInstrucciones;

typedef struct infopcb {
    int pid;
    t_nodoInstrucciones listaInstrucciones;
    int programCounter; //Numero que nos dice la posicion del puntero de la lista de instrucciones
    int registrosCpu[15];//Ver el tipo dependiendo informacion
    int tablaSegmentos[2];//Ver el tipo dependiendo informacion
	int estimadoProxRafaga;
	int tiempoLlegadaReady;
	t_nodoArchivos punterosArchivos;
} t_infopcb;

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
