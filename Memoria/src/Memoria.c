#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include "utils.h"
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <pthread.h>
#include <semaphore.h>

char* ip_memoria;
char* puerto_memoria;
char* ipKernel;
char* puertoKernel;
sem_t semKernelServer;
sem_t semKernelClient;
t_config* config;

void* clientKernel() {
	int config=1;
    int conn;
    conn = crear_conexion(ipKernel, puertoKernel);
    log_info(logger, "Ingrese sus mensajes para la CPU: ");
    paquete(conn);
    liberar_conexion(conn);

    sem_post(&semKernelClient);
	return NULL;
}

void iniciarHilosCliente(pthread_t clientMemory) {
	int err = pthread_create( &clientMemory, NULL, clientKernel, NULL);

	 if (err != 0) {
	  printf("\nNo se pudo crear el hilo de la conexión kernel-CPU.\n");
	  exit(7);
	 }
	 printf("\nEl hilo de la conexión kernel-CPU se creo correctamente.\n");
}

int main(void) {
	pthread_t clientMemory;
	sem_init(&semKernelServer,0,1);
	sem_init(&semKernelClient,0,0);

	logger = log_create("memory.log", "Memory", 1, LOG_LEVEL_DEBUG);

	config = iniciar_config();
	config = config_create("./memoria.config");

	if (config == NULL) {
		printf("No se pudo crear el config.");
		exit(5);
	}

	ip_memoria = config_get_string_value(config, "IP");
	puerto_memoria = config_get_string_value(config, "PUERTO");

	//THREADS CONEXIÓN

	//thread clientes CPU, FS, Memoria
	iniciarHilosCliente(clientMemory);

//	pthread_join(client_CPU,NULL);
	pthread_join(clientMemory, NULL);
	//pthread_join(client_Memoria,NULL);
	//pthread_join(client_FS,NULL);
//	pthread_join(serverKernel_thread,NULL);

	//libero memoria
	log_destroy(logger);
	config_destroy(config);

	return EXIT_SUCCESS;
}
