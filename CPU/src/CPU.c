#include "CPU.h"
t_config* config;
int cliente_fd;


int main(void) {

	sem_init(&semCPUClientMemoria,0,0);

    logger = log_create("CPU.log", "CPU", 1, LOG_LEVEL_DEBUG);

    config = config_create("/home/utnso/tp-2023-1c-Los-operadores/CPU/CPU.config");

    if (config == NULL) {
        log_info(logger,"No se pudo crear el config.\n");
        exit(5);
    }

    ip_memoria= config_get_string_value(config, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    retardo_instruccion = config_get_int_value(config, "RETARDO_INSTRUCCION");
    tam_max_segmento = config_get_int_value(config, "TAM_MAX_SEGMENTO");

    //Inicializo contexto
    contexto = malloc(sizeof(t_contextoEjecucion));
    contexto->listaInstrucciones = list_create();
    vaciarContexto();

    //Hilo Cliente
    //iniciarHiloClienteMemoria();
    enviar_handshake_memoria();

    //thread server

	server_fd = iniciar_servidor();
	//log_info(logger, "CPU lista para escuchar al cliente\n");

	while(1){
		iniciarHiloServer();
		pthread_join(serverCPU_thread, NULL);
	}


    //pthread_join
    pthread_join(client_Memoria,NULL);
    pthread_join(serverCPU_thread,NULL);

    //libero memoria
    log_destroy(logger);
    config_destroy(config);

    return EXIT_SUCCESS;
}

void iniciarHiloClienteMemoria(int cod_memoria, char* registro, int direcFisica) {
	ClientMemoriaArgs *args = malloc(sizeof(ClientMemoriaArgs));
	args->cod_memoria = cod_memoria;
	args->registro = malloc(strlen(registro)+1);
	strcpy(args->registro,registro);
	args->direccionFisica = direcFisica;

	int err = pthread_create( 	&client_Memoria,	// puntero al thread
	     	        			NULL,
								clientMemoria, // le paso la def de la función que quiero que ejecute mientras viva
								(void *)args); // argumentos de la función

	if (err != 0) {
	log_info(logger,"No se pudo crear el hilo del cliente Memoria del CPU.\n");
	exit(7);
	}
}

void* clientMemoria(void *arg) {
	ClientMemoriaArgs *args = (ClientMemoriaArgs *)arg;
	int cod_memoria = args->cod_memoria;
	int direcFisica = args->direccionFisica;
	char* registro = args->registro;
	char* valorRegistroMO = string_new();
	int tamanio = tamanioRegistro(registro);
	char* valorRegistro = contenidoRegistro(registro);

    int conexion_Memoria;
    conexion_Memoria = crear_conexion(ip_memoria, puerto_memoria);

    t_paquete* paquete = crear_paquete_cod_operacion(cod_memoria);
        switch(cod_memoria){
        	case 11: //MOV_IN - ORDEN PARAMETROS: (PID, CPU/FS, DIRECCION, TAMAÑO)
        		char* pidMI = string_new();
        		char* CPUMI = string_new();
        		char* direcFisicaMI = string_new();
        		char* tamanioMI = string_new();

                string_append_with_format(&pidMI, "%d", contexto->pid);
                string_append_with_format(&CPUMI, "%s", "CPU");
                string_append_with_format(&direcFisicaMI, "%d", direcFisica);
                string_append_with_format(&tamanioMI, "%d", tamanio);

            	agregar_a_paquete(paquete, pidMI, strlen(pidMI)+1);
            	agregar_a_paquete(paquete, CPUMI, strlen(CPUMI)+1);
            	agregar_a_paquete(paquete, direcFisicaMI, strlen(direcFisicaMI)+1);
            	agregar_a_paquete(paquete, tamanioMI, strlen(tamanioMI)+1);

            	enviar_paquete(paquete, conexion_Memoria);


//            	printf("MOV_IN enviado a MEMORIA.\n");
//            	printf("pid enviado a Memoria: %s\n", pidMI);
//            	printf("quienSoy enviado a Memoria: %s\n", CPUMI);
//            	printf("direcFisica enviado a Memoria: %s\n", direcFisicaMI);
//            	printf("tamanio enviado a Memoria: %s\n", tamanioMI);

            	eliminar_paquete(paquete);

            break;
        	case 12: //MOV_OUT - ORDEN PARAMETROS: (PID, CPU/FS, VALOR_REGISTRO, TAMAÑO, DIRECCION)
        		char* pidMO = string_new();
				char* CPUMO = string_new();

				char* tamanioMO = string_new();
				char* direcFisicaMO = string_new();

				string_append_with_format(&pidMO, "%d", contexto->pid);
				string_append_with_format(&CPUMO, "%s", "CPU");
				string_append_with_format(&valorRegistroMO, "%s", valorRegistro);
				string_append_with_format(&tamanioMO, "%d", tamanio);
				string_append_with_format(&direcFisicaMO, "%d", direcFisica);

				agregar_a_paquete(paquete, pidMO, strlen(pidMO)+1);
				agregar_a_paquete(paquete, CPUMO, strlen(CPUMO)+1);
				agregar_a_paquete(paquete, valorRegistroMO, strlen(valorRegistroMO)+1);
				agregar_a_paquete(paquete, tamanioMO, strlen(tamanioMO)+1);
				agregar_a_paquete(paquete, direcFisicaMO, strlen(direcFisicaMO)+1);

				enviar_paquete(paquete, conexion_Memoria);

//				printf("MOV_OUT enviado a MEMORIA.\n");
//				printf("pid enviado a MEMORIA: %s\n", pidMO);
//				printf("quienSoy enviado a MEMORIA: %s\n", CPUMO);
//				printf("valorRegistro enviado a MEMORIA: %s\n", valorRegistroMO);
//				printf("direcFisica enviado a MEMORIA: %s\n", direcFisicaMO);
//				printf("tamanio enviado a MEMORIA: %s\n", tamanioMO);

                eliminar_paquete(paquete);

        	break;
    		default:
    			log_warning(logger," Operacion desconocida. NO se envió nada a Memoria.\n");
    			liberar_conexion(conexion_Memoria);
    		break;
        }
    int cod_op = recibir_operacion(conexion_Memoria);
    //printf("RECIBO OPERACION MEMORIA: %d\n", cod_op);
    switch (cod_op) {
    		case 11: //MOV_IN, RECIBO EL CONTENIDO DE LA DIRECFISICA Y LO SETTEO EN EL REGISTRO
    			char* valorLeido = recibir_handshake(conexion_Memoria);
    			set_tp(registro,valorLeido);
    			log_info(logger, "PID: %d - Acción: LEER - Segmento: %d - Dirección Física: %d - Valor leido: %s\n", contexto->pid, numSegmentoLog, direcFisica, valorLeido);
    			liberar_conexion(conexion_Memoria);
    		break;
            case 12:  //RECIBO UN OK
            	char* respuesta = recibir_handshake(conexion_Memoria);
            	log_info(logger, "PID: %d - Acción: ESCRIBIR - Segmento: %d - Dirección Física: %d - Valor escrito: %s\n", contexto->pid, numSegmentoLog, direcFisica, valorRegistroMO);
            break;
    		default:
    			log_warning(logger,"\nOperacion recibida de MEMORIA desconocida.\n");
    			liberar_conexion(conexion_Memoria);
    		break;
        }

    liberar_conexion(conexion_Memoria);
    sem_post(&semCPUClientMemoria);
	return NULL;
}



char* contenidoRegistro(char* nombreRegistro){
	char* valorRegistro = string_new();

	if (strncmp(nombreRegistro, "AX",2) == 0) {
		strcpy(valorRegistro, contexto->registrosCpu.AX);
	} else if (strncmp(nombreRegistro, "BX",2) == 0) {
		strcpy(valorRegistro, contexto->registrosCpu.BX);
	} else if (strncmp(nombreRegistro, "CX",2) == 0) {
		strcpy(valorRegistro, contexto->registrosCpu.CX);
	} else if (strncmp(nombreRegistro, "DX",2) == 0) {
		strcpy(valorRegistro, contexto->registrosCpu.DX);
	} else if (strncmp(nombreRegistro, "EAX",3) == 0) {
		strcpy(valorRegistro, contexto->registrosCpu.EAX);
	} else if (strncmp(nombreRegistro, "EBX",3) == 0) {
		strcpy(valorRegistro, contexto->registrosCpu.EBX);
	} else if (strncmp(nombreRegistro, "ECX",3) == 0) {
		strcpy(valorRegistro, contexto->registrosCpu.ECX);
	} else if (strncmp(nombreRegistro, "EDX",3) == 0) {
		strcpy(valorRegistro, contexto->registrosCpu.EDX);
	} else if (strncmp(nombreRegistro, "RAX",3) == 0) {
		strcpy(valorRegistro, contexto->registrosCpu.RAX);
	} else if (strncmp(nombreRegistro, "RBX",3) == 0) {
		strcpy(valorRegistro, contexto->registrosCpu.RBX);
	} else if (strncmp(nombreRegistro, "RCX",3) == 0) {
		strcpy(valorRegistro, contexto->registrosCpu.RCX);
	} else if (strncmp(nombreRegistro, "RDX",3) == 0) {
		strcpy(valorRegistro, contexto->registrosCpu.RDX);
	} else {
		log_info(logger,"Registro no válido.\n");
	}

	return valorRegistro;
}

int tamanioRegistro(char* registro){
	int tamanio = -1;
	if (strncmp(registro, "AX",2) == 0) {
		tamanio = 4;
	} else if (strncmp(registro, "BX",2) == 0) {
		tamanio = 4;
	} else if (strncmp(registro, "CX",2) == 0) {
		tamanio = 4;
	} else if (strncmp(registro, "DX",2) == 0) {
		tamanio = 4;
	} else if (strncmp(registro, "EAX",3) == 0) {
		tamanio = 8;
	} else if (strncmp(registro, "EBX",3) == 0) {
		tamanio = 8;
	} else if (strncmp(registro, "ECX",3) == 0) {
		tamanio = 8;
	} else if (strncmp(registro, "EDX",3) == 0) {
		tamanio = 8;
	} else if (strncmp(registro, "RAX",3) == 0) {
		tamanio = 16;
	} else if (strncmp(registro, "RBX",3) == 0) {
		tamanio = 16;
	} else if (strncmp(registro, "RCX",3) == 0) {
		tamanio = 16;
	} else if (strncmp(registro, "RDX",3) == 0) {
		tamanio = 16;
	} else {
		log_info(logger,"Registro no válido.\n");
	}

	return tamanio;
}

void iniciarHiloClienteKernel() {

	int err = pthread_create( 	&client_Kernel,	// puntero al thread
	     	        			NULL,
								clientKernel, // le paso la def de la función que quiero que ejecute mientras viva
								NULL); // argumentos de la función

	if (err != 0) {
	log_info(logger,"No se pudo crear el hilo del cliente Kernel del CPU.\n");
	exit(7);
	}
	//printf("El hilo cliente de la Memoria se creo correctamente.\n");
}

void* clientKernel(void* ptr) {
	int config=1;
	//serializarContexto(cliente_fd); //enviamos el contexto
	liberar_conexion(cliente_fd);
	return NULL;
}

void iniciarHiloServer() {

    int err = pthread_create( 	&serverCPU_thread,	// puntero al thread
    	            			NULL,
								&serverCPU, // le paso la def de la función que quiero que ejecute mientras viva
								NULL); // argumentos de la función

	 if (err != 0) {
	  log_info(logger,"No se pudo crear el hilo de la conexión kernel-CPU \n");
	  exit(7);
	 }
	 //printf("El hilo de la conexión kernel-CPU se creo correctamente.\n");
}

void* serverCPU(void* ptr){


    //int server_fd = iniciar_servidor();
	//log_info(logger, "CPU lista para recibir al cliente");
	cliente_fd = esperar_cliente(server_fd);
    int contadorContexto = 0;
    t_list* lista;
    while (1) {

    	int cod_op = recibir_operacion(cliente_fd);

    	switch (cod_op) {
    		case MENSAJE:
    			recibir_mensaje(cliente_fd);
    			break;
    		case PAQUETE:
    			lista = recibir_paquete(cliente_fd);
    			//log_info(logger, "Me llegaron los siguientes valores:\n");
    			list_iterate(lista, (void*) iterator);
    			list_destroy(lista);
    			break;
    			//COMENTADO Y CAMBIE A CONTADOCONTEXTO == 2
    		case 6: //TABLA_SEGMENTOS
    			t_list* tablaSegmentosRecibida = recibir_paquete(cliente_fd);
    			contexto->tablaSegmentos = tablaSegmentosActualizada(tablaSegmentosRecibida);
//    			log_info(logger, "Tabla de Segmentos recibida de Kernel. \n");

//            	t_list_iterator* iterador = list_iterator_create(contexto->tablaSegmentos);
//            	while (list_iterator_has_next(iterador)) {
//            		t_infoTablaSegmentos* siguiente = list_iterator_next(iterador);
//            		printf("IdSegmento: %d\n",siguiente->id);
//            		printf("Tamaño: %zu\n",siguiente->tamanio);
//            		printf("Base: %zu\n",siguiente->direccionBase);
//
//            	}
    			contadorContexto++;
    			if(contadorContexto == 3){
    				iniciar_ejecucion();
    			}
    		break;
    		case INSTRUCCIONES:
    			contexto->listaInstrucciones = recibir_paquete(cliente_fd);
    			contadorContexto++;
//    			log_info(logger, "Instrucciones recibidas de Kernel:\n");
//    			list_iterate(contexto->listaInstrucciones, (void*) iterator);
//    			if(contadorContexto == 3){
//    				iniciar_ejecucion();
//    			}
    			break;
    		case CONTEXTO:
    			//printf("CONTEXTO RECIBIDO\n");
    			contexto = recibir_contexto(cliente_fd);
    			contadorContexto++;
//    			printf("pid recibido de Kernel = %d\n",contexto->pid);
//    			printf("programCounter recibido de Kernel = %d\n",contexto->programCounter);
//    			printf("AX recibido de Kernel = %s\n",contexto->registrosCpu.AX);
//    			printf("CX recibido de Kernel = %s\n",contexto->registrosCpu.BX);
//    			printf("BX recibido de Kernel = %s\n",contexto->registrosCpu.CX);
//    			printf("DX recibido de Kernel = %s\n",contexto->registrosCpu.DX);
//
//    			printf("EAX recibido de Kernel = %s\n",contexto->registrosCpu.EAX);
//    			printf("EBX recibido de Kernel = %s\n",contexto->registrosCpu.EBX);
//    			printf("ECX recibido de Kernel = %s\n",contexto->registrosCpu.ECX);
//    			printf("EDX recibido de Kernel = %s\n",contexto->registrosCpu.EDX);
//
//    			printf("RAX recibido de Kernel = %s\n",contexto->registrosCpu.RAX);
//    			printf("RBX recibido de Kernel = %s\n",contexto->registrosCpu.RBX);
//    			printf("RCX recibido de Kernel = %s\n",contexto->registrosCpu.RCX);
//    			printf("RDX recibido de Kernel = %s\n",contexto->registrosCpu.RDX);

    			if(contadorContexto == 3){
    				iniciar_ejecucion();
    			}

    			break;
    		case -1:
    			//log_error(logger, "el kernel se desconecto. Terminando servidor\n");
    			return EXIT_FAILURE;
			default:
				log_warning(logger,"Operacion desconocida. No quieras meter la pata\n");
				break;
    	}
    }

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

void vaciarContexto(){
	contexto->instruccion_length = 0;
	contexto->programCounter = 0;
	contexto->tiempoBloqueado = 0;
	contexto->nombreArchivo_length = 0;
	contexto->posicionArchivo = 0;
	contexto->cantBytesArchivo = 0;
	contexto->direcFisicaArchivo = 0;
	contexto->tamanioArchivo = 0;
	contexto->idSegmento = 0;
	contexto->tamanioSegmento = 0;

	for (int i = 0; i < sizeof(contexto->registrosCpu.AX); i++) {
		contexto->registrosCpu.AX[i] = '\0';
	}
	for (int i = 0; i < sizeof(contexto->registrosCpu.BX); i++) {
		contexto->registrosCpu.BX[i] = '\0';
	}
	for (int i = 0; i < sizeof(contexto->registrosCpu.CX); i++) {
		contexto->registrosCpu.CX[i] = '\0';
	}
	for (int i = 0; i < sizeof(contexto->registrosCpu.DX ); i++) {
		contexto->registrosCpu.DX[i] = '\0';
	}
	for (int i = 0; i < sizeof(contexto->registrosCpu.EAX ); i++) {
		contexto->registrosCpu.EAX[i] = '\0';
	}
	for (int i = 0; i < sizeof(contexto->registrosCpu.EBX ); i++) {
		contexto->registrosCpu.EBX[i] = '\0';
	}
	for (int i = 0; i < sizeof(contexto->registrosCpu.ECX ); i++) {
		contexto->registrosCpu.ECX[i] = '\0';
	}
	for (int i = 0; i < sizeof(contexto->registrosCpu.EDX ); i++) {
		contexto->registrosCpu.EDX[i] = '\0';
	}
	for (int i = 0; i < sizeof(contexto->registrosCpu.RAX ); i++) {
		contexto->registrosCpu.RAX[i] = '\0';
	}
	for (int i = 0; i < sizeof(contexto->registrosCpu.RBX ); i++) {
		contexto->registrosCpu.RBX[i] = '\0';
	}
	for (int i = 0; i < sizeof(contexto->registrosCpu.RCX ); i++) {
		contexto->registrosCpu.RCX[i] = '\0';
	}
	for (int i = 0; i < sizeof(contexto->registrosCpu.RDX ); i++) {
		contexto->registrosCpu.RDX[i] = '\0';
	}

	return;

}

void iniciar_ejecucion(){
	int continuarLeyendo = 1;
    contexto->instruccion = string_new();
    contexto->recursoSolicitado = string_new();
    contexto->recursoALiberar = string_new();
    contexto->nombreArchivo = string_new();

	while(continuarLeyendo>0){

		//list_get retorna el contenido de una posicion determianda de la lista
		char* proximaInstruccion = string_new();
		proximaInstruccion = list_get(contexto->listaInstrucciones, contexto->programCounter);

		//printf("INSTRUCCION A EJECUTAR: %s\n", proximaInstruccion );

		//ejecutarFuncion: ejecuta la función que corresponde y retorna un int para saber si debe seguir ejecutando
		continuarLeyendo = ejecutarFuncion(proximaInstruccion);

	}

	//Una vez que no se deba seguir ejecutando va a serializar el contexto actualizado y lo va a enviar
//	printf("FIN DE INSTRUCCIONES \n");
//	printf("ULTIMA INTRUCCION EJECUTADA: %s\n",contexto->instruccion);
//	printf("PROGRAM COUNTER: %i\n",contexto->programCounter);
//	printf("TIEMPO BLOQUEADO: %i\n",contexto->tiempoBloqueado);
//	printf("RECURSO SOLICITADO: %s\n",contexto->recursoSolicitado);
//	printf("RECURSO A LIBERAR: %s\n",contexto->recursoALiberar);
//	printf("ID SEGMENTO: %i\n",contexto->idSegmento);
//	printf("TAMANIO SEGMENTO: %i\n",contexto->tamanioSegmento);
//	printf("AX = %s\n",contexto->registrosCpu.AX);
//	printf("BX = %s\n",contexto->registrosCpu.BX);
//	printf("CX = %s\n",contexto->registrosCpu.CX);
//	printf("DX = %s\n",contexto->registrosCpu.DX);
//
//	printf("EAX = %s\n",contexto->registrosCpu.EAX);
//	printf("EBX = %s\n",contexto->registrosCpu.EBX);
//	printf("ECX = %s\n",contexto->registrosCpu.ECX);
//	printf("EDX = %s\n",contexto->registrosCpu.EDX);
//
//	printf("RAX = %s\n",contexto->registrosCpu.RAX);
//	printf("RBX = %s\n",contexto->registrosCpu.RBX);
//	printf("RCX = %s\n",contexto->registrosCpu.RCX);
//	printf("RDX = %s\n",contexto->registrosCpu.RDX);
//
//	printf("ARCHIVO = %s\n",contexto->nombreArchivo);
//	printf("POSICION = %d\n",contexto->posicionArchivo);
//	printf("CANTIDAD BYTES = %d\n",contexto->cantBytesArchivo);
//	printf("DIRECCIÓN FÍSICA = %d\n",contexto->direcFisicaArchivo);
//	printf("TAMAÑO  = %d\n",contexto->tamanioArchivo);

	serializarContexto(cliente_fd);

	return;
}

void serializarContexto(int unSocket){

	contexto->instruccion_length = strlen(contexto->instruccion)+1;
	//recursos
	contexto->recursoALiberar_length = strlen(contexto->recursoALiberar)+1;
	contexto->recursoSolicitado_length = strlen(contexto->recursoSolicitado)+1;
	//archivo
	contexto->nombreArchivo_length = strlen(contexto->nombreArchivo)+1;

	//BUFFER
	t_buffer* buffer = malloc(sizeof(t_buffer));

	                                //length
	buffer->size = sizeof(int)*8 + sizeof(int)*4 + sizeof(contexto->registrosCpu.AX) * 4 + sizeof(contexto->registrosCpu.EAX) *4 + sizeof(contexto->registrosCpu.RAX)*4 + contexto->instruccion_length + contexto->recursoALiberar_length + contexto->recursoSolicitado_length + contexto->nombreArchivo_length;


	void* stream = malloc(buffer->size);
	int offset = 0; //desplazamiento

	memcpy(stream + offset, &contexto->programCounter, sizeof(int));
	offset += sizeof(int);

	memcpy(stream + offset, &contexto->tiempoBloqueado, sizeof(int));
	offset += sizeof(int);

	memcpy(stream + offset, &contexto->posicionArchivo, sizeof(int));
	offset += sizeof(int);

	memcpy(stream + offset, &contexto->cantBytesArchivo, sizeof(int));
	offset += sizeof(int);

	memcpy(stream + offset, &contexto->direcFisicaArchivo, sizeof(int));
	offset += sizeof(int);

	memcpy(stream + offset, &contexto->tamanioArchivo, sizeof(int));
	offset += sizeof(int);

	memcpy(stream + offset, &contexto->idSegmento, sizeof(int));
	offset += sizeof(int);

	memcpy(stream + offset, &contexto->tamanioSegmento, sizeof(int));
	offset += sizeof(int);

	memcpy(stream + offset, &contexto->registrosCpu.AX, sizeof(contexto->registrosCpu.AX));
	offset += sizeof(contexto->registrosCpu.AX);

	memcpy(stream + offset, &contexto->registrosCpu.BX, sizeof(contexto->registrosCpu.BX));
	offset += sizeof(contexto->registrosCpu.BX);

	memcpy(stream + offset, &contexto->registrosCpu.CX, sizeof(contexto->registrosCpu.CX));
	offset += sizeof(contexto->registrosCpu.CX);

	memcpy(stream + offset, &contexto->registrosCpu.DX, sizeof(contexto->registrosCpu.DX));
	offset += sizeof(contexto->registrosCpu.DX);

	memcpy(stream + offset, &contexto->registrosCpu.EAX, sizeof(contexto->registrosCpu.EAX));
	offset += sizeof(contexto->registrosCpu.EAX);

	memcpy(stream + offset, &contexto->registrosCpu.EBX, sizeof(contexto->registrosCpu.EBX));
	offset += sizeof(contexto->registrosCpu.EBX);

	memcpy(stream + offset, &contexto->registrosCpu.ECX, sizeof(contexto->registrosCpu.ECX));
	offset += sizeof(contexto->registrosCpu.ECX);

	memcpy(stream + offset, &contexto->registrosCpu.EDX, sizeof(contexto->registrosCpu.EDX));
	offset += sizeof(contexto->registrosCpu.EDX);

	memcpy(stream + offset, &contexto->registrosCpu.RAX, sizeof(contexto->registrosCpu.RAX));
	offset += sizeof(contexto->registrosCpu.RAX);

	memcpy(stream + offset, &contexto->registrosCpu.RBX, sizeof(contexto->registrosCpu.RBX));
	offset += sizeof(contexto->registrosCpu.RBX);

	memcpy(stream + offset, &contexto->registrosCpu.RCX, sizeof(contexto->registrosCpu.RCX));
	offset += sizeof(contexto->registrosCpu.RCX);

	memcpy(stream + offset, &contexto->registrosCpu.RDX, sizeof(contexto->registrosCpu.RDX));
	offset += sizeof(contexto->registrosCpu.RDX);

	//instruccion
	memcpy(stream + offset, &contexto->instruccion_length, sizeof(int));
	offset += sizeof(int);

	memcpy(stream + offset, contexto->instruccion, contexto->instruccion_length);
	offset += contexto->instruccion_length;

	//recurso solicitado
	memcpy(stream + offset, &contexto->recursoSolicitado_length, sizeof(int));
	offset += sizeof(int);

	memcpy(stream + offset, contexto->recursoSolicitado, contexto->recursoSolicitado_length);
	offset += contexto->recursoSolicitado_length;

	//recurso a liberar
	memcpy(stream + offset, &contexto->recursoALiberar_length, sizeof(int));
	offset += sizeof(int);

	memcpy(stream + offset, contexto->recursoALiberar, contexto->recursoALiberar_length);
	offset += contexto->recursoALiberar_length;

	//nombre del archivo
	memcpy(stream + offset, &contexto->nombreArchivo_length, sizeof(int));
	offset += sizeof(int);

	memcpy(stream + offset, contexto->nombreArchivo, contexto->nombreArchivo_length);
	offset += contexto->nombreArchivo_length;



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


	//printf("Contexto actualizado enviado a KERNEL. \n");

	//free memoria dinámica
	free(contexto->instruccion);
	free(contexto->recursoSolicitado);
	free(contexto->recursoALiberar);
	free(contexto->nombreArchivo);

	// Liberamos la memoria
	free(a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);

	return;
}


int ejecutarFuncion(char* proximaInstruccion){

	int continuarLeyendo = 0;
	//REVISAR: NO NOS SIRVE STRING_SPLIT, VAMOS A TENER QUE CREAR UNA FUNCIÓN NOSOTROS
	char** arrayInstruccion= string_array_new();
	arrayInstruccion = string_split(proximaInstruccion, " ");
	char* nombreInstruccion = arrayInstruccion[0];

	contexto->programCounter++;

    if (strcmp(nombreInstruccion, "SET") == 0) {
		char* setParam1 = string_new();
		char* setParam2= string_new();
		setParam1 = string_duplicate(arrayInstruccion[1]);
		setParam2 = string_duplicate(arrayInstruccion[2]);



		//log minimo y obligatorio
		log_info(logger, "PID: %d - Ejecutando: SET - %s, %s\n", contexto->pid, setParam1, setParam2);

		set_tp(setParam1, setParam2);


		free(setParam1);
		free(setParam2);
		continuarLeyendo = 1;


    } else if (  string_contains(nombreInstruccion,"YIELD")  ) {
    	//log minimo y obligatorio
    	log_info(logger, "PID: %d - Ejecutando: YIELD\n", contexto->pid);
    	yield_tp();
    } else if (string_contains(nombreInstruccion,"EXIT")) {
    	exit_tp();

    	//log minimo y obligatorio
    	log_info(logger, "PID: %d - Ejecutando: EXIT\n", contexto->pid);
    } else if (strcmp(nombreInstruccion, "I/O") == 0) {
    	int ioParam = atoi(arrayInstruccion[1]);
    	//log minimo y obligatorio
    	log_info(logger, "PID: %d - Ejecutando: I/O - [%d]\n", contexto->pid, ioParam);
    	i_o_tp(ioParam);
    } else if (strcmp(nombreInstruccion, "WAIT") == 0) {
    	char* recursoWait = string_new();
    	recursoWait = string_duplicate(arrayInstruccion[1]);
    	//log minimo y obligatorio
    	log_info(logger, "PID: %d - Ejecutando: WAIT - %s\n", contexto->pid, recursoWait);
    	wait_tp(recursoWait);
    } else if (strcmp(nombreInstruccion, "SIGNAL") == 0) {
    	char* recursoSignal = string_new();
    	recursoSignal = string_duplicate(arrayInstruccion[1]);
    	//log minimo y obligatorio
    	log_info(logger, "PID: %d - Ejecutando: SIGNAL - %s\n", contexto->pid, recursoSignal);
    	signal_tp(recursoSignal);
    } else if (strcmp(nombreInstruccion, "MOV_IN") == 0) {

    	char* mov_in_param1 = string_new();
    	mov_in_param1 = string_duplicate(arrayInstruccion[1]);

    	int mov_in_param2 = atoi(arrayInstruccion[2]);

    	//log minimo y obligatorio
    	log_info(logger, "PID: %d - Ejecutando: MOV_IN - %s, %d\n", contexto->pid, mov_in_param1, mov_in_param2);

    	int direcFisica = mov_in_tp(mov_in_param1,mov_in_param2);

    	free(mov_in_param1);

    	if(direcFisica>=0){
    		continuarLeyendo = 1;
    	}

    } else if (strcmp(nombreInstruccion, "MOV_OUT") == 0) {

    	int mov_out_param1 = atoi(arrayInstruccion[1]);

    	char* mov_out_param2 = string_new();
    	mov_out_param2 = string_duplicate(arrayInstruccion[2]);

    	//log minimo y obligatorio
    	log_info(logger, "PID: %d - Ejecutando: MOV_OUT - %d, %s\n", contexto->pid, mov_out_param1, mov_out_param2);


    	int direcFisica = mov_out_tp(mov_out_param1,mov_out_param2);

    	free(mov_out_param2);

    	if(direcFisica>=0){
    		continuarLeyendo = 1;
    	}


    } else if (strcmp(nombreInstruccion, "F_OPEN") == 0) {
    	char* fopenParam1 = string_new();
    	fopenParam1 = string_duplicate(arrayInstruccion[1]);
    	//log minimo y obligatorio
    	log_info(logger, "PID: %d - Ejecutando: F_OPEN - [%s]\n", contexto->pid, fopenParam1);
    	fopen_tp(fopenParam1);
    	free(fopenParam1);

    } else if (strcmp(nombreInstruccion, "F_CLOSE") == 0) {
    	char* fcloseParam1 = string_new();
    	fcloseParam1 = string_duplicate(arrayInstruccion[1]);
    	//log minimo y obligatorio
    	log_info(logger, "PID: %d - Ejecutando: F_CLOSE - [%s]\n", contexto->pid, fcloseParam1);
    	fclose_tp(fcloseParam1);

    	free(fcloseParam1);
    } else if (strcmp(nombreInstruccion, "F_SEEK") == 0) {
    	char* fseekParam1 = string_new();
    	fseekParam1 = string_duplicate(arrayInstruccion[1]);
    	int fseekParam2 = atoi(arrayInstruccion[2]);

    	//log minimo y obligatorio
    	log_info(logger, "PID: %d - Ejecutando: F_SEEK - [%s, %d]\n", contexto->pid, fseekParam1, fseekParam2);
    	fseek_tp(fseekParam1,fseekParam2);
    	free(fseekParam1);
    } else if (strcmp(nombreInstruccion, "F_READ") == 0) {
    	char* freadParam1 = string_new();
    	freadParam1 = string_duplicate(arrayInstruccion[1]);

    	int freadParam2 = atoi(arrayInstruccion[2]);
    	int freadParam3 = atoi(arrayInstruccion[3]);
    	//log minimo y obligatorio
    	log_info(logger, "PID: %d - Ejecutando: F_READ - [%s, %d, %d]\n", contexto->pid, freadParam1, freadParam2, freadParam3);
    	fread_tp(freadParam1,freadParam2,freadParam3);

    	free(freadParam1);
    } else if (strcmp(nombreInstruccion, "F_WRITE") == 0) {
    	char* fwriteParam1 = string_new();
    	fwriteParam1 = string_duplicate(arrayInstruccion[1]);

    	int fwriteParam2 = atoi(arrayInstruccion[2]);
    	int fwriteParam3 = atoi(arrayInstruccion[3]);

    	//log minimo y obligatorio
    	log_info(logger, "PID: %d - Ejecutando: F_WRITE - [%s, %d, %d]\n", contexto->pid, fwriteParam1, fwriteParam2, fwriteParam3);
    	fwrite_tp(fwriteParam1,fwriteParam2,fwriteParam3);

    	free(fwriteParam1);
    } else if (strcmp(nombreInstruccion, "F_TRUNCATE") == 0) {
    	char* ftruncateParam1 = string_new();
    	ftruncateParam1 = string_duplicate(arrayInstruccion[1]);
    	int ftruncateParam2 = atoi(arrayInstruccion[2]);

    	//log minimo y obligatorio
    	log_info(logger, "PID: %d - Ejecutando: F_TRUNCATE - [%s, %d]\n", contexto->pid, ftruncateParam1, ftruncateParam2);
    	ftruncate_tp(ftruncateParam1,ftruncateParam2);

    	free(ftruncateParam1);
    } else if (strcmp(nombreInstruccion, "CREATE_SEGMENT") == 0) {
    	int createParam1 = atoi(arrayInstruccion[1]);
    	int createParam2 = atoi(arrayInstruccion[2]);

    	//log minimo y obligatorio
    	log_info(logger, "PID: %d - Ejecutando: CREATE_SEGMENT - [%d, %d]\n", contexto->pid, createParam1, createParam2);

    	createSeg_tp(createParam1,createParam2);
    } else if (strcmp(nombreInstruccion, "DELETE_SEGMENT") == 0) {
    	int deleteParam1 = atoi(arrayInstruccion[1]);

    	//log minimo y obligatorio
    	log_info(logger, "PID: %d - Ejecutando: DELETE_SEGMENT - [%d]\n", contexto->pid, deleteParam1);
    	deleteSeg_tp(deleteParam1);
    } else {
        log_info(logger,"Instruccion no reconocida.\n");
    }
	return continuarLeyendo;
}

t_list* tablaSegmentosActualizada(t_list* tablaSegmentosRecibida){

	t_list_iterator* iterador = list_iterator_create(tablaSegmentosRecibida);
	t_list* tablaSegmentosActualizadaLista = list_create();

	char** arraySegmento = string_array_new();

	while (list_iterator_has_next(iterador)) {

		t_infoTablaSegmentos* nuevoSegmento = malloc(sizeof(t_infoTablaSegmentos));
		nuevoSegmento->id = 0;
		nuevoSegmento->direccionBase = (size_t)0;
		nuevoSegmento->tamanio = (size_t)0;

		char* siguiente = list_iterator_next(iterador);

		arraySegmento = string_split(siguiente, " ");

		int idSegmento = atoi(arraySegmento[0]);
		int tamanioSegmento = atoi(arraySegmento[1]);
		int baseSegmento = atoi(arraySegmento[2]);

//    	printf("Datos del segmento DENTRO DE LA FUNCION: \n");
//    	printf("idSegmento: %d\n",idSegmento);
//    	printf("tamanioSegmento: %d\n",tamanioSegmento);
//    	printf("baseSegmento: %d\n",baseSegmento);

		nuevoSegmento->id = idSegmento;
		nuevoSegmento->direccionBase = baseSegmento;
		nuevoSegmento->tamanio = tamanioSegmento;

		list_add(tablaSegmentosActualizadaLista,nuevoSegmento);
	 }

	return tablaSegmentosActualizadaLista;
}


int MMU(int direcLogica, int cantBytes){

	int error = -2;
	int num_segmento = floor(direcLogica / tam_max_segmento);
	int desplazamiento_segmento = direcLogica % tam_max_segmento;

	numSegmentoLog = num_segmento;
//	printf("num_segmento: %d\n",num_segmento); // = al id del segmento
//	printf("desplazamiento_segmento: %d\n",desplazamiento_segmento);

	if(  (desplazamiento_segmento + cantBytes) > tam_max_segmento  ){
		log_info(logger, "PID: %d - Error SEG_FAULT- Segmento: %d - Offset: %d - Tamaño: %d\n", contexto->pid, num_segmento, desplazamiento_segmento, cantBytes);
		return -1;
	}

	t_list_iterator* iterador = list_iterator_create(contexto->tablaSegmentos);

	while (list_iterator_has_next(iterador)) {
		t_infoTablaSegmentos* siguiente = list_iterator_next(iterador);

		if(num_segmento == siguiente->id){
			return siguiente->direccionBase + desplazamiento_segmento;
		}
	}

	return error;
}

// FUNCIONES INSTRUCCIONES

//MOV_IN (Registro, Dirección Lógica): Lee el valor de memoria correspondiente a la Dirección Lógica y lo almacena en el Registro.
int mov_in_tp(char* registro, int direccionLogica){

	int cantBytes = tamanioRegistro(registro); //TAMAÑO
	int direcFisica = MMU(direccionLogica,cantBytes);

	if(direcFisica == -1){
		contexto->instruccion = string_duplicate("SEG_FAULT");
	}else{
		iniciarHiloClienteMemoria(11,registro,direcFisica);
		sem_wait(&semCPUClientMemoria);
		contexto->instruccion = string_duplicate("MOV_IN");
	}

    return direcFisica;
}

//MOV_OUT (Dirección Lógica, Registro): Lee el valor del Registro y lo escribe en la dirección física de memoria obtenida a partir de la Dirección Lógica.
int mov_out_tp(int direccionLogica, char* registro){

	int cantBytes = tamanioRegistro(registro); //TAMAÑO
	int direcFisica = MMU(direccionLogica,cantBytes);
	if(direcFisica == -1){
		contexto->instruccion = string_duplicate("SEG_FAULT");
	}else{
		iniciarHiloClienteMemoria(12,registro,direcFisica);
		sem_wait(&semCPUClientMemoria);
		contexto->instruccion = string_duplicate("MOV_OUT");
	}

    return direcFisica;
}


//CREATE_SEGMENT (Id del Segmento, Tamaño): Esta instrucción solicita al kernel la creación del segmento con el Id y tamaño indicado por parámetro.
void createSeg_tp(int idSegmento, int tamanioSegmento){

	contexto->idSegmento = idSegmento;
	contexto->tamanioSegmento = tamanioSegmento;
	contexto->instruccion = string_duplicate("CREATE_SEGMENT");
    return;
}


//DELETE_SEGMENT (Id del Segmento): Esta instrucción solicita al kernel que se elimine el segmento cuyo Id se pasa por parámetro.
void deleteSeg_tp(int idSegmento){

	contexto->idSegmento = idSegmento;
	contexto->instruccion = string_duplicate("DELETE_SEGMENT");
    return;
}

//F_OPEN (Nombre Archivo): Esta instrucción solicita al kernel que abra o cree el archivo pasado por parámetro.
void fopen_tp(char* archivo){
	contexto->nombreArchivo = string_duplicate(archivo);
	contexto->instruccion = string_duplicate("F_OPEN");
    return;
}

//F_CLOSE (Nombre Archivo): Esta instrucción solicita al kernel que cierre el archivo pasado por parámetro.
void fclose_tp(char* archivo){
	contexto->nombreArchivo = string_duplicate(archivo);
	contexto->instruccion = string_duplicate("F_CLOSE");
    return;
}

//F_SEEK (Nombre Archivo, Posición): Esta instrucción solicita al kernel actualizar el puntero del archivo a la posición pasada por parámetro.
void fseek_tp(char* archivo, int posicion){
	contexto->nombreArchivo = string_duplicate(archivo);
	contexto->posicionArchivo = posicion;
	contexto->instruccion = string_duplicate("F_SEEK");
    return;
}

//F_READ (Nombre Archivo, Dirección Lógica, Cantidad de Bytes): Esta instrucción solicita al Kernel que se lea del archivo indicado, la cantidad de bytes pasada por parámetro y se escriba en la dirección física de Memoria la información leída.
void fread_tp(char* archivo, int direcLogica, int cantBytes){
	contexto->nombreArchivo = string_duplicate(archivo);
	//printf("Nombre del archivo: %s\n", contexto->nombreArchivo);

	int direcFisica = MMU(direcLogica, cantBytes);

	if (direcFisica == -1){
		contexto->instruccion = string_duplicate("SEG_FAULT");
	}
	else{
		contexto->direcFisicaArchivo = direcFisica;
		contexto->cantBytesArchivo = cantBytes;
		contexto->instruccion = string_duplicate("F_READ");
	}

    return;
}

//F_WRITE (Nombre Archivo, Dirección Lógica, Cantidad de bytes): Esta instrucción solicita al Kernel que se escriba en el archivo indicado, la cantidad de bytes pasada por parámetro cuya información es obtenida a partir de la dirección física de Memoria.
void fwrite_tp(char* archivo, int direcLogica, int cantBytes){
	contexto->nombreArchivo = string_duplicate(archivo);


	int direcFisica = MMU(direcLogica, cantBytes);

	if (direcFisica == -1){
		contexto->instruccion = string_duplicate("SEG_FAULT");
	}else if(direcFisica == -2){
		//printf("ERROR AL CALCULAR LA DIRECCION FÍSICA");
	}
	else{
		contexto->direcFisicaArchivo = direcFisica;
		contexto->cantBytesArchivo = cantBytes;
		contexto->instruccion = string_duplicate("F_WRITE");
	}

    return;
}

//F_TRUNCATE (Nombre Archivo, Tamaño): Esta instrucción solicita al Kernel que se modifique el tamaño del archivo al indicado por parámetro.
void ftruncate_tp(char* archivo, int tamanio){
	contexto->nombreArchivo = string_duplicate(archivo);
	contexto->tamanioArchivo = tamanio;
	contexto->instruccion = string_duplicate("F_TRUNCATE");
    return;
}


// YIELD: Esta instrucción desaloja voluntariamente el proceso de la CPU.
void yield_tp(){
	contexto->instruccion = string_duplicate("YIELD");
    return;
}

// EXIT: Esta instrucción representa la syscall de finalización del proceso.
void exit_tp(){
	contexto->instruccion = string_duplicate("EXIT");
	return;
}

//I/O (Tiempo): Esta instrucción representa una syscall de I/O bloqueante.
//Se deberá devolver el Contexto de Ejecución actualizado al Kernel junto a la cantidad de unidades de tiempo que va a bloquearse el proceso
void i_o_tp(int tiempoBloqueado) {
	contexto->instruccion = string_duplicate("I/O");
	contexto->tiempoBloqueado = tiempoBloqueado;
	return;
}

////WAIT (Recurso): Esta instrucción solicita al Kernel que se asigne una instancia del recurso indicado por parámetro.
void wait_tp(char* recurso) {
	contexto->instruccion = string_duplicate("WAIT");
	contexto->recursoSolicitado = string_duplicate(recurso);
	return;
}

//SIGNAL (Recurso): Esta instrucción solicita al Kernel que se libere una instancia del recurso indicado por parámetro.
void signal_tp(char* recurso) {
	contexto->instruccion = string_duplicate("SIGNAL");
	contexto->recursoALiberar = string_duplicate(recurso);
	return;
}


// SET: (Registro, Valor): Asigna al registro el valor pasado como parámetro.
void set_tp(char* registro, char* valor){

	//ESTABA ANTES DE INVOCAR SET_TP EN LA INSTRUCCION SET
	int tamanioValor = string_length(valor);
	valor[tamanioValor-1] = '\0';

	sleep_ms(retardo_instruccion);
//	printf("REGISTRO A SETEAR: %s\n", registro);
//	printf("VALOR A SETEAR: %s\n", valor);

	if (strcmp(registro, "AX") == 0) {
		strcpy(contexto->registrosCpu.AX,valor);
//		printf("AX MODIFICADO\n");

	} else if (strcmp(registro, "BX") == 0) {
		strcpy(contexto->registrosCpu.BX,valor);
//		printf("BX MODIFICADO\n");

	} else if (strcmp(registro, "CX") == 0) {
		strcpy(contexto->registrosCpu.CX,valor);
//		printf("CX MODIFICADO\n");

	} else if (strcmp(registro, "DX") == 0) {
		strcpy(contexto->registrosCpu.DX,valor);
//		printf("DX MODIFICADO\n");

	} else if (strcmp(registro, "EAX") == 0) {
		strcpy(contexto->registrosCpu.EAX,valor);
//		printf("EAX MODIFICADO\n");

	} else if (strcmp(registro, "EBX") == 0) {
		strcpy(contexto->registrosCpu.EBX,valor);
//		printf("EBX MODIFICADO\n");

	} else if (strcmp(registro, "ECX") == 0) {
		strcpy(contexto->registrosCpu.ECX,valor);
//		printf("ECX MODIFICADO\n");

	} else if (strcmp(registro, "EDX") == 0) {
		strcpy(contexto->registrosCpu.EDX,valor);
//		printf("EDX MODIFICADO\n");

	} else if (strcmp(registro, "RAX") == 0) {
		strcpy(contexto->registrosCpu.RAX,valor);
//		printf("RAX MODIFICADO\n");

	} else if (strcmp(registro, "RBX") == 0) {
		strcpy(contexto->registrosCpu.RBX,valor);
//		printf("RBX MODIFICADO\n");

	} else if (strcmp(registro, "RCX") == 0) {
		strcpy(contexto->registrosCpu.RCX,valor);
//		printf("RCX MODIFICADO\n");

	} else if (strcmp(registro, "RDX") == 0) {
		strcpy(contexto->registrosCpu.RDX,valor);
//		printf("RDX MODIFICADO\n");
	} else {
		log_info(logger,"Registro no válido.\n");
	}

	contexto->instruccion = string_duplicate("SET");

	return;
}
void enviar_handshake_memoria(){
	int config = 1;
	int conexion_Memoria;

	conexion_Memoria = crear_conexion(ip_memoria, puerto_memoria);
	enviar_mensaje("CPU",conexion_Memoria);
	int cod_op = recibir_operacion(conexion_Memoria);
	recibir_mensaje(conexion_Memoria);
	liberar_conexion(conexion_Memoria);
}
