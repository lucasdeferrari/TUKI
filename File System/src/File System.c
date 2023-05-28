#include "File System.h"

t_config* config;

int block_size = 0;
int block_count = 0;

int main(void) {

	char* p_superbloque = string_new();
	char* p_bitmap = string_new();
	char* p_bloques = string_new();

	char* linea = string_new();

	char* superbloque[2];

	FILE* archivo_superbloque;
	FILE* archivo_bitmap;
	FILE* archivo_bloques;

	sem_init(&semFileSystemServer,0,1);
	sem_init(&semFileSystemClientMemoria,0,0);

    logger = log_create("FileSystem.log", "FileSystem", 1, LOG_LEVEL_DEBUG);

//    config = config_create("/home/utnso/tp-2023-1c-Los-operadores/File System/FileSystem.config");
    config = config_create("./FileSystem.config");

    if (config == NULL) {
        printf("No se pudo crear el config.\n");
        exit(5);
    }

    if (config_has_property(config, "PATH_SUPERBLOQUE")) {
    	 printf("Existe el path al superbloque.\n");
    	 p_superbloque = config_get_string_value(config, "PATH_SUPERBLOQUE");
    	 }
    	 else {
    	 printf("No existe el path al superbloque.\n");
    	 exit(5);
    	 }
    archivo_superbloque = fopen(p_superbloque, "r");
    	//"/home/utnso/tp-2023-1c-Los-operadores/Consola/prueba.txt"

    	if (archivo_superbloque == NULL) {
    		fprintf(stderr, "Error al abrir el archivo de superbloque.\n");
    		exit(1);
    	}
//	  if (config_has_property(p_superbloque, "BLOCK_SIZE")) {
//			 printf("Existe el la clave block size.\n");
//			 block_size = config_get_int_value(config, "BLOCK_SIZE");
//			 }
//			 else {
//			 printf("No existe la clave block size.\n");
//			 exit(5);
//			 }
//
//	  if (config_has_property(p_superbloque, "BLOCK_COUNT")) {
//			 printf("Existe el la clave block count.\n");
//			 block_count = config_get_int_value(config, "BLOCK_COUNT");
//			 }
//			 else {
//			 printf("No existe la clave block count.\n");
//			 exit(5);
//			 }
//
//	  printf("Block count: %i", block_count);
//	  printf("Block size: %i", block_size);

    if (config_has_property(config, "PATH_BITMAP")) {
       	 printf("Existe el path al bitmap.\n");
       	 p_bitmap = config_get_string_value(config, "PATH_BITMAP");
       	 }
       	 else {
       	 printf("No existe el path al bitmap.\n");
       	 exit(5);
       	 }
    archivo_bitmap = fopen(p_bitmap, "r");
    	//"/home/utnso/tp-2023-1c-Los-operadores/Consola/prueba.txt"

    	if (archivo_bitmap == NULL) {
    		fprintf(stderr, "Error al abrir el archivo de bitmap.\n");
    		exit(1);
    	}

    if (config_has_property(config, "PATH_BLOQUES")) {
		 printf("Existe el path a los bloques.\n");
		 p_bloques= config_get_string_value(config, "PATH_BLOQUES");
		 }
		 else {
		 printf("No existe el path a los bloques.\n");
		 exit(5);
		 }
    archivo_bloques = fopen(p_bloques, "r");
    	//"/home/utnso/tp-2023-1c-Los-operadores/Consola/prueba.txt"

    	if (archivo_bloques == NULL) {
    		fprintf(stderr, "Error al abrir el archivo de bloques.\n");
    		exit(1);
    	}

    ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");

    //THREADS CONEXIÓN

    //thread cliente Memoria
    iniciarHiloCliente();

    //thread server consola
    iniciarHiloServer();

    //pthread_join
    pthread_join(client_Memoria,NULL);
    pthread_join(serverFileSystem_thread,NULL);

    //libero memoria
    log_destroy(logger);
    config_destroy(config);

    return EXIT_SUCCESS;
}

void iniciarHiloCliente() {

	int err = pthread_create( &client_Memoria,	// puntero al thread
	     	        NULL,
	     	    	clientMemoria, // le paso la def de la función que quiero que ejecute mientras viva
	     	    	NULL); // argumentos de la función

	     	 if (err != 0) {
	     	  printf("\nNo se pudo crear el hilo del cliente Memoria del File System.");
	     	  exit(7);
	     	 }
	     	 printf("El hilo cliente de la Memoria se creo correctamente.");

}

void* clientMemoria(void* ptr) {
	int config = 1;
    int conexion_Memoria;
    conexion_Memoria = crear_conexion(ip_memoria, puerto_memoria);
    enviar_mensaje("filesystem",conexion_Memoria);
    log_info(logger, "Ingrese sus mensajes para la Memoria: ");
    paquete(conexion_Memoria);
    int cod_op = recibir_operacion(conexion_Memoria);
    printf("codigo de operacion: %i\n", cod_op);
    recibir_mensaje(conexion_Memoria);
    liberar_conexion(conexion_Memoria);

    sem_post(&semFileSystemClientMemoria);
	return NULL;
}

void iniciarHiloServer() {

    int err = pthread_create( &serverFileSystem_thread,	// puntero al thread
    	            NULL,
    	        	&serverFileSystem, // le paso la def de la función que quiero que ejecute mientras viva
    				NULL); // argumentos de la función

    	     if (err != 0) {
    	      printf("\nNo se pudo crear el hilo de la conexión Kernel-Filesystem.\n");
    	      exit(7);
    	     }
    	     printf("\nEl hilo de la conexión Kernel-FileSystem se creo correctamente.\n");

}


void* serverFileSystem(void* ptr){

	sem_wait(&semFileSystemClientMemoria);

    int server_fd = iniciar_servidor();
    log_info(logger, "FileSystem listo para recibir a kernel");
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

    sem_post(&semFileSystemServer);

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
