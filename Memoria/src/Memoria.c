//Crear un archivo global de funciones comunes
#include "Memoria.h"

char* ip_memoria;
char* puerto_memoria;
int server_fd;
int contadorSegmentos = 0, cantidadSegmentos;

t_config* config;

bool seConectoKernel = 0;
bool seConectoCPU = 0;
bool seConectoFS = 0;

Segmento *segmento0;


// memoria es un void* que apunta al inicio del espacio de memoria contiguo del espacio de usuario
// y tamanio es la cantidad de bytes disponibles en ese espacio


int crear_segmento(int idProceso, int idSegmento, size_t tamanio) {

	// ME FIJO QUE NO HAYA LUGAR PARA CREAR EL SEGMENTO
	if(!hayLugarParaCrearSegmento(tamanio)) {
		return SIN_ESPACIO;
	}

	// ME FIJO QUE HAYA LUGAR, PERO QUE NO ESTE CONTIGUO
	else if(!hayLugarContiguoPara(tamanio)) {
		return PEDIR_COMPACTACION;
	}

	if(!hayTablaSegmentosDe(idProceso)) {
		crearTablaSegmentosDe(idProceso);
	}

	Segmento *segmento= malloc(sizeof(Segmento));
		if(segmento != NULL) {
			segmento->desplazamiento = tamanio;
			segmento->base = buscarLugarParaElSegmento(tamanio);
			segmento->idSegmentoMemoria = asignarIdSegmento();
			segmento->idSegmentoKernel = idSegmento;
			agregarSegmentoATabla(segmento, idProceso);
			list_add(segmentos,segmento);
		}
	base = segmento->base;

	return CREATE_SEGMENT;
}

Segmento *crearSegmento0(size_t tamanio){
	segmento0 = malloc(sizeof(Segmento));
	if(segmento0 != NULL) {
		segmento0->base= 0;
		segmento0->desplazamiento=tamanio;
		segmento0->idSegmentoMemoria=0;
		list_add(segmentos,segmento0);
	}
	return segmento0;
}

bool hayTablaSegmentosDe(int idProceso) {
	t_list_iterator* iterador = list_iterator_create(tablasDeSegmento);

	while(list_iterator_has_next(iterador)) {
		TablaDeSegmentos *siguiente = list_iterator_next(iterador);
		if(idProceso == siguiente->pid) {
			return true;
		}
	}
	return false;
}

int asignarIdSegmento() {
	contadorSegmentos++;
	return contadorSegmentos;
}

size_t buscarLugarParaElSegmento(size_t tamanio) {
	if (string_contains(algoritmoAsignacion, "FIRST")) {
		return buscarPorFirst(tamanio);
	}
	if (string_contains(algoritmoAsignacion, "WORST")) {
		return buscarPorWorst(tamanio);
	}
	if (string_contains(algoritmoAsignacion, "BEST")) {
		return buscarPorBest(tamanio);
	}

}

size_t buscarPorFirst (size_t tamanio) {
	t_list_iterator* iterador = list_iterator_create(listaDeHuecosLibres);

	while(list_iterator_has_next(iterador)) {
		HuecoLibre *siguiente = list_iterator_next(iterador);
		if(siguiente->desplazamiento >= tamanio) {
			actualizarHuecosLibres(siguiente, tamanio);
			return siguiente->base;
		}
	}
}

size_t buscarPorBest(size_t tamanio) {
	t_list_iterator* iterador = list_iterator_create(listaDeHuecosLibres);
	HuecoLibre *elegido;
	elegido->desplazamiento = tamanioMemoria;

	while(list_iterator_has_next(iterador)) {
		HuecoLibre *siguiente = list_iterator_next(iterador);
		if(siguiente->desplazamiento >= tamanio && siguiente->desplazamiento <= elegido->desplazamiento) {
				elegido = siguiente;
		}
	}
	actualizarHuecosLibres(elegido, tamanio);
	return elegido->base;
}

