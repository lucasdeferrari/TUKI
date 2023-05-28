#include "kernel.h"

t_config* config;

//Lo necesitamos para escuchar constantemente
int server_fd;

int main(void) {
	estadoEnEjecucion.programCounter = 3;
	strcpy(estadoEnEjecucion.registrosCpu.AX,"HOLA");

	sem_init(&semKernelClientCPU,0,1);
	sem_init(&semKernelClientMemoria,0,0);
	sem_init(&semKernelClientFileSystem,0,0);
	sem_init(&semKernelServer,0,0);


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

    hrrn_alfa = config_get_string_value(config, "HRRN_ALFA");
    grado_max_multiprogramación = config_get_string_value(config, "GRADO_MAX_MULTIPROGRAMACION");
    //recursos = config_get_string_value(config, "RECURSOS");
    //instancias_recursos = config_get_string_value(config, "INSTANCIAS_RECURSOS");

    //THREADS CONEXIÓN
    //thread clients CPU, FS, Memoria
    iniciarHiloClienteCPU();
    iniciarHiloClienteMemoria();
    iniciarHiloClienteFileSystem();

    //thread server

	server_fd = iniciar_servidor();
	log_info(logger, "Kernel listo para escuchar al cliente\n");

	while(1){
		iniciarHiloServer();
		pthread_join(serverKernel_thread, NULL);
	}

    pthread_detach(client_CPU);
    pthread_detach(client_Memoria);
    pthread_detach(client_FileSystem);



    //libero memoria
    log_destroy(logger);
    config_destroy(config);

    // Destruye los semáforos
    sem_destroy(&semKernelClientCPU);
    sem_destroy(&semKernelClientMemoria);
    sem_destroy(&semKernelClientFileSystem);
    sem_destroy(&semKernelServer);


    return EXIT_SUCCESS;
}

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

void iniciarHiloClienteMemoria() {

	int err = pthread_create( 	&client_Memoria,	// puntero al thread
								NULL,
								clientMemoria, // le paso la def de la función que quiero que ejecute mientras viva
								NULL); // argumentos de la función

	if (err != 0) {
	printf("\nNo se pudo crear el hilo del cliente Memoria del kernel.");
	exit(7);
	}
	//printf("El hilo cliente de la Memoria se creo correctamente.");

}

void iniciarHiloClienteFileSystem() {

	int err = pthread_create( 	&client_FileSystem,	// puntero al thread
								NULL,
								clientFileSystem, // le paso la def de la función que quiero que ejecute mientras viva
								NULL); // argumentos de la función

	if (err != 0) {
	printf("\nNo se pudo crear el hilo del cliente FileSystem del kernel.");
	exit(7);
	}
	//printf("\nEl hilo cliente del FileSystem se creo correctamente.");

}

void serializarContexto(int unSocket){

	t_contextoEjecucion contextoPRUEBA;
	contextoPRUEBA.programCounter = estadoEnEjecucion.programCounter;
	strcpy(contextoPRUEBA.registrosCpu.AX, estadoEnEjecucion.registrosCpu.AX);
	contextoPRUEBA.instruccion = calloc(1, 4+1);
	strcpy(contextoPRUEBA.instruccion, "Hola");
	contextoPRUEBA.instruccion_length = strlen(contextoPRUEBA.instruccion)+1;


	//BUFFER

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(int)*2 + strlen(contextoPRUEBA.instruccion)+1 + sizeof(contextoPRUEBA.registrosCpu.AX); //Program counter e instruccion

	void* stream = malloc(buffer->size);
	int offset = 0; //desplazamiento

	memcpy(stream + offset, &contextoPRUEBA.programCounter, sizeof(int));
	offset += sizeof(int); //No tiene sentido seguir calculando el desplazamiento, ya ocupamos el buffer completo

	memcpy(stream + offset, &contextoPRUEBA.registrosCpu.AX, sizeof(contextoPRUEBA.registrosCpu.AX));
	offset += sizeof(contextoPRUEBA.registrosCpu.AX);

	//instruccion, dinamica
	memcpy(stream + offset, &contextoPRUEBA.instruccion_length, sizeof(int));
	offset += sizeof(int);
	memcpy(stream + offset, contextoPRUEBA.instruccion, strlen(contextoPRUEBA.instruccion) + 1);

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


	printf("programCounter enviado a CPU = %d\n",contextoPRUEBA.programCounter);
	printf("instruccion enviado a CPU = %s\n", contextoPRUEBA.instruccion);
	printf("RegistroAX enviado a CPU = %s\n", contextoPRUEBA.registrosCpu.AX);

	//free memoria dinámica
	free(contextoPRUEBA.instruccion);
	// Liberamos la memoria
	free(a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);

	return;
}

