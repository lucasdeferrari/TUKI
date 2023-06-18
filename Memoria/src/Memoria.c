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


int crear_segmento(int idProceso, int idSegmento, size_t tamanio) {
	return 2;
}

void eliminar_segmento(Segmento *segmento) {

}

//typedef struct {
//    void* memoria;
//    size_t tamanio;
//    t_list* segmentos;
//} EspacioUsuario;
//
//EspacioUsuario espacioUsuario;
//
//EspacioUsuario crearEspacioUsuario(size_t tamanio, int tamanioSeg0) {
//	EspacioUsuario espacio;
//    espacio.tamanio = tamanio;
//    espacio.memoria = malloc(tamanio);
//    espacio.segmentos = list_create();
//
//
//    Segmento segmento0 = crearSegmento0(tamanioSeg0);
//    list_add(espacio.segmentos, segmento0);
//    return espacio;
//}

//void liberarEspacioUsuario(EspacioUsuario* espacio) {
//    free(espacio->memoria);
//    espacio->tamanio = 0;
//}



typedef struct {
    size_t base;
    size_t desplazamiento;
} HuecoLibre;

HuecoLibre* crearHuecoLibre(size_t tamanio, size_t base) {
	HuecoLibre *hueco = malloc(sizeof(HuecoLibre));
	if(hueco != NULL) {
		hueco->desplazamiento = tamanio;
		hueco->base = base;
	}
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
	char* algoritmoAsignacion = string_new();


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
			 const
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




	void* espacioUsuario = malloc(tamanioMemoria);
	if(espacioUsuario == NULL) {
		log_error(logger, "Error al crear la memoria.");
		exit(-1);
	}


	Segmento *segmento0 = crearSegmento0(tamanioSeg0);
	if(segmento0 == NULL) {
		log_error(logger, "Erorr al crear el segmento 0.");
		exit(-1);
	}

	memcpy(espacioUsuario, segmento0, sizeof(Segmento));

	t_list * listaDeHuecosLibres = list_create();
	HuecoLibre *huecoBase = crearHuecoLibre(tamanioMemoria-tamanioSeg0, 0);
	if(huecoBase == NULL) {
		log_error(logger, "No se pudo crear el hueco libre base.");
		exit(-1);
	}

	huecoBase->base = segmento0->base + segmento0->desplazamiento;

	list_add(listaDeHuecosLibres, huecoBase);

	while(1){
			iniciarHiloServer();
			pthread_join(serverMemoria_thread, NULL);
		}



	free(espacioUsuario);
	free(segmento0);
	return EXIT_SUCCESS;
}

void compactar_memoria() {
	//un while donde se fija si hay un segmento con base == tamaño+base del hueco libre, si hay,
	// se pasa el segmento a esa base y se borra el hueco libre
}

bool puedoCrearSegmentoEnProceso (size_t tamanio, int idProceso) {
	return true;
}

bool puedoCrearSegmentoPorTamanio(int tamanio) {
	return true;
}

Segmento *crearSegmento0(size_t tamanio){
	Segmento *segmento0 = malloc(sizeof(Segmento));
	if(segmento0 != NULL) {
		segmento0->base= 0;
		segmento0->desplazamiento=tamanio;
		segmento0->idSegmento=0;
	}
	return segmento0;
}

void agregarSegmentoATabla(int idProceso, Segmento segmento) {

}

void asignarPorFirstFit() {

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

    			char* handshake = recibir_buffer_mio(cliente_fd);

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


    		//crearSegmento(pid= 1, id= 1, tamanio= 100); EJEMPLO DE LO QUE MANDARIA KERNEL
    		// LO QUE HARIA EL KERNEL SERIA CREAR UN PAQUETE, DONDE CADA RENGLON DEL MISMO
    		// SEA UN PARAMETRO DE ESA FUNCION. ES DECIR, MANDARIA UN PAQUETE CON 3 ELEMENTOS:
    		// PID, ID Y TAMANIO.
    		case CREAR_SEGMENTO:
    			lista = recibir_paquete(cliente_fd);
    			t_list_iterator* iterador = list_iterator_create(lista);

    			int arrayPaquete[3] = {};

    			 for (int i = 0; i<3; i++) {
					char* siguiente = list_iterator_next(iterador);
					arrayPaquete[i] = siguiente;
    			    }

    			 int pid = arrayPaquete[0];
    			 int idSegmento = arrayPaquete[1];
    			 int tamanio = arrayPaquete[2];

    			int resultado = crear_segmento(pid, idSegmento,tamanio);
    			enviar_respuesta_crearSegmento(cliente_fd, resultado);
    			break;

    		case ELIMINAR_SEGMENTO:
    			char* segmento = string_new();
    			segmento = recibir_buffer_mio(cliente_fd);
    			eliminar_segmento(segmento);
    			break;

    		case COMPACTAR_MEMORIA:
    			compactar_memoria();
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

void enviar_respuesta_crearSegmento(int socket_cliente, int resultado) {
	char* respuesta = string_new();

	// IDEA BASICA, NO ESTA BIEN PORQUE NO SE MANDARIAN STRINGS, SINO OTRO TIPO DE DATO
	// QUE PUEDA SER INTERPRETADO POR EL KERNEL, PERO NO ESTOY DEL TODO SEGURO DE QUE SERIA

	if (resultado == -1) {
		respuesta = "No hay espacio libre para crear el segmento.";
	}

	else if (resultado == 0) {
		respuesta = "Se debe solicitar una compactacion previa a crear el segmento.";
	}

	else {
		char* resultadoString = string_itoa(resultado);
		respuesta= "La direccion base del segmento es: ";
		string_append(respuesta, resultadoString);
	}

	enviar_mensaje(respuesta,socket_cliente);
}

char* recibir_buffer_mio(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	return buffer;
}