size_t buscarPorWorst(size_t tamanio) {
	t_list_iterator* iterador = list_iterator_create(listaDeHuecosLibres);
		HuecoLibre *elegido;
		elegido->desplazamiento = 0;

		while(list_iterator_has_next(iterador)) {
			HuecoLibre *siguiente = list_iterator_next(iterador);
			if(siguiente->desplazamiento >= tamanio && siguiente->desplazamiento >= elegido->desplazamiento) {
					elegido = siguiente;
			}
		}
		actualizarHuecosLibres(elegido, tamanio);
		return elegido->base;
}

void actualizarHuecosLibres(HuecoLibre *siguiente, size_t tamanio) {
	bool resultado = list_remove_element(listaDeHuecosLibres, siguiente);
	if(!resultado) {
		log_error(logger, "Error al eliminar hueco libre. ");
	}

	HuecoLibre *nuevoHueco = malloc(sizeof(HuecoLibre));
	nuevoHueco->base = siguiente->base + tamanio;
	size_t base = buscarSiguienteLugarOcupado(nuevoHueco->base);
	nuevoHueco->desplazamiento = base - nuevoHueco->base;
	list_add(listaDeHuecosLibres,nuevoHueco);
}

size_t buscarSiguienteLugarOcupado(size_t base) {
	int cantidadTablasSegmentos = list_size(tablasDeSegmento);
	size_t baseMenor = tamanioMemoria;

	for(int i = 0; i<cantidadTablasSegmentos; i++) {
		TablaDeSegmentos *tablaActual = list_get(tablasDeSegmento, i);
		int cantidadSegmentos = list_size(tablaActual->segmentos);
		for(int j= 0; j<cantidadSegmentos; j++) {
			Segmento *segmentoActual = list_get(tablaActual->segmentos, j);
			if(segmentoActual->base < baseMenor && segmentoActual->base > base) {
				baseMenor = segmentoActual->base;
			}
		}
	}
	return baseMenor;
}


TablaDeSegmentos* crearTablaSegmentosDe(int idProceso) {
	TablaDeSegmentos *tablaDeSegmentos = malloc(sizeof(tablaDeSegmentos));
	t_list* segmentos = list_create();
	tablaDeSegmentos->pid = idProceso;
	tablaDeSegmentos->segmentos = segmentos;
	list_add(tablaDeSegmentos->segmentos, segmento0);
	list_add(tablasDeSegmento, tablaDeSegmentos);
	return tablaDeSegmentos;
}

void agregarSegmentoATabla(Segmento *segmento, int idProceso) {
	t_list_iterator* iterador = list_iterator_create(tablasDeSegmento);


	while(list_iterator_has_next(iterador)) {
		TablaDeSegmentos *siguiente = list_iterator_next(iterador);
		if(idProceso == siguiente->pid) {
			list_add(siguiente->segmentos, segmento);
			memcpy(espacioUsuario + segmento->base, segmento, segmento->desplazamiento);
		}
	}
}