void* clientCPU(void* ptr) {
	sem_wait(&semKernelClientCPU);
	int config=1;
    int conexion_CPU;
    conexion_CPU = crear_conexion(ip_cpu, puerto_cpu);
    //log_info(logger, "Ingrese sus mensajes para la CPU: ");
    //paquete(conexion_CPU);
    serializarContexto(conexion_CPU);
    liberar_conexion(conexion_CPU);

    sem_post(&semKernelClientMemoria);
	return NULL;
}

void* clientMemoria(void* ptr) {
	sem_wait(&semKernelClientMemoria);
	int config = 1;
    int conexion_Memoria;
    conexion_Memoria = crear_conexion(ip_memoria, puerto_memoria);
    enviar_mensaje("kernel",conexion_Memoria);
    log_info(logger, "Ingrese sus mensajes para la Memoria: ");
    paquete(conexion_Memoria);
    int cod_op = recibir_operacion(conexion_Memoria);
    printf("codigo de operacion: %i\n", cod_op);
    recibir_mensaje(conexion_Memoria);
    liberar_conexion(conexion_Memoria);

    sem_post(&semKernelClientFileSystem);
	return NULL;
}

void* clientFileSystem(void* ptr) {
	sem_wait(&semKernelClientFileSystem);
	int config = 1;
    int conexion_FileSystem;
    conexion_FileSystem = crear_conexion(ip_filesystem, puerto_filesystem);
    log_info(logger, "Ingrese sus mensajes para el FileSystem: ");
    paquete(conexion_FileSystem);
    liberar_conexion(conexion_FileSystem);

    sem_post(&semKernelServer);
	return NULL;
}

void iniciarHiloServer() {

    int err = pthread_create( 	&serverKernel_thread,	// puntero al thread
								NULL,
								&serverKernel, // le paso la def de la función que quiero que ejecute mientras viva
								NULL); // argumentos de la función

	if (err != 0) {
	printf("\nNo se pudo crear el hilo de la conexión consola-kernel.\n");
	exit(7);
	}
	//printf("\nEl hilo de la conexión consola-kernel se creo correctamente.\n");
}

void* serverKernel(void* ptr){

	sem_wait(&semKernelServer);

    //int server_fd = iniciar_servidor();
    log_info(logger, "Kernel listo para recibir al cliente");
    int cliente_fd = esperar_cliente(server_fd);

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
    		case PAQUETE:   //Recibe paquete de instrucciones, arma el PCB y lo encola en NEW
    			lista = recibir_paquete(cliente_fd);
    			if (strcmp(handshake, "consola") == 0) {
    				log_info(logger, "Iniciando procedimiento al recibir un paquete de CONSOLA");
        			armarPCB(lista);
        			printf("PCB encolado en NEW\n");
        			encolarReady();
        			printf("Cola NEW:\n");
        			mostrarCola(frenteColaNew);
        			printf("Cola READY:\n");
        			mostrarCola(frenteColaReady);
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

    			//log_info(logger, "Me llegaron los siguientes valores:\n");
    			//list_iterate(lista, (void*) iterator);
    			break;
    		case -1:
    			free(handshake);
    			sem_post(&semKernelServer);
    			log_error(logger, "\nel cliente se desconecto. Terminando servidor");
    			return EXIT_FAILURE;
    		default:
    			log_warning(logger,"\nOperacion desconocida. No quieras meter la pata");
    			break;
    	}
    }

	return NULL;
}

