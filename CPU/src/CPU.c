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

//t_paquete* deserializar(t_buffer* buffer) {
//    t_paquete* persona = malloc(sizeof(t_persona));
//
//    void* stream = buffer->stream;
//    // Deserializamos los campos que tenemos en el buffer
//    memcpy(&(persona->dni), stream, sizeof(uint32_t));
//    stream += sizeof(uint32_t);
//    memcpy(&(persona->edad), stream, sizeof(uint8_t));
//    stream += sizeof(uint8_t);
//    memcpy(&(persona->pasaporte), stream, sizeof(uint32_t));
//    stream += sizeof(uint32_t);
//
//    // Por último, para obtener el nombre, primero recibimos el tamaño y luego el texto en sí:
//    memcpy(&(persona->nombre_length), stream, sizeof(uint32_t));
//    stream += sizeof(uint32_t);
//    persona->nombre = malloc(persona->nombre_length);
//    memcpy(persona->nombre, stream, persona->nombre_length);
//
//    return persona;
//}


int main(void) {

	//DESERIALIZACIÓN DE CONTEXTO


	sem_init(&semCPUServer,0,1);
	sem_init(&semCPUClientMemoria,0,0);

	//exit1();

    logger = log_create("CPU.log", "CPU", 1, LOG_LEVEL_DEBUG);

    config = config_create("../CPU.config");

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
    t_contextoEjecucion* contextoPRUEBA;

    while (1) {

    	int cod_op = recibir_operacion(cliente_fd);
    	printf("%d", cod_op);
    	switch (cod_op) {
    		case MENSAJE:
    			recibir_mensaje(cliente_fd);
    			break;
    		case PAQUETE:
    			lista = recibir_paquete(cliente_fd);
    			log_info(logger, "Me llegaron los siguientes valores:\n");
    			list_iterate(lista, (void*) iterator);
    			break;
    		case CONTEXTO:

    			contextoPRUEBA = recibir_contexto(cliente_fd);
    			printf("programCounter recibido de Kernel = %d\n",contextoPRUEBA->programCounter);

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
