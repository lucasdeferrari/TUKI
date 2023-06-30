#include "FileSystem.h"

t_config* config;

int block_size = 0;
int block_count = 0;
int server_fd;

int main(void) {

	char* p_superbloque = string_new();
	char* p_bitmap = string_new();
	char* p_bloques = string_new();

	FILE* archivo_superbloque;
	FILE* archivo_bitmap;
	FILE* archivo_bloques;


    logger = log_create("FileSystem.log", "FileSystem", 1, LOG_LEVEL_DEBUG);

    config = config_create("/home/utnso/tp-2023-1c-Los-operadores/FileSystem/FileSystem.config");

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

    //CHEQUEO SI EL ARCHIVO YA EXISTE
    if (access(p_bitmap, F_OK) == -1) {
       printf("El archivo bitmap no existe.\n");
       archivo_bitmap = fopen(p_bitmap, "w");
       if (archivo_bitmap) {
              printf("El archivo bitmap se ha creado exitosamente.\n");
              fclose(archivo_bitmap);
          } else {
              printf("No se pudo crear el archivo bitmap.\n");
          }
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
   bitarray_set_bit(bitarray_mapeado, 1);
   bitarray_set_bit(bitarray_mapeado, 2);

   // Ver el valor que acabo de modificar
   bool valor = bitarray_test_bit(bitarray_mapeado, 0);
   bool valor2 = bitarray_test_bit(bitarray_mapeado, 1);
   bool valor3 = bitarray_test_bit(bitarray_mapeado, 2);
   bool valor4 = bitarray_test_bit(bitarray_mapeado, 3);


   printf("El valor del bit 0 es %i\n", valor);
   printf("El valor del bit 1 es %i\n", valor2);
   printf("El valor del bit 2 es %i\n", valor3);
   printf("El valor del bit 3 es %i\n", valor4);

   // Sincronizar los cambios con el archivo en disco
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

    if (access(p_bloques, F_OK) == -1) {
           printf("El archivo bloques no existe.\n");
           archivo_bloques = fopen(p_bloques, "w");
           if (archivo_bloques) {
                  printf("El archivo bloques se ha creado exitosamente.\n");
                  fclose(archivo_bloques);
              } else {
                  printf("No se pudo crear el archivo bloques.\n");
              }
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

	// CAMBIAR EL PRIMER VALOR DEL ARCHIVO BLOQUES
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

    //Inicializo la lista de FCBs
    listaFCB = list_create();

    //THREADS CONEXIÓN

    //thread cliente Memoria
    iniciarHiloCliente();


	server_fd = iniciar_servidor();
	log_info(logger, "FileSystem listo para escuchar al cliente\n");

    //thread server
    while(1){
    	iniciarHiloServer();
    	pthread_join(serverFileSystem_thread, NULL);
    }

    //pthread_join
    pthread_join(client_Memoria,NULL);
    pthread_join(serverFileSystem_thread,NULL);

    //libero memoria
    log_destroy(logger);
    config_destroy(config);

    return EXIT_SUCCESS;
}


////////////////////////////////////    SERVER FS     ////////////////////////////////////////


void iniciarHiloServer() {
    int err = pthread_create( &serverFileSystem_thread,	// puntero al thread
    	            NULL,
    	        	&serverFileSystem, // le paso la def de la función que quiero que ejecute mientras viva
    				NULL); // argumentos de la función

    	     if (err != 0) {
    	      printf("\nNo se pudo crear el hilo de la conexión Kernel-Filesystem.\n");
    	      exit(7);
    	     }
    	     //printf("\nEl hilo de la conexión Kernel-FileSystem se creo correctamente.\n");
}

void* serverFileSystem(void* ptr){

    log_info(logger, "FileSystem listo para recibir a kernel");
    int cliente_fd = esperar_cliente(server_fd); //LO VUELVO A AGREGAR ACA

    t_list* lista;
    while (1) {
    	int cod_op = recibir_operacion(cliente_fd);
    	char* nombreArchivo = string_new();
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
    		case F_OPEN:
    			//PODRÍAMOS SIMPLEMENTE TENER EL CASE F_OPEN
    			//SI EXISTE LO ABRIMOS Y DEVOLVEMOS OK
    			//SI NO EXISTE, LO CREAMOS, LO ABRIMOS Y DEVOLVEMOS OK

    			nombreArchivo = recibir_buffer_mio(cliente_fd);
    			abrir_archivo(nombreArchivo);
    			enviar_mensaje_cod_operacion("Archivo abierto.",cliente_fd,F_OPEN);
    			liberar_conexion(cliente_fd);
    			break;
//    		case CREAR_ARCHIVO:
//    			nombreArchivo = recibir_buffer_mio(cliente_fd);
//    			crearArchivo(nombreArchivo);
//    			break;
    		case F_READ:
    			// ORDEN PARÁMETROS: nombreArchivo - puntero - cantBytes - direcFisica
    			lista = recibir_paquete(cliente_fd);
    			t_list_iterator* iteradorRead = list_iterator_create(lista);

    			char* paqueteRead[4] = {};

    			for (int i = 0; i<4; i++) {
    				char* siguiente = list_iterator_next(iteradorRead);
    				paqueteRead[i] = siguiente;
    			}

    			list_iterator_destroy(iteradorRead);

    			nombreArchivo = paqueteRead[0];
    			int punteroArchivo = atoi(paqueteRead[1]);
    			int cantBytesRead = atoi(paqueteRead[2]);
    			int direcFisicaRead = atoi(paqueteRead[3]);

    			//FUNCIÓN F_READ

    			enviar_mensaje_cod_operacion("",cliente_fd,F_READ);
    			liberar_conexion(cliente_fd);

    			break;
    		case F_WRITE:
    			// ORDEN PARÁMETROS: nombreArchivo - cantBytes - direcFisica
    			lista = recibir_paquete(cliente_fd);
    			t_list_iterator* iteradorWrite = list_iterator_create(lista);

    			char* paqueteWrite[3] = {};

    			for (int i = 0; i<3; i++) {
    				char* siguiente = list_iterator_next(iteradorWrite);
    				paqueteWrite[i] = siguiente;
    			}

    			list_iterator_destroy(iteradorWrite);

    			nombreArchivo = paqueteWrite[0];
    			int cantBytesWrite = atoi(paqueteWrite[1]);
    			int direcFisicaWrite = atoi(paqueteWrite[2]);

    			//FUNCIÓN F_WRITE

    			enviar_mensaje_cod_operacion("",cliente_fd,F_WRITE);
    			liberar_conexion(cliente_fd);

    			break;
    		case F_TRUNCATE:
    			lista = recibir_paquete(cliente_fd);
    			t_list_iterator* iteradorTruncate = list_iterator_create(lista);

    			char* paqueteTruncate[2] = {};

    			for (int i = 0; i<2; i++) {
    				char* siguiente = list_iterator_next(iteradorTruncate);
    				paqueteTruncate[i] = siguiente;
    			}

    			list_iterator_destroy(iteradorTruncate);

    			nombreArchivo = paqueteTruncate[0];
    			int tamanioArchivo = atoi(paqueteTruncate[1]);

    			truncar_archivo(nombreArchivo, tamanioArchivo);
    			enviar_mensaje_cod_operacion("",cliente_fd,F_TRUNCATE);
    			liberar_conexion(cliente_fd);
    			break;
    		case -1:
    			log_error(logger, "\nel cliente se desconecto. Terminando servidor.\n");
    			return EXIT_FAILURE;
    		default:
    			log_warning(logger,"\nOperacion recibida desconocida.\n");
    		break;

    	}

    }

	return NULL;
}

////////////////////////////////////    CLIENTE MEMORIA     ////////////////////////////////////////


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
    int cod_op = recibir_operacion(conexion_Memoria);
    printf("codigo de operacion: %i\n", cod_op);
    recibir_mensaje(conexion_Memoria);
    liberar_conexion(conexion_Memoria);

	return NULL;
}


