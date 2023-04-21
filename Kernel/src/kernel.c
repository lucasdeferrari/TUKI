#include "kernel.h"

t_config* config;

int main(void) {

	//sem_init(&semKernelServer,0,1);
	sem_init(&semKernelClientCPU,0,0);
	sem_init(&semKernelClientMemoria,0,0);
	sem_init(&semKernelClientFileSystem,0,0);


    logger = log_create("kernel.log", "Kernel", 1, LOG_LEVEL_DEBUG);

    config = config_create("/home/utnso/tp-2023-1c-Los-operadores/Kernel/kernel.config");

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

    //thread clients CPU, FS, Memoria		//alternativa con hilos
    iniciarHiloClienteCPU();
    iniciarHiloClienteMemoria();
    iniciarHiloClienteFileSystem();

    //thread server consola
    iniciarHiloServer();


    pthread_detach(client_CPU);
    pthread_detach(client_Memoria);
    pthread_detach(client_FileSystem);
    pthread_join(serverKernel_thread,NULL);

    //libero memoria
    log_destroy(logger);
    config_destroy(config);

    return EXIT_SUCCESS;
}

void iniciarHiloClienteCPU() {

	int err = pthread_create( &client_CPU,	// puntero al thread
	            NULL,
	        	clientCPU, // le paso la def de la función que quiero que ejecute mientras viva
				NULL); // argumentos de la función

	     if (err != 0) {
	      printf("\nNo se pudo crear el hilo de la conexión kernel-CPU.\n");
	      exit(7);
	     }
	     printf("\nEl hilo de la conexión kernel-CPU se creo correctamente.\n");

}

void iniciarHiloClienteMemoria() {

	int err = pthread_create( &client_Memoria,	// puntero al thread
	     	        NULL,
	     	    	clientMemoria, // le paso la def de la función que quiero que ejecute mientras viva
	     	    	NULL); // argumentos de la función

	     	 if (err != 0) {
	     	  printf("\nNo se pudo crear el hilo del cliente Memoria del kernel.");
	     	  exit(7);
	     	 }
	     	 printf("El hilo cliente de la Memoria se creo correctamente.");

}
void iniciarHiloClienteFileSystem() {

	int err = pthread_create( &client_FileSystem,	// puntero al thread
	     	        NULL,
					clientFileSystem, // le paso la def de la función que quiero que ejecute mientras viva
	     	    	NULL); // argumentos de la función

	     	 if (err != 0) {
	     	  printf("\nNo se pudo crear el hilo del cliente FileSystem del kernel.");
	     	  exit(7);
	     	 }
	     	 printf("\nEl hilo cliente del FileSystem se creo correctamente.");

}
void* clientCPU(void* ptr) {
	int config=1;
    int conexion_CPU;
    conexion_CPU = crear_conexion(ip_cpu, puerto_cpu);
    log_info(logger, "Ingrese sus mensajes para la CPU: ");
    paquete(conexion_CPU);
    liberar_conexion(conexion_CPU);

    sem_post(&semKernelClientCPU);
	return NULL;
}

void* clientMemoria(void* ptr) {
	sem_wait(&semKernelClientCPU);
	int config = 1;
    int conexion_Memoria;
    conexion_Memoria = crear_conexion(ip_memoria, puerto_memoria);
    log_info(logger, "Ingrese sus mensajes para la Memoria: ");
    paquete(conexion_Memoria);
    liberar_conexion(conexion_Memoria);

    sem_post(&semKernelClientMemoria);
	return NULL;
}

void* clientFileSystem(void* ptr) {
	sem_wait(&semKernelClientMemoria);
	int config = 1;
    int conexion_FileSystem;
    conexion_FileSystem = crear_conexion(ip_filesystem, puerto_filesystem);
    log_info(logger, "Ingrese sus mensajes para el FileSystem: ");
    paquete(conexion_FileSystem);
    liberar_conexion(conexion_FileSystem);

    sem_post(&semKernelClientFileSystem);
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

	sem_wait(&semKernelClientFileSystem);

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

    //sem_post(&semKernelServer);

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

//PCB
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
    int programCounter;
    int registrosCpu[15];//Ver el tipo dependiendo informacion
    int tablaSegmentos[2];//Ver el tipo dependiendo informacion
	int estimadoProxRafaga;
	int tiempoLlegadaReady;
	t_nodoArchivos punterosArchivos;
} t_infopcb;

typedef struct nodoPcb {
    t_infopcb info_pcb;
    struct nodoPcb* sgte;
} t_nodoPcb;



