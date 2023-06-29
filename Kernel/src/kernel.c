#include "kernel.h"

t_config* config;

int main(void) {

	// LOGGER - CONFIG

    logger = log_create("kernel.log", "Kernel", 1, LOG_LEVEL_DEBUG);

    config = config_create("/home/utnso/tp-2023-1c-Los-operadores/Kernel/kernel.config");

    if (config == NULL) {
        printf("No se pudo crear el config.");
        exit(5);
    }

    ip_cpu = config_get_string_value(config, "IP_CPU");
    puerto_cpu = config_get_string_value(config, "PUERTO_CPU");
    ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    ip_filesystem = config_get_string_value(config, "IP_FILESYSTEM");
    puerto_filesystem = config_get_string_value(config, "PUERTO_FILESYSTEM");
    puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
    algoritmo_planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
    hrrn_alfa = config_get_int_value(config, "HRRN_ALFA");
    grado_max_multiprogramación = config_get_int_value(config, "GRADO_MAX_MULTIPROGRAMACION");


    //INICILIZACIÓN

	sem_init(&semKernelClientMemoria,0,0);

    estadoEnEjecucion = malloc(sizeof(t_infopcb));
    int ningunEstado = -1;
    estadoEnEjecucion->pid = ningunEstado;

    tablaGlobalArchivosAbiertos = list_create();

    listaReady = list_create();
    cantidadElementosSistema = 0;

    listaRecursos = list_create();
    inicializarRecursos();
    cantidadElementosBloqueados = 0;

    //HANDSHAKES NECESARIOS
    enviar_handshake_memoria();

    //THREAD SERVER
    server_fd = iniciar_servidor();
    log_info(logger, "Kernel listo para escuchar al cliente\n");

    while(1){
    	iniciarHiloServer(server_fd);
    }

//    pthread_detach(client_CPU);
//    pthread_detach(interrupcion_IO);
//    pthread_detach(client_Memoria);
//    pthread_detach(client_FileSystem);


    // LIBERAR MEMORIA

    log_destroy(logger);
    config_destroy(config);

    sem_destroy(&semKernelClientMemoria);

    free(estadoEnEjecucion);
    return EXIT_SUCCESS;
}



void inicializarRecursos(){
	//Manejo de recursos, VER COMO INICIALIZAR VARIABLES
	char** recursos = config_get_array_value(config, "RECURSOS"); // El array que devuelve termina en NULL
	char** instancias_recursos = config_get_array_value(config, "INSTANCIAS_RECURSOS");

	while(!string_array_is_empty(recursos)){
		t_recursos* unRecurso;
		unRecurso = malloc(sizeof(t_recursos));

		unRecurso->recurso = string_array_pop(recursos);

		int instanciaRecurso= atoi(string_array_pop(instancias_recursos));
		unRecurso->instancias = instanciaRecurso;
		unRecurso->colaBloqueados = queue_create();
		list_add(listaRecursos, unRecurso);

	}

		free(recursos);
		free(instancias_recursos);

}

//////////////////////////////////////   SERVER KERNEL  ////////////////////////////////////////

int iniciarHiloServer(int server_fd) {
	log_info(logger, "Entre a hilo server");
	int cliente_fd = esperar_cliente(server_fd); // se conecta el cliente
	log_info(logger, "espere cliente");
			if(cliente_fd != -1) {
				pthread_t hilo_cliente;
				pthread_create(&hilo_cliente, NULL, (void*) serverKernel, (void *)cliente_fd); // creo el hilo con la funcion manejar conexion a la que le paso el socket del cliente y sigo en la otra funcion
				log_info(logger, "Kernel creo el hilo");
				pthread_detach(hilo_cliente);
				return 1;
			} else {
				log_error(logger, "Error al escuchar clientes... Finalizando servidor \n"); // log para fallo de comunicaciones
			}

	return 0;
}

void* serverKernel(int cliente_fd){

    log_info(logger, "Kernel listo para recibir al cliente");

    t_list* lista;
   while (1) {
    int cod_op = recibir_operacion(cliente_fd);
    	switch (cod_op) {
    		case MENSAJE:
    			char* handshake = recibir_handshake(cliente_fd);
    			if (strcmp(handshake, "consola") == 0) {
    				log_info(logger, "Se conecto una consola");
    				//cosas de consola
    			}
    			if (strcmp(handshake, "kernel") == 0) {
    				log_info(logger, "Se conecto el kernel");
    				//cosas de kernel
    			}
    			if (strcmp(handshake, "CPU") == 0) {
    				log_info(logger, "Se conecto la cpu");
    				//cosas de cpu
    			}
    			if (strcmp(handshake, "filesystem") == 0) {
    				log_info(logger, "Se conecto el filesystem");
    				//cosas de fs
    			}
    			break;
    		case PAQUETE:
    			lista = recibir_paquete(cliente_fd); //Recibe paquete de instrucciones
    			if (strcmp(handshake, "consola") == 0) {
    			log_info(logger, "Iniciando procedimiento al recibir un paquete de CONSOLA");
        		armarPCB(lista);  //arma el PCB y lo encola en NEW

    			//cosas de consola
    			}
    			if (strcmp(handshake, "memoria") == 0) {
    				log_info(logger, "Iniciando procedimiento al recibir un paquete de KERNEL");
    				//cosas de kernel
    			}
    			if (strcmp(handshake, "CPU") == 0) {
    				log_info(logger, "Iniciando procedimiento al recibir un paquete de CPU");
    				//cosas de cpu
    			}
    			if (strcmp(handshake, "filesystem") == 0) {
    				log_info(logger, "Iniciando procedimiento al recibir un paquete de FILESYSTEM");
    				//cosas de fs
    			}
    			break;
    		case -1:
    		    	free(handshake);
    		    	log_error(logger, "\nel cliente se desconecto. Terminando servidor");
    		    	return EXIT_FAILURE;
    		default:
    			log_warning(logger,"\nOperacion desconocida. No quieras meter la pata");
    			break;
    	}
    }

	return NULL;
}


///////////////////////////////////// CLIENT MEMORIA ////////////////////////////////////////////

void iniciarHiloClienteMemoria(int cod_memoria,int pidProceso) {
	ClientMemoriaArgs *args = malloc(sizeof(ClientMemoriaArgs));
	args->cod_memoria = cod_memoria;
	args->pidProceso= pidProceso;

	int err = pthread_create( 	&client_Memoria,	// puntero al thread
								NULL,
								clientMemoria, // le paso la def de la función que quiero que ejecute mientras viva
								(void *)args); // argumentos de la función

	if (err != 0) {
	printf("\nNo se pudo crear el hilo del cliente Memoria del kernel.");
	exit(7);
	}

}


