#include "kernel.h"

t_config* config;

int main(void) {


	sem_init(&semKernelClientCPU,0,1);
	sem_init(&semKernelClientMemoria,0,0);
	sem_init(&semKernelClientFileSystem,0,0);
	sem_init(&semKernelServer,0,0);
	//sem_init(&semReady,0,0);

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
    //thread clients CPU, FS, Memoria		//alternativa con hilos
    iniciarHiloClienteCPU();
    iniciarHiloClienteMemoria();
    iniciarHiloClienteFileSystem();

    //thread server consola
    iniciarHiloServer();



    pthread_detach(client_CPU);
    pthread_detach(client_Memoria);
    pthread_detach(client_FileSystem);
    pthread_join(serverKernel_thread,NULL);
    pthread_detach(encolar_ready);


    //libero memoria
    log_destroy(logger);
    config_destroy(config);

    // Destruye los semáforos
    sem_destroy(&semKernelClientCPU);
    sem_destroy(&semKernelClientMemoria);
    sem_destroy(&semKernelClientFileSystem);
    sem_destroy(&semKernelServer);
    //sem_destroy(&semReady);

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

void* clientCPU(void* ptr) {
	sem_wait(&semKernelClientCPU);
	int config=1;
    int conexion_CPU;
    conexion_CPU = crear_conexion(ip_cpu, puerto_cpu);
    log_info(logger, "Ingrese sus mensajes para la CPU: ");
    paquete(conexion_CPU);
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

    int server_fd = iniciar_servidor();
    log_info(logger, "Kernel listo para recibir a la consola");
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
        			printf("PCB encolado en NEW:\n");
        			encolarReady();
        			printf("Cola NEW:\n");
        			mostrarCola(frenteColaNew);
        			printf("Cola READY:\n");
        			mostrarCola(frenteColaReady);
    				//cosas de consola
    			}
    			if (strcmp(handshake, "kernel") == 0) {
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
    			free(handshake);
    			break;
    		case -1:
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

	nuevoPCB.registrosCpu.AX = NULL;
	nuevoPCB.registrosCpu.BX = NULL;
	nuevoPCB.registrosCpu.CX = NULL;
	nuevoPCB.registrosCpu.DX = NULL;
	nuevoPCB.registrosCpu.EAX = NULL;
	nuevoPCB.registrosCpu.EBX = NULL;
	nuevoPCB.registrosCpu.ECX = NULL;
	nuevoPCB.registrosCpu.EDX = NULL;
	nuevoPCB.registrosCpu.RAX = NULL;
	nuevoPCB.registrosCpu.RBX = NULL;
	nuevoPCB.registrosCpu.RCX = NULL;
	nuevoPCB.registrosCpu.RDX = NULL;

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
	char* leido;
	t_paquete* paquete;

	paquete = crear_paquete();

	// Leemos y esta vez agregamos las lineas al paquete
	leido = readline("> ");

	while(strcmp(leido, "") != 0){
		agregar_a_paquete(paquete, leido, strlen(leido));
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
