#include "CPU.h"
t_config* config;
int cliente_fd;



void serializarContexto(int unSocket){

	//ELEMENTOS A SERIALIZAR
	//char* recursoSolicitado;
	//char* recursoALiberar
	//char* instruccion;


	//SERIALIZACION
	//int programCounter;
	//t_registrosCPU registrosCpu;
	//int tiempoBloqueado;


	//contexto->instruccion = calloc(1, 4+1);
//	strcpy(contextoPRUEBA.instruccion, "Hola");
//	contextoPRUEBA.instruccion_length = strlen(contextoPRUEBA.instruccion)+1;
	contexto->instruccion_length = strlen(contexto->instruccion)+1;
	contexto->recursoALiberar_length = strlen(contexto->recursoALiberar)+1;
	contexto->recursoSolicitado_length = strlen(contexto->recursoSolicitado)+1;

	//BUFFER
	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(int)*5 + sizeof(contexto->registrosCpu.AX) * 4 + sizeof(contexto->registrosCpu.EAX) *4 + sizeof(contexto->registrosCpu.RAX)*4 + contexto->instruccion_length + contexto->recursoALiberar_length + contexto->recursoSolicitado_length;


	void* stream = malloc(buffer->size);
	int offset = 0; //desplazamiento

	memcpy(stream + offset, &contexto->programCounter, sizeof(int));
	offset += sizeof(int);

	memcpy(stream + offset, &contexto->tiempoBloqueado, sizeof(int));
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
	printf("tamaño enviado a KERNEL = %d\n", contexto->instruccion_length);
	printf("instruccion enviado a KERNEL = %s\n", contexto->instruccion);

//	printf("tamaño enviado a KERNEL = %d\n", contexto->recursoSolicitado_length);
//	printf("recurso solicitado a KERNEL = %s\n", contexto->recursoSolicitado);
//
//	printf("tamaño enviado a KERNEL = %d\n", contexto->recursoALiberar_length);
//	printf("recurso a liberar KERNEL = %s\n", contexto->recursoALiberar);

	//free memoria dinámica
	free(contexto->instruccion);
	// Liberamos la memoria
	free(a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);

	return;
}

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

    //Inicializo contexto
    contexto = malloc(sizeof(t_contextoEjecucion));
    contexto->listaInstrucciones = list_create();
    vaciarContexto();

    //Server
    iniciarHiloServer();

    //Hilo Cliente
    iniciarHiloCliente();

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

    sem_post(&semCPUClientMemoria);
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

	sem_wait(&semCPUClientMemoria);

    int server_fd = iniciar_servidor();
    log_info(logger, "CPU lista para recibir al Kernel");
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
    			printf("CONTEXTO RECIBIDO");
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

	serializarContexto(cliente_fd);
	//enviar_mensaje("SOY CPU", cliente_fd);

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
    } else if (strcmp(nombreInstruccion, "YIELD") == 0) {
    	yield_tp();
    } else if (strcmp(nombreInstruccion, "EXIT") == 0) {
    	exit_tp();
    } else if (strcmp(nombreInstruccion, "I/O") == 0) {
    	int ioParam = arrayInstruccion[1];
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

    } else if (strcmp(nombreInstruccion, "F_CLOSE") == 0) {

    } else if (strcmp(nombreInstruccion, "F_SEEK") == 0) {

    } else if (strcmp(nombreInstruccion, "F_READ") == 0) {

    } else if (strcmp(nombreInstruccion, "F_WRITE") == 0) {

    } else if (strcmp(nombreInstruccion, "F_TRUNCATE") == 0) {

    } else if (strcmp(nombreInstruccion, "CREATE_SEGMENT") == 0) {

    } else if (strcmp(nombreInstruccion, "DELETE_SEGMENT") == 0) {

    } else if (strcmp(nombreInstruccion, "MOV_OUT") == 0) {

    } else {
        printf("Instruccion no reconocida.\n");
    }

	return continuarLeyendo;
}



// FUNCIONES INSTRUCCIONES

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
