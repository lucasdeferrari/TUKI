#include "FileSystem.h"

t_config* config;
t_config* configFCB;
t_config* configFCBT;
t_config* configFCBL;
t_config* configFCBE;
int cliente_fd;

int block_size = 0;
int block_count = 0;
int server_fd;
t_bitarray* bitarray_mapeado;
char* p_fcb;
FILE* archivo_fcb;
void* mapping;
void* mapping2;
size_t tamanio_bloques;
char* textoLeidoMemoria = "";

int main(void) {

	sem_init(&semFileSystemClientMemoriaMoveIn,0,0);
	sem_init(&semFileSystemClientMemoriaMoveOut,0,0);

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
    mapping = mmap(NULL, block_count, PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapping == MAP_FAILED) {
        perror("Error en mmap");
        exit(1);
    }

    //lo agregue por afuera porque lo necesitaba
    bitarray_mapeado = bitarray_create_with_mode(mapping, block_count, LSB_FIRST);

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

   //No se debería liberar al final?
    // Liberar recursos después de su uso
    munmap(mapping, block_count);
    close(fd);
    fclose(archivo_bitmap);


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

	tamanio_bloques = sizeof(struct Bloque) * block_count;

	// Ajustar el tamaño del archivo para que coincida con el tamaño del bitarray
	off_t result2 = lseek(fd2, tamanio_bloques - 1, SEEK_SET);
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

	mapping2 = mmap(NULL, tamanio_bloques, PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0);
	if (mapping2 == MAP_FAILED) {
		log_error(logger, "Error en mmap");
	    close(fd);
	    exit(1);
	 }


//	  for (int i = 0; i < block_count; i++) {
//	        char* block = (char*)mapping2 + (i * block_size);
//	        sprintf(block, "Bloque %d", i + 1);
//	    }

	  // escribir en bloque x
//	  char* bloqueX;
//	  char* dataAEscribir;
//
//	  memcpy(bloqueX, dataAEscribir, block_size);

	  // acceder a bloque x
	  //char* bloqueX = (char*)mapping2 + (INDICE_DEL_BLOQUE * block_size) + desplazamietno;
	  //mapping_archivo_bloques+num_bloque*BLOCK_SIZE+offset
	  //Por ejemplo, con un BLOCK_SIZE=64 si quisiera acceder a la posición (al byte) 200 del archivo,
	  //éste se encontraría en su bloque número 3, offset 8, ya que 3*64 + 8 = 200.

	// Sincronizar los cambios con el archivo en disco
		if (msync(mapping2, tamanio_bloques, MS_SYNC) == -1) {
			perror("Error en msync");
			munmap(mapping2, block_count);
			close(fd);
			exit(1);
		}



	// Realizar el mapeo
//	void* mapping2 = mmap(NULL, tamanio_total, PROT_WRITE, MAP_SHARED, fd2, 0);
//	if (mapping2 == MAP_FAILED) {
//		perror("Error en mmap");
//		exit(1);
//	}
//
//	// CAMBIAR EL PRIMER VALOR DEL ARCHIVO BLOQUES
////		char* mapped_data = (char*) mapping2;
////		mapped_data[0] = 'A';
//
//   // Sincronizar los cambios con el archivo en disco
//   if (msync(mapping2, tamanio_total, MS_SYNC) == -1) {
//	   perror("Error en msync");
//	   exit(1);
//	   }
//
//   // Liberando recursos
//	munmap(mapping2, tamanio_total);
//	close(fd2);
//	fclose(archivo_bloques);

    ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");

    //THREADS CONEXIÓN

    //thread cliente Memoria
    //iniciarHiloCliente();
    enviar_handshake_memoria();

    //thread server
	server_fd = iniciar_servidor();
	log_info(logger, "FileSystem listo para escuchar al cliente\n");


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

    //sem_destroy(&semFileSystemClientMemoria);

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
    cliente_fd = esperar_cliente(server_fd); //LO VUELVO A AGREGAR ACA

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
    			//SI EXISTE LO ABRIMOS Y DEVOLVEMOS OK
    			//SI NO EXISTE, LO CREAMOS, LO ABRIMOS Y DEVOLVEMOS OK
    			printf("DENTRO DE F_OPEN\n");
    			nombreArchivo = recibir_buffer_mio(cliente_fd);
    			printf("Archivo recibido de Kernel: %s\n",nombreArchivo);
    			abrir_archivo(nombreArchivo);
    			enviar_mensaje_cod_operacion("",cliente_fd,F_OPEN);
    			printf("F_OPEN ENVIADO A KERNEL\n");
    			liberar_conexion(cliente_fd);
    			break;

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
    			int punteroArchivoRead = atoi(paqueteRead[1]);
    			int cantBytesRead = atoi(paqueteRead[2]);
    			int direcFisicaRead = atoi(paqueteRead[3]);

    			printf("Archivo recibido de Kernel: %s\n",nombreArchivo);
    			printf("Puntero recibido de Kernel: %d\n",punteroArchivoRead);
    			printf("CantBytes recibida de Kernel: %d\n",cantBytesRead);
    			printf("DirecFisica recibida de Kernel: %d\n",direcFisicaRead);

    			//FUNCIÓN F_READ
    			leerArchivo(nombreArchivo, punteroArchivoRead, cantBytesRead, direcFisicaRead);

    			sem_wait(&semFileSystemClientMemoriaMoveOut);

    			enviar_mensaje_cod_operacion("",cliente_fd,F_READ);
    			printf("F_READ ENVIADO A KERNEL\n");
    			liberar_conexion(cliente_fd);

    			break;
    		case F_WRITE:
    			// ORDEN PARÁMETROS: nombreArchivo - cantBytes - direcFisica
    			lista = recibir_paquete(cliente_fd);
    			t_list_iterator* iteradorWrite = list_iterator_create(lista);

    			char* paqueteWrite[4] = {};

    			for (int i = 0; i<4; i++) {
    				char* siguiente = list_iterator_next(iteradorWrite);
    				paqueteWrite[i] = siguiente;
    			}

    			list_iterator_destroy(iteradorWrite);

    			nombreArchivo= paqueteWrite[0];
    			int punteroArchivoWrite = atoi(paqueteWrite[1]);
    			int cantBytesWrite = atoi(paqueteWrite[2]);
    			int direcFisicaWrite = atoi(paqueteWrite[3]);

    			printf("Archivo recibido de Kernel: %s\n",nombreArchivo);
    			printf("Puntero recibido de Kernel: %d\n",punteroArchivoWrite);
    			printf("CantBytes recibida de Kernel: %d\n",cantBytesWrite);
    			printf("DirecFisica recibida de Kernel: %d\n",direcFisicaWrite);

    			//Le pido a memoria lo que le tengo que escribir
    			iniciarHiloCliente(11, "", direcFisicaWrite, cantBytesWrite);

    			//Semaforos para esperar la respuesta de Memoria
    			sem_wait(&semFileSystemClientMemoriaMoveIn);

    			//FUNCIÓN F_WRITE
    			escribirArchivo(nombreArchivo, punteroArchivoWrite, cantBytesWrite, direcFisicaWrite);

    			enviar_mensaje_cod_operacion("",cliente_fd,F_WRITE);
    			printf("F_WRITE ENVIADO A KERNEL\n");
    			liberar_conexion(cliente_fd);

    			break;
    		case F_TRUNCATE:
    			printf("DENTRO DE F_TRUNCATE\n");
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
    			printf("Nombre del archivo: %s.\n",nombreArchivo);
    			printf("Nuevo tamaño del archivo: %d \n",tamanioArchivo);
    			truncar_archivo(nombreArchivo, tamanioArchivo);
    			enviar_mensaje_cod_operacion("",cliente_fd,F_TRUNCATE);
    			printf("F_TRUNCATE ENVIADO A KERNEL\n");
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


void iniciarHiloCliente(int cod_memoria, char* registro, int direcFisica, int tamanio){
	ClientMemoriaArgs *args = malloc(sizeof(ClientMemoriaArgs));
	args->cod_memoria = cod_memoria;
	args->registro = malloc(strlen(registro)+1);
	strcpy(args->registro,registro);
	args->direccionFisica = direcFisica;
	args->tamanio = tamanio;

	int err = pthread_create( &client_Memoria,	// puntero al thread
	     	        NULL,
	     	    	clientMemoria, // le paso la def de la función que quiero que ejecute mientras viva
					(void *)args); // argumentos de la función

	     	 if (err != 0) {
	     	  printf("\nNo se pudo crear el hilo del cliente Memoria del File System.");
	     	  exit(7);
	     	 }
	     	 printf("El hilo cliente de la Memoria se creo correctamente.");
}

void* clientMemoria(void* arg) {

	ClientMemoriaArgs *args = (ClientMemoriaArgs *)arg;
	int cod_memoria = args->cod_memoria;
	int direcFisica = args->direccionFisica;
	char* registro = args->registro;
	int tamanio = args->tamanio;

    int conexion_Memoria;
    conexion_Memoria = crear_conexion(ip_memoria, puerto_memoria);

    t_paquete* paquete = crear_paquete_cod_operacion(cod_memoria);

    switch(cod_memoria){
    	case 11: //MOV_IN - ORDEN PARAMETROS: (PID, CPU/FS, DIRECCION, TAMAÑO)
        	char* pidMI = string_new();
        	char* fsMI = string_new();
        	char* direcFisicaMI = string_new();
        	char* tamanioMI = string_new();

            string_append_with_format(&pidMI, "%d", 99);
            string_append_with_format(&fsMI, "%s", "FS");
            string_append_with_format(&direcFisicaMI, "%d", direcFisica);
            string_append_with_format(&tamanioMI, "%d", tamanio);

            agregar_a_paquete(paquete, pidMI, strlen(pidMI)+1);
            agregar_a_paquete(paquete, fsMI, strlen(fsMI)+1);
            agregar_a_paquete(paquete, direcFisicaMI, strlen(direcFisicaMI)+1);
            agregar_a_paquete(paquete, tamanioMI, strlen(tamanioMI)+1);

            enviar_paquete(paquete, conexion_Memoria);

            printf("MOV_IN enviado a MEMORIA.\n");
            printf("pid enviado a Memoria: %s\n", pidMI);
            printf("quienSoy enviado a Memoria: %s\n", fsMI);
            printf("direcFisica enviado a Memoria: %s\n", direcFisicaMI);
            printf("tamanio enviado a Memoria: %s\n", tamanioMI);

            eliminar_paquete(paquete);
        break;
        case 12: //MOV_OUT - ORDEN PARAMETROS: (PID, CPU/FS, VALOR_REGISTRO, TAMAÑO, DIRECCION)
        	char* pidMO = string_new();
			char* fsMO = string_new();
			char* valorRegistroMO = string_new();
			char* tamanioMO = string_new();
			char* direcFisicaMO = string_new();


			string_append_with_format(&pidMO, "%d", 99);
			string_append_with_format(&fsMO, "%s", "FS");
			string_append_with_format(&valorRegistroMO, "%s", registro);
			string_append_with_format(&tamanioMO, "%d", tamanio);
			string_append_with_format(&direcFisicaMO, "%d", direcFisica);


			agregar_a_paquete(paquete, pidMO, strlen(pidMO)+1);
			agregar_a_paquete(paquete, fsMO, strlen(fsMO)+1);
			agregar_a_paquete(paquete, valorRegistroMO, strlen(valorRegistroMO)+1);
			agregar_a_paquete(paquete, tamanioMO, strlen(tamanioMO)+1);
			agregar_a_paquete(paquete, direcFisicaMO, strlen(direcFisicaMO)+1);


			enviar_paquete(paquete, conexion_Memoria);

			printf("MOV_OUT enviado a MEMORIA.\n");
			printf("pid: %s\n", pidMO);
			printf("quienSoy: %s\n", fsMO);
			printf("valorRegistro: %s\n", valorRegistroMO);
			printf("direcFisica: %s\n", direcFisicaMO);
			printf("tamanio: %s\n", tamanioMO);

            eliminar_paquete(paquete);
        break;
    	default:
    		log_warning(logger," Operacion desconocida. NO se envió nada a Memoria.\n");
    	break;
    }


    int cod_op = recibir_operacion(conexion_Memoria);

    switch (cod_op) {
    	case 11:
    		textoLeidoMemoria = recibir_handshake(conexion_Memoria);
    		sem_post(&semFileSystemClientMemoriaMoveIn);
    	break;
        case 12:  //RECIBO UN OK
        	char* respuesta = recibir_handshake(conexion_Memoria);
        	printf("Respuesta MOV_OUT: %s\n",respuesta);
        	sem_post(&semFileSystemClientMemoriaMoveOut);
        break;
        default:
        	log_warning(logger,"\nOperacion recibida de MEMORIA desconocida.\n");
    	break;
    }

    liberar_conexion(conexion_Memoria);

	return NULL;
}


/////////////////////////////////////  FUNCIONES ARCHIVOS ////////////////////////////////////////////
char* crearPathArchivoFOpen(char* nombreArchivoOriginal){
	int largo = string_length(nombreArchivoOriginal) - 1;
	char* nombreArchivo = string_substring_until(nombreArchivoOriginal, largo);
	p_fcb = string_new();

	printf("El nombre es: %s\n", nombreArchivo);

	string_append(&p_fcb, "/home/utnso/tp-2023-1c-Los-operadores/FileSystem/fcb/");
	string_append(&p_fcb, nombreArchivo);
	string_append(&p_fcb, ".config");

	return nombreArchivo;
}

void crearPathArchivo(char* nombreArchivo){
	p_fcb = string_new();

	printf("El nombre es: %s\n", nombreArchivo);

	string_append(&p_fcb, "/home/utnso/tp-2023-1c-Los-operadores/FileSystem/fcb/");
	string_append(&p_fcb, nombreArchivo);
	string_append(&p_fcb, ".config");
}

void abrir_archivo(char* nombreArchivoOriginal){
	//Creo archivo FCB para crear config FCB

//	int largo = string_length(nombreArchivoOriginal) - 1;
//	char* nombreArchivo = string_substring_until(nombreArchivoOriginal, largo);
//
//	string_append(&p_fcb, "/home/utnso/tp-2023-1c-Los-operadores/FileSystem/");
//	string_append(&p_fcb, nombreArchivo);
//	string_append(&p_fcb, ".config");

	char* nombreArchivo = crearPathArchivoFOpen(nombreArchivoOriginal);

	printf("El path es: %s\n", p_fcb);

	configFCB = config_create(p_fcb);

	if (configFCB == NULL) {
		printf("No exise el archivo.\n");
		archivo_fcb = fopen(p_fcb, "w");
		if (archivo_fcb) {

			configFCB = config_create(p_fcb);

			config_set_value(configFCB, "NOMBRE_ARCHIVO", nombreArchivo);
			config_set_value(configFCB, "TAMANIO_ARCHIVO", "0");
			config_set_value(configFCB, "PUNTERO_DIRECTO", "");
			config_set_value(configFCB, "PUNTERO_INDIRECTO", "");

			config_save(configFCB);

			char* nomArch = config_get_string_value(configFCB, "NOMBRE_ARCHIVO");

			printf("El nombre del archivo es: %s\n", nomArch);

			printf("El archivo FCB se ha creado exitosamente.\n");

		    fclose(archivo_fcb);
		} else {
			printf("No se pudo crear el archivo FCB.\n");
		}
	}else {
		char* nomArch = config_get_string_value(configFCB, "NOMBRE_ARCHIVO");
		printf("El nombre del archivo es: %s\n", nomArch);
	}
	//config_destroy(configFCB);
}

void truncar_archivo(char* nombreArchivoOriginal, int tamanio){

//	int largo = string_length(nombreArchivoOriginal) - 1;
//	char* nombreArchivo = string_substring_until(nombreArchivoOriginal, largo);
//
//	string_append(&p_fcb, "/home/utnso/tp-2023-1c-Los-operadores/FileSystem/");
//	string_append(&p_fcb, nombreArchivo);
//	string_append(&p_fcb, ".config");

	crearPathArchivo(nombreArchivoOriginal);

	printf("El path es: %s\n", p_fcb);

	configFCBT = config_create(p_fcb);
	printf("Hola\n");

	int tamanioArchivo = 0;
	int punteroIndirecto = 0;
	int punteroDirecto = 0;
	char tam[20];

	if (configFCBT == NULL) {
	        printf("No se pudo crear el config.\n");
	        exit(5);
	 }else{

	    if (config_has_property(configFCBT, "TAMANIO_ARCHIVO")) {
	    	printf("El config se creo bien\n");
	    	tamanioArchivo =  config_get_int_value(configFCBT, "TAMANIO_ARCHIVO");
	    	printf("TamanioArchivo %d\n", tamanioArchivo);
	    	printf("Tamanio %d\n", tamanio);
	    	punteroIndirecto =  config_get_int_value(configFCBT, "PUNTERO_INDIRECTO");
	    	printf("PunteroIndirecto %d\n", punteroIndirecto);
	    	punteroDirecto =  config_get_int_value(configFCBT, "PUNTERO_DIRECTO");
	    	printf("PunteroDirecto %d\n", punteroDirecto);
	    }else {
	    	printf("No existe el path al superbloque.\n");
	    	exit(5);
	    }

	//if (configFCBT != NULL) {

    	snprintf(tam, sizeof(tam), "%d", tamanio);
		config_set_value(configFCB, "TAMANIO_ARCHIVO", tam);

		printf("tamanio %d\n", tamanio);
		printf("block_size %d\n", block_size);

		int cantidadBloquesNecesarios = ceil(tamanio / block_size)+1;

		int cantidadBloquesActual = 0;
		int contador = 0;

		printf("cantidadBloquesActual1 %d\n", cantidadBloquesActual);
		printf("cantidadBloquesNecesarios %d\n", cantidadBloquesNecesarios);

		if (punteroDirecto != 0){
			cantidadBloquesActual++;
		}

		if (punteroIndirecto != 0){
			char* blockChar = (char*)mapping2 + punteroIndirecto*block_size + (contador*4);
			uint32_t block = atoi(blockChar);

			//mapping_archivo_bloques+num_bloque*BLOCK_SIZE+offset

			printf("block %u\n", block);

			bool valor = bitarray_test_bit(bitarray_mapeado, block);
			printf("El valor del bit %u es %i\n", block, valor);

			//Puede que no sea NULL, ver contra que hay que comparar
			while (bitarray_test_bit(bitarray_mapeado, block) != 0){
			//while (block != 0){
			//lo modifique para probar pero el que debería ir es el de arriba
			//while (contador <= 7){
				contador++;
				//MAL
				//uint32_t block = (uint32_t)mapping2 + punteroIndirecto + (contador*4);
				//BIEN
				char* blockChar = (char*)mapping2 + punteroIndirecto*block_size + (contador*4);

				block = atoi(blockChar);

				printf("block %u\n", block);
				bool valor = bitarray_test_bit(bitarray_mapeado, block);
				printf("El valor del bit %u es %i\n", block, valor);
			}
		}

		cantidadBloquesActual += contador;

		printf("cantidadBloquesActual3 %d\n", cantidadBloquesActual);

		if (tamanio < tamanioArchivo){
//				Reducir el tamaño del archivo: Se deberá asignar el nuevo tamaño del archivo en el FCB y
//				se deberán marcar como libres todos los bloques que ya no sean necesarios para direccionar
//				el tamaño del archivo (descartando desde el final del archivo hacia el principio).

			if(cantidadBloquesNecesarios < (cantidadBloquesActual)){

				//liberar bloques del bitmap.
				int i = 0;
				int diferencia = cantidadBloquesActual - cantidadBloquesNecesarios;
				int bloquesALiberar = diferencia-1;

				if (cantidadBloquesNecesarios == 0){
					bitarray_clean_bit(bitarray_mapeado, punteroDirecto);
				}

				if(cantidadBloquesNecesarios <= 1){
					bitarray_clean_bit(bitarray_mapeado, punteroIndirecto);
				}

				while (i < diferencia){
					char* blockALiberarChar = (char*)mapping2 + punteroIndirecto*block_size + (bloquesALiberar*4);
					int blockALiberar = atoi(blockALiberarChar);
					bitarray_clean_bit(bitarray_mapeado, blockALiberar);
					i++;
					bloquesALiberar--;
				}
			}
		} else if (tamanio > tamanioArchivo){
//				Ampliar el tamaño del archivo: Al momento de ampliar el tamaño del archivo deberá actualizar
//				el tamaño del archivo en el FCB y se le deberán asignar tantos bloques como sea necesario para
//				poder direccionar el nuevo tamaño.

			if(cantidadBloquesNecesarios > cantidadBloquesActual){

			//Agregar los bloques que sean necesarios y modificar el bit en el bitarray a 1.
				int diferencia = cantidadBloquesNecesarios - cantidadBloquesActual;

				printf("diferencia %d\n", diferencia);

				int i = 0;
				while (i < diferencia){
					uint32_t bloqueNuevo = 1;
					while(bitarray_test_bit(bitarray_mapeado, bloqueNuevo) != 0){
						bloqueNuevo++;
					}

					printf("BloqueNuevo %u\n", bloqueNuevo);

					if(punteroDirecto == 0){
						punteroDirecto = bloqueNuevo;
						char pd[20];
				    	snprintf(pd, sizeof(pd), "%d", punteroDirecto);
						config_set_value(configFCB, "PUNTERO_DIRECTO", pd);
						bitarray_set_bit(bitarray_mapeado, bloqueNuevo);
						printf("BloqueNuevoPD %u\n", bloqueNuevo);
						while(bitarray_test_bit(bitarray_mapeado, bloqueNuevo) != 0){
							bloqueNuevo++;
						}
						i++;
					}

					if (punteroIndirecto == 0 && cantidadBloquesNecesarios > 1){
						punteroIndirecto = bloqueNuevo;
						char pi[20];
						snprintf(pi, sizeof(pi), "%d", punteroIndirecto);
						config_set_value(configFCB, "PUNTERO_INDIRECTO", pi);
						bitarray_set_bit(bitarray_mapeado, bloqueNuevo);
						printf("BloqueNuevoPI %u\n", bloqueNuevo);
						while(bitarray_test_bit(bitarray_mapeado, bloqueNuevo) != 0){
							bloqueNuevo++;
						}
					}

					if (diferencia > i){
						//ASI SE ESCRIBE
						char* block = (char*)mapping2 + punteroIndirecto*block_size + ((cantidadBloquesActual)*4);
						//memcpy(bloqueX, dataAEscribir, block_size);
						printf("Escribiendo en bloque de archivos\n");
						printf("BloqueNuevoPII %u\n", bloqueNuevo);
						//memcpy(punteroIndirecto, &bloqueNuevo, cantidadBloquesActual*4);
						sprintf(block, "%d", bloqueNuevo);
						bitarray_set_bit(bitarray_mapeado, bloqueNuevo);
						bool valor = bitarray_test_bit(bitarray_mapeado, bloqueNuevo);
						printf("El valor del bit %u es %i\n", bloqueNuevo, valor);
						i++;
						cantidadBloquesActual++;
					}
				}
			}
		}
	}
	config_save(configFCBT);
	printf("Config guardado\n");
}

//FALTA MANDAR A MEMORIA
void leerArchivo(char* nombreArchivoOriginal, int punteroArchivo, int cantBytesRead, int direcFisicaRead) {
//	Esta operación deberá leer la información correspondiente de los bloques a partir del puntero y el
//	tamaño recibidos. Esta información se deberá enviar a la Memoria para ser escrita a partir de la
//	dirección física recibida por parámetro y esperar su finalización para poder confirmar el éxito de
//	la operación al Kernel. MOVE_OUT

//	string_append(&p_fcb, "/home/utnso/tp-2023-1c-Los-operadores/FileSystem/");
//	string_append(&p_fcb, nombreArchivo);
//	string_append(&p_fcb, ".config");
//
	crearPathArchivo(nombreArchivoOriginal);

	configFCBL = config_create(p_fcb);

	int punteroIndirecto = 0;
	int punteroDirecto = 0;

	if (configFCBL == NULL) {
		printf("No se pudo crear el config.\n");
		exit(5);
	}else{

		if (config_has_property(configFCBL, "TAMANIO_ARCHIVO")) {
			printf("El config se creo bien\n");
			punteroIndirecto =  config_get_int_value(configFCBL, "PUNTERO_INDIRECTO");
			printf("PunteroIndirecto %d\n", punteroIndirecto);
			punteroDirecto =  config_get_int_value(configFCBL, "PUNTERO_DIRECTO");
			printf("PunteroDirecto %d\n", punteroDirecto);
		}else {
			printf("No existe el path al superbloque.\n");
			exit(5);
		}

		uint32_t bloqueALeer = floor(punteroArchivo / block_size);


		char porcionLeida[cantBytesRead];
		int	tamanioMenorLeer ;

		if (bloqueALeer == 0){
			int bytesALeer = cantBytesRead;
			int indice = 0;

			for (int i = 0; i < cantBytesRead; i++){
				char* bloqueLecturaString = (char*)mapping2 + punteroDirecto*block_size + punteroArchivo + i;
				//char bloqueLectura = (char)mapping2 + 6*block_size + 0 + i;
				//El de abajo lo use de prueba porque se que en ese bloque a esa altura hay algo escrito con seguridad
				//char* bloqueLecturaString = (char*)mapping2 + 6*block_size + 0 + i;

				printf("String: %s\n", bloqueLecturaString);
				char bloqueLectura = bloqueLecturaString[0];
				porcionLeida[i] = bloqueLectura;
				printf("Char: %c\n", bloqueLectura);
				printf("PorcionLeida: %c.\n", porcionLeida[i]);
				indice++;
				bytesALeer--;
			}

			printf("PorcionLeida: %c.\n", porcionLeida[1]);

			while (bytesALeer != 0){
				int bloque = 0;
				tamanioMenorLeer = minimo(bytesALeer, block_size);

				//uint32_t block = (uint32_t)mapping2 + punteroIndirecto + (bloque*4);
				char* blockChar = (char*)mapping2 + punteroIndirecto*block_size + (bloque*4);
				uint32_t block = atoi(blockChar);

				for (int x = 0; x < tamanioMenorLeer ; x++){
					//char bloqueLectura = (char)mapping2 + block*block_size + x;
					char* bloqueLecturaString = (char*)mapping2 + block*block_size + punteroArchivo + x;
					char bloqueLectura = bloqueLecturaString[0];
					porcionLeida[indice] = bloqueLectura;
					indice++;
					bytesALeer--;
				}

				bloque++;
			}
		}else {
			int bytesALeer = cantBytesRead;
			int indice = 0;
			//uint32_t block = (uint32_t)mapping2 + punteroIndirecto + (bloqueALeer*4);
			char* blockChar = (char*)mapping2 + punteroIndirecto*block_size + (bloqueALeer*4);
			uint32_t block = atoi(blockChar);

			for (int i = 0; i < cantBytesRead; i++){
				char* bloqueLecturaString = (char*)mapping2 + block*block_size + punteroArchivo + i;
				char bloqueLectura = bloqueLecturaString[0];
				porcionLeida[indice] = bloqueLectura;
				indice++;
				bloqueALeer++;
				bytesALeer--;
			}

			while (bytesALeer != 0){

				char* blockChar = (char*)mapping2 + punteroIndirecto*block_size + (bloqueALeer*4);
				uint32_t block = atoi(blockChar);

				tamanioMenorLeer = minimo(bytesALeer, block_size);

				for (int x = 0; x < tamanioMenorLeer ; x++){
					char* bloqueLecturaString = (char*)mapping2 + block*block_size + punteroArchivo + x;
					char bloqueLectura = bloqueLecturaString[0];
					porcionLeida[indice] = bloqueLectura;
					indice++;
					bytesALeer--;
				}

				bloqueALeer++;
			}
		}
		iniciarHiloCliente(12, porcionLeida, direcFisicaRead, cantBytesRead);
	}
	//config_destroy(configFCB);
}

void escribirArchivo(char* nombreArchivoOriginal, int punteroArchivo, int cantBytesWrite, int direcFisicaWrite){
//	Se deberá solicitar a la Memoria la información que se encuentra a partir de la dirección física y
//	escribirlo en los bloques correspondientes del archivo a partir del puntero recibido.
//	El tamaño de la información a leer de la memoria y a escribir en los bloques también deberá recibirse
//	por parámetro desde el Kernel. MOVE_IN

//	string_append(&p_fcb, "/home/utnso/tp-2023-1c-Los-operadores/FileSystem/");
//	string_append(&p_fcb, nombreArchivo);
//	string_append(&p_fcb, ".config");

	crearPathArchivo(nombreArchivoOriginal);

	configFCBE = config_create(p_fcb);

	int punteroIndirecto = 0;
	int punteroDirecto = 0;

	if (configFCBE == NULL) {
		printf("No se pudo crear el config.\n");
		exit(5);
	}else{

		if (config_has_property(configFCBE, "TAMANIO_ARCHIVO")) {
			printf("El config se creo bien\n");
			punteroIndirecto =  config_get_int_value(configFCBE, "PUNTERO_INDIRECTO");
			printf("PunteroIndirecto %d\n", punteroIndirecto);
			punteroDirecto =  config_get_int_value(configFCBE, "PUNTERO_DIRECTO");
			printf("PunteroDirecto %d\n", punteroDirecto);
		}else {
			printf("No existe el path al superbloque.\n");
			exit(5);
		}

		//SOLICITAR A MEMORIA, suponemos que escribe en una variable global llamada textoLeido

		uint32_t bloqueAEscribir = floor(punteroArchivo / block_size);

		int	tamanioMenorEscribir;

		if (bloqueAEscribir == 0){
			int bytesAEscribir = cantBytesWrite;

			tamanioMenorEscribir = minimo(bytesAEscribir, (block_size-punteroArchivo));

			char* porcionAEscribir = string_new();
			porcionAEscribir = string_substring(textoLeidoMemoria, 0, tamanioMenorEscribir);

//			(char*)mapping2 + punteroDirecto + (punteroArchivo*4) = porcionAEscribir;
			//memcpy(punteroDirecto, porcionAEscribir, sizeof(porcionAEscribir));
			char* block = (char*)mapping2 + punteroIndirecto*block_size;
			sprintf(block, "%s", porcionAEscribir);

			bytesAEscribir -= tamanioMenorEscribir;
			int ultimoIndiceEscrito = tamanioMenorEscribir;

			while (bytesAEscribir != 0){
				int bloque = 0;
				int tamanioMenorEscribir2 = minimo(bytesAEscribir, block_size);

				char* blockChar = (char*)mapping2 + punteroIndirecto*block_size + (bloque*4);
				//uint32_t block = atoi(blockChar);

				porcionAEscribir = string_substring(textoLeidoMemoria, ultimoIndiceEscrito, tamanioMenorEscribir2);

//				(char*)mapping2 + block = porcionAEscribir;
				sprintf(blockChar, "%s", porcionAEscribir);

				ultimoIndiceEscrito += tamanioMenorEscribir2;
				bytesAEscribir -= tamanioMenorEscribir2;
				bloque++;
			}
		}else {
			int bytesAEscribir = cantBytesWrite;
			char* blockChar = (char*)mapping2 + punteroIndirecto*block_size + (bloqueAEscribir*4);
			//uint32_t block = atoi(blockChar);

			tamanioMenorEscribir = minimo(bytesAEscribir, block_size-punteroArchivo);

			char* porcionAEscribir = string_new();
			porcionAEscribir = string_substring(textoLeidoMemoria, 0, tamanioMenorEscribir);

//			(char*)mapping2 + block + (punteroArchivo*4) = porcionAEscribir;
			sprintf(blockChar, "%s", porcionAEscribir);

			bloqueAEscribir++;
			bytesAEscribir -= tamanioMenorEscribir;
			int ultimoIndiceEscrito = tamanioMenorEscribir;

			while (bytesAEscribir != 0){
				int tamanioMenorEscribir2 = minimo(bytesAEscribir, block_size);

				porcionAEscribir = string_substring(textoLeidoMemoria, ultimoIndiceEscrito, tamanioMenorEscribir2);

//				(char*)mapping2 + block = porcionAEscribir;
				sprintf(blockChar, "%s", porcionAEscribir);

				ultimoIndiceEscrito += tamanioMenorEscribir2;
				bytesAEscribir -= tamanioMenorEscribir2;
				bloqueAEscribir++;
			}
		}
	}
	//config_destroy(configFCB);
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

void enviar_handshake_memoria(){
	//    int conexion_Memoria;
	//    conexion_Memoria = crear_conexion(ip_memoria, puerto_memoria);
	//    enviar_mensaje("filesystem",conexion_Memoria);
	//    int cod_op = recibir_operacion(conexion_Memoria);
	//    printf("codigo de operacion: %i\n", cod_op);
	//    recibir_mensaje(conexion_Memoria);
	//    liberar_conexion(conexion_Memoria);

	int config = 1;
	int conexion_Memoria;

	conexion_Memoria = crear_conexion(ip_memoria, puerto_memoria);
	enviar_mensaje("filesystem",conexion_Memoria);
	int cod_op = recibir_operacion(conexion_Memoria);
	recibir_mensaje(conexion_Memoria);
	liberar_conexion(conexion_Memoria);
}

int minimo(int a, int b){
	if (a < b) {
		return a;
	}else {
		return b;
	}
}