/////////////////////////////////////  FUNCIONES ARCHIVOS ////////////////////////////////////////////


void crearArchivo(char* nombreArchivo) {

	t_infofcb* nuevoFCB = malloc(sizeof(t_infofcb));

	nuevoFCB->nombreArchivo = nombreArchivo;
	nuevoFCB->tamanioArchivo = 0;

	list_add(listaFCB, nuevoFCB);

	printf("Archivo creado.\n");

	//Agregar una lista? Al archivo de bloques? La lista de FCBs tiene que persistir.
	return;
}

void abrir_archivo(char* nombreArchivo){
	t_list_iterator* iterador = list_iterator_create(listaFCB);
	char* nombreArchivoSeleccionado;

	while(list_iterator_has_next(iterador)) {
		t_infofcb *siguiente = list_iterator_next(iterador);
		if(   strcmp(nombreArchivo,siguiente->nombreArchivo) == 0   ) {
			strcpy(nombreArchivoSeleccionado,nombreArchivo);
		}
	}

	if (string_is_empty(nombreArchivoSeleccionado)) {
		//Enviar mensaje Kernel "Archivo Inexistente"
		//enviar_mensaje("Archivo inexistente", cliente_fd);
		//SI NO EXISTE, LO CREAMOS
		printf("Archivo inexistente.\n");
		crearArchivo(nombreArchivoSeleccionado);
	}

	//ACA NO TENDRÍAMOS QUE ABRIR EL ARCHIVO ?? NO SE HACE UN F_OPEN O ALGO????

	//Enviar mensaje Kernel OK
	//enviar_mensaje("Archivo abierto", cliente_fd);   ->  LO HACEMOS DIRECTO EN EL SERVER

	list_iterator_destroy(iterador);
	return;
}

void truncar_archivo(char* nombreArchivo, int tamanio){
	t_list_iterator* iterador = list_iterator_create(listaFCB);

	while(list_iterator_has_next(iterador)) {
		t_infofcb *siguiente = list_iterator_next(iterador);
		if(nombreArchivo == siguiente->nombreArchivo) {
			if (tamanio < siguiente->tamanioArchivo){
//				Reducir el tamaño del archivo: Se deberá asignar el nuevo tamaño del archivo en el FCB y
//				se deberán marcar como libres todos los bloques que ya no sean necesarios para direccionar
//				el tamaño del archivo (descartando desde el final del archivo hacia el principio).
			} else if (tamanio < siguiente->tamanioArchivo){
//				Ampliar el tamaño del archivo: Al momento de ampliar el tamaño del archivo deberá actualizar
//				el tamaño del archivo en el FCB y se le deberán asignar tantos bloques como sea necesario para
//				poder direccionar el nuevo tamaño.
			}
		}
	}
}


char* recibir_buffer_mio(int socket_cliente) {
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	return buffer;
}


//////////////////////////////////// FUNCIONES EXTRAS //////////////////////////////////////

void enviar_respuesta(int socket_cliente, char* quien_es) {
	//Para que esta el handshake?
	char* handshake = quien_es;
	char* respuesta = string_new();
	respuesta = "Hola kernel, gracias por comunicarte con el fileSystem!";
	enviar_mensaje(respuesta, socket_cliente);
}

void iterator(char* value) {
    log_info(logger, value);
}

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


