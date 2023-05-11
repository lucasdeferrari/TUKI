#include "CPU.h"

t_config* config;

typedef struct infoTablaSegmentos {
    int id;
    char* direccionBase; //VER TIPO
    int tamanio;
} t_infoTablaSegmentos;

typedef struct nodoTablaSegmentos {
	t_infoTablaSegmentos info_tablaSegmentos;
    struct nodoTablaSegmentos* sgte;
} t_nodoTablaSegmentos;

typedef struct {
	char* instruccion;
	t_list* listaInstrucciones;
	int programCounter; // numero de la siguiente instrucción a ejecutar
	char* registrosCpu[12];// el CPU debe tener un 'mapa' y conocer que posición corresponde a cada registro
	t_nodoTablaSegmentos* tablaSegmentos;// direccion base = char*?
} contextExecution;

// SET: (Registro, Valor): Asigna al registro el valor pasado como parámetro.
void set(char* reg, char* valor){
	strcpy(reg, valor);
	//delay 1000ml
}

// YIELD: Esta instrucción desaloja voluntariamente el proceso de la CPU. Se deberá devolver el Contexto de Ejecución actualizado al Kernel
contextExecution yield(contextExecution contexto){

	contexto.instruccion = "yield";
//	contexto.listaInstrucciones = instrucciones;
	contexto.programCounter++; // numero de la siguiente instrucción a ejecutar

//	for(int i = 0 ; i < 12; i++)
//		set(contexto.registrosCpu[i], registrosCpu[i]);

//	contexto.tablaSegmentos = tablaSegmentos;// direccion base = char*?

	//Modificaciones Contexto

    return contexto;
}

// EXIT: Esta instrucción representa la syscall de finalización del proceso.
// Se deberá devolver el Contexto de Ejecución actualizado al Kernel para su finalización.
contextExecution exit_tp(contextExecution contexto){

	contexto.instruccion = "exit";
//	contexto.listaInstrucciones = instrucciones;
	contexto.programCounter++; // numero de la siguiente instrucción a ejecutar

//	for(int i = 0 ; i < 12; i++)
//		set(contexto.registrosCpu[i], registrosCpu[i]);

//	contexto.tablaSegmentos = tablaSegmentos;// direccion base = char*?

	//Modificaciones Contexto

	return contexto;
}