void* clientMemoria(void *arg) {

	ClientMemoriaArgs *args = (ClientMemoriaArgs *)arg;
	int cod_memoria = args->cod_memoria;
	int pidProceso = args->pidProceso;
	t_infopcb* procesoADesencolar = malloc(sizeof(t_infopcb));
	int config = 1;
    int conexion_Memoria;
    conexion_Memoria = crear_conexion(ip_memoria, puerto_memoria);

    t_paquete* paquete = crear_paquete_cod_operacion(cod_memoria);
    switch(cod_memoria){
    	case 2:
    		char* pid = string_new();
    		char* idSegmento = string_new();
    		char* tamanioSegmento = string_new();

            string_append_with_format(&pid, "%d", estadoEnEjecucion->pid);
            string_append_with_format(&idSegmento, "%d", estadoEnEjecucion->idSegmento);
            string_append_with_format(&tamanioSegmento, "%d", estadoEnEjecucion->tamanioSegmento);

        	agregar_a_paquete(paquete, pid, strlen(pid)+1);
        	agregar_a_paquete(paquete, idSegmento, strlen(idSegmento)+1);
        	agregar_a_paquete(paquete, tamanioSegmento, strlen(tamanioSegmento)+1);

        	enviar_paquete(paquete, conexion_Memoria);

        	printf("CREATE_SEGMENT enviado a MEMORIA.\n");
        	printf("pid enviado a Memoria: %s\n", pid);
        	printf("idSegmento enviado a Memoria: %s\n", idSegmento);
        	printf("tamanioSegmento enviado a Memoria: %s\n", tamanioSegmento);

        	eliminar_paquete(paquete);
        break;
    	case 3:

    		char* pidDelete = string_new();
    		char* idSegmentoDelete = string_new();

    		string_append_with_format(&pidDelete, "%d", estadoEnEjecucion->pid);
    		string_append_with_format(&idSegmentoDelete, "%d", estadoEnEjecucion->idSegmento);

    		printf("pid enviado a Memoria: %s\n", pidDelete);
    		printf("idSegmento enviado a Memoria: %s\n", idSegmentoDelete);

        	agregar_a_paquete(paquete, pidDelete, strlen(pidDelete)+1);
        	agregar_a_paquete(paquete, idSegmentoDelete, strlen(idSegmentoDelete)+1);

    		enviar_paquete(paquete, conexion_Memoria);

    		printf("DELETE_SEGMENT enviado a MEMORIA.\n");
    		eliminar_paquete(paquete);
    	break;
    	case 5:
    		char* pidNuevo = string_new();
    		procesoADesencolar = unqueue(&frenteColaNew,&finColaNew);
    		string_append_with_format(&pidNuevo, "%d", procesoADesencolar->pid);

    		printf("PROCESO_NUEVO - Pid enviado a Memoria: %s\n", pidNuevo);
    		enviar_mensaje_cod_operacion(pidNuevo,conexion_Memoria,5);

    	break;
    	case 4: //compactar memoria
    		enviar_paquete(paquete, conexion_Memoria);
    		eliminar_paquete(paquete);
    	break;
    	case 9: //Eliminar proceso
    		char* pidNuevoEliminarProceso = string_new();
    		string_append_with_format(&pidNuevoEliminarProceso, "%d", pidProceso);
    		enviar_mensaje_cod_operacion(pidNuevoEliminarProceso,conexion_Memoria,9);
    		liberar_conexion(conexion_Memoria);
    		return NULL;
    	break;
		default:
			log_warning(logger," Operacion desconocida. NO se envió nada a Memoria.\n");
		break;
    }

    int cod_op = recibir_operacion(conexion_Memoria);
    //printf("codigo de operacion: %i\n", cod_op);

    switch (cod_op) {
		case 6:
			t_list* tablaSegmentos = recibir_paquete(conexion_Memoria);
			procesoADesencolar->tablaSegmentos = tablaSegmentosActualizada(tablaSegmentos);
			printf("PROCESO_NUEVO - Tabla inicial actualizada.\n");

			if(strcmp(algoritmo_planificacion,"FIFO") == 0){
				queue(&frenteColaReady, &finColaReady,procesoADesencolar);
				//Log minimo y obligatorio
				log_info(logger, "PID: %d - Estado Anterior: New - Estado Actual: Ready\n", procesoADesencolar->pid);
				sem_post(&semKernelClientMemoria);
			}
			if(strcmp(algoritmo_planificacion,"HHRN") == 0){
				list_add(listaReady, procesoADesencolar);
				procesoADesencolar->entraEnColaReady = tomarTiempo();
				//Log minimo y obligatorio
				log_info(logger, "PID: %d - Estado Anterior: New - Estado Actual: Ready\n", procesoADesencolar->pid);
				sem_post(&semKernelClientMemoria);
			}

			liberar_conexion(conexion_Memoria);

		break;
        case 2:
        	char* mensajeMemoria = recibir_handshake(conexion_Memoria);
        	size_t base = strtol(mensajeMemoria, NULL, 10);

        	t_infoTablaSegmentos* nuevoSegmento = malloc(sizeof(t_infoTablaSegmentos));
        	nuevoSegmento->id = estadoEnEjecucion->idSegmento;
        	nuevoSegmento->direccionBase = base;
        	nuevoSegmento->tamanio = estadoEnEjecucion->tamanioSegmento;

        	list_add(estadoEnEjecucion->tablaSegmentos, nuevoSegmento);

        	printf("Tabla de segmentos: \n");
        	t_list_iterator* iterador = list_iterator_create(estadoEnEjecucion->tablaSegmentos);
        	while (list_iterator_has_next(iterador)) {
        		t_infoTablaSegmentos* siguiente = list_iterator_next(iterador);
        		printf("IdSegmento: %d\n",siguiente->id);
        		printf("Base: %zu\n",siguiente->direccionBase);
        		printf("Tamaño: %zu\n",siguiente->tamanio);
        	}
        	liberar_conexion(conexion_Memoria);
        	iniciarHiloClienteCPU();
        break;
        case 7:
    		//Log minimo y obligaotrio
    		//log_info(logger, "Finaliza el proceso &d - Motivo: OUT OF MEMORY\n", unProceso->pid);
        	liberar_conexion(conexion_Memoria);
        	pasarAExit();
        break;
        case 8:
    		//Revisar si hay conexion entre FileSystem y Memoria
    		//Mandarle a memoria que compacte
        	//Una vez terminada esta rutina, el Kernel repetirá la solicitud de creación del segmento.

        	procedimiento_compactar();

        break;
        case 3:   //Después de delete_segment
        	t_list* tablaSegmentosRecibida = recibir_paquete(conexion_Memoria);
        	estadoEnEjecucion->tablaSegmentos = tablaSegmentosActualizada(tablaSegmentosRecibida);
        	liberar_conexion(conexion_Memoria);
        break;
        case 4:
        	printf("FALTA RECIBIR LA COMPACTACIÓN\n");
        	//NOS MANDAN UN PAQUETE POR CADA TABLA DE SEGMENTOS
        break;
		default:
			log_warning(logger,"\nOperacion recibida de MEMORIA desconocida.\n");
			 liberar_conexion(conexion_Memoria);
		break;

    }

    free(args);
	return NULL;
}

t_list* tablaSegmentosActualizada(t_list* tablaSegmentosRecibida){

	t_list_iterator* iterador = list_iterator_create(tablaSegmentosRecibida);
	t_list* tablaSegmentosActualizada= list_create();
	t_infoTablaSegmentos* nuevoSegmento = malloc(sizeof(t_infoTablaSegmentos));
	printf("Tabla de segmentos: \n");
	while (list_iterator_has_next(iterador)) {

		char* siguiente = list_iterator_next(iterador);

		char** arraySegmento = string_array_new();
		arraySegmento = string_split(siguiente, " ");

		int pid = atoi(arraySegmento[0]);
		int idSegmento = atoi(arraySegmento[1]);
		int baseSegmento = atoi(arraySegmento[2]);
		int tamanioSegmento = atoi(arraySegmento[3]);

		printf("idSegmento: %d\n",idSegmento);
		printf("baseSegmento: %d\n",baseSegmento);
		printf("tamanioSegmento: %d\n",tamanioSegmento);

		nuevoSegmento->id = idSegmento;
		nuevoSegmento->direccionBase = baseSegmento;
		nuevoSegmento->tamanio = tamanioSegmento;

		list_add(tablaSegmentosActualizada,nuevoSegmento);
	 }

	return tablaSegmentosActualizada;
}

void procedimiento_compactar(){
	printf("FALTA HACER PROCEDIMIENTO COMPACTAR\n");
	//Revisar si hay conexion entre FileSystem y Memoria
	//Mandarle a memoria que compacte
	//compactar();
	return;
}

void compactar(){
	//t_clientMemoria* infoClientMemoria;
	//infoClientMemoria->cod_memoria = 4;
	iniciarHiloClienteMemoria(4,0);
	//Una vez terminada esta rutina, el Kernel repetirá la solicitud de creación del segmento.
	return;
}

///////////////////////////////////// CLIENT FILESYSTEM ////////////////////////////////////////////

void iniciarHiloClienteFileSystem(int cod_fs, char* nombreArchivo,int posicionPuntero,int cantBytes, size_t direcFisica, size_t nuevoTamanioArchivo) {
	ClientFSArgs* args = malloc(sizeof(ClientFSArgs));
	args->cod_fs = cod_fs;
	args->nombreArchivo = nombreArchivo;
	args->posicionPuntero = posicionPuntero;
	args->cantBytes = cantBytes;
	args->direcFisica = direcFisica;
	args->nuevoTamanioArchivo = nuevoTamanioArchivo;

	int err = pthread_create( 	&client_FileSystem,	// puntero al thread
								NULL,
								clientFileSystem, // le paso la def de la función que quiero que ejecute mientras viva
								(void *)args); // argumentos de la función

	if (err != 0) {
	printf("\nNo se pudo crear el hilo del cliente FileSystem del kernel.");
	exit(7);
	}
	//printf("\nEl hilo cliente del FileSystem se creo correctamente.");

}

void* clientFileSystem(void *arg) {

	// 1 -> F_OPEN
	// 2 -> F_READ
	// 3 -> F_WRITE
	// 4 -> F_TRUNCATE

	ClientFSArgs *args = (ClientFSArgs *)arg;
	int cod_fs = args->cod_fs;
	char* nombreArchivo = args->nombreArchivo;
	int posicionPuntero = args->posicionPuntero;
	int cantBytes = args->cantBytes;
	size_t direcFisica = args->direcFisica;
	size_t nuevoTamanioArchivo = args->nuevoTamanioArchivo;

    int conexion_FileSystem;
    conexion_FileSystem = crear_conexion(ip_filesystem, puerto_filesystem);

    switch(cod_fs){
		case 1: //F_OPEN

		break;
		case 2: //F_READ

		break;
		case 3: //F_WRITE

		break;
		case 4: //F_TRUNCATE

		break;
		default:
			log_warning(logger," Operacion desconocida. NO se envió nada a FileSystem.\n");
		break;
	}

	int cod_op = recibir_operacion(conexion_FileSystem);

	switch (cod_op) {
		case 1: //F_OPEN

		break;
		case 2: //F_READ

		break;
		case 3: //F_WRITE

		break;
		case 4: //F_TRUNCATE

		break;

		default:
			log_warning(logger,"\nOperacion recibida de FileSystem desconocida.\n");
			 liberar_conexion(conexion_FileSystem);
		break;

	}

	free(args);
	return NULL;
}