void armarPCB(t_list* lista){

	t_infopcb nuevoPCB;
	estimacion_inicial = config_get_string_value(config, "ESTIMACION_INICIAL");

	//inicializamos estructura
	nuevoPCB.pid = pid;
	nuevoPCB.listaInstrucciones = lista;
	nuevoPCB.programCounter = 0;

	for (int i = 0; i < sizeof(nuevoPCB.registrosCpu.AX); i++) {
		nuevoPCB.registrosCpu.AX[i] = '\0';
	    }
	for (int i = 0; i < sizeof(nuevoPCB.registrosCpu.BX); i++) {
		nuevoPCB.registrosCpu.BX[i] = '\0';
		    }
	for (int i = 0; i < sizeof(nuevoPCB.registrosCpu.CX); i++) {
		nuevoPCB.registrosCpu.CX[i] = '\0';
		    }
	for (int i = 0; i < sizeof(nuevoPCB.registrosCpu.DX ); i++) {
		nuevoPCB.registrosCpu.DX[i] = '\0';
		    }
	for (int i = 0; i < sizeof(nuevoPCB.registrosCpu.EAX ); i++) {
		nuevoPCB.registrosCpu.EAX[i] = '\0';
			    }
	for (int i = 0; i < sizeof(nuevoPCB.registrosCpu.EBX ); i++) {
			nuevoPCB.registrosCpu.EBX[i] = '\0';
				    }
	for (int i = 0; i < sizeof(nuevoPCB.registrosCpu.ECX ); i++) {
				nuevoPCB.registrosCpu.ECX[i] = '\0';
					    }
	for (int i = 0; i < sizeof(nuevoPCB.registrosCpu.EDX ); i++) {
					nuevoPCB.registrosCpu.EDX[i] = '\0';
						    }
	for (int i = 0; i < sizeof(nuevoPCB.registrosCpu.RAX ); i++) {
			nuevoPCB.registrosCpu.RAX[i] = '\0';
				    }
	for (int i = 0; i < sizeof(nuevoPCB.registrosCpu.RBX ); i++) {
				nuevoPCB.registrosCpu.RBX[i] = '\0';
					    }
	for (int i = 0; i < sizeof(nuevoPCB.registrosCpu.RCX ); i++) {
					nuevoPCB.registrosCpu.RCX[i] = '\0';
						    }
	for (int i = 0; i < sizeof(nuevoPCB.registrosCpu.RDX ); i++) {
					nuevoPCB.registrosCpu.RDX[i] = '\0';
						    }


	nuevoPCB.tablaSegmentos = NULL; //YA NO TIRA ERROR, SE VE Q FALLABA OTRA COSA - REVISAR

//solo sería necesario si quisiera inicializar tablaSegmentos con un valor distinto a NULL

//	t_nodoTablaSegmentos* nodoTablaSegmentos = malloc(sizeof(t_nodoTablaSegmentos));
//	nuevoPCB.tablaSegmentos->sgte=NULL;
//	nuevoPCB.tablaSegmentos->info_tablaSegmentos.id=0;
//	nuevoPCB.tablaSegmentos->info_tablaSegmentos.direccionBase = NULL;
//	nuevoPCB.tablaSegmentos->info_tablaSegmentos.tamaño = 0;
//	nuevoPCB.tablaSegmentos = nodoTablaSegmentos;

	nuevoPCB.estimadoProxRafaga = atof(estimacion_inicial);
	nuevoPCB.tiempoLlegadaReady = 0;

	nuevoPCB.punterosArchivos = NULL; //YA NO TIRA ERROR, SE VE Q FALLABA OTRA COSA - REVISAR

//solo sería necesario si quisiera inicializar punterosArchivos con un valor distinto a NULL
//	t_nodoArchivos* nodoPunterosArchivos = malloc(sizeof(t_nodoArchivos));
//	nuevoPCB.punterosArchivos->info_archivos=NULL;
//	nuevoPCB.punterosArchivos->sgte=NULL;
//	nuevoPCB.punterosArchivos = nodoPunterosArchivos;

	//Encolamos en NEW (FIFO)
	queue(&frenteColaNew, &finColaNew, nuevoPCB);

	pid++;
}