int main(void) {

	sem_init(&semCPUServer,0,1);
	sem_init(&semCPUClientMemoria,0,0);

	exit1();

    logger = log_create("CPU.log", "CPU", 1, LOG_LEVEL_DEBUG);

    config = config_create("./CPU.config");

    if (config == NULL) {
        printf("No se pudo crear el config.");
        exit(5);
    }

    ip_memoria= config_get_string_value(config, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");

    //Hilo Cliente
    iniciarHiloCliente();

    //Server
    iniciarHiloServer();

    //pthread_join
    pthread_join(client_Memoria,NULL);
    pthread_join(serverCPU_thread,NULL);

    //libero memoria
    log_destroy(logger);
    config_destroy(config);

    return EXIT_SUCCESS;
}

void iniciarHiloCliente() {

	int err = pthread_create( 	&client_Memoria,	// puntero al thread
	     	        			NULL,
								clientMemoria, // le paso la def de la función que quiero que ejecute mientras viva
								NULL); // argumentos de la función

	if (err != 0) {
	printf("\nNo se pudo crear el hilo del cliente Memoria del CPU.");
	exit(7);
	}
	printf("El hilo cliente de la Memoria se creo correctamente.");

}

void* clientMemoria(void* ptr) {
	int config = 1;
    int conexion_Memoria;

    conexion_Memoria = crear_conexion(ip_memoria, puerto_memoria);
    enviar_mensaje("CPU",conexion_Memoria);
    log_info(logger, "Ingrese sus mensajes para la Memoria: ");
    paquete(conexion_Memoria);
    liberar_conexion(conexion_Memoria);

    sem_post(&semCPUClientMemoria);
	return NULL;
}

void iniciarHiloServer() {

    int err = pthread_create( 	&serverCPU_thread,	// puntero al thread
    	            			NULL,
								&serverCPU, // le paso la def de la función que quiero que ejecute mientras viva
								NULL); // argumentos de la función

	 if (err != 0) {
	  printf("\nNo se pudo crear el hilo de la conexión kernel-CPU \n");
	  exit(7);
	 }
	 printf("\nEl hilo de la conexión kernel-CPU se creo correctamente.\n");

}

void* serverCPU(void* ptr){

	sem_wait(&semCPUClientMemoria);

    int server_fd = iniciar_servidor();
    log_info(logger, "CPU lista para recibir al Kernel");
    int cliente_fd = esperar_cliente(server_fd);

    t_list* lista;
    while (1) {
    	int cod_op = recibir_operacion(cliente_fd);
    	switch (cod_op) {
    		case MENSAJE:
    			recibir_mensaje(cliente_fd);
    			break;
    		case PAQUETE:
    			lista = recibir_paquete(cliente_fd);
    			log_info(logger, "Me llegaron los siguientes valores:\n");
    			list_iterate(lista, (void*) iterator);
    			break;
    		case -1:
    			log_error(logger, "\nel kernel se desconecto. Terminando servidor");
    			return EXIT_FAILURE;
    			default:
    			log_warning(logger,"\nOperacion desconocida. No quieras meter la pata");
    		break;
    	}
    }

    sem_post(&semCPUServer);

	return NULL;
}


void iterator(char* value) {
    log_info(logger, value);
}

//Funciones client

t_config* iniciar_config(void)
{
	t_config* nuevo_config;

	return nuevo_config;
}

void paquete(int conexion)
{
	// Ahora toca lo divertido!
	char* leido;
	t_paquete* paquete;

	paquete = crear_paquete();

	// Leemos y esta vez agregamos las lineas al paquete
	leido = readline("> ");

	while(strcmp(leido, "") != 0){
		agregar_a_paquete(paquete, leido, strlen(leido));
		leido = readline("> ");
	}

	enviar_paquete(paquete, conexion);

	free(leido);
	eliminar_paquete(paquete);

}

// Registros de 4 bytes: AX, BX, CX, DX.
		// 0.  AX (Accumulator Register): Es utilizado como registro acumulador para operaciones aritméticas y lógicas.
		//     Es comúnmente usado para almacenar resultados de operaciones y como fuente o destino de datos en instrucciones.

		// 1.  BX (Base Register): Es utilizado como registro base en operaciones de direccionamiento.
		//	   Puede ser usado para almacenar una dirección base, un puntero a una estructura de datos o un índice en un arreglo.

		// 2.  CX (Counter Register): Es utilizado como registro contador en bucles y repeticiones.
		//	   Puede ser utilizado en combinación con instrucciones de salto y comparación para controlar la repetición de un bloque de código.

		// 3.  DX (Data Register): Es utilizado como registro de datos en operaciones de entrada y salida.
		//     Puede ser utilizado para almacenar datos temporalmente antes de ser enviados o después de ser recibidos.

	// Registros de 8 bytes: EAX, EBX, ECX, EDX
		// 4.  EAX (Extended Accumulator Register): Es utilizado como registro acumulador para operaciones aritméticas y lógicas.
		//	   Al igual que AX, EAX es ampliamente utilizado para almacenar resultados de operaciones y como fuente o destino de datos en instrucciones.

		// 5.  EBX (Extended Base Register): Es utilizado como registro base en operaciones de direccionamiento.
		//	   Al igual que BX, EBX puede ser usado para almacenar una dirección base, un puntero a una estructura de datos o un índice en un arreglo.

		// 6.  ECX (Extended Counter Register): Es utilizado como registro contador en bucles y repeticiones.
		//	   Al igual que CX, ECX se utiliza en combinación con instrucciones de salto y comparación para controlar la repetición de un bloque de código.

		// 7.  EDX (Extended Data Register): Es utilizado como registro de datos en operaciones de entrada y salida.
		//	   Al igual que DX, EDX puede ser utilizado para almacenar datos temporalmente antes de ser enviados o después de ser recibidos.

	// Registros de 16 bytes: RAX, RBX, RCX, RDX
		// 8.  RAX (Accumulator Register): Es utilizado como registro acumulador para operaciones aritméticas y lógicas.
		//	   También es utilizado para retornar valores desde una función.

		// 9.  RBX (Base Register): Es utilizado como registro base en operaciones de direccionamiento.
		//	   Puede ser utilizado para almacenar una dirección base, un puntero a una estructura de datos o un índice en un arreglo.

		// 10. RCX (Counter Register): Es utilizado como registro contador en bucles y repeticiones.
		//	   Se utiliza en combinación con instrucciones de salto y comparación para controlar la repetición de un bloque de código.

		// 11. RDX (Data Register): Es utilizado como registro de datos en operaciones de entrada y salida.
		//	   También puede ser utilizado para el almacenamiento temporal de datos.