///////////////////////////////////// CLIENT CPU ////////////////////////////////////////////

void iniciarHiloClienteCPU() {

	int err = pthread_create( 	&client_CPU,	// puntero al thread
								NULL,
								clientCPU, // le paso la def de la función que quiero que ejecute mientras viva
								NULL); // argumentos de la función

	if (err != 0) {
	printf("\nNo se pudo crear el hilo de la conexión kernel-CPU.\n");
	exit(7);
	}
	//printf("\nEl hilo de la conexión kernel-CPU se creo correctamente.\n");

}

void* clientCPU(void* ptr) {

	int config=1;
    int conexion_CPU;
    conexion_CPU = crear_conexion(ip_cpu, puerto_cpu);

    serializarContexto(conexion_CPU); //enviamos el contexto sin las instrucciones

    //enviamos las intrucciones
    t_list_iterator* iterador = list_iterator_create(estadoEnEjecucion->listaInstrucciones);
    t_paquete* paquete = empaquetarInstrucciones(estadoEnEjecucion->listaInstrucciones);
    enviar_paquete(paquete, conexion_CPU);
    eliminar_paquete(paquete);
    printf("Contexto enviado a CPU. \n");

    //enviamos tabla Segmentos
    t_paquete* tabla = empaquetarTabla(estadoEnEjecucion->tablaSegmentos);
    enviar_paquete(tabla, conexion_CPU);
    eliminar_paquete(tabla);
    printf("Tabla de Segmentos enviada a CPU. \n");

    //RESPUESTA DE CPU
    int cod_op = recibir_operacion(conexion_CPU);

    //contextoActualizado = recibir_contexto(conexion_CPU);
    recibir_contexto(conexion_CPU);
    liberar_conexion(conexion_CPU);

    printf("Después de recibir el contexto\n");

    printf("programCounter recibido de CPU = %d\n",estadoEnEjecucion->programCounter);
    printf("Última instruccion ejecutada = %s\n",estadoEnEjecucion->ultimaInstruccion);
    printf("Tiempo bloqueado recibido de CPU = %d\n",estadoEnEjecucion->tiempoBloqueado);
    printf("Id del segmento = %d\n",estadoEnEjecucion->idSegmento);
    printf("Tamaño del segmento = %d\n",estadoEnEjecucion->tamanioSegmento);
    printf("AX recibido = %s\n",estadoEnEjecucion->registrosCpu.AX);
    printf("BX recibido = %s\n",estadoEnEjecucion->registrosCpu.BX);
    printf("CX recibido = %s\n",estadoEnEjecucion->registrosCpu.CX);
    printf("DX recibido = %s\n",estadoEnEjecucion->registrosCpu.DX);

    printf("EAX recibido  = %s\n",estadoEnEjecucion->registrosCpu.EAX);
    printf("EBX recibido  = %s\n",estadoEnEjecucion->registrosCpu.EBX);
    printf("ECX recibido  = %s\n",estadoEnEjecucion->registrosCpu.ECX);
    printf("EDX recibido  = %s\n",estadoEnEjecucion->registrosCpu.EDX);

    printf("RAX recibido  = %s\n",estadoEnEjecucion->registrosCpu.RAX);
    printf("RBX recibido  = %s\n",estadoEnEjecucion->registrosCpu.RBX);
    printf("RCX recibido  = %s\n",estadoEnEjecucion->registrosCpu.RCX);
    printf("RDX recibido  = %s\n",estadoEnEjecucion->registrosCpu.RDX);

    printf("Recurso solicitado = %s\n",estadoEnEjecucion->recursoSolicitado);

    printf("Recurso a liberar = %s\n",estadoEnEjecucion->recursoALiberar);

    printf("Nombre del archivo = %s\n",estadoEnEjecucion->nombreArchivo);
    printf("Posicion del archivo = %d\n",estadoEnEjecucion->posicionArchivo);
    printf("Cant bytes del archivo = %d\n",estadoEnEjecucion->cantBytesArchivo);
    printf("Direccion física del archivo = %d\n",estadoEnEjecucion->direcFisicaArchivo);
    printf("Tamaño del archivo = %d\n",estadoEnEjecucion->tamanioArchivo);

    manejar_recursos();


	return NULL;
}

void serializarContexto(int unSocket){

	//BUFFER

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(int)+ sizeof(int) + sizeof(estadoEnEjecucion->registrosCpu.AX) * 4 + sizeof(estadoEnEjecucion->registrosCpu.EAX) *4 + sizeof(estadoEnEjecucion->registrosCpu.RAX)*4;

	void* stream = malloc(buffer->size);
	int offset = 0; //desplazamiento

	memcpy(stream + offset, &estadoEnEjecucion->pid, sizeof(int));
	offset += sizeof(int);

	memcpy(stream + offset, &estadoEnEjecucion->programCounter, sizeof(int));
	offset += sizeof(int);

	memcpy(stream + offset, &estadoEnEjecucion->registrosCpu.AX, sizeof(estadoEnEjecucion->registrosCpu.AX));
	offset += sizeof(estadoEnEjecucion->registrosCpu.AX);

	memcpy(stream + offset, &estadoEnEjecucion->registrosCpu.BX, sizeof(estadoEnEjecucion->registrosCpu.BX));
	offset += sizeof(estadoEnEjecucion->registrosCpu.BX);

	memcpy(stream + offset, &estadoEnEjecucion->registrosCpu.CX, sizeof(estadoEnEjecucion->registrosCpu.CX));
	offset += sizeof(estadoEnEjecucion->registrosCpu.CX);

	memcpy(stream + offset, &estadoEnEjecucion->registrosCpu.DX, sizeof(estadoEnEjecucion->registrosCpu.DX));
	offset += sizeof(estadoEnEjecucion->registrosCpu.DX);

	memcpy(stream + offset, &estadoEnEjecucion->registrosCpu.EAX, sizeof(estadoEnEjecucion->registrosCpu.EAX));
	offset += sizeof(estadoEnEjecucion->registrosCpu.EAX);

	memcpy(stream + offset, &estadoEnEjecucion->registrosCpu.EBX, sizeof(estadoEnEjecucion->registrosCpu.EBX));
	offset += sizeof(estadoEnEjecucion->registrosCpu.EBX);

	memcpy(stream + offset, &estadoEnEjecucion->registrosCpu.ECX, sizeof(estadoEnEjecucion->registrosCpu.ECX));
	offset += sizeof(estadoEnEjecucion->registrosCpu.ECX);

	memcpy(stream + offset, &estadoEnEjecucion->registrosCpu.EDX, sizeof(estadoEnEjecucion->registrosCpu.EDX));
	offset += sizeof(estadoEnEjecucion->registrosCpu.EDX);

	memcpy(stream + offset, &estadoEnEjecucion->registrosCpu.RAX, sizeof(estadoEnEjecucion->registrosCpu.RAX));
	offset += sizeof(estadoEnEjecucion->registrosCpu.RAX);

	memcpy(stream + offset, &estadoEnEjecucion->registrosCpu.RBX, sizeof(estadoEnEjecucion->registrosCpu.RBX));
	offset += sizeof(estadoEnEjecucion->registrosCpu.RBX);

	memcpy(stream + offset, &estadoEnEjecucion->registrosCpu.RCX, sizeof(estadoEnEjecucion->registrosCpu.RCX));
	offset += sizeof(estadoEnEjecucion->registrosCpu.RCX);

	memcpy(stream + offset, &estadoEnEjecucion->registrosCpu.RDX, sizeof(estadoEnEjecucion->registrosCpu.RDX));
	offset += sizeof(estadoEnEjecucion->registrosCpu.RDX);


	buffer->stream = stream;


	//llenar el PAQUETE con el buffer

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = CONTEXTO;
	paquete->buffer = buffer; // Nuestro buffer de antes.

	// Armamos el stream a enviar
	//    tamaño               stream        size       codigo_operación
	void* a_enviar = malloc(buffer->size + sizeof(int) + sizeof(op_code)); //op_code -> int
	offset = 0;

	memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(op_code));
	offset += sizeof(int);

	memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(int));
    offset += sizeof(int);

	memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
	//offset += paquete->buffer->size;  //No tiene sentido seguir calculando el desplazamiento

	// Lo enviamos
	send(unSocket, a_enviar, buffer->size + sizeof(int) +sizeof(op_code), 0);

//	printf("instruccion enviado a CPU = %s\n", contextoPRUEBA.listaInstrucciones->head->data);
	printf("Contexto sin instrucciones enviado a CPU. \n");

	//free memoria dinámica

	// Liberamos la memoria
	free(a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);

	return;
}

