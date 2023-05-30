#include "File System.h"

t_config* config;

int block_size = 0;
int block_count = 0;

int main(void) {

	char* p_superbloque = string_new();
	char* p_bitmap = string_new();
	char* p_bloques = string_new();

	FILE* archivo_superbloque;
	FILE* archivo_bitmap;
	FILE* archivo_bloques;

	sem_init(&semFileSystemServer,0,1);
	sem_init(&semFileSystemClientMemoria,0,0);

    logger = log_create("FileSystem.log", "FileSystem", 1, LOG_LEVEL_DEBUG);

    config = config_create("/home/utnso/tp-2023-1c-Los-operadores/File System/FileSystem.config");

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
	t_config* superbloque;
	superbloque = config_create(p_superbloque);

	 if (superbloque == NULL) {
			printf("No se pudo crear el config para leer el superbloque.\n");
			exit(5);
		}

	  if (config_has_property(superbloque, "BLOCK_SIZE")) {
			 printf("Existe el la clave block size.\n");
			 block_size = config_get_int_value(superbloque, "BLOCK_SIZE");
			 }
			 else {
			 printf("No existe la clave block size.\n");
			 exit(5);
			 }

	  if (config_has_property(superbloque, "BLOCK_COUNT")) {
			 printf("Existe el la clave block count.\n");
			 block_count = config_get_int_value(superbloque, "BLOCK_COUNT");
			 }
			 else {
			 printf("No existe la clave block count.\n");
			 exit(5);
			 }

	  printf("Block count: %i\n", block_count);
	  printf("Block size: %i\n", block_size);

	  int tamanio_total = block_size * block_count;

    if (config_has_property(config, "PATH_BITMAP")) {
       	 printf("Existe el path al bitmap.\n");
       	 p_bitmap = config_get_string_value(config, "PATH_BITMAP");
       	 }
       	 else {
       	 printf("No existe el path al bitmap.\n");
       	 exit(5);
       	 }
    archivo_bitmap = fopen(p_bitmap, "r+");
    if (archivo_bitmap == NULL) {
        fprintf(stderr, "Error al abrir el archivo de bitmap.\n");
        exit(1);
    }

    // Obtener el descriptor de archivo a partir del puntero de archivo
    int fd = fileno(archivo_bitmap);
    printf("File descriptor: %i\n" , fd);

    // Ajustar el tamaño del archivo para que coincida con el tamaño del bitarray
	off_t result = lseek(fd, block_count - 1, SEEK_SET);
	if (result == -1) {
		perror("Error al ajustar el tamaño del archivo");
		exit(1);
	}

	// Escribir un byte nulo al final del archivo para que ocupe espacio
	result = write(fd, "", 1);
	if (result == -1) {
		perror("Error al escribir en el archivo");
		exit(1);
	}


    // Realizar el mapeo
    void* mapping = mmap(NULL, block_count, PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapping == MAP_FAILED) {
        perror("Error en mmap");
        exit(1);
    }

    t_bitarray* bitarray_mapeado = bitarray_create_with_mode(mapping, block_count, LSB_FIRST);

   // Pregunto la cantidad maxima de bits
   size_t cantMaxBits = bitarray_get_max_bit(bitarray_mapeado);
   printf("%lu\n", cantMaxBits);

   // Escribir en la memoria mapeada
   bitarray_set_bit(bitarray_mapeado, 0);

   // Ver el valor que acabo de modificar
   bool valor = bitarray_test_bit(bitarray_mapeado, 0);

   printf("El valor del bit 0 es %i\n", valor);

//       // Sincronizar los cambios con el archivo en disco
       if (msync(mapping, block_count, MS_SYNC) == -1) {
           perror("Error en msync");
           exit(1);
       }
    // Liberar recursos después de su uso
    munmap(mapping, block_count);
    close(fd);
    fclose(archivo_bitmap);

	//config_set_value()

    if (config_has_property(config, "PATH_BLOQUES")) {
		 printf("Existe el path a los bloques.\n");
		 p_bloques= config_get_string_value(config, "PATH_BLOQUES");
		 }
		 else {
		 printf("No existe el path a los bloques.\n");
		 exit(5);
		 }
    archivo_bloques = fopen(p_bloques, "r+");
    	//"/home/utnso/tp-2023-1c-Los-operadores/Consola/prueba.txt"

    	if (archivo_bloques == NULL) {
    		fprintf(stderr, "Error al abrir el archivo de bloques.\n");
    		exit(1);
    	}

    	int fd2 = fileno(archivo_bloques);
		printf("File descriptor: %i\n" , fd2);

		// Ajustar el tamaño del archivo para que coincida con el tamaño del bitarray
		off_t result2 = lseek(fd2, tamanio_total - 1, SEEK_SET);
		if (result2 == -1) {
			perror("Error al ajustar el tamaño del archivo");
			exit(1);
		}

		// Escribir un byte nulo al final del archivo para que ocupe espacio
		result2 = write(fd2, "", 1);
		if (result2 == -1) {
			perror("Error al escribir en el archivo");
			exit(1);
		}


		// Realizar el mapeo
		void* mapping2 = mmap(NULL, tamanio_total, PROT_WRITE, MAP_SHARED, fd2, 0);
		if (mapping2 == MAP_FAILED) {
			perror("Error en mmap");
			exit(1);
		}

//		char* mapped_data = (char*) mapping2;
//		mapped_data[0] = 'A';

	   // Sincronizar los cambios con el archivo en disco
	   if (msync(mapping2, tamanio_total, MS_SYNC) == -1) {
		   perror("Error en msync");
		   exit(1);
		   }

	   // Liberando recursos
		munmap(mapping2, tamanio_total);
		close(fd2);
		fclose(archivo_bloques);

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
    			enviar_respuesta(cliente_fd, "kernel");
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

void enviar_respuesta(int socket_cliente, char* quien_es) {
	char* handshake = quien_es;
	char* respuesta = string_new();
	respuesta = "Hola kernel, gracias por comunicarte con el fileSystem!";
	enviar_mensaje(respuesta, socket_cliente);

}
