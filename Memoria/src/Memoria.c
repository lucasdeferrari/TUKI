//Crear un archivo global de funciones comunes
#include "Memoria.h"

char* ip_memoria;
char* puerto_memoria;
int server_fd;

t_config* config;

bool seConectoKernel = 0;
bool seConectoCPU = 0;
bool seConectoFS = 0;

const char* algoritmoAsignacion = string_new();


// memoria es un void* que apunta al inicio del espacio de memoria contiguo del espacio de usuario
// y tamanio es la cantidad de bytes disponibles en ese espacio

typedef struct {
    void* memoria;
    size_t tamanio;
} EspacioUsuario;

EspacioUsuario espacioUsuario;

EspacioUsuario crearEspacioUsuario(size_t tamanio) {
	EspacioUsuario espacio;
    espacio.tamanio = tamanio;
    espacio.memoria = malloc(tamanio);
    return espacio;
}

void liberarEspacioUsuario(EspacioUsuario* espacio) {
    free(espacio->memoria);
    espacio->tamanio = 0;
}

typedef struct {
    int numeroSegmento;
    size_t base;
    size_t desplazamiento;
} Segmento;

int crearSegmento(int numeroSegmento, size_t base, size_t tamanio) {
	if(puedoCrearSegmento(tamanio)) {
	Segmento segmento;
	segmento.numeroSegmento = numeroSegmento;
	segmento.base = base;
	segmento.desplazamiento = tamanio;
    agregarSegmentoAMemoria(segmento);
	return 1;}

	else {
		return -1;
	}
}

typedef struct {
    void* base;
    size_t desplazamiento;
} HuecoLibre;

HuecoLibre crearHuecoLibre(size_t tamanio) {
	HuecoLibre hueco;
	hueco.desplazamiento = tamanio;
	hueco.base = malloc(tamanio);
    return hueco;
}

void iterator(char* value) {
	log_info(logger,"%s", value);
}

//bool sePuedecrearSegmento(size_t tamanio) {
//
//}

HuecoLibre listaDeHuecosLibres[1];

int main(void) {
	int tamanioSeg0, tamanioMemoria;


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

	if (config_has_property(config, "TAM_MEMORIA")) {
			 printf("Existe el valor para el tamaño de la memoria.\n");
			 tamanioMemoria = config_get_int_value(config, "TAM_MEMORIA");
			 }
			 else {
				 log_error(logger, "No existe el valor para el tamaño de la memoria.\n");
				 exit(-1);
			 }

	if (config_has_property(config, "ALGORITMO_ASIGNACION")) {
			 printf("Existe el valor para el algoritmo de asignacion.\n");
			 algoritmoAsignacion = config_get_string_value(config, "ALGORITMO_ASIGNACION");
			 }
			 else {
				 log_error(logger, "No existe el valor para el algoritmo de asignacion.\n");
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

	espacioUsuario = crearEspacioUsuario(tamanioMemoria);

	int resultado = crearSegmento(0,0,tamanioSeg0);

	if(resultado < 0) {
		log_error(logger, "No se pudo crear el segmento.");
		exit(-1);
	}

	t_list * listaDeHuecosLibres = list_create();
	HuecoLibre huecoBase = crearHuecoLibre(tamanioMemoria-tamanioSeg0);
//	huecoBase->base = &
//	list_add(listaDeHuecosLibres, )



	return EXIT_SUCCESS;
}


void informarKernelFaltaDeEspacio() {

}

void agregarSegmentoAMemoria(Segmento unSegmento) {

}

bool puedoCrearSegmento (int tamanio) {
	return true;
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