t_paquete* empaquetarInstrucciones(t_list* cabeza) {

    t_list_iterator* iterador = list_iterator_create(cabeza);
    t_paquete* paquete = crear_paquete_instrucciones();

    while (list_iterator_has_next(iterador)) {

    	char* siguiente = list_iterator_next(iterador);
    	int tamanio = (strlen(siguiente))+1;
    	agregar_a_paquete(paquete, siguiente,tamanio );

    }
    return paquete;
}


t_paquete* empaquetarTabla(t_list* cabezaTabla) {

    t_list_iterator* iterador = list_iterator_create(cabezaTabla);
    t_paquete* paquete = crear_paquete_cod_operacion(6);


    while (list_iterator_has_next(iterador)) {
    	char* idSegmento = string_new();
    	char* tamanioSegmento = string_new();
    	char* baseSegmento = string_new();
    	char* segmento = string_new();
    	t_infoTablaSegmentos* siguiente = list_iterator_next(iterador);
    	printf("Datos del segmento: \n");
    	printf("idSegmento: %d\n",siguiente->id);
    	printf("tamanioSegmento: %zu\n",siguiente->tamanio);
    	printf("baseSegmento: %zu\n",siguiente->direccionBase);

        string_append_with_format(&idSegmento, "%d", siguiente->id);
        string_append_with_format(&tamanioSegmento, "%zu", siguiente->tamanio);
        string_append_with_format(&baseSegmento, "%zu", siguiente->direccionBase);

        string_append(&segmento, idSegmento);
        string_append(&segmento, " ");

        string_append(&segmento, tamanioSegmento);
        string_append(&segmento, " ");

        string_append(&segmento, baseSegmento);
        printf("Segmento: %s\n",segmento);

        agregar_a_paquete(paquete, segmento, strlen(segmento)+1);
    }

    return paquete;
}

void recibir_contexto(int socket_cliente){

	//printf("DENTRO DE RECIBIR CONTEXTO\n");
	//comentario

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));

	// Recibimos el buffer.
	//Recibimos el tamaño del buffer
	recv(socket_cliente, &(paquete->buffer->size), sizeof(int), 0);

	//Recibimos el contenido del buffer
	paquete->buffer->stream = malloc(paquete->buffer->size);
	recv(socket_cliente, paquete->buffer->stream, paquete->buffer->size, 0);

	//Desserializamos el contenido

	void* stream = paquete->buffer->stream;

	// Deserializamos los campos que tenemos en el buffer
	 memcpy(&(estadoEnEjecucion->programCounter), stream, sizeof(int));
	 stream += sizeof(int);

	 memcpy(&(estadoEnEjecucion->tiempoBloqueado), stream, sizeof(int));
	 stream += sizeof(int);

	 memcpy(&(estadoEnEjecucion->posicionArchivo), stream, sizeof(int));
	 stream += sizeof(int);

	 memcpy(&(estadoEnEjecucion->cantBytesArchivo), stream, sizeof(int));
	 stream += sizeof(int);

	 memcpy(&(estadoEnEjecucion->direcFisicaArchivo), stream, sizeof(int));
	 stream += sizeof(int);

	 memcpy(&(estadoEnEjecucion->tamanioArchivo), stream, sizeof(int));
	 stream += sizeof(int);

	 memcpy(&(estadoEnEjecucion->idSegmento), stream, sizeof(int));
	 stream += sizeof(int);

	 memcpy(&(estadoEnEjecucion->tamanioSegmento), stream, sizeof(int));
	 stream += sizeof(int);

	 memcpy(&(estadoEnEjecucion->registrosCpu.AX), stream, sizeof(estadoEnEjecucion->registrosCpu.AX));
	 stream += sizeof(estadoEnEjecucion->registrosCpu.AX);

	 memcpy(&(estadoEnEjecucion->registrosCpu.BX), stream, sizeof(estadoEnEjecucion->registrosCpu.BX));
	 stream += sizeof(estadoEnEjecucion->registrosCpu.BX);

	 memcpy(&(estadoEnEjecucion->registrosCpu.CX), stream, sizeof(estadoEnEjecucion->registrosCpu.CX));
	 stream += sizeof(estadoEnEjecucion->registrosCpu.CX);

	 memcpy(&(estadoEnEjecucion->registrosCpu.DX), stream, sizeof(estadoEnEjecucion->registrosCpu.DX));
	 stream += sizeof(estadoEnEjecucion->registrosCpu.DX);

	 memcpy(&(estadoEnEjecucion->registrosCpu.EAX), stream, sizeof(estadoEnEjecucion->registrosCpu.EAX));
	 stream += sizeof(estadoEnEjecucion->registrosCpu.EAX);

	 memcpy(&(estadoEnEjecucion->registrosCpu.EBX), stream, sizeof(estadoEnEjecucion->registrosCpu.EBX));
	 stream += sizeof(estadoEnEjecucion->registrosCpu.EBX);

	 memcpy(&(estadoEnEjecucion->registrosCpu.ECX), stream, sizeof(estadoEnEjecucion->registrosCpu.ECX));
	 stream += sizeof(estadoEnEjecucion->registrosCpu.ECX);

	 memcpy(&(estadoEnEjecucion->registrosCpu.EDX), stream, sizeof(estadoEnEjecucion->registrosCpu.EDX));
	 stream += sizeof(estadoEnEjecucion->registrosCpu.EDX);

	 memcpy(&(estadoEnEjecucion->registrosCpu.RAX), stream, sizeof(estadoEnEjecucion->registrosCpu.RAX));
	 stream += sizeof(estadoEnEjecucion->registrosCpu.RAX);

	 memcpy(&(estadoEnEjecucion->registrosCpu.RBX), stream, sizeof(estadoEnEjecucion->registrosCpu.RBX));
	 stream += sizeof(estadoEnEjecucion->registrosCpu.RBX);

	 memcpy(&(estadoEnEjecucion->registrosCpu.RCX), stream, sizeof(estadoEnEjecucion->registrosCpu.RCX));
	 stream += sizeof(estadoEnEjecucion->registrosCpu.RCX);

	 memcpy(&(estadoEnEjecucion->registrosCpu.RDX), stream, sizeof(estadoEnEjecucion->registrosCpu.RDX));
	 stream += sizeof(estadoEnEjecucion->registrosCpu.RDX);

	 //ultima intruccion
	 memcpy(&(estadoEnEjecucion->ultimaInstruccion_length), stream, sizeof(int));
	 stream += sizeof(int);

	 estadoEnEjecucion->ultimaInstruccion = malloc(estadoEnEjecucion->ultimaInstruccion_length);

	 memcpy( estadoEnEjecucion->ultimaInstruccion, stream, estadoEnEjecucion->ultimaInstruccion_length);
	 stream += estadoEnEjecucion->ultimaInstruccion_length;

	 //recurso solicitado
	 memcpy(&(estadoEnEjecucion->recursoSolicitado_length), stream, sizeof(int));
	 stream += sizeof(int);

	 estadoEnEjecucion->recursoSolicitado = malloc(estadoEnEjecucion->recursoSolicitado_length);

	 memcpy( estadoEnEjecucion->recursoSolicitado, stream, estadoEnEjecucion->recursoSolicitado_length);
	 stream += estadoEnEjecucion->recursoSolicitado_length;

	 //recurso a liberar
	 memcpy(&(estadoEnEjecucion->recursoALiberar_length), stream, sizeof(int));
	 stream += sizeof(int);

	 estadoEnEjecucion->recursoALiberar = malloc(estadoEnEjecucion->recursoALiberar_length);

	 memcpy( estadoEnEjecucion->recursoALiberar, stream, estadoEnEjecucion->recursoALiberar_length);
	 stream += estadoEnEjecucion->recursoALiberar_length;

	 //archivo
	 memcpy(&(estadoEnEjecucion->nombreArchivo_length), stream, sizeof(int));
	 stream += sizeof(int);

	 estadoEnEjecucion->nombreArchivo = malloc(estadoEnEjecucion->nombreArchivo_length);

	 memcpy( estadoEnEjecucion->nombreArchivo, stream, estadoEnEjecucion->nombreArchivo_length);
	 stream += estadoEnEjecucion->nombreArchivo_length;


	 //finalizamos
	 estadoEnEjecucion->terminaEjecutar = tomarTiempo();
	 eliminar_paquete(paquete);

}


/////////////////////////////////// INSTRUCCIONES CPU /////////////////////////////////////////

