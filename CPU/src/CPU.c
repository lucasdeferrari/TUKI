#include "CPU.h"
t_config* config;
int cliente_fd;


int main(void) {

	sem_init(&semCPUServer,0,1);
	sem_init(&semCPUClientMemoria,0,0);

    logger = log_create("CPU.log", "CPU", 1, LOG_LEVEL_DEBUG);

    config = config_create("/home/utnso/tp-2023-1c-Los-operadores/CPU/CPU.config");

    if (config == NULL) {
        printf("No se pudo crear el config.\n");
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
    iniciarHiloCliente();

    //thread server

	server_fd = iniciar_servidor();
	log_info(logger, "CPU lista para escuchar al cliente\n");

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

void iniciarHiloCliente() {

	int err = pthread_create( 	&client_Memoria,	// puntero al thread
	     	        			NULL,
								clientMemoria, // le paso la def de la función que quiero que ejecute mientras viva
								NULL); // argumentos de la función

	if (err != 0) {
	printf("No se pudo crear el hilo del cliente Memoria del CPU.\n");
	exit(7);
	}
	printf("El hilo cliente de la Memoria se creo correctamente.\n");
}

void* clientMemoria(void* ptr) {
	int config = 1;
    int conexion_Memoria;

    conexion_Memoria = crear_conexion(ip_memoria, puerto_memoria);
    enviar_mensaje("CPU",conexion_Memoria);
    log_info(logger, "Ingrese sus mensajes para la Memoria: ");
    paquete(conexion_Memoria);
    int cod_op = recibir_operacion(conexion_Memoria);
    recibir_mensaje(conexion_Memoria);
    liberar_conexion(conexion_Memoria);

    //sem_post(&semCPUClientMemoria);
	return NULL;
}

void iniciarHiloClienteKernel() {

	int err = pthread_create( 	&client_Kernel,	// puntero al thread
	     	        			NULL,
								clientKernel, // le paso la def de la función que quiero que ejecute mientras viva
								NULL); // argumentos de la función

	if (err != 0) {
	printf("No se pudo crear el hilo del cliente Kernel del CPU.\n");
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
	  printf("No se pudo crear el hilo de la conexión kernel-CPU \n");
	  exit(7);
	 }
	 printf("El hilo de la conexión kernel-CPU se creo correctamente.\n");
}

void* serverCPU(void* ptr){

	//sem_wait(&semCPUClientMemoria);

    //int server_fd = iniciar_servidor();
	log_info(logger, "CPU lista para recibir al cliente");
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
    			log_info(logger, "Me llegaron los siguientes valores:\n");
    			list_iterate(lista, (void*) iterator);
    			list_destroy(lista);
    			break;
    		case INSTRUCCIONES:
    			contexto->listaInstrucciones = recibir_paquete(cliente_fd);
    			contadorContexto++;
    			log_info(logger, "Instrucciones recibidas de Kernel:\n");
    			list_iterate(contexto->listaInstrucciones, (void*) iterator);
    			if(contadorContexto == 2){
    				iniciar_ejecucion();
    			}
    			break;
    		case CONTEXTO:
    			printf("CONTEXTO RECIBIDO\n");
    			contexto = recibir_contexto(cliente_fd);
    			contadorContexto++;
    			printf("programCounter recibido de Kernel = %d\n",contexto->programCounter);
    			printf("AX recibido de Kernel = %s\n",contexto->registrosCpu.AX);
    			printf("CX recibido de Kernel = %s\n",contexto->registrosCpu.BX);
    			printf("BX recibido de Kernel = %s\n",contexto->registrosCpu.CX);
    			printf("DX recibido de Kernel = %s\n",contexto->registrosCpu.DX);

    			printf("EAX recibido de Kernel = %s\n",contexto->registrosCpu.EAX);
    			printf("EBX recibido de Kernel = %s\n",contexto->registrosCpu.EBX);
    			printf("ECX recibido de Kernel = %s\n",contexto->registrosCpu.ECX);
    			printf("EDX recibido de Kernel = %s\n",contexto->registrosCpu.EDX);

    			printf("RAX recibido de Kernel = %s\n",contexto->registrosCpu.RAX);
    			printf("RBX recibido de Kernel = %s\n",contexto->registrosCpu.RBX);
    			printf("RCX recibido de Kernel = %s\n",contexto->registrosCpu.RCX);
    			printf("RDX recibido de Kernel = %s\n",contexto->registrosCpu.RDX);

    			if(contadorContexto == 2){
    				iniciar_ejecucion();
    			}

    			break;
    		case -1:
    			log_error(logger, "el kernel se desconecto. Terminando servidor\n");
    			return EXIT_FAILURE;
			default:
				log_warning(logger,"Operacion desconocida. No quieras meter la pata\n");
				break;
    	}
    }

    sem_post(&semCPUServer);

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
		char* proximaInstruccion =string_new();
		proximaInstruccion = list_get(contexto->listaInstrucciones, contexto->programCounter);

		printf("INSTRUCCION A EJECUTAR: %s\n", proximaInstruccion );

		//ejecutarFuncion: ejecuta la función que corresponde y retorna un int para saber si debe seguir ejecutando
		continuarLeyendo = ejecutarFuncion(proximaInstruccion);

	}

	//Una vez que no se deba seguir ejecutando va a serializar el contexto actualizado y lo va a enviar
	printf("FIN DE INSTRUCCIONES \n");
	printf("ULTIMA INTRUCCION EJECUTADA: %s\n",contexto->instruccion);
	printf("PROGRAM COUNTER: %i\n",contexto->programCounter);
	printf("TIEMPO BLOQUEADO: %i\n",contexto->tiempoBloqueado);
	printf("RECURSO SOLICITADO: %s\n",contexto->recursoSolicitado);
	printf("RECURSO A LIBERAR: %s\n",contexto->recursoALiberar);
	printf("ID SEGMENTO: %i\n",contexto->idSegmento);
	printf("TAMANIO SEGMENTO: %i\n",contexto->tamanioSegmento);
	printf("AX = %s\n",contexto->registrosCpu.AX);
	printf("BX = %s\n",contexto->registrosCpu.BX);
	printf("CX = %s\n",contexto->registrosCpu.CX);
	printf("DX = %s\n",contexto->registrosCpu.DX);

	printf("EAX = %s\n",contexto->registrosCpu.EAX);
	printf("EBX = %s\n",contexto->registrosCpu.EBX);
	printf("ECX = %s\n",contexto->registrosCpu.ECX);
	printf("EDX = %s\n",contexto->registrosCpu.EDX);

	printf("RAX = %s\n",contexto->registrosCpu.RAX);
	printf("RBX = %s\n",contexto->registrosCpu.RBX);
	printf("RCX = %s\n",contexto->registrosCpu.RCX);
	printf("RDX = %s\n",contexto->registrosCpu.RDX);

	printf("ARCHIVO = %s\n",contexto->nombreArchivo);
	printf("POSICION = %d\n",contexto->posicionArchivo);
	printf("CANTIDAD BYTES = %d\n",contexto->cantBytesArchivo);
	printf("DIRECCIÓN FÍSICA = %d\n",contexto->direcFisicaArchivo);
	printf("TAMAÑO  = %d\n",contexto->tamanioArchivo);

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


	printf("Contexto actualizado enviado a KERNEL. \n");

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

		int tamanioValor = string_length(setParam2);
		setParam2[tamanioValor-1] = '\0';

		set_tp(setParam1, setParam2);
		free(setParam1);
		free(setParam2);
		continuarLeyendo = 1;
    } else if (  string_contains(nombreInstruccion,"YIELD")  ) {
    	yield_tp();
    } else if (string_contains(nombreInstruccion,"EXIT")) {
    	exit_tp();
    } else if (strcmp(nombreInstruccion, "I/O") == 0) {
    	int ioParam = atoi(arrayInstruccion[1]);
    	i_o_tp(ioParam);
    } else if (strcmp(nombreInstruccion, "WAIT") == 0) {
    	char* recursoWait = string_new();
    	recursoWait = string_duplicate(arrayInstruccion[1]);
    	wait_tp(recursoWait);
    } else if (strcmp(nombreInstruccion, "SIGNAL") == 0) {
    	char* recursoSignal = string_new();
    	recursoSignal = string_duplicate(arrayInstruccion[1]);
    	signal_tp(recursoSignal);
    } else if (strcmp(nombreInstruccion, "MOV_IN") == 0) {
    	continuarLeyendo = 1;
    } else if (strcmp(nombreInstruccion, "MOV_OUT") == 0) {
    	continuarLeyendo = 1;
    } else if (strcmp(nombreInstruccion, "F_OPEN") == 0) {
    	char* fopenParam1 = string_new();
    	fopenParam1 = string_duplicate(arrayInstruccion[1]);
    	fopen_tp(fopenParam1);
    	free(fopenParam1);
    } else if (strcmp(nombreInstruccion, "F_CLOSE") == 0) {
    	char* fcloseParam1 = string_new();
    	fcloseParam1 = string_duplicate(arrayInstruccion[1]);
    	fclose_tp(fcloseParam1);
    	free(fcloseParam1);
    } else if (strcmp(nombreInstruccion, "F_SEEK") == 0) {
    	char* fseekParam1 = string_new();
    	fseekParam1 = string_duplicate(arrayInstruccion[1]);
    	int fseekParam2 = atoi(arrayInstruccion[2]);
    	fseek_tp(fseekParam1,fseekParam2);
    	free(fseekParam1);
    } else if (strcmp(nombreInstruccion, "F_READ") == 0) {
    	char* freadParam1 = string_new();
    	freadParam1 = string_duplicate(arrayInstruccion[1]);

    	int freadParam2 = atoi(arrayInstruccion[2]);
    	int freadParam3 = atoi(arrayInstruccion[3]);

    	fread_tp(freadParam1,freadParam2,freadParam3);

    	free(freadParam1);
    } else if (strcmp(nombreInstruccion, "F_WRITE") == 0) {
    	char* fwriteParam1 = string_new();
    	fwriteParam1 = string_duplicate(arrayInstruccion[1]);

    	int fwriteParam2 = atoi(arrayInstruccion[2]);
    	int fwriteParam3 = atoi(arrayInstruccion[3]);

    	fwrite_tp(fwriteParam1,fwriteParam2,fwriteParam3);

    	free(fwriteParam1);
    } else if (strcmp(nombreInstruccion, "F_TRUNCATE") == 0) {
    	char* ftruncateParam1 = string_new();
    	ftruncateParam1 = string_duplicate(arrayInstruccion[1]);
    	int ftruncateParam2 = atoi(arrayInstruccion[2]);
    	ftruncate_tp(ftruncateParam1,ftruncateParam2);
    	free(ftruncateParam1);
    } else if (strcmp(nombreInstruccion, "CREATE_SEGMENT") == 0) {
    	int createParam1 = atoi(arrayInstruccion[1]);
    	int createParam2 = atoi(arrayInstruccion[2]);

    	createSeg_tp(createParam1,createParam2);

    } else if (strcmp(nombreInstruccion, "DELETE_SEGMENT") == 0) {
    	int deleteParam1 = atoi(arrayInstruccion[1]);

    	deleteSeg_tp(deleteParam1);
    } else {
        printf("Instruccion no reconocida.\n");
    }

	return continuarLeyendo;
}

