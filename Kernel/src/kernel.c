#include "kernel.h"

t_config* config;

int main(void) {

	sem_init(&semKernelServer,0,1);
	sem_init(&semKernelClient,0,0);
	pthread_t memoria;

    logger = log_create("kernel.log", "Kernel", 1, LOG_LEVEL_DEBUG);

    config = config_create("./kernel.config");

    if (config == NULL) {
        printf("No se pudo crear el config.");
        exit(5);
    }

    ip_cpu = config_get_string_value(config, "IP_CPU");
    puerto_cpu = config_get_string_value(config, "PUERTO_CPU");
    ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    ip_filesystem = config_get_string_value(config, "IP_FILESYSTEM");
    puerto_filesystem = config_get_string_value(config, "PUERTO_FILESYSTEM");

    //THREADS CONEXIÓN

    //thread clientes CPU, FS, Memoria
    iniciarHilosCliente();

    //thread server consola
    iniciarHiloServer();


    pthread_join(client_CPU,NULL);
    //pthread_join(client_Memoria,NULL);
    //pthread_join(client_FS,NULL);
    pthread_join(serverKernel_thread,NULL);

    //libero memoria
    log_destroy(logger);
    config_destroy(config);

    return EXIT_SUCCESS;
}

void iniciarHilosCliente() {
//	int err = pthread_create( &client_FS,	// puntero al thread
//	        NULL,
//	    	&clientFS, // le paso la def de la función que quiero que ejecute mientras viva
//	    	NULL); // argumentos de la función
//
//	 if (err != 0) {
//	  printf("\nNo se pudo crear el hilo del cliente FS del kernel.");
//	  exit(7);
//	 }
//	 printf("El hilo cliente del File System se creo correctamente.");

	int err = pthread_create( &client_CPU,	// puntero al thread
	            NULL,
	        	clientCPU, // le paso la def de la función que quiero que ejecute mientras viva
				NULL); // argumentos de la función

	     if (err != 0) {
	      printf("\nNo se pudo crear el hilo de la conexión kernel-CPU.\n");
	      exit(7);
	     }
	     printf("\nEl hilo de la conexión kernel-CPU se creo correctamente.\n");


	     err = pthread_create( &client_Memoria,	// puntero al thread
	     	        NULL,
	     	    	clientMemoria, // le paso la def de la función que quiero que ejecute mientras viva
	     	    	NULL); // argumentos de la función

	     	 if (err != 0) {
	     	  printf("\nNo se pudo crear el hilo del cliente Memoria del kernel.");
	     	  exit(7);
	     	 }
	     	 printf("El hilo cliente de la Memoria se creo correctamente.");

}


void* clientCPU(void* ptr) {
	int config=1;
    int conexion_CPU;
    conexion_CPU = crear_conexion(ip_cpu, puerto_cpu);
    log_info(logger, "Ingrese sus mensajes para la CPU: ");
    paquete(conexion_CPU);
    liberar_conexion(conexion_CPU);

    sem_post(&semKernelClient);
	return NULL;
}

void* clientMemoria(void* ptr) {
	int config = 1;
    int conexion_Memoria;
    conexion_Memoria = crear_conexion(ip_memoria, puerto_memoria);
    log_info(logger, "Ingrese sus mensajes para la CPU: ");
    paquete(conexion_Memoria);
    liberar_conexion(conexion_Memoria);

    sem_post(&semKernelClient);
	return NULL;
}

void iniciarHiloServer() {

    int err = pthread_create( &serverKernel_thread,	// puntero al thread
    	            NULL,
    	        	&serverKernel, // le paso la def de la función que quiero que ejecute mientras viva
    				NULL); // argumentos de la función

    	     if (err != 0) {
    	      printf("\nNo se pudo crear el hilo de la conexión consola-kernel.\n");
    	      exit(7);
    	     }
    	     printf("\nEl hilo de la conexión consola-kernel se creo correctamente.\n");

}


void* serverKernel(void* ptr){

	sem_wait(&semKernelClient);

    int server_fd = iniciar_servidor();
    log_info(logger, "Kernel listo para recibir a la consola");
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
    			log_error(logger, "\nel cliente se desconecto. Terminando servidor");
    			return EXIT_FAILURE;
    			default:
    			log_warning(logger,"\nOperacion desconocida. No quieras meter la pata");
    		break;
    	}
    }

    sem_post(&semKernelServer);

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