void manejar_recursos() {

	t_infopcb* unProceso = (t_infopcb*)malloc(sizeof(t_infopcb));
	memcpy(unProceso, estadoEnEjecucion, sizeof(t_infopcb));

	if (strcmp(unProceso->ultimaInstruccion, "WAIT") == 0) {
		printf("Estoy dentro de wait.\n");

		//log minimo y obligatorio
		//log_info(logger, "PID: %d - Wait: %s - Instancias: %d\n", unProceso->pid, unProceso->recursoSolicitado, );

		int i,tamanio_lista = list_size(listaRecursos);
		int recursoEncontrado = 0;
		for (i = 0; i<tamanio_lista; i++) {
			t_recursos* recurso = list_get(listaRecursos, i);

			if (string_contains(unProceso->recursoSolicitado, recurso->recurso)){
				recursoEncontrado++;

				if (recurso->instancias > 0) {
					printf("recurso asignado %s\n", recurso->recurso);
					recurso->instancias--;

					//log minimo y obligatorio
					//log_info(logger, "PID: %d - Wait: %s - Instancias: %d\n", unProceso->pid, unProceso->recursoSolicitado, recurso->instancias);

					t_recursos* unRecurso;
					unRecurso = malloc(sizeof(t_recursos));
					unRecurso->recurso = string_duplicate(recurso->recurso);
					list_add(unProceso->recursosAsignados, unRecurso);

					encolar_ready_ejecucion(unProceso);
				}
				else {
					printf("proceso bloqueado %s\n", recurso->recurso);
					estadoEnEjecucion->pid = -1; //Sino el que llega después no se ejecuta hasta que no vuelva
					//queue(&recurso->frenteBloqueados, &recurso->finBloqueados, unProceso);
					queue_push(recurso->colaBloqueados,unProceso);

					//log minimo y obligatorio
					//log_info(logger, "PID: %d - Estado Anterior: Ejecucion - Estado Actual: Bloqueado\n", unProceso->pid);
					//log minimo y obligatorio
					//log_info(logger, "PID: %d - Bloqueado por: %s>\n", unProceso->pid, unProceso->recursoSolicitado);

					cantidadElementosBloqueados++;

					printf("Cantidad de elementos bloqueados: %d\n",cantidadElementosBloqueados);

					if( cantidadElementosBloqueados  == grado_max_multiprogramación){
						printf("HAY DEADLOCK\n");
					}

				}

				//HAY Q VERIFICAR SI HAY ALGO EN READY PARA DESENCOLAR
				//PODRIAMOS HACER ESTO DIRECTAMENTE DENTRO DE DESENCOLARREADY ASI NO REPETIMOS

				if(strcmp(algoritmo_planificacion,"FIFO") == 0){

					if(frenteColaReady != NULL){
						desencolarReady();
					}
				}

				if(strcmp(algoritmo_planificacion,"HRRN") == 0){

					if( !list_is_empty(listaReady) ){
						desencolarReady();
					}
				}

			}
		}

		if (recursoEncontrado == 0) {
			int pid = unProceso->pid;
			pasarAExit();

			//Log minimo y obligaotrio
			//log_info(logger, "Finaliza el proceso &d - Motivo: Recurso no encontrado\n", unProceso->pid);
		}
	}

	else if (strcmp(unProceso->ultimaInstruccion, "SIGNAL") == 0) {
		int recursoEncontrado = 0;
		int i, tamanio_lista = list_size(listaRecursos);
		for (i = 0; i<tamanio_lista; i++) {
		t_recursos* recurso = list_get(listaRecursos, i);

		if (string_contains(unProceso->recursoALiberar,recurso->recurso )){
			recursoEncontrado++;
			printf("recurso liberado %s\n", recurso->recurso);
			recurso->instancias++;

			//log minimo y obligatorio
			//log_info(logger, "PID: %d - Signal: %s - Instancias: %d\n", unProceso->pid, unProceso->recursoSolicitado, recurso->instancias);

			list_remove_element(unProceso->recursosAsignados,recurso->recurso);


			iniciarHiloClienteCPU();
			if(!queue_is_empty(recurso->colaBloqueados)) {
				printf("proceso desbloqueado %s\n", recurso->recurso);
				encolar_ready_ejecucion(queue_pop(recurso->colaBloqueados));

				//log minimo y obligatorio
				//log_info(logger, "PID: %d - Estado Anterior: Bloqueado - Estado Actual: Ready\n", unProceso->pid);

				recurso->instancias--;

				t_recursos* unRecurso;
				unRecurso = malloc(sizeof(t_recursos));
				strcpy(unRecurso->recurso,recurso->recurso);
				list_add(unProceso->recursosAsignados, unRecurso);

				cantidadElementosBloqueados--;
				}
			}
		}
		if (recursoEncontrado == 0) {
			int pid = unProceso->pid;
			pasarAExit();

			//Log minimo y obligaotrio
			//log_info(logger, "Finaliza el proceso &d - Motivo: Recurso no encontrado\n", unProceso->pid);
		}
	}

	else if (strcmp(unProceso->ultimaInstruccion, "YIELD") == 0) {
		encolar_ready_ejecucion(unProceso);
		desencolarReady();
	}

	else if (strcmp(unProceso->ultimaInstruccion, "EXIT") == 0) {
		int pid = unProceso->pid;
		pasarAExit();

		//Log minimo y obligaotrio
		//log_info(logger, "Finaliza el proceso &d - Motivo: SUCCESS\n", unProceso->pid);
	}
	else if (strcmp(unProceso->ultimaInstruccion, "I/O") == 0) {
		//log minimo y obligatorio
		//log_info(logger, "PID: %d - Estado Anterior: Ejecucion - Estado Actual: Bloqueado\n", unProceso->pid);
		//log_info(logger, "PID: %d - Bloqueado por: I/O>\n", unProceso->pid);
		iniciarHiloIO();
	}
	else if (strcmp(unProceso->ultimaInstruccion, "F_OPEN") == 0) {

		if ( elArchivoEstaAbierto(unProceso->nombreArchivo) ){
			printf("El archivo ya se encuentra en la tabla global de archivos.\n");

			//Agrego el archivo a la tabla del proceso
			t_infoTablaArchivos* nuevoArchivo = malloc(sizeof(t_infoTablaArchivos));
			strcpy(nuevoArchivo->nombreArchivo,unProceso->nombreArchivo);
			nuevoArchivo->posicionPuntero = 0;
			list_add(unProceso->tablaArchivosAbiertos, nuevoArchivo);
			printf("Archivo agregado a la tabla del proceso.\n");

			//Bloqueo el proceso en la tabla global de archivos
			t_list_iterator* iterador = list_iterator_create(tablaGlobalArchivosAbiertos);

		    while (list_iterator_has_next(iterador)) {

		    	t_infoTablaGlobalArchivos* siguiente = list_iterator_next(iterador);
		    	if(string_contains(siguiente->nombreArchivo,unProceso->nombreArchivo)){
		    		queue_push(siguiente->colaProcesosBloqueados, unProceso);
		    		printf("Proceso bloqueado en la tabla global de archivos: %s\n",siguiente->nombreArchivo);
		    	}
//		    	if(strcmp(siguiente->nombreArchivo,unProceso->nombreArchivo) == 0){
//		    		queue_push(siguiente->colaProcesosBloqueados, unProceso);
//		    	}
		    }

		    //Desencolo ready si es que hay algun proceso en la lista
			if(strcmp(algoritmo_planificacion,"FIFO") == 0){

				if(frenteColaReady != NULL){
					desencolarReady();
				}
			}

			if(strcmp(algoritmo_planificacion,"HRRN") == 0){

				if( !list_is_empty(listaReady) ){
					desencolarReady();
				}
			}

		}else{
			printf("El archivo no se encuentra en la tabla global de archivos.\n");
			//int cod_fs
			//char* nombreArchivo
			//int posicionPuntero
			//int cantBytes
			//size_t direcFisica
			//size_t nuevoTamanioArchivo

			iniciarHiloClienteFileSystem(1,unProceso->nombreArchivo,0,0,0,0);

		}
	}
	else if (strcmp(unProceso->ultimaInstruccion, "F_CLOSE") == 0) {

		//Elimino el archivo de la tabla del proceso
		t_list_iterator* iterador = list_iterator_create(unProceso->tablaArchivosAbiertos);
	    while (list_iterator_has_next(iterador)) {

	    	t_infoTablaArchivos* siguiente = list_iterator_next(iterador);

	    	if(string_contains(siguiente->nombreArchivo,unProceso->nombreArchivo)){
	    		queue_pop(siguiente);
	    		printf("Archivo eliminado de la tabla del proceso.\n");
	    	}
//	    	if(strcmp(siguiente->nombreArchivo,unProceso->nombreArchivo) == 0){
//	    		queue_pop(siguiente);
//	    	}
	    }

	    //Encolo el proceso en Ready
	    encolar_ready_ejecucion(unProceso);

		//Elimino el archivo de la tabla global de archivos o desbloqueo un proceso
		t_list_iterator* iteradorGlobal = list_iterator_create(tablaGlobalArchivosAbiertos);

	    while (list_iterator_has_next(iteradorGlobal)) {

	    	t_infoTablaGlobalArchivos* siguiente = list_iterator_next(iteradorGlobal);
	    	if(string_contains(siguiente->nombreArchivo,unProceso->nombreArchivo)){

	    		if(queue_is_empty(siguiente->colaProcesosBloqueados)){
	    			//Elimino el archivo de la tabla global de archivos
	    			list_remove_element(tablaGlobalArchivosAbiertos, siguiente);
	    			printf("Archivo eliminado de la tabla global de archivos.\n");
	    		}
	    		else{
	    			//Desbloqueo y encolo en Ready el proceso desbloqueado
	    			encolar_ready_ejecucion(queue_pop(siguiente->colaProcesosBloqueados));
	    			printf("Archivo desbloqueado y encolado en Ready.\n");
	    		}
	    	}
	    }

	    desencolarReady();

	}
	else if (strcmp(unProceso->ultimaInstruccion, "F_SEEK") == 0) {

		//Actualizo el puntero del archivo
		t_list_iterator* iterador = list_iterator_create(unProceso->tablaArchivosAbiertos);
	    while (list_iterator_has_next(iterador)) {

	    	t_infoTablaArchivos* siguiente = list_iterator_next(iterador);

	    	if(string_contains(siguiente->nombreArchivo,unProceso->nombreArchivo)){
	    		siguiente->posicionPuntero = unProceso->posicionArchivo;
	    		printf("Puntero del archivo: %s actualizado.\n", siguiente->nombreArchivo );
	    	}
	    }
	    //Devuelvo el contexto de ejecución a la CPU para que continúe el mismo proceso.
		iniciarHiloClienteCPU();

	}
	else if (strcmp(unProceso->ultimaInstruccion, "F_READ") == 0) {
		//int cod_fs
		//char* nombreArchivo
		//int posicionPuntero
		//int cantBytes
		//size_t direcFisica
		//size_t nuevoTamanioArchivo
		int punteroArchivo = puntero(unProceso, unProceso->nombreArchivo);

		iniciarHiloClienteFileSystem(2,unProceso->nombreArchivo,punteroArchivo,unProceso->cantBytesArchivo,unProceso->direcFisicaArchivo,0);

	    //Desencolo ready si es que hay algun proceso en la lista
		if(strcmp(algoritmo_planificacion,"FIFO") == 0){

			if(frenteColaReady != NULL){
				desencolarReady();
			}
		}

		if(strcmp(algoritmo_planificacion,"HRRN") == 0){

			if( !list_is_empty(listaReady) ){
				desencolarReady();
			}
		}
	}
	else if (strcmp(unProceso->ultimaInstruccion, "F_WRITE") == 0) {
		//int cod_fs
		//char* nombreArchivo
		//int posicionPuntero
		//int cantBytes
		//size_t direcFisica
		//size_t nuevoTamanioArchivo
		iniciarHiloClienteFileSystem(3,unProceso->nombreArchivo,0,unProceso->cantBytesArchivo,unProceso->direcFisicaArchivo,0);

	    //Desencolo ready si es que hay algun proceso en la lista
		if(strcmp(algoritmo_planificacion,"FIFO") == 0){

			if(frenteColaReady != NULL){
				desencolarReady();
			}
		}

		if(strcmp(algoritmo_planificacion,"HRRN") == 0){

			if( !list_is_empty(listaReady) ){
				desencolarReady();
			}
		}
	}
	else if (strcmp(unProceso->ultimaInstruccion, "F_TRUNCATE") == 0) {
		//int cod_fs
		//char* nombreArchivo
		//int posicionPuntero
		//int cantBytes
		//size_t direcFisica
		//size_t nuevoTamanioArchivo
		iniciarHiloClienteFileSystem(3,unProceso->nombreArchivo,0,0,0,0);

		//Desencolo ready si es que hay algun proceso en la lista
		if(strcmp(algoritmo_planificacion,"FIFO") == 0){

			if(frenteColaReady != NULL){
				desencolarReady();
			}
		}

		if(strcmp(algoritmo_planificacion,"HRRN") == 0){

			if( !list_is_empty(listaReady) ){
				desencolarReady();
			}
		}
	}
	else if (strcmp(unProceso->ultimaInstruccion, "CREATE_SEGMENT") == 0) {

		//log minimo y obligatorio
		//log_info(logger, "“PID: %d - Crear Segmento - Id: <ID SEGMENTO> - Tamaño: <TAMAÑO>\n", unProceso->pid, unProceso-> , unProceso-> );

		iniciarHiloClienteMemoria(2,0);
	}
	else if (strcmp(unProceso->ultimaInstruccion, "DELETE_SEGMENT") == 0) {

		iniciarHiloClienteMemoria(3,0);

		//log minimo y obligatorio
		//log_info(logger, "“PID: %d - Eliminar Segmento - Id Segmento: <ID SEGMENTO>\n", unProceso->pid);

	}else if (strcmp(unProceso->ultimaInstruccion, "SEG_FAULT") == 0) {

		pasarAExit();
	}
	else{
		printf("Instruccion no reconocida.\n");
	}
}