void eliminar_segmento(int id_proceso, int id_segmento) {
	t_list_iterator* iterador = list_iterator_create(tablasDeSegmento);

		while(list_iterator_has_next(iterador)) {
			TablaDeSegmentos *siguiente = list_iterator_next(iterador);
			if(id_proceso == siguiente->pid) {
				int largoLista = list_size(siguiente->segmentos);
				int i = 0;
				t_list_iterator* iterador2 = list_iterator_create(siguiente->segmentos);
				while(list_iterator_has_next(iterador2)){
					Segmento *segmento = list_iterator_next(iterador2);
					if(segmento->idSegmentoKernel == id_segmento){
						HuecoLibre *nuevoHueco = malloc(sizeof(HuecoLibre));
						nuevoHueco->base = segmento->base;
						nuevoHueco->desplazamiento = segmento ->desplazamiento;
						list_add(listaDeHuecosLibres,nuevoHueco);
						list_remove_element(siguiente->segmentos, segmento);
						juntarHuecosContiguos(listaDeHuecosLibres);
						list_remove_element(segmentos,segmento);
					}
				}
			}
		}
}
void juntarHuecosContiguos(t_list* listaDeHuecosLibres){
	t_list_iterator* iterador = list_iterator_create(listaDeHuecosLibres);
	while(list_iterator_has_next(iterador)) {
		HuecoLibre *nuevoHueco = list_iterator_next(iterador);
		t_list_iterator* iterador2 = list_iterator_create(listaDeHuecosLibres);
		while(list_iterator_has_next(iterador2)){
			HuecoLibre *nuevoHueco2 = list_iterator_next(iterador2);
			if(nuevoHueco->base + nuevoHueco->desplazamiento == nuevoHueco2->base){
				nuevoHueco2->base = nuevoHueco->base;
				nuevoHueco2->desplazamiento = nuevoHueco->desplazamiento + nuevoHueco2->desplazamiento;
				list_remove_element(listaDeHuecosLibres, nuevoHueco);
			}
		}
	}
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

int main(void) {

	algoritmoAsignacion = string_new();

	listaDeHuecosLibres = list_create();
	tablasDeSegmento = list_create();
	segmentos = list_create();


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

	if (config_has_property(config, "CANT_SEGMENTOS")) {
			 printf("Existe el valor para la cantidad de segmentos.\n");
			 cantidadSegmentos = config_get_int_value(config, "CANT_SEGMENTOS");
			 }
			 else {
				 log_error(logger, "No existe el valor para la cantidad de segmentos.\n");
				 exit(-1);
			 }

	//printf("Tamanio del segmento 0: %i\n" , tamanioSeg0);

//	esperarTodasLasConexiones


	//FALTA AGREGAR seConectoKernel CUANDO HAGAN EL HANDSHAKE AHI
	while(!( seConectoCPU && seConectoFS && seConectoKernel)){
		iniciarHiloServer();
		pthread_join(serverMemoria_thread, NULL);
	}

	log_info(logger, "Se conectaron todos los modulos.\n");
	pthread_detach(client_Kernel);

	// LA FUNCION crearSegmento() DEBE VERIFICAR SI HAY ESPACIO PARA CREAR EL SEGMENTO. EN CASO DE QUE
	// HAYA ESPACIO CREARA EL SEGMENTO Y DEVOLVERA SU DIRECCION BASE




	espacioUsuario = malloc(tamanioMemoria);
	if(espacioUsuario == NULL) {
		log_error(logger, "Error al crear la memoria.\n");
		exit(-1);
	}


	Segmento *segmento0 = crearSegmento0(tamanioSeg0);
	if(segmento0 == NULL) {
		log_error(logger, "Erorr al crear el segmento 0.\n");
		exit(-1);
	}

	memcpy(espacioUsuario, segmento0, segmento0->desplazamiento);

	HuecoLibre *huecoBase = crearHuecoLibre(tamanioMemoria-tamanioSeg0, 0);
	if(huecoBase == NULL) {
		log_error(logger, "No se pudo crear el hueco libre base.\n");
		exit(-1);
	}

	huecoBase->base = segmento0->base + segmento0->desplazamiento;

	list_add(listaDeHuecosLibres, huecoBase);

	log_info(logger, "Se crearon todas las estructuras correctamente.\n");

	while(1){
			iniciarHiloServer();
			pthread_join(serverMemoria_thread, NULL);
		}



	free(espacioUsuario);
	free(segmento0);
	return EXIT_SUCCESS;
}

bool hayLugarParaCrearSegmento(size_t tamanio) {
	t_list_iterator* iterador = list_iterator_create(listaDeHuecosLibres);
	int tamanioLibre = 0;

	while(list_iterator_has_next(iterador)) {
		HuecoLibre *siguiente = list_iterator_next(iterador);
		int desplazamientoSiguiente = siguiente->desplazamiento;
		tamanioLibre += desplazamientoSiguiente;
	}
	return tamanio <= tamanioLibre;
}

bool hayLugarContiguoPara(size_t tamanio) {
	t_list_iterator* iterador = list_iterator_create(listaDeHuecosLibres);
	int max = 0;

	while(list_iterator_has_next(iterador)) {
		HuecoLibre *siguiente = list_iterator_next(iterador);
		int desplazamientoSiguiente = siguiente->desplazamiento;

		if(desplazamientoSiguiente > max) {
			max = desplazamientoSiguiente;
		}
	}
	return tamanio <= max;
}

void compactar_memoria() {
	//un while donde se fija si hay un segmento con base == tamaño+base del hueco libre, si hay,
	// se pasa el segmento a esa base y se borra el hueco libre
	t_list_iterator* iterador = list_iterator_create(segmentos);
	while(list_iterator_has_next(iterador)){
		Segmento *segmento = list_iterator_next(iterador);
		t_list_iterator* iterador2 = list_iterator_create(listaDeHuecosLibres);
		while(list_iterator_has_next(iterador2)){
			HuecoLibre *huecoLibre = list_iterator_next(iterador2);
			Segmento *segmento2 = list_get(segmentos, list_iterator_index(iterator)+1);
			t_list* listaOrdenada = list_sorted(segmentos,comparador(segmento, segmento2));
			Segmento *ultimoSegmento = list_get(listaOrdenada, list_size(listaOrdenada));
			if(segmento->base + segmento->desplazamiento == huecoLibre->base && segmento!=ultimoSegmento){
				Segmento *proximoSegmento = list_get(listaOrdenada, list_iterator_index(iterador)+1);
				proximoSegmento->base = segmento->base + segmento->desplazamiento;
				actualizarHuecosLibres(huecoLibre, proximoSegmento->desplazamiento);
			}
		}
	}
}

bool* comparador(void* elem1, void* elem2) {
		Segmento* seg1 = (Segmento*)elem1;
		Segmento* seg2 = (Segmento*)elem2;
	return seg1->base < seg2->base;
	}

void enviarTodasLasTablas(int cliente_fd){
	t_list_iterator* iterador = list_iterator_create(tablasDeSegmento);
	while(list_iterator_has_next(iterador)){
		TablaDeSegmentos *tablaDeSegmentos = list_iterator_next(iterador);
		t_paquete* paquete = empaquetarTabla(tablaDeSegmentos->pid, tablaDeSegmentos->segmentos, TABLA_GLOBAL);
		enviar_paquete(paquete, cliente_fd);
		eliminar_paquete(paquete);
	}
}

bool segmentoEsElUltimo(Segmento* segmentoAVerificar, t_list* segmentos){
	int tamanioMaximo = 0;
	t_list_iterator* iterador = list_iterator_create(segmentos);
	while(list_iterator_has_next(iterador)){
		Segmento *segmento = list_iterator_next(iterador);
		if(segmento->base + segmento->desplazamiento > tamanioMaximo){
			tamanioMaximo = segmento->base + segmento->desplazamiento;
		}
	}
	return (segmentoAVerificar->base + segmentoAVerificar->desplazamiento) >= tamanioMaximo;

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

void crearYDevolverProceso(int pid, int cliente_fd) {
	if(!hayTablaSegmentosDe(pid)){
		TablaDeSegmentos *tablaDeSegmentos;
		tablaDeSegmentos = crearTablaSegmentosDe(pid);

		//EMPAQUETAR LA TABLA Y ENVIAR A KERNEL
		t_paquete* paquete = empaquetarTabla(tablaDeSegmentos->pid, tablaDeSegmentos->segmentos, TABLA_SEGMENTOS);
		enviar_paquete(paquete, cliente_fd);
		eliminar_paquete(paquete);
	}
	else {
		log_info(logger, "Este proceso ya esta creado.");
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

    		case PROCESO_NUEVO:
    			int pid = recibir_buffer_mio(cliente_fd);
    			crearYDevolverProceso(pid, cliente_fd);
    			log_info(logger, "Creación de Proceso PID: %d", pid);
    			break;

    		// crearSegmento(pid= 1, id= 1, tamanio= 100); EJEMPLO DE LO QUE MANDARIA KERNEL
    		// LO QUE HARIA EL KERNEL SERIA CREAR UN PAQUETE, DONDE CADA RENGLON DEL MISMO
    		// SEA UN PARAMETRO DE ESA FUNCION. ES DECIR, MANDARIA UN PAQUETE CON 3 ELEMENTOS:
    		// PID, ID Y TAMANIO.
    		case CREATE_SEGMENT:
    			lista = recibir_paquete(cliente_fd);
    			t_list_iterator* iterador = list_iterator_create(lista);

    			int arrayPaquete[3] = {};

    			 for (int i = 0; i<3; i++) {
					char* siguiente = list_iterator_next(iterador);
					arrayPaquete[i] = siguiente;
    			    }

    			 pid = arrayPaquete[0];
    			 int idSegmento = arrayPaquete[1];
    			 int tamanio = arrayPaquete[2];

    			 //CALCULAMOS NUESTRO IdSEGMENTO

    			int resultado = crear_segmento(pid, idSegmento,tamanio);
    			iniciarHiloClienteKernel(resultado, cliente_fd);
    			break;

    		case DELETE_SEGMENT:
    			lista = recibir_paquete(cliente_fd);
				t_list_iterator* iterador1 = list_iterator_create(lista);

				int intPaquete[2] = {};

				 for (int i = 0; i<3; i++) {
						int siguiente = list_iterator_next(iterador1);
						intPaquete[i] = siguiente;
				    	}
				int idProceso = intPaquete[0];
				int id_Segmento = intPaquete[1];
    			eliminar_segmento(idProceso, id_Segmento);
    			t_list_iterator* iterador2 = list_iterator_create(tablasDeSegmento);

				while(list_iterator_has_next(iterador2)) {
					TablaDeSegmentos *siguiente = list_iterator_next(iterador2);
					if(idProceso == siguiente->pid) {
						t_paquete* paquete = empaquetarTabla(siguiente->pid, siguiente->segmentos, TABLA_SEGMENTOS);
						enviar_paquete(paquete, cliente_fd);
						eliminar_paquete(paquete);
					}
				}

    			break;
    		case COMPACTAR_MEMORIA:
    			compactar_memoria();
    			enviarTodasLasTablas(cliente_fd);
    			break;
    		case ELIMINAR_PROCESO:
    			eliminar_proceso();

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

void iniciarHiloClienteKernel(int cod_kernel,int cliente_fd) {
	ClientKernelArgs args;
	args.cod_kernel = cod_kernel;
	args.cliente_fd = cliente_fd;

	int err = pthread_create( 	&client_Kernel,	// puntero al thread
								NULL,
								clientKernel, // le paso la def de la función que quiero que ejecute mientras viva
								(void *)&args); // argumentos de la función

	if (err != 0) {
	printf("\nNo se pudo crear el hilo del cliente Kernel de memoria.");
	exit(7);
	}
	//printf("El hilo cliente de la Memoria se creo correctamente.");

}


void* clientKernel(int cod_kernel, int cliente_fd) {
//	MENSAJE --> 0
//	PAQUETE --> 1
//	CREATE_SEGMENT --> 2
//	DELETE_SEGMENT --> 3
//	COMPACTAR_MEMORIA --> 4
//	PROCESO_NUEVO --> 5
//	TABLA_SEGMENTOS --> 6
// 	SIN_ESPACIO --> 7
//	PEDIR_COMPACTACION-->8

	switch(cod_kernel){
		case CREATE_SEGMENT:
			char* baseStr = string_from_format("%zu", base);
			enviar_cod_operacion(baseStr ,cliente_fd, CREATE_SEGMENT);
		break;
		case SIN_ESPACIO:
			enviar_cod_operacion("",cliente_fd, SIN_ESPACIO);
		break;
		case PEDIR_COMPACTACION:
			enviar_cod_operacion("",cliente_fd, PEDIR_COMPACTACION);
		break;
	}
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


char* recibir_buffer_mio(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	return buffer;
}
