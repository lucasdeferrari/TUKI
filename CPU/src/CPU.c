#include "CPU.h"
t_config* config;

//SE DEBEN MODIFICAR LAS FUNCIONES PARA QUE SEAN DE TIPO VOID Y MODIFIQUEN DIRECTAMENTE EL STRUCT DE CONTEXTO RECIBIDO POR KERNEL

// SET: (Registro, Valor): Asigna al registro el valor pasado como parámetro.
void set(char* reg, char* valor){
	strcpy(reg, valor);
	//delay 1000ml
}

// YIELD: Esta instrucción desaloja voluntariamente el proceso de la CPU. Se deberá devolver el Contexto de Ejecución actualizado al Kernel
t_contextoEjecucion yield(t_contextoEjecucion contexto){

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
t_contextoEjecucion exit_tp(t_contextoEjecucion contexto){

	contexto.instruccion = "exit";
//	contexto.listaInstrucciones = instrucciones;
	contexto.programCounter++; // numero de la siguiente instrucción a ejecutar

//	for(int i = 0 ; i < 12; i++)
//		set(contexto.registrosCpu[i], registrosCpu[i]);

//	contexto.tablaSegmentos = tablaSegmentos;// direccion base = char*?

	//Modificaciones Contexto

	return contexto;
}



//I/O (Tiempo): Esta instrucción representa una syscall de I/O bloqueante.
//Se deberá devolver el Contexto de Ejecución actualizado al Kernel junto a la
//cantidad de unidades de tiempo que va a bloquearse el proceso
void* i_o(int socket) {
//	printf("\n antes -> %d", contextoPRUEBA->programCounter);
	contextoPRUEBA->programCounter++;
	contextoPRUEBA->tiempoBloqueado = 1000;
	contextoPRUEBA->instruccion = "i/o";
	contextoPRUEBA->instruccion_length = string_length(contextoPRUEBA->instruccion) + 1;

//	printf("\n Intruccion -> %s", contextoPRUEBA->instruccion);
//	printf("\n Intruccion length-> %d", contextoPRUEBA->instruccion_length);
//	printf("\n programCounter -> %d", contextoPRUEBA->programCounter);
//	printf("\n tiempo bloqueado -> %d", contextoPRUEBA->tiempoBloqueado);

	return NULL;
}

////WAIT (Recurso): Esta instrucción solicita al Kernel que se asigne una instancia
////del recurso indicado por parámetro.
void* wait(char* recurso, int socket) {
//		printf("\n antes -> %d", contextoPRUEBA->programCounter);
		contextoPRUEBA->programCounter++;
		contextoPRUEBA->instruccion = "wait";
		strcpy(contextoPRUEBA->recursoSolicitado, recurso);
		contextoPRUEBA->instruccion_length = string_length(contextoPRUEBA->instruccion) + 1;

//		printf("\n Intruccion -> %s", contextoPRUEBA->instruccion);
//		printf("\n Intruccion length-> %d", contextoPRUEBA->instruccion_length);
//		printf("\n programCounter -> %d", contextoPRUEBA->programCounter);
//		printf("\n recurso -> %s", contextoPRUEBA->recursoSolicitado);

		return NULL;
}

//SIGNAL (Recurso): Esta instrucción solicita al Kernel que se libere una instancia
//del recurso indicado por parámetro.
void* signal_tp(char* recurso, int socket) {
//	printf("\n antes -> %d", contextoPRUEBA->programCounter);
	contextoPRUEBA->programCounter++;
	contextoPRUEBA->instruccion = "signal";
	strcpy(contextoPRUEBA->recursoSolicitado, recurso);
	contextoPRUEBA->instruccion_length = string_length(contextoPRUEBA->instruccion) + 1;

//	printf("\n Intruccion -> %s", contextoPRUEBA->instruccion);
//	printf("\n Intruccion length-> %d", contextoPRUEBA->instruccion_length);
//	printf("\n programCounter -> %d", contextoPRUEBA->programCounter);
//	printf("\n recurso -> %s", contextoPRUEBA->recursoSolicitado);

	return NULL;
}

int main(void) {

	//DESERIALIZACIÓN DE CONTEXTO


	sem_init(&semCPUServer,0,1);
	sem_init(&semCPUClientMemoria,0,0);

	//exit1();

    logger = log_create("CPU.log", "CPU", 1, LOG_LEVEL_DEBUG);

    config = config_create("/home/utnso/tp-2023-1c-Los-operadores/CPU/CPU.config");

    if (config == NULL) {
        printf("No se pudo crear el config.");
        exit(5);
    }

    ip_memoria= config_get_string_value(config, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");

    //Server
    iniciarHiloServer();

    //Hilo Cliente
    iniciarHiloCliente();

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
    char buffer[1024] = {0};

    conexion_Memoria = crear_conexion(ip_memoria, puerto_memoria);
    enviar_mensaje("CPU",conexion_Memoria);
    log_info(logger, "Ingrese sus mensajes para la Memoria: ");
    paquete(conexion_Memoria);
    int cod_op = recibir_operacion(conexion_Memoria);
    printf("codigo de operacion: %i\n", cod_op);
    recibir_mensaje(conexion_Memoria);
//    int valread = read(conexion_Memoria, buffer, 1024);
//    printf("Respuesta del servidor: %s\n", buffer);
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
    		case INSTRUCCIONES:
    			contextoPRUEBA->listaInstrucciones = recibir_paquete(cliente_fd);
    			log_info(logger, "Me llegaron las siguientes intrucciones:\n");
    			list_iterate(contextoPRUEBA->listaInstrucciones, (void*) iterator);
    			break;
    		case CONTEXTO:
    			contextoPRUEBA = recibir_contexto(cliente_fd);
    			printf("\nprogramCounter recibido de Kernel = %d\n",contextoPRUEBA->programCounter);
    			printf("AX recibido de Kernel = %s\n",contextoPRUEBA->registrosCpu.AX);
    			printf("CX recibido de Kernel = %s\n",contextoPRUEBA->registrosCpu.BX);
    			printf("BX recibido de Kernel = %s\n",contextoPRUEBA->registrosCpu.CX);
    			printf("DX recibido de Kernel = %s\n",contextoPRUEBA->registrosCpu.DX);

    			printf("EAX recibido de Kernel = %s\n",contextoPRUEBA->registrosCpu.EAX);
    			printf("EBX recibido de Kernel = %s\n",contextoPRUEBA->registrosCpu.EBX);
    			printf("ECX recibido de Kernel = %s\n",contextoPRUEBA->registrosCpu.ECX);
    			printf("EDX recibido de Kernel = %s\n",contextoPRUEBA->registrosCpu.EDX);

    			printf("RAX recibido de Kernel = %s\n",contextoPRUEBA->registrosCpu.RAX);
    			printf("RBX recibido de Kernel = %s\n",contextoPRUEBA->registrosCpu.RBX);
    			printf("RCX recibido de Kernel = %s\n",contextoPRUEBA->registrosCpu.RCX);
    			printf("RDX recibido de Kernel = %s\n",contextoPRUEBA->registrosCpu.RDX);

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
	char* leido = string_new();
	t_paquete* paquete = crear_paquete();



	// Leemos y esta vez agregamos las lineas al paquete
	leido = readline("> ");

	while(strcmp(leido, "") != 0){
		agregar_a_paquete(paquete, leido, strlen(leido)+1);
		leido = readline("> ");
	}

	enviar_paquete(paquete, conexion);

	free(leido);
	eliminar_paquete(paquete);

}