void pasarAExit() {

//		Dar aviso al modulo de Memoria para que lo libere.
//		Liberar recursos que tenga asignados.

	iniciarHiloClienteMemoria(9,estadoEnEjecucion->pid);

	liberarRecursosAsignados();
	log_info(logger,"Proceso finalizado: %d\n",estadoEnEjecucion->pid);

	//Log minimo y obligatorio
	//log_info(logger, "PID: %d - Estado Anterior: Ejecucion - Estado Actual: Exit\n", estadoEnEjecucion->pid);

	//Si la cola de ready tiene elementos desencolar, si esta vacia pasar pid de estadoEnEjecucion a -1
	if(strcmp(algoritmo_planificacion,"FIFO") == 0){

		if(frenteColaReady != NULL){
			cantidadElementosSistema--;
			desencolarReady();
		}
		else{
			estadoEnEjecucion->pid = -1;
			cantidadElementosSistema--;
		}

	}
	if(strcmp(algoritmo_planificacion,"HRRN") == 0){


		if( !list_is_empty(listaReady) ){
			printf("lista ready NO vacia\n");

			cantidadElementosSistema--;
			desencolarReady();
		}
		else{
			printf("lista ready vacia\n");
			estadoEnEjecucion->pid = -1;
			cantidadElementosSistema--;
		}

	}

}

void liberarRecursosAsignados(){
	int cantidadRecursos = list_size(estadoEnEjecucion->recursosAsignados);
	int i,j;
	int tamanio_listaRecursos = list_size(listaRecursos);

	for(i=0; i<cantidadRecursos; i++){
		t_recursos* structALiberar = list_get(estadoEnEjecucion->recursosAsignados, i);
		char* recursoALiberar = structALiberar->recurso;

		for(j=0;j<tamanio_listaRecursos;j++){
			t_recursos* recurso = list_get(listaRecursos, i);

			if(string_contains(recurso->recurso,recursoALiberar)){
				recurso->instancias++;
				printf("Recurso liberado: %s\n",recursoALiberar);
			}

		}

	}
}


void iniciarHiloIO() {

	int err = pthread_create( 	&interrupcion_IO,	// puntero al thread
								NULL,
								interrupcionIO, // le paso la def de la función que quiero que ejecute mientras viva
								NULL); // argumentos de la función

	if (err != 0) {
	printf("\nNo se pudo crear el hilo de interrupcion I/O.");
	exit(7);
	}
	//printf("El hilo cliente de la Memoria se creo correctamente.");

}


void* interrupcionIO(void* ptr) {
	printf("dentro del hilo IO\n");

	//t_infopcb* unProceso = estadoEnEjecucion;
	//cualquier modificación realizada en el objeto al que apuntan unProceso o estadoEnEjecucion
	//se reflejará en ambas variables, ya que apuntan al mismo lugar en la memoria.

	t_infopcb* unProceso = (t_infopcb*)malloc(sizeof(t_infopcb));
	memcpy(unProceso, estadoEnEjecucion, sizeof(t_infopcb));


	estadoEnEjecucion->pid = -1; //Sino el que llega después no se ejecuta hasta que no vuelva
	printf("Soy el proceso: %d , pase a -1 el pid\n",unProceso->pid);

	if(strcmp(algoritmo_planificacion,"FIFO") == 0){

		if(frenteColaReady != NULL){
			desencolarReady();
			printf("Lista no vacia, proceso desencolado de ready \n");
		}
	}
	if(strcmp(algoritmo_planificacion,"HRRN") == 0){


		if( !list_is_empty(listaReady) ){
			desencolarReady();
			printf("Lista no vacia, proceso desencolado de ready \n");
		}
	}

	sleep_ms(unProceso->tiempoBloqueado);

	//Log minimo y obligatorio
	//log_info(logger, "“PID: %d - Ejecuta IO: %d\n", unProceso->pid, unProceso->tiempoBloqueado);


	printf("Proceso desbloqueado: %d\n",unProceso->pid);

	if(strcmp(algoritmo_planificacion,"FIFO") == 0){
		printf("PID DEL ESTADO EJ EJECUCION: %d\n",estadoEnEjecucion->pid);
		if(frenteColaReady == NULL && estadoEnEjecucion->pid == -1){
			encolar_ready_ejecucion(unProceso);
			printf("Después del tiempo bloqueado, proceso encolado en Ready: %d\n",unProceso->pid);
			desencolarReady();
			printf("Lista vacia, proceso desencolado de ready: %d\n",unProceso->pid);
		}
		else{
			encolar_ready_ejecucion(unProceso);
			printf("ENCOLE EN READY DESDE IO\n");
		}
	}
	else if(strcmp(algoritmo_planificacion,"HRRN") == 0){


		if( list_is_empty(listaReady) && estadoEnEjecucion->pid == -1){
			encolar_ready_ejecucion(unProceso);
			printf("Después del tiempo bloqueado, proceso encolado en Ready\n");
			desencolarReady();
			printf("Lista vacia, proceso desencolado de ready \n");
		}
		else{
			encolar_ready_ejecucion(unProceso);
			printf("ENCOLE EN READY DESDE IO\n");
		}
	}

	//Log minimo y obligaotrio
	//log_info(logger, "PID: %d - Estado Anterior: I/O - Estado Actual: Ready\n", unProceso->pid);

	return NULL;
}