//FALTA TERMINAR MMU
int MMU(int direcLogica,int cantBytes){

	int num_segmento = floor(direcLogica / tam_max_segmento);
	int desplazamiento_segmento = direcLogica % tam_max_segmento;

	printf("num_segmento: %d\n",num_segmento);
	printf("desplazamiento_segmento: %d\n",desplazamiento_segmento);

	if((desplazamiento_segmento+cantBytes)>tam_max_segmento){
		printf("ERROR: SEGMENTATION FAULT\n");
		return -1;
	}

	int direcFisica = -2;
	printf("FALTA CALCULAR LA DIRECCIÓN FÍSICA \n");
	//int direcFisica = baseSegmento + desplazamiento_segmento;

	return direcFisica;
}

// FUNCIONES INSTRUCCIONES

//MOV_IN (Registro, Dirección Lógica): Lee el valor de memoria correspondiente a la Dirección Lógica y lo almacena en el Registro.
//MOV_OUT (Dirección Lógica, Registro): Lee el valor del Registro y lo escribe en la dirección física de memoria obtenida a partir de la Dirección Lógica.

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
	sleep_ms(retardo_instruccion);
	printf("REGISTRO A SETEAR: %s\n", registro);
	printf("VALOR A SETEAR: %s\n", valor);

	if (strcmp(registro, "AX") == 0) {
		strcpy(contexto->registrosCpu.AX,valor);
		printf("AX MODIFICADO\n");

	} else if (strcmp(registro, "BX") == 0) {
		strcpy(contexto->registrosCpu.BX,valor);
		printf("BX MODIFICADO\n");

	} else if (strcmp(registro, "CX") == 0) {
		strcpy(contexto->registrosCpu.CX,valor);
		printf("CX MODIFICADO\n");

	} else if (strcmp(registro, "DX") == 0) {
		strcpy(contexto->registrosCpu.DX,valor);
		printf("DX MODIFICADO\n");

	} else if (strcmp(registro, "EAX") == 0) {
		strcpy(contexto->registrosCpu.EAX,valor);
		printf("EAX MODIFICADO\n");

	} else if (strcmp(registro, "EBX") == 0) {
		strcpy(contexto->registrosCpu.EBX,valor);
		printf("EBX MODIFICADO\n");

	} else if (strcmp(registro, "ECX") == 0) {
		strcpy(contexto->registrosCpu.ECX,valor);
		printf("ECX MODIFICADO\n");

	} else if (strcmp(registro, "EDX") == 0) {
		strcpy(contexto->registrosCpu.EDX,valor);
		printf("EDX MODIFICADO\n");

	} else if (strcmp(registro, "RAX") == 0) {
		strcpy(contexto->registrosCpu.RAX,valor);
		printf("RAX MODIFICADO\n");

	} else if (strcmp(registro, "RBX") == 0) {
		strcpy(contexto->registrosCpu.RBX,valor);
		printf("RBX MODIFICADO\n");

	} else if (strcmp(registro, "RCX") == 0) {
		strcpy(contexto->registrosCpu.RCX,valor);
		printf("RCX MODIFICADO\n");

	} else if (strcmp(registro, "RDX") == 0) {
		strcpy(contexto->registrosCpu.RDX,valor);
		printf("RDX MODIFICADO\n");
	} else {
		printf("Registro no válido.\n");
	}

	contexto->instruccion = string_duplicate("SET");

	return;
}