void encolarReady() {

	//SI EL ALGORTIMO DE PLANIFICACIÓN ES FIFO, VERIFICA EL GRADO MAX DE MULTIPROGRAMCIÓN Y ENCOLA EN READY SI CORRESPONDE

	if(strcmp(algoritmo_planificacion,"FIFO") == 0){

		int cantidadElementosReady = cantidadElementosCola(frenteColaReady);
		int lugaresDisponiblesReady = atof(grado_max_multiprogramación) - cantidadElementosReady;

		printf("Lugares disponibles en READY: %d \n",lugaresDisponiblesReady);


		if(lugaresDisponiblesReady > 0 ){

			if(frenteColaNew != NULL){
				queue(&frenteColaReady, &finColaReady,unqueue(&frenteColaNew,&finColaNew));

				cantidadElementosReady = cantidadElementosCola(frenteColaReady);
				lugaresDisponiblesReady = atof(grado_max_multiprogramación) - cantidadElementosReady;
				printf("PCB encolado en READY - lugares disponibles en READY: %d \n",lugaresDisponiblesReady);
			}
		}
		else{
			printf("Grado máximo de multiprogramación alcanzado. \n");
		}

		//SI ES HRRN
	}

	return;
}

void iterator(char* value) {
    log_info(logger, value);
}

//Funciones client

t_config* iniciar_config(void){
	t_config* nuevo_config;

	return nuevo_config;
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

//encolar
void queue(t_nodoCola** frenteColaNew, t_nodoCola** finColaNew, t_infopcb pcb) {
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

//desencolar
t_infopcb unqueue(t_nodoCola** frenteColaNew, t_nodoCola** finColaNew) {
	t_infopcb pcb_puntero;
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
        printf("PID: %d\n", frenteColaNew->info_pcb.pid);

        printf("Lista de instrucciones:\n");
        t_list* lista = frenteColaNew->info_pcb.listaInstrucciones;
        for (int i = 0; i < list_size(lista); i++) {
        	char* instruccion = list_get(lista, i);
        	printf("\t%s\n", instruccion);
        }

        printf("Program Counter: %d\n", frenteColaNew->info_pcb.programCounter);
//        printf("Registros CPU:\n");
//        for (int i = 0; i < 12; i++) {
//            printf("Registro %d: %s\n", i, frenteColaNew->info_pcb.registrosCpu[i]);
//        }
        printf("Tabla de segmentos:\n");
        t_nodoTablaSegmentos* tabla = frenteColaNew->info_pcb.tablaSegmentos;
        while (tabla != NULL) {
        	printf("Id: %p\n", tabla->info_tablaSegmentos.id);
            printf("Base: %p\n", tabla->info_tablaSegmentos.direccionBase);
            printf("Base: %p\n", tabla->info_tablaSegmentos.tamanio);
            tabla = tabla->sgte;
        }
        printf("Estimado próxima ráfaga: %f\n", frenteColaNew->info_pcb.estimadoProxRafaga);
        printf("Tiempo llegada a Ready: %d\n", frenteColaNew->info_pcb.tiempoLlegadaReady);
        printf("Punteros a archivos:\n");
        t_nodoArchivos* punteros = frenteColaNew->info_pcb.punterosArchivos;
        while (punteros != NULL) {
            printf("%s\n", punteros->info_archivos);
            punteros = punteros->sgte;
        }
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

char* recibir_handshake(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	//log_info(logger, "Me llego el mensaje %s", buffer);
	return buffer;
}