int elArchivoEstaAbierto(char* nombreArchivo){
	int estaAbierto = 0;

	t_list_iterator* iterador = list_iterator_create(tablaGlobalArchivosAbiertos);

	while (list_iterator_has_next(iterador)) {

		t_infoTablaGlobalArchivos* siguiente = list_iterator_next(iterador);
		if(string_contains(siguiente->nombreArchivo,nombreArchivo)){
			estaAbierto = 1;
		}
	}

	return estaAbierto;
}


///////////////////////////// FUNCIONES PLANIFICADORES //////////////////////////////////////7

void armarPCB(t_list* lista){

	t_infopcb* nuevoPCB = malloc(sizeof(t_infopcb));
	estimacion_inicial = config_get_double_value(config, "ESTIMACION_INICIAL");

	//inicializamos estructura
	nuevoPCB->pid = pid;
	nuevoPCB->listaInstrucciones = lista;
	nuevoPCB->programCounter = 0;
	nuevoPCB->tiempoBloqueado = 0;
	nuevoPCB->ultimaInstruccion_length = 0;
	nuevoPCB->recursoSolicitado_length = 0;
	nuevoPCB->recursoALiberar_length = 0;

	nuevoPCB->nombreArchivo_length = 0;
	nuevoPCB->posicionArchivo = 0;
	nuevoPCB->cantBytesArchivo = 0;
	nuevoPCB->direcFisicaArchivo = 0;
	nuevoPCB->tamanioArchivo = 0;

	nuevoPCB->recursosAsignados = list_create();
	nuevoPCB->tablaArchivosAbiertos = list_create();
	nuevoPCB->tablaSegmentos = list_create();

	for (int i = 0; i < sizeof(nuevoPCB->registrosCpu.AX); i++) {
		nuevoPCB->registrosCpu.AX[i] = '\0';
	    }
	for (int i = 0; i < sizeof(nuevoPCB->registrosCpu.BX); i++) {
		nuevoPCB->registrosCpu.BX[i] = '\0';
		    }
	for (int i = 0; i < sizeof(nuevoPCB->registrosCpu.CX); i++) {
		nuevoPCB->registrosCpu.CX[i] = '\0';
		    }
	for (int i = 0; i < sizeof(nuevoPCB->registrosCpu.DX ); i++) {
		nuevoPCB->registrosCpu.DX[i] = '\0';
		    }
	for (int i = 0; i < sizeof(nuevoPCB->registrosCpu.EAX ); i++) {
		nuevoPCB->registrosCpu.EAX[i] = '\0';
			}
	for (int i = 0; i < sizeof(nuevoPCB->registrosCpu.EBX ); i++) {
		nuevoPCB->registrosCpu.EBX[i] = '\0';
			}
	for (int i = 0; i < sizeof(nuevoPCB->registrosCpu.ECX ); i++) {
		nuevoPCB->registrosCpu.ECX[i] = '\0';
			}
	for (int i = 0; i < sizeof(nuevoPCB->registrosCpu.EDX ); i++) {
		nuevoPCB->registrosCpu.EDX[i] = '\0';
			}
	for (int i = 0; i < sizeof(nuevoPCB->registrosCpu.RAX ); i++) {
		nuevoPCB->registrosCpu.RAX[i] = '\0';
			}
	for (int i = 0; i < sizeof(nuevoPCB->registrosCpu.RBX ); i++) {
		nuevoPCB->registrosCpu.RBX[i] = '\0';
			}
	for (int i = 0; i < sizeof(nuevoPCB->registrosCpu.RCX ); i++) {
		nuevoPCB->registrosCpu.RCX[i] = '\0';
			}
	for (int i = 0; i < sizeof(nuevoPCB->registrosCpu.RDX ); i++) {
		nuevoPCB->registrosCpu.RDX[i] = '\0';
			}

	//HRRN
	nuevoPCB->estimadoAnterior = estimacion_inicial;
	nuevoPCB->estimadoProxRafaga = 0;
	nuevoPCB->empiezaAEjecutar = 0;
	nuevoPCB->entraEnColaReady = 0;
	nuevoPCB->terminaEjecutar = 0;

	//Encolamos en NEW (FIFO)
	queue(&frenteColaNew, &finColaNew, nuevoPCB);
	printf("PCB encolado en NEW\n");

	//Log minimo y obligatorio
	//log_info(logger, "Se crea el proceso %d en NEW\n", nuevoPCB->pid);

	printf("Cola NEW:\n");
	mostrarCola(frenteColaNew);

	pid++;

	finalizarEncolarPCB();
}

void finalizarEncolarPCB(){
	encolarReady();  //Si corresponde lo encola en Ready
	printf("PID EN EJECUCION: %d\n", estadoEnEjecucion->pid );
	if(estadoEnEjecucion->pid == -1){  //Si no hay un proceso en ejecucion, lo ejecuto
		desencolarReady();
	}
}

void encolarReady() {

	// SI EL ALGORITMO DE PLANIFICACION ES FIFO VERIFICA EL GRADO MAX DE MULTIPROGRAMCIÓN Y ENCOLA EN READY SI CORRESPONDE

	if(strcmp(algoritmo_planificacion,"FIFO") == 0){

		int lugaresDisponiblesReady = grado_max_multiprogramación - cantidadElementosSistema;

		printf("Lugares disponibles en READY: %d \n",lugaresDisponiblesReady);


		if(lugaresDisponiblesReady > 0 ){

			if(frenteColaNew != NULL){

				//t_infopcb* procesoADesencolar = unqueue(&frenteColaNew,&finColaNew);
				//queue(&frenteColaReady, &finColaReady,procesoADesencolar);

				iniciarHiloClienteMemoria(5,0);
				sem_wait(&semKernelClientMemoria);
				cantidadElementosSistema++;

				lugaresDisponiblesReady = grado_max_multiprogramación - cantidadElementosSistema;
				printf("PCB encolado en READY - lugares disponibles en READY: %d \n",lugaresDisponiblesReady);
			}
		}
		else{
			printf("Grado máximo de multiprogramación alcanzado. \n");
		}

		printf("Cola NEW:\n");
		mostrarCola(frenteColaNew);
		//printf("Cola READY:\n");
		//mostrarCola(frenteColaReady);

	}




	// SI EL ALGORITMO DE PLANIFICACION ES HRRN VERIFICA EL GRADO MAX DE MULTIPROGRAMCIÓN Y ENCOLA EN READY SI CORRESPONDE

	if(strcmp(algoritmo_planificacion,"HRRN") == 0){

		printf("Cantidad de elementos en READY: %d \n",cantidadElementosSistema);


		int lugaresDisponiblesReady = grado_max_multiprogramación - cantidadElementosSistema;
		printf("Lugares disponibles en READY: %d \n",lugaresDisponiblesReady);


		if(lugaresDisponiblesReady > 0 ){

			if(frenteColaNew != NULL){

				//t_infopcb* procesoADesencolar = unqueue(&frenteColaNew,&finColaNew);
				//list_add(listaReady, procesoADesencolar);
				//procesoADesencolar->entraEnColaReady = tomarTiempo();

				iniciarHiloClienteMemoria(5,0);
				sem_wait(&semKernelClientMemoria);
				cantidadElementosSistema++;

				lugaresDisponiblesReady = grado_max_multiprogramación - cantidadElementosSistema;
				printf("PCB agregado en READY - lugares disponibles en READY: %d \n",lugaresDisponiblesReady);

				//log minimo y obligatorio
				//log_info(logger, "PID: %d - Estado Anterior: New - Estado Actual: Ready\n", procesoADesencolar->pid);

				printf("Cola READY:\n");
				t_infopcb* proceso = list_get(listaReady,0); //que queriamos hacer con esto? porque mostraria el primero?
				printf("PID: %d\n", proceso->pid);
			}

		}else{
			printf("Grado máximo de multiprogramación alcanzado. \n");
		}

		printf("Cola NEW:\n");
		mostrarCola(frenteColaNew);
		//printf("Cola READY:\n");
		//mostrarListaReady(listaReady);

	}

	return;
}

