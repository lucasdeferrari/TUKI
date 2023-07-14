//Crear un archivo global de funciones comunes
#include "Memoria.h"
////////////////////////////////////////////////VARIABLES GLOBALES////////////////////////////////////////////////
char* ip_memoria;
char* puerto_memoria;
int server_fd;
int contadorSegmentos = 0, cantidadSegmentos;

int estaConectadoFS = 0;

t_config* config;

bool seConectoKernel = 0;
bool seConectoCPU = 0;
bool seConectoFS = 0;

Segmento *segmento0;

void iterator(char* value) {
	log_info(logger,"%s", value);
}

int main(int argc, char *argv[]) {
	if(argc == 2){
	char* pathConfig = string_new();
	pathConfig = string_duplicate(argv[1]);

	char* PUERTO = config_get_string_value(config, "PUERTO_ESCUCHA");

	algoritmoAsignacion = string_new();
	listaDeHuecosLibres = list_create();
	tablasDeSegmento = list_create();
	segmentos = list_create();

	logger = log_create("memoria.log", "Memoria", 1, LOG_LEVEL_DEBUG);
	config = config_create(pathConfig);
	server_fd = iniciar_servidor(PUERTO);
	//log_info(logger, "Memoria lista para recibir al cliente\n");



	    if (config == NULL) {
	        log_error(logger, "No se pudo crear el config.\n");
	        exit(-1);
	    }

	if (config_has_property(config, "TAM_SEGMENTO_0")) {
	    	 tamanioSeg0 = config_get_int_value(config, "TAM_SEGMENTO_0");
	    	 }
	    	 else {
	    		 log_error(logger, "No existe el valor para el tamaño del segmento 0.\n");
	    		 exit(-1);
	    	 }

	if (config_has_property(config, "TAM_MEMORIA")) {
			 tamanioMemoria = config_get_int_value(config, "TAM_MEMORIA");
			 }
			 else {
				 log_error(logger, "No existe el valor para el tamaño de la memoria.\n");
				 exit(-1);
			 }

	if (config_has_property(config, "ALGORITMO_ASIGNACION")) {
			 algoritmoAsignacion = config_get_string_value(config, "ALGORITMO_ASIGNACION");
			 }
			 else {
				 log_error(logger, "No existe el valor para el algoritmo de asignacion.\n");
				 exit(-1);
			 }

	if (config_has_property(config, "CANT_SEGMENTOS")) {
			 cantidadSegmentos = config_get_int_value(config, "CANT_SEGMENTOS");
			 }
			 else {
				 log_error(logger, "No existe el valor para la cantidad de segmentos.\n");
				 exit(-1);
			 }

	if (config_has_property(config, "RETARDO_MEMORIA")) {
			 retardoMemoria = config_get_int_value(config, "RETARDO_MEMORIA");
			 }
			 else {
				 log_error(logger, "No existe el valor para el retardo memoria.\n");
				 exit(-1);
			 }

	if (config_has_property(config, "RETARDO_COMPACTACION")) {
			 retardoCompactacion = config_get_int_value(config, "RETARDO_COMPACTACION");
			 }
			 else {
				 log_error(logger, "No existe el valor para el retardo compactacion.\n");
				 exit(-1);
			 }

	while(!( seConectoCPU && seConectoFS && seConectoKernel)){
				iniciarHiloServer();
				pthread_join(serverMemoria_thread, NULL);
			}
//	while(!( seConectoCPU && seConectoKernel)){
//				iniciarHiloServer();
//				pthread_join(serverMemoria_thread, NULL);
//			}

	log_info(logger, "Se conectaron todos los modulos.\n");
	pthread_detach(client_Kernel);

	// memoria es un void* que apunta al inicio del espacio de memoria contiguo del espacio de usuario
	// y tamanio es la cantidad de bytes disponibles en ese espacio
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

	memcpy(espacioUsuario, segmento0, sizeof(Segmento));

	HuecoLibre *huecoBase = crearHuecoLibre(tamanioMemoria-tamanioSeg0, segmento0->desplazamiento);
	if(huecoBase == NULL) {
		log_error(logger, "No se pudo crear el hueco libre base.\n");
		exit(-1);
	}

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
}

/////////////////////////////////////////CREAR SEGMENTO 0 Y ASIGNACION IDS/////////////////////////////////////////
Segmento *crearSegmento0(size_t tamanio){
	segmento0 = malloc(sizeof(Segmento));
	if(segmento0 != NULL) {
		segmento0->base= 0;
		segmento0->desplazamiento=tamanio;
		segmento0->idSegmentoMemoria=0;
		segmento0->idSegmentoKernel=0;
		list_add(segmentos,segmento0);
	}
	return segmento0;
}

int asignarIdSegmento() {
	contadorSegmentos++;
	return contadorSegmentos;
}

////////////////////////////////////////FUNCIONES PARA CREAR HUECO LIBRE/////////////////////////////////////////

HuecoLibre* crearHuecoLibre(size_t tamanio, size_t base) {
	HuecoLibre *hueco = malloc(sizeof(HuecoLibre));
	if(hueco != NULL) {
		hueco->desplazamiento = tamanio;
		hueco->base = base;
		//printf("HUECO BASE DESPLAZAMIENTO: %zu\n", hueco->desplazamiento);
	}
	return hueco;
}

/////////////////////////////////////////////////SERVER MEMORIA//////////////////////////////////////////////////

void iniciarHiloServer() {
    int err = pthread_create( &serverMemoria_thread,	// puntero al thread
    	            NULL,
    	        	&serverMemoria, // le paso la def de la función que quiero que ejecute mientras viva
    				NULL); // argumentos de la función
    	     if (err != 0) {
    	      //printf("\nNo se pudo crear el hilo de la conexión.\n");
    	      exit(7);
    	     }
}

void* serverMemoria(void* ptr){

    int cliente_fd = esperar_cliente(server_fd);

//    !(seConectoKernel && seConectoCPU && seConectoFS)

    t_list* lista;
    while (1) {
    	int cod_op = recibir_operacion(cliente_fd);
    		if(cod_op == MENSAJE) {

    			char* handshake = recibir_buffer_mio(cliente_fd);

    			if (strcmp(handshake, "kernel") == 0) {
    				//log_info(logger, "se conecto el kernel");
    				seConectoKernel = 1;
    				enviar_respuesta(cliente_fd, handshake);
    			}

    			if (strcmp(handshake, "CPU") == 0) {
    				//log_info(logger, "se conecto la cpu");
    				seConectoCPU = 1;
    				enviar_respuesta(cliente_fd, handshake);
    			}

    			if (strcmp(handshake, "filesystem") == 0) {
    				//log_info(logger, "se conecto el filesystem");
    				seConectoFS = 1;
    				enviar_respuesta(cliente_fd, handshake);
    			}
    		}
    		else if (cod_op == PAQUETE) {
    			char* handshake = recibir_buffer_mio(cliente_fd);

    			lista = recibir_paquete(cliente_fd);
    			//log_info(logger, "Me llegaron los siguientes valores:");
    			list_iterate(lista, (void*) iterator);
    			enviar_respuesta(cliente_fd, handshake);
    		}

    		else if(cod_op == PROCESO_NUEVO) {
    			char* pid = recibir_buffer_mio(cliente_fd);
    			int pidInt = atoi(pid);
    			crearYDevolverProceso(pidInt, cliente_fd);
    		}

    		// crearSegmento(pid= 1, id= 1, tamanio= 100); EJEMPLO DE LO QUE MANDARIA KERNEL
    		// LO QUE HARIA EL KERNEL SERIA CREAR UN PAQUETE, DONDE CADA RENGLON DEL MISMO
    		// SEA UN PARAMETRO DE ESA FUNCION. ES DECIR, MANDARIA UN PAQUETE CON 3 ELEMENTOS:
    		// PID, ID Y TAMANIO.
    		else if(cod_op == CREATE_SEGMENT) {
    			//printf("Instruccion recibida de Kernel: CREATE_SEGMENT.\n");
    			lista = recibir_paquete(cliente_fd);
    			t_list_iterator* iterador = list_iterator_create(lista);

    			int arrayPaquete[3] = {};

    			 for (int i = 0; i<3; i++) {
					char* siguiente = list_iterator_next(iterador);
					int siguienteInt = atoi(siguiente);

					arrayPaquete[i] = siguienteInt;
    			    }

    			 list_iterator_destroy(iterador);

    			 int pidInt = arrayPaquete[0];
    			 //printf("PID recibido de Kernel: %d\n",pidInt);
    			 int idSegmento = arrayPaquete[1];
    			// printf("IdSegmento recibido de Kernel: %d\n",idSegmento);
    			 int tamanio = arrayPaquete[2];
    			// printf("Tamaño recibido de Kernel: %d\n",tamanio);

    			 //CALCULAMOS NUESTRO IdSEGMENTO

    			int resultado = crear_segmento(pidInt, idSegmento,tamanio);

    			iniciarHiloClienteKernel(resultado, cliente_fd);
    		}

    		else if(cod_op == DELETE_SEGMENT) {
    			lista = recibir_paquete(cliente_fd);
				t_list_iterator* iterador1 = list_iterator_create(lista);

				int intPaquete[2] = {};

				 for (int i = 0; i<2; i++) {
						char* siguiente = list_iterator_next(iterador1);
						int siguienteInt = atoi(siguiente);
						intPaquete[i] = siguienteInt;
				    	}
				 list_iterator_destroy(iterador1);

				int idProceso = intPaquete[0];
				int id_Segmento = intPaquete[1];
    			eliminar_segmento(idProceso, id_Segmento);

    			t_list_iterator* iteradorTablas = list_iterator_create(tablasDeSegmento);
				while(list_iterator_has_next(iteradorTablas)) {
					TablaDeSegmentos *siguienteTabla = malloc(sizeof(TablaDeSegmentos));
					siguienteTabla = list_iterator_next(iteradorTablas);
					if(idProceso == siguienteTabla->pid) {
						t_paquete* paquete = empaquetarTabla(siguienteTabla->pid, siguienteTabla->segmentos, DELETE_SEGMENT);
						enviar_paquete(paquete, cliente_fd);
						eliminar_paquete(paquete);
					}
				}
				list_iterator_destroy(iteradorTablas);
    		}

    		else if(cod_op == COMPACTAR_MEMORIA) {
    			log_info(logger, "Solicitud de Compactación\n");
    			compactar_memoria();
    			sleep_ms(retardoCompactacion);
    			enviarTodasLasTablas(cliente_fd);
    			liberar_conexion(cliente_fd);
    		}

    		else if(cod_op == ELIMINAR_PROCESO) {
    			char* pid2 = recibir_buffer_mio(cliente_fd);
    			int pid2Int = atoi(pid2);

    			eliminar_proceso(pid2Int);
    			log_info(logger, "Eliminación de Proceso PID: %d\n", pid2Int);
    		}

    		else if(cod_op == MOV_IN) {
    			estaConectadoFS = 1;
    			lista = recibir_paquete(cliente_fd);
				t_list_iterator* iterador_mov_in = list_iterator_create(lista);

				// ORDEN PARAMETROS: (PID, CPU/FS, DIRECCION, TAMAÑO)
				char* paquete[4] = {};

				 for (int i = 0; i<4; i++) {
						char* siguiente = list_iterator_next(iterador_mov_in);
						paquete[i] = siguiente;
						}
				 list_iterator_destroy(iterador_mov_in);

				char* pid = paquete[0];
				char* quienMeHabla = paquete[1];
				char* direccionFisicaStr = paquete[2];
				char* tamanioStr = paquete[3];

				int tamanio = atoi(tamanioStr);
				//printf("tamanio: %d\n", tamanio);
    			int direccionFisica = atoi(direccionFisicaStr);

    			char destinoArray[tamanio];
    			strcpy(destinoArray, "");
    			for(int i =0; i < tamanio; i++) {
    				memcpy(&destinoArray[i],  espacioUsuario + direccionFisica + i, sizeof(char));
				}
    			printf("VALOR A ENVIAR: %s\n",destinoArray);
    			destinoArray[tamanio] = '\0';
    			printf("VALOR ENVIADO: %s\n",destinoArray);
    			//memcpy(destinoArray[0], espacioUsuario + direccionFisica, strlen(espacioUsuario + direccionFisica) + 1);
    			//printf("destino array:%s\n ", destinoArray[0]);
    			log_info(logger, "PID: %s - Acción: LEER - Dirección física: %i - Tamaño: %i - Origen: %s\n", pid, direccionFisica, tamanio, quienMeHabla);

    			sleep_ms(retardoMemoria);
    			enviar_mensaje_cod_operacion(&destinoArray,cliente_fd,MOV_IN);
    			estaConectadoFS = 0;
    		}


    		// ORDEN PARAMETROS: (PID, CPU/FS, VALOR_REGISTRO, TAMAÑO, DIRECCION)
    		else if(cod_op == MOV_OUT) {
    			//printf("Dentro de mov_out\n");
    			estaConectadoFS = 1;
    			lista = recibir_paquete(cliente_fd);
				t_list_iterator* iterador_mov_out = list_iterator_create(lista);

				char* paqueteDireccion[5] = {};

				 for (int i = 0; i<5; i++) {
						char* siguiente = list_iterator_next(iterador_mov_out);
						paqueteDireccion[i] = siguiente;
						}
				 list_iterator_destroy(iterador_mov_out);

				 char* pid = paqueteDireccion[0];
				 char* quienMeHabla = paqueteDireccion[1];
				 int tamanio = atoi(paqueteDireccion[3]);
				 char valorRegistro[tamanio];
				 strcpy(valorRegistro, paqueteDireccion[2]);
				 int direccionFisicaRecibida = atoi(paqueteDireccion[4]);

				 for(int i =0; i<= tamanio; i++) {
					 memcpy(espacioUsuario + direccionFisicaRecibida + i,  &valorRegistro[i], sizeof(valorRegistro[0]));
				     }
				printf("VALOR RECIBIDO: %s\n",valorRegistro);
				log_info(logger, "PID: %s - Acción: ESCRIBIR - Dirección física: %i - Tamaño: %i - Origen: %s\n", pid, direccionFisicaRecibida, tamanio, quienMeHabla);
				sleep_ms(retardoMemoria);
				//printf("ENVIO MOV OUT");
				enviar_mensaje_cod_operacion("OK",cliente_fd,MOV_OUT);

				estaConectadoFS = 0;

    		}
    		else if(cod_op == DESOCUPADO){
    			//printf("ENTRE A DESOCUPADO\n");
    			char* estaConectadoFSstr = string_itoa(estaConectadoFS);
    			enviar_cod_operacion(estaConectadoFSstr, cliente_fd, DESOCUPADO);
    			liberar_conexion(cliente_fd);
    		}
    		else if(cod_op == -1) {
    			liberar_conexion(cliente_fd);
    			//log_error(logger, "\nel cliente se desconecto. Terminando servidor");
    			return EXIT_FAILURE;
    		}

    		else {
    			log_warning(logger,"\nOperacion desconocida\n");
    		}

    }

	return NULL;
}

/////////////////////////////////////////////////RECIBIR BUFFER//////////////////////////////////////////////////
char* recibir_buffer_mio(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	return buffer;
}

/////////////////////////////////////////////////ENVIAR REPUESTA//////////////////////////////////////////////////
void enviar_respuesta(int socket_cliente, char* handshake) {
	char* respuesta = string_new();
	//printf("Socket: %i\n", socket_cliente);
	//printf("Me conecte con: %s\n", handshake);

		if (strcmp(handshake, "kernel") == 0) {
			respuesta = "Hola kernel, gracias por comunicarte con la memoria!";
			enviar_mensaje(respuesta, socket_cliente);
		}

		if (strcmp(handshake, "CPU") == 0) {
			respuesta = "Hola cpu, gracias por comunicarte con la memoria!";
			enviar_mensaje(respuesta, socket_cliente);
		}

		if (strcmp(handshake, "filesystem") == 0) {
			respuesta = "Hola fs, gracias por comunicarte con la memoria!";
			enviar_mensaje(respuesta, socket_cliente);
		}
		free(handshake);
}

///////////////////////////////////////////////CREAR PROCESO NUEVO////////////////////////////////////////////////
void crearYDevolverProceso(int pid, int cliente_fd) {
	if(hayTablaSegmentosDe(pid) == 0){
		TablaDeSegmentos *tablaDeSegmentos;
		tablaDeSegmentos = crearTablaSegmentosDe(pid);

		//EMPAQUETAR LA TABLA Y ENVIAR A KERNEL
		log_info(logger, "Creación de Proceso PID: %d\n", pid);
		t_paquete* paquete = empaquetarTabla(tablaDeSegmentos->pid, tablaDeSegmentos->segmentos, TABLA_SEGMENTOS);
		enviar_paquete(paquete, cliente_fd);
		eliminar_paquete(paquete);
	}
}

/////////////////////////////////////////////////CREAR SEGMENTO//////////////////////////////////////////////////
// LA FUNCION crearSegmento() DEBE VERIFICAR SI HAY ESPACIO PARA CREAR EL SEGMENTO. EN CASO DE QUE
// HAYA ESPACIO CREARA EL SEGMENTO Y DEVOLVERA SU DIRECCION BASE
int crear_segmento(int idProceso, int idSegmento, size_t tamanio) {
	// ME FIJO QUE NO HAYA LUGAR PARA CREAR EL SEGMENTO
	if(hayLugarParaCrearSegmento(tamanio) == 0) {
		//printf("SIN_ESPACIO.\n");
		return SIN_ESPACIO;
	}

	// ME FIJO QUE HAYA LUGAR, PERO QUE NO ESTE CONTIGUO
	else if(hayLugarContiguoPara(tamanio) == 0) {
		//printf("PEDIR_COMPACTACION.\n");
		return PEDIR_COMPACTACION;
	}

	if(hayTablaSegmentosDe(idProceso) == 0) {
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
	log_info(logger, "PID: %d - Crear Segmento: %d - Base: %zu - TAMAÑO: %zu\n", idProceso, idSegmento, segmento->base, tamanio);
	//printf("CREATE_SEGMENT.\n");
	return CREATE_SEGMENT;
}

/////////////////////////////////////////FUNCIONES QUE USA CREAR SEGMENTO/////////////////////////////////////////
int hayLugarParaCrearSegmento(size_t tamanio) {
	t_list_iterator* iterador = list_iterator_create(listaDeHuecosLibres);
	int tamanioLibre = 0;

	while(list_iterator_has_next(iterador)) {
		HuecoLibre *siguiente = list_iterator_next(iterador);
		int desplazamientoSiguiente = siguiente->desplazamiento;
		tamanioLibre += desplazamientoSiguiente;
	}
	list_iterator_destroy(iterador);
	int booleano = 0;
	if(tamanio <= tamanioLibre){
		booleano = 1;
	}
	return booleano;
}

int hayLugarContiguoPara(size_t tamanio) {
	t_list_iterator* iterador = list_iterator_create(listaDeHuecosLibres);
	size_t max = 0;

	while(list_iterator_has_next(iterador)) {
		HuecoLibre *siguiente = list_iterator_next(iterador);
		size_t desplazamientoSiguiente = siguiente->desplazamiento;
		if(desplazamientoSiguiente > max) {
			max = desplazamientoSiguiente;
		}
	}
	list_iterator_destroy(iterador);
	int booleano = 0;
		if(tamanio <= max){
			booleano = 1;
		}
	return booleano;
}

int hayTablaSegmentosDe(int idProceso) {
	t_list_iterator* iterador = list_iterator_create(tablasDeSegmento);

	while(list_iterator_has_next(iterador)) {
		TablaDeSegmentos *siguiente = list_iterator_next(iterador);
		if(idProceso == siguiente->pid) {
			int booleano = 1;
			return booleano;
		}
	}
	list_iterator_destroy(iterador);
	int booleano2 = 0;
	return booleano2;
}

TablaDeSegmentos* crearTablaSegmentosDe(int idProceso) {
	TablaDeSegmentos *tablaDeSegmentos = malloc(sizeof(TablaDeSegmentos));
	t_list* segmentos = list_create();
	list_add(segmentos, segmento0);
	tablaDeSegmentos->pid = idProceso;
	tablaDeSegmentos->segmentos = segmentos;
	list_add(tablasDeSegmento, tablaDeSegmentos);
	return tablaDeSegmentos;
}

void agregarSegmentoATabla(Segmento *segmento, int idProceso) {
	t_list_iterator* iterador = list_iterator_create(tablasDeSegmento);

	while(list_iterator_has_next(iterador)) {
		TablaDeSegmentos *siguiente = malloc(sizeof(TablaDeSegmentos));
		siguiente->segmentos = list_create();
		siguiente = list_iterator_next(iterador);
		if(idProceso == siguiente->pid) {
			list_add(siguiente->segmentos, segmento);
			memcpy(espacioUsuario + segmento->base, segmento, sizeof(Segmento));
		}
	}
	list_iterator_destroy(iterador);
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

///////////////////////////////////////////////////ALGORITMOS///////////////////////////////////////////////////
size_t buscarPorFirst (size_t tamanio) {
	t_list_iterator* iterador = list_iterator_create(listaDeHuecosLibres);
	int iteradorIndex = 0;

	while(list_iterator_has_next(iterador)) {
		HuecoLibre *siguiente = list_iterator_next(iterador);
		if(siguiente->desplazamiento >= tamanio) {
			iteradorIndex = list_iterator_index(iterador);
			actualizarHuecosLibres(siguiente, tamanio,iteradorIndex);
			list_iterator_destroy(iterador);
			return siguiente->base;
		}
	}
}

size_t buscarPorBest(size_t tamanio) {
	t_list_iterator* iterador = list_iterator_create(listaDeHuecosLibres);
	HuecoLibre *elegido = malloc(sizeof(HuecoLibre));
	elegido->desplazamiento = tamanioMemoria;
	int iteradorIndex = 0;

	while(list_iterator_has_next(iterador)) {
		HuecoLibre *siguiente = list_iterator_next(iterador);
		if(siguiente->desplazamiento >= tamanio && siguiente->desplazamiento <= elegido->desplazamiento) {
				elegido->base = siguiente->base;
				elegido->desplazamiento = siguiente->desplazamiento;
				iteradorIndex = list_iterator_index(iterador);
		}
	}
	actualizarHuecosLibres(elegido, tamanio, iteradorIndex);
	list_iterator_destroy(iterador);
	return elegido->base;
}

size_t buscarPorWorst(size_t tamanio) {
	t_list_iterator* iterador = list_iterator_create(listaDeHuecosLibres);
		HuecoLibre *elegido = malloc(sizeof(HuecoLibre));
		elegido->desplazamiento = 0;
		int iteradorIndex = 0;

		while(list_iterator_has_next(iterador)) {
			HuecoLibre *siguiente = list_iterator_next(iterador);
			if(siguiente->desplazamiento >= tamanio && siguiente->desplazamiento >= elegido->desplazamiento) {
				elegido->base = siguiente->base;
				elegido->desplazamiento = siguiente->desplazamiento;
				iteradorIndex = list_iterator_index(iterador);
			}
		}
		actualizarHuecosLibres(elegido, tamanio, iteradorIndex);
		list_iterator_destroy(iterador);
		return elegido->base;
}

////////////////////////////////////////FUNCIONES QUE USAN LOS ALGORITMOS/////////////////////////////////////////
void actualizarHuecosLibres(HuecoLibre *siguiente, size_t tamanio, int iteradorIndex) {
	list_remove(listaDeHuecosLibres, iteradorIndex);

	HuecoLibre *nuevoHueco = malloc(sizeof(HuecoLibre));
	nuevoHueco->base = siguiente->base + tamanio;
	size_t base = buscarSiguienteLugarOcupado(nuevoHueco->base);
	nuevoHueco->desplazamiento = base - nuevoHueco->base;
	//printf("HUECO DESPLAZAMIENTO: %zu\n", nuevoHueco->desplazamiento);
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

////////////////////////////////////////////////ELIMINAR SEGMENTO/////////////////////////////////////////////////
void eliminar_segmento(int id_proceso, int id_segmento) {
	t_list_iterator* iterador = list_iterator_create(tablasDeSegmento);
		while(list_iterator_has_next(iterador)) {
			TablaDeSegmentos *siguiente = list_iterator_next(iterador);
			if(id_proceso == siguiente->pid) {
				t_list_iterator* iterador2 = list_iterator_create(siguiente->segmentos);
				while(list_iterator_has_next(iterador2)){
					Segmento *segmentoSiguiente = list_iterator_next(iterador2);
					if(segmentoSiguiente->idSegmentoKernel == id_segmento){
						HuecoLibre *nuevoHueco = malloc(sizeof(HuecoLibre));
						nuevoHueco->base = segmentoSiguiente->base;
						nuevoHueco->desplazamiento = segmentoSiguiente ->desplazamiento;
						list_add(listaDeHuecosLibres,nuevoHueco);
						juntarHuecosContiguos();
						list_iterator_remove(iterador2);
						list_remove_element(segmentos,segmentoSiguiente);
						log_info(logger, "PID: %d - Eliminar Segmento: %d - Base: %zu - TAMAÑO: %zu\n",id_proceso, id_segmento,segmentoSiguiente->base,segmentoSiguiente ->desplazamiento);
					}

				}
				list_iterator_destroy(iterador2);
			}
		}
		list_iterator_destroy(iterador);
}

////////////////////////////////////////FUNCIONES QUE USA ELIMINAR SEGMENTO////////////////////////////////////////
void juntarHuecosContiguos(){
	t_list_iterator* iterador = list_iterator_create(listaDeHuecosLibres);

	while(list_iterator_has_next(iterador)) {
		HuecoLibre *nuevoHueco = list_iterator_next(iterador);
		t_list_iterator* iterador2 = list_iterator_create(listaDeHuecosLibres);
		while(list_iterator_has_next(iterador2)){
			HuecoLibre *nuevoHueco2 = list_iterator_next(iterador2);
			if(nuevoHueco->base + nuevoHueco->desplazamiento == nuevoHueco2->base){
				nuevoHueco2->base = nuevoHueco->base;
				nuevoHueco2->desplazamiento = nuevoHueco->desplazamiento + nuevoHueco2->desplazamiento;
				list_iterator_remove(iterador); // Utilizar list_iterator_remove en lugar de list_remove_element
				break;
			}
		}
		list_iterator_destroy(iterador2);
	}
	list_iterator_destroy(iterador);
}

////////////////////////////////////////////////COMPACTAR MEMORIA/////////////////////////////////////////////////
void compactar_memoria() {
	//un while donde se fija si hay un segmento con base == tamaño+base del hueco libre, si hay,
	// se pasa el segmento a esa base y se borra el hueco libre
	t_list* listaOrdenada = list_sorted(segmentos, comparador);
	//t_list* copialistaOrdenada = list_duplicate(listaOrdenada);
	t_list_iterator* iterador = list_iterator_create(listaOrdenada);

	t_list* copiaHuecosLibres = list_duplicate(listaDeHuecosLibres);
	while(list_iterator_has_next(iterador)){
		Segmento *segmento = list_iterator_next(iterador);
		t_list_iterator* iterador2 = list_iterator_create(copiaHuecosLibres);
		while(list_iterator_has_next(iterador2)){
			HuecoLibre *huecoLibre = list_iterator_next(iterador2);
			Segmento *ultimoSegmento = list_get(listaOrdenada, list_size(listaOrdenada)-1);
			if(segmento->base + segmento->desplazamiento == huecoLibre->base && segmento!=ultimoSegmento){
				Segmento *proximoSegmento = list_get(listaOrdenada, list_iterator_index(iterador)+1);
				memcpy(espacioUsuario + segmento->base + segmento->desplazamiento, espacioUsuario+proximoSegmento->base, proximoSegmento->desplazamiento);
				proximoSegmento->base = segmento->base + segmento->desplazamiento;
				int pid = buscarIdMemoria(proximoSegmento->idSegmentoMemoria);
				int iteradorIndex = list_iterator_index(iterador2);
				actualizarHuecosLibres(huecoLibre, proximoSegmento->desplazamiento, iteradorIndex);
				log_info(logger,"PID: %d - Segmento: %d - Base: %zu - Tamaño %zu\n", pid, proximoSegmento->idSegmentoKernel, proximoSegmento->base, proximoSegmento->desplazamiento);
			}
		}
		list_iterator_destroy(iterador2);
	}
	list_iterator_destroy(iterador);
}

////////////////////////////////////////FUNCIONES QUE USA COMPACTAR MEMORIA////////////////////////////////////////
int buscarIdMemoria(int idSegmentoMemoria){
	t_list_iterator* iterador = list_iterator_create(tablasDeSegmento);
	while(list_iterator_has_next(iterador)){
		TablaDeSegmentos *tablaDeSegmentos = list_iterator_next(iterador);
		t_list_iterator* iterador2 = list_iterator_create(tablaDeSegmentos->segmentos);
		while(list_iterator_has_next(iterador2)){
			Segmento *segmento = list_iterator_next(iterador2);
			if(segmento->idSegmentoMemoria == idSegmentoMemoria){
				list_iterator_destroy(iterador2);
				return tablaDeSegmentos->pid;
			}
		}
	}
	list_iterator_destroy(iterador);
	return -1;
}

bool comparador(void* elem1, void* elem2) {
	Segmento* seg1 = (Segmento*) elem1;
	Segmento* seg2 = (Segmento*) elem2;

	return (seg1->base < seg2->base);
	}

////////////////////////////////////////ENVIAR TODAS LAS TABLAS////////////////////////////////////////
void enviarTodasLasTablas(int cliente_fd){
	int tamanioLista = list_size(tablasDeSegmento);
	//t_list_iterator* iterador = list_iterator_create(tablasDeSegmento);
	for(int i =0; i<tamanioLista; i++){
		TablaDeSegmentos *tablaDeSegmentos = list_get(tablasDeSegmento, i);
		t_paquete* paquete = empaquetarTabla(tablaDeSegmentos->pid, tablaDeSegmentos->segmentos, TABLA_GLOBAL);
		enviar_paquete(paquete, cliente_fd);
		eliminar_paquete(paquete);
	}



//	while(list_iterator_has_next(iterador)){
//		TablaDeSegmentos *tablaDeSegmentos = list_iterator_next(iterador);
//		t_paquete* paquete = empaquetarTabla(tablaDeSegmentos->pid, tablaDeSegmentos->segmentos, TABLA_GLOBAL);
//		enviar_paquete(paquete, cliente_fd);
//		eliminar_paquete(paquete);
//	}
//
//	list_iterator_destroy(iterador);
	//printf("TABLAS ENVIADAS\n");
}

//////////////////////////////////////////////////ELIMINAR PROCESO/////////////////////////////////////////////////
void eliminar_proceso(int idProceso) {
    t_list_iterator* iteradorTablas = list_iterator_create(tablasDeSegmento);

    while (list_iterator_has_next(iteradorTablas)) {
        TablaDeSegmentos *siguiente = list_iterator_next(iteradorTablas);
        t_list* copiaListaSegmentos = list_duplicate(siguiente->segmentos);
        if (idProceso == siguiente->pid) {
            t_list_iterator* iteradorSegmentos = list_iterator_create(copiaListaSegmentos);

            while (list_iterator_has_next(iteradorSegmentos)) {
                Segmento *segmento = list_iterator_next(iteradorSegmentos);
                if(segmento->idSegmentoKernel !=0){
                eliminar_segmento(idProceso, segmento->idSegmentoKernel);
                }
            }

            list_iterator_destroy(iteradorSegmentos);
            list_iterator_remove(iteradorTablas);
        }
    }

    list_iterator_destroy(iteradorTablas);
}

//////////////////////////////////////////////////CLIENTE KERNEL/////////////////////////////////////////////////

void iniciarHiloClienteKernel(int codigo_kernel,int cliente_fd) {
	ClientKernelArgs args;
	args.cod_kernel = codigo_kernel;
	args.cliente_fd = cliente_fd;

	int err = pthread_create( 	&client_Kernel,	// puntero al thread
								NULL,
								clientKernel, // le paso la def de la función que quiero que ejecute mientras viva
								(void *)&args); // argumentos de la función

	if (err != 0) {
	//printf("\nNo se pudo crear el hilo del cliente Kernel de memoria.");
	exit(7);
	}
	//printf("El hilo cliente de la Memoria se creo correctamente.");

}


void* clientKernel(void *arg) {
	ClientKernelArgs *args = (ClientKernelArgs *)arg;
	int codigo_kernel = malloc(sizeof(int));
	codigo_kernel = 0;
	codigo_kernel = args->cod_kernel;
	int cliente_fd = args->cliente_fd;

//	MENSAJE --> 0
//	PAQUETE --> 1
//	CREATE_SEGMENT --> 2
//	DELETE_SEGMENT --> 3
//	COMPACTAR_MEMORIA --> 4
//	PROCESO_NUEVO --> 5
//	TABLA_SEGMENTOS --> 6
// 	SIN_ESPACIO --> 7
//	PEDIR_COMPACTACION-->8
//	ELIMINAR_PROCESO-->9
//	TABLA_GLOBAL-->10

	switch(codigo_kernel){
		case CREATE_SEGMENT:
			char* baseStr = string_from_format("%zu", base);
			//printf("Base enviada a Kernel: %s\n", baseStr);
			enviar_cod_operacion(baseStr ,cliente_fd, CREATE_SEGMENT);
			//printf("CREATE_SEGMENT ENVIADO\n");
		break;
		case SIN_ESPACIO:
			enviar_cod_operacion("",cliente_fd, SIN_ESPACIO);
			//printf("SIN_ESPACIO ENVIADO\n");
		break;
		case PEDIR_COMPACTACION:
			enviar_cod_operacion("",cliente_fd, PEDIR_COMPACTACION);
			//printf("PEDIR_COMPACTACION ENVIADO\n");
		break;
	}
	return NULL;
}
