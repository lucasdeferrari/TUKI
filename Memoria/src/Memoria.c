//Crear un archivo global de funciones comunes
#include "Memoria.h"

char* ip_memoria;
char* puerto_memoria;
int server_fd;

t_config* config;

bool seConectoKernel = 0;
bool seConectoCPU = 0;
bool seConectoFS = 0;


// memoria es un void* que apunta al inicio del espacio de memoria contiguo del espacio de usuario
// y tamanio es la cantidad de bytes disponibles en ese espacio

typedef struct {
    void* memoria;
    size_t tamanio;
} EspacioUsuario;

EspacioUsuario crearEspacioMemoria(size_t tamanio) {
	EspacioUsuario espacio;
    espacio.tamanio = tamanio;
    espacio.memoria = malloc(tamanio);
    return espacio;
}

void liberarEspacioMemoria(EspacioUsuario* espacio) {
    free(espacio->memoria);
    espacio->tamanio = 0;
}

void iterator(char* value) {
	log_info(logger,"%s", value);
}

int main(void) {
	int tamanioSeg0;


	logger = log_create("memoria.log", "Memoria", 1, LOG_LEVEL_DEBUG);
	server_fd = iniciar_servidor();
	log_info(logger, "Memoria lista para recibir al cliente\n");

	config = config_create("/home/utnso/tp-2023-1c-Los-operadores/Memoria/memoria.config");

	    if (config == NULL) {
	        printf("No se pudo crear el config.\n");
	        log_error(logger, "No se pudo crear el config.\n");
	        exit(-1);
	    }

	if (config_has_property(config, "TAM_SEGMENTO_0")) {
	    	 printf("Existe el valor para el tamaño del segmento 0.\n");
	    	 tamanioSeg0 = config_get_int_value(config, "TAM_SEGMENTO_0");
	    	 }
	    	 else {
	    		 log_error(logger, "No existe el valor para el tamaño del segmento 0.\n");
	    		 exit(-1);
	    	 }

	//printf("Tamanio del segmento 0: %i\n" , tamanioSeg0);

//	esperarTodasLasConexiones


	//FALTA AGREGAR seConectoKernel CUANDO HAGAN EL HANDSHAKE AHI
	while(!( seConectoCPU && seConectoFS)){
		iniciarHiloServer();
		pthread_join(serverMemoria_thread, NULL);
	}

	log_info(logger, "Se conectaron todos los modulos.");


	// LA FUNCION crearSegmento() DEBE VERIFICAR SI HAY ESPACIO PARA CREAR EL SEGMENTO. EN CASO DE QUE
	// HAYA ESPACIO CREARA EL SEGMENTO Y DEVOLVERA SU DIRECCION BASE
	int segmentoCero = crearSegmento(tamanioSeg0);

	if (segmentoCero == -1) {
		// ESA FUNCION LE INFORMA AL KERNEL QUE NO HAY ESPACIO LIBRE PARA CREAR EL SEGMENTO
		informarKernelFaltaDeEspacio();
		log_error(logger, "Error al crear el segmento.");
		exit(-1);
	}









	return EXIT_SUCCESS;
}

int crearSegmento(int tamanio) {
	return 1;
}

void informarKernelFaltaDeEspacio() {

}

void iniciarHiloServer() {
    int err = pthread_create( &serverMemoria_thread,	// puntero al thread
    	            NULL,
    	        	&serverMemoria, // le paso la def de la función que quiero que ejecute mientras viva
    				NULL); // argumentos de la función

    	     if (err != 0) {
    	      printf("\nNo se pudo crear el hilo de la conexión.\n");
    	      exit(7);
    	     }
}

void* serverMemoria(void* ptr){

	//sem_wait(&semKernelClientFileSystem);

	//int server_fd = iniciar_servidor();
    int cliente_fd = esperar_cliente(server_fd);


//    !(seConectoKernel && seConectoCPU && seConectoFS)

    t_list* lista;
    while (1) {
    	int cod_op = recibir_operacion(cliente_fd);
    	switch (cod_op) {
    		case MENSAJE:

    			char* handshake = recibir_handshake(cliente_fd);

    			if (strcmp(handshake, "kernel") == 0) {
    				log_info(logger, "se conecto el kernel");
    				seConectoKernel = 1;

    			}
    			if (strcmp(handshake, "CPU") == 0) {
    				log_info(logger, "se conecto la cpu");
    				seConectoCPU = 1;

    			}
    			if (strcmp(handshake, "filesystem") == 0) {
    				log_info(logger, "se conecto el filesystem");
    				seConectoFS = 1;
    			}
    			//free(handshake);
    			break;
    		case PAQUETE:
    			lista = recibir_paquete(cliente_fd);
    			log_info(logger, "Me llegaron los siguientes valores:");
    			list_iterate(lista, (void*) iterator);
    			enviar_respuesta(cliente_fd, handshake);
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

void enviar_respuesta(int socket_cliente, char* quien_es) {
	char* handshake = quien_es;
	char* respuesta = string_new();
	printf("Socket: %i\n", socket_cliente);
	printf("Me conecte con: %s\n", handshake);


		if (strcmp(handshake, "kernel") == 0) {
			respuesta = "Hola kernel, gracias por comunicarte con la memoria!";
			enviar_mensaje(respuesta, socket_cliente);

		}
		if (strcmp(handshake, "CPU") == 0) {
			printf("matchee con cpu");
			respuesta = "Hola cpu, gracias por comunicarte con la memoria!";
			enviar_mensaje(respuesta, socket_cliente);

		}
		if (strcmp(handshake, "filesystem") == 0) {
			respuesta = "Hola fs, gracias por comunicarte con la memoria!";
			enviar_mensaje(respuesta, socket_cliente);
		}
		free(handshake);
}

char* recibir_handshake(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	return buffer;
}