void encolar_ready_ejecucion(t_infopcb* proceso) {

	t_infopcb* unProceso = (t_infopcb*)malloc(sizeof(t_infopcb));
	memcpy(unProceso, proceso, sizeof(t_infopcb));


	if(strcmp(algoritmo_planificacion,"FIFO") == 0){
		queue(&frenteColaReady, &finColaReady, unProceso);
	}

	if(strcmp(algoritmo_planificacion,"HRRN") == 0){
		list_add(listaReady, unProceso);
		unProceso->entraEnColaReady = tomarTiempo();
	}
	return;
}


void desencolarReady (){

	 //SI EL ALGORITMO DE PLANIFICACION ES FIFO

	if(strcmp(algoritmo_planificacion,"FIFO") == 0){
		estadoEnEjecucion = unqueue(&frenteColaReady,&finColaReady);
		printf("Proceso pasado a estadoEnEjecucion por FIFO. \n");

		iniciarHiloClienteCPU();
		//printf("Cola READY:\n");
		//mostrarCola(frenteColaReady);

		printf("Proceso en ejecucion: %d\n",estadoEnEjecucion->pid);

		//log minimo y obligatorio
		//log_info(logger, "PID: %d - Estado Anterior: Ready - Estado Actual: Ejecucion\n", estadoEnEjecucion->pid);
	}


	// SI EL ALGORITMO DE PLANIFICACION ES HRRN

	if(strcmp(algoritmo_planificacion,"HRRN") == 0){


		int proxRafaga;
		int maxRafaga = 0;
		int pidMaxRafaga = 0;
		int i=0;
		int cantidadElementosReady = list_size(listaReady);
		t_infopcb* procesoActual = NULL;


		while( i < cantidadElementosReady ){

			procesoActual = list_get(listaReady,i);
			calcularHRRN(procesoActual);
			proxRafaga = procesoActual->rafaga;

			if(proxRafaga > maxRafaga){
				maxRafaga = proxRafaga;
				pidMaxRafaga = i;
			}

			i++;
		}

		printf("PID MAX RAFAGA: %d\n", pidMaxRafaga);


		estadoEnEjecucion = list_get(listaReady,pidMaxRafaga);
		iniciarHiloClienteCPU();
		list_remove(listaReady,pidMaxRafaga);

		printf("Proceso pasado a estadoEnEjecucion por HRRN. \n");
		printf("Proceso en ejecucion: %d\n",estadoEnEjecucion->pid);

		//log minimo y obligatorio
		//log_info(logger, "PID: %d - Estado Anterior: Ready - Estado Actual: Ejecucion\n", estadoEnEjecucion->pid);

		//CALCULA CUANDO SE VA DE KERNEL
		estadoEnEjecucion->empiezaAEjecutar = tomarTiempo();
	}


	return;
}

void calcularHRRN(t_infopcb* unProceso){

	//ESTA LOGICA VA CUANDO SE RECIBE EL CONTEXTO DE CPU
	//tiempo transcurridoEnCpu
	uint32_t tiempoRealCPU = unProceso->terminaEjecutar - unProceso->empiezaAEjecutar; //(falta poner terminaEjecutar)
	unProceso->estimadoProxRafaga = unProceso->estimadoAnterior * hrrn_alfa + tiempoRealCPU * (1 - hrrn_alfa);

	//tiempo transcurrido en la cola de Ready
	int tiempoEsperaReady = unProceso->empiezaAEjecutar - unProceso->entraEnColaReady;
	printf("el proceso %d tarda %d: ", unProceso->pid, tiempoEsperaReady);
	unProceso->rafaga = (tiempoEsperaReady + unProceso->estimadoProxRafaga) / unProceso->estimadoProxRafaga;

	unProceso->estimadoAnterior = unProceso->estimadoProxRafaga;

	//max R = (w + s) / s-> w = tiempo de espera en ready

	// E = ESTIMACION_INICIAL         S =  Estimacion = estimacion anterior * hrrn_alfa + real CPU (1 - hrrn_alfa)
	//estimacion anterior en la primera es estimacion inicial
	//estimacion anterior despues es S
}


////////////////////////////////////  FUNCIONES EXTRAS  ////////////////////////////////////////

t_config* iniciar_config(void){
	t_config* nuevo_config;

	return nuevo_config;
}

void iterator(char* value) {
    log_info(logger, value);
}

void paquete(int conexion){
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


char* recibir_handshake(int socket_cliente)  //MENSAJE
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	//log_info(logger, "Me llego el mensaje %s", buffer);
	return buffer;
}

void enviar_handshake_memoria(){
	int config = 1;
	int conexion_Memoria;

	conexion_Memoria = crear_conexion(ip_memoria, puerto_memoria);
	enviar_mensaje("kernel",conexion_Memoria);
//	log_info(logger, "Ingrese sus mensajes para la Memoria: ");
//	paquete(conexion_Memoria);
	int cod_op = recibir_operacion(conexion_Memoria);
	recibir_mensaje(conexion_Memoria);
	liberar_conexion(conexion_Memoria);
}


///////////////////////////////// FUNCIONES DE COLAS  ///////////////////////////////////////

void queue(t_nodoCola** frenteColaNew, t_nodoCola** finColaNew, t_infopcb* pcb) {
	t_nodoCola* nuevo = malloc(sizeof(t_nodoCola)); // Reservamos memoria para el nuevo nodo
    nuevo->info_pcb = pcb; // Guardamos el valor en el nuevo nodo
    nuevo->sgte = NULL; // El siguiente nodo de nuevo es nulo
    if (*frenteColaNew == NULL) { // Si la cola está vacía
        *frenteColaNew = nuevo; // El nuevo nodo es el frente
    } else { // Si no está vacía
        (*finColaNew)->sgte = nuevo; // El siguiente del último nodo es el nuevo nodo
    }
    *finColaNew = nuevo; // El nuevo nodo es el nuevo fin
}

t_infopcb* unqueue(t_nodoCola** frenteColaNew, t_nodoCola** finColaNew) {
	t_infopcb* pcb_puntero;
	t_nodoCola* temp;
    if (*frenteColaNew == NULL) { // Si la cola está vacía
        printf("La cola esta vacia\n");
        exit(5); //Ver como salir de la funcion
    }
    pcb_puntero = (*frenteColaNew)->info_pcb; // Obtenemos el valor del frente
    temp = *frenteColaNew; // Guardamos el frente temporalmente
    *frenteColaNew = (*frenteColaNew)->sgte; // El siguiente del frente es el nuevo frente
    if (*frenteColaNew == NULL) { // Si la cola quedó vacía
        *finColaNew = NULL; // El fin es nulo
    }
free(temp); // Liberamos la memoria del frente anterior
    return pcb_puntero; // Devolvemos el valor del frente
}

void mostrarCola(t_nodoCola* frenteColaNew) {
    printf("Contenido de la cola:\n");
    while (frenteColaNew != NULL) {
        printf("PID: %d\n", frenteColaNew->info_pcb->pid);

        printf("Lista de instrucciones:\n");
        t_list* lista = frenteColaNew->info_pcb->listaInstrucciones;
        for (int i = 0; i < list_size(lista); i++) {
        	char* instruccion = list_get(lista, i);
        	printf("\t%s\n", instruccion);
        }

        printf("Program Counter: %d\n", frenteColaNew->info_pcb->programCounter);
//        printf("Registros CPU:\n");
//        for (int i = 0; i < 12; i++) {
//            printf("Registro %d: %s\n", i, frenteColaNew->info_pcb.registrosCpu[i]);
//        }
//        printf("Tabla de segmentos:\n");
//        t_nodoTablaSegmentos* tabla = frenteColaNew->info_pcb->tablaSegmentos;
//        while (tabla != NULL) {
//        	printf("Id: %p\n", tabla->info_tablaSegmentos.id);
//            printf("Base: %p\n", tabla->info_tablaSegmentos.direccionBase);
//            printf("Base: %p\n", tabla->info_tablaSegmentos.tamanio);
//            tabla = tabla->sgte;
//        }
        printf("Estimado próxima ráfaga: %d\n", frenteColaNew->info_pcb->estimadoProxRafaga);
        printf("Tiempo llegada a Ready: %d\n", frenteColaNew->info_pcb->entraEnColaReady);
//        printf("Punteros a archivos:\n");
//        t_nodoArchivos* punteros = frenteColaNew->info_pcb->punterosArchivos;
//        while (punteros != NULL) {
//            printf("%s\n", punteros->info_archivos);
//            punteros = punteros->sgte;
//        }
        frenteColaNew = frenteColaNew->sgte;
    }
}

int cantidadElementosCola(t_nodoCola* frenteCola) {
    int contador = 0;
    t_nodoCola* temp = frenteCola;

    while (temp != NULL) {
        contador++;
        temp = temp->sgte;
    }

    return contador;
}
