#include "kernel.h"

t_config* config;

//void inicializarRecursos(){
//	//Manejo de recursos, VER COMO INICIALIZAR VARIABLES
//	char** recursos = config_get_array_value(config, "RECURSOS"); // El array que devuelve termina en NULL
//	char** instancias_recursos = config_get_array_value(config, "INSTANCIAS_RECURSOS");
//
//	//int cantidadRecursos = string_array_size(recursos);
//	//int contador = 0;
//
//	while(!string_array_is_empty(recursos)){
//		t_recursos* unRecurso;
//		unRecurso = malloc(sizeof(t_recursos));
//
//		unRecurso->recurso = string_array_pop(recursos);
//		int instanciaRecurso= string_array_pop(instancias_recursos);
//		unRecurso->instancias = strtol(instanciaRecurso, NULL, 10);
//		list_add(listaRecursos, unRecurso); //lista de recursos en kernel.h - linea: 75 y la cree en kenel.c - linea:76
//		//contador ++;
//	}
//
//	    //valores prueba
//	//    int instancias_recursos[] = {1,2};
//	//    char* recursos[] = {"DISCO", "RECURSO_1"};
//
//	//   int cantidadDeRecursos = sizeof(instancias_recursos) / sizeof(instancias_recursos[0]);
//
//
//
//	    // POR CADA RECURSO HAY QUE CREAR UNA COLA DE BLOQUEADOS
//	    // podriamos tener un ARRAY DE COLAS para manejar todos los array con el mismo índice
////	    int i;
////	    for (i=0;i<cantidadDeRecursos;i++){
////
////
////	    }
//		free(recursos);
//		free(instancias_recursos);
//
//
//}

int main(void) {
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
    hrrn_alfa = config_get_int_value(config, "HRRN_ALFA");
    grado_max_multiprogramación = config_get_int_value(config, "GRADO_MAX_MULTIPROGRAMACION");



    //Representa que no hay ningun estado en ejecucion
    estadoEnEjecucion = malloc(sizeof(t_infopcb));
    int ningunEstado = -1;
    estadoEnEjecucion->pid = ningunEstado;

    listaReady = list_create();
    listaRecursos = list_create();
    //inicializarRecursos();


//    //THREADS CONEXIÓN
//    //thread clients CPU, FS, Memoria
//  iniciarHiloClienteCPU();
//  iniciarHiloClienteMemoria();
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

    free(estadoEnEjecucion);
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
    //log_info(logger, "Ingrese sus mensajes para la CPU ");
    //paquete(conexion_CPU);
    serializarContexto(conexion_CPU); //enviamos el contexto sin las instrucciones

    //enviamos las intrucciones del contexto
    t_list_iterator* iterador = list_iterator_create(estadoEnEjecucion->listaInstrucciones);
    t_paquete* paquete = empaquetar(estadoEnEjecucion->listaInstrucciones);
    enviar_paquete(paquete, conexion_CPU);
    eliminar_paquete(paquete);
    printf("Instrucciones enviadas a CPU. \n");


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
	//sem_wait(&semKernelClientFileSystem);
	int config = 1;
    int conexion_FileSystem;
    conexion_FileSystem = crear_conexion(ip_filesystem, puerto_filesystem);
    log_info(logger, "Ingrese sus mensajes para el FileSystem: ");
    paquete(conexion_FileSystem);
    int cod_op = recibir_operacion(conexion_FileSystem);
    printf("codigo de operacion: %i\n", cod_op);
    recibir_mensaje(conexion_FileSystem);
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
    		case PAQUETE:
    			lista = recibir_paquete(cliente_fd); //Recibe paquete de instrucciones
    			if (strcmp(handshake, "consola") == 0) {
    				log_info(logger, "Iniciando procedimiento al recibir un paquete de CONSOLA");
        			armarPCB(lista);  //arma el PCB y lo encola en NEW
        			printf("PCB encolado en NEW\n");
        			encolarReady();  //Si corresponde lo encola en Ready
        			printf("PID EN EJECUCION: %d\n", estadoEnEjecucion->pid );
        			if(estadoEnEjecucion->pid == -1){  //Si no hay un proceso en ejecucion, lo ejecuto
        				desencolarReady();
        			}

    				//cosas de consola

        			iniciarHiloClienteCPU();
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

	t_infopcb* nuevoPCB = malloc(sizeof(t_infopcb));
	estimacion_inicial = config_get_double_value(config, "ESTIMACION_INICIAL");

	//inicializamos estructura
	nuevoPCB->pid = pid;
	nuevoPCB->listaInstrucciones = lista;
	nuevoPCB->programCounter = 0;

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


	nuevoPCB->tablaSegmentos = NULL; //YA NO TIRA ERROR, SE VE Q FALLABA OTRA COSA - REVISAR
	nuevoPCB->estimadoAnterior = 0;
	nuevoPCB->estimadoProxRafaga = estimacion_inicial;
	nuevoPCB->empiezaAEjecutar = 0;
	nuevoPCB->entraEnColaReady = 0;
	nuevoPCB->terminaEjecutar = 0;
	nuevoPCB->punterosArchivos = NULL; //YA NO TIRA ERROR, SE VE Q FALLABA OTRA COSA - REVISAR

	//Encolamos en NEW (FIFO)
	queue(&frenteColaNew, &finColaNew, nuevoPCB);

	printf("Cola NEW:\n");
	mostrarCola(frenteColaNew);

	pid++;
}

void encolarReady() {

	// SI EL ALGORITMO DE PLANIFICACION ES FIFO VERIFICA EL GRADO MAX DE MULTIPROGRAMCIÓN Y ENCOLA EN READY SI CORRESPONDE

	if(strcmp(algoritmo_planificacion,"FIFO") == 0){

		int cantidadElementosReady = cantidadElementosCola(frenteColaReady);
		int lugaresDisponiblesReady = grado_max_multiprogramación - cantidadElementosReady;

		printf("Lugares disponibles en READY: %d \n",lugaresDisponiblesReady);


		if(lugaresDisponiblesReady > 0 ){

			if(frenteColaNew != NULL){
				queue(&frenteColaReady, &finColaReady,unqueue(&frenteColaNew,&finColaNew));

				cantidadElementosReady = cantidadElementosCola(frenteColaReady);
				lugaresDisponiblesReady = grado_max_multiprogramación - cantidadElementosReady;
				printf("PCB encolado en READY - lugares disponibles en READY: %d \n",lugaresDisponiblesReady);
			}
		}
		else{
			printf("Grado máximo de multiprogramación alcanzado. \n");
		}

		printf("Cola NEW:\n");
		mostrarCola(frenteColaNew);
		printf("Cola READY:\n");
		//mostrarCola(frenteColaReady);

	}




	// SI EL ALGORITMO DE PLANIFICACION ES HRRN VERIFICA EL GRADO MAX DE MULTIPROGRAMCIÓN Y ENCOLA EN READY SI CORRESPONDE

	if(strcmp(algoritmo_planificacion,"HRRN") == 0){

		int cantidadElementosReady = list_size(listaReady);
		printf("Cantidad de elementos en READY: %d \n",cantidadElementosReady);


		int lugaresDisponiblesReady = grado_max_multiprogramación - cantidadElementosReady;
		printf("Lugares disponibles en READY: %d \n",lugaresDisponiblesReady);


		if(lugaresDisponiblesReady > 0 ){

			if(frenteColaNew != NULL){
				t_infopcb* procesoADesencolar = unqueue(&frenteColaNew,&finColaNew);

				list_add(listaReady, procesoADesencolar);

				procesoADesencolar->entraEnColaReady = tomarTiempo();

				cantidadElementosReady = list_size(listaReady);
				lugaresDisponiblesReady = grado_max_multiprogramación - cantidadElementosReady;
				printf("PCB agregado en READY - lugares disponibles en READY: %d \n",lugaresDisponiblesReady);
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

void desencolarReady (){

	 //SI EL ALGORITMO DE PLANIFICACION ES FIFO

	if(strcmp(algoritmo_planificacion,"FIFO") == 0){
		estadoEnEjecucion = unqueue(&frenteColaReady,&finColaReady);
		printf("Proceso pasado a estadoEnEjecucion por FIFO. \n");

		printf("Cola READY:\n");
		mostrarCola(frenteColaReady);

		printf("Proceso en ejecucion: %d\n",estadoEnEjecucion->pid);
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

		list_remove(listaReady,pidMaxRafaga);

		printf("Proceso pasado a estadoEnEjecucion por HRRN. \n");
		printf("Proceso en ejecucion: %d\n",estadoEnEjecucion->pid);

		//CALCULA CUANDO SE VA DE KERNEL
		estadoEnEjecucion->empiezaAEjecutar = tomarTiempo();
	}


	return;
}

void calcularHRRN(t_infopcb* unProceso){

	//ESTA LOGICA VA CUANDO SE RECIBE EL CONTEXTO DE CPU
//	//tiempo transcurridoEnCpu
//	uint32_t tiempoRealCPU = unProceso->terminaEjecutar - unProceso->empiezaAEjecutar; //(falta poner terminaEjecutar)
//
//	unProceso->estimadoProxRafaga = unProceso->estimadoAnterior * hrrn_alfa + tiempoRealCPU * (1 - hrrn_alfa);


	//tiempo transcurrido en la cola de Ready
	int tiempoEsperaReady = unProceso->empiezaAEjecutar - unProceso->entraEnColaReady;

	unProceso->rafaga = (tiempoEsperaReady + unProceso->estimadoProxRafaga) / unProceso->estimadoProxRafaga;

	unProceso->estimadoAnterior = unProceso->estimadoProxRafaga;

	//max R = (w + s) / s     -> w = tiempo de espera en ready

	// E = ESTIMACION_INICIAL         S =  Estimacion = estimacion anterior * hrrn_alfa + real CPU (1 - hrrn_alfa)
	//estimacion anterior en la primera es estimacion inicial
	//estimacion anterior despues es S
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

//desencolar
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
        printf("Tabla de segmentos:\n");
        t_nodoTablaSegmentos* tabla = frenteColaNew->info_pcb->tablaSegmentos;
        while (tabla != NULL) {
        	printf("Id: %p\n", tabla->info_tablaSegmentos.id);
            printf("Base: %p\n", tabla->info_tablaSegmentos.direccionBase);
            printf("Base: %p\n", tabla->info_tablaSegmentos.tamanio);
            tabla = tabla->sgte;
        }
        printf("Estimado próxima ráfaga: %d\n", frenteColaNew->info_pcb->estimadoProxRafaga);
        printf("Tiempo llegada a Ready: %d\n", frenteColaNew->info_pcb->entraEnColaReady);
        printf("Punteros a archivos:\n");
        t_nodoArchivos* punteros = frenteColaNew->info_pcb->punterosArchivos;
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

void agregarElementoListaReady(t_nodoCola** lista, t_infopcb* info) {
    // Crear un nuevo nodo
	t_nodoCola* nuevoNodo = (t_nodoCola*)malloc(sizeof(t_nodoCola));
    nuevoNodo->info_pcb = info;
    nuevoNodo->sgte = NULL;

    // Si la lista está vacía, el nuevo nodo se convierte en el primer nodo
    if (*lista == NULL) {
        *lista = nuevoNodo;
    } else {
        // Recorrer la lista hasta llegar al último nodo
    	t_nodoCola* nodoActual = *lista;
        while (nodoActual->sgte != NULL) {
            nodoActual = nodoActual->sgte;
        }

        // Enlazar el nuevo nodo al último nodo de la lista
        nodoActual->sgte = nuevoNodo;
    }
}

//void mostrarListaReady(t_list* lista) {
//	t_list* nodoActual;
//	nodoActual = list_create();
//	nodoActual = lista;
//	int c =0;
//
//    // Recorrer la lista y mostrar la información de cada nodo
//    while (nodoActual != NULL) {
//        // Mostrar la información del nodo actual
//    	t_infopcb* primeroLista =malloc(sizeof(t_infopcb));
//    	primeroLista=  list_get(lista,c);
//        printf("PID: %d\n", primeroLista->pid);
//
//                printf("Lista de instrucciones:\n");
//
//                t_list* lista;
//                lista = list_create();
//                lista	=	primeroLista->listaInstrucciones;
//                for (int i = 0; i < list_size(lista); i++) {
//                	char* instruccion = list_get(lista, i);
//                	printf("\t%s\n", instruccion);
//                }
//
//                printf("Program Counter: %d\n", primeroLista->programCounter);
//        //        printf("Registros CPU:\n");
//        //        for (int i = 0; i < 12; i++) {
//        //            printf("Registro %d: %s\n", i, frenteColaNew->info_pcb.registrosCpu[i]);
//        //        }
//                printf("Tabla de segmentos:\n");
//                t_nodoTablaSegmentos* tabla = primeroLista->tablaSegmentos;
//                while (tabla != NULL) {
//                	printf("Id: %p\n", tabla->info_tablaSegmentos.id);
//                    printf("Base: %p\n", tabla->info_tablaSegmentos.direccionBase);
//                    printf("Base: %p\n", tabla->info_tablaSegmentos.tamanio);
//                    tabla = tabla->sgte;
//                }
//                printf("Estimado próxima ráfaga: %f\n", primeroLista->estimadoProxRafaga);
//                printf("Tiempo llegada a Ready: %d\n", primeroLista->tiempoLlegadaReady);
//
//        // Avanzar al siguiente nodo
//        nodoActual = nodoActual->head->next;
//        c++;
//    }
//}

int cantidadElementosListaReady(t_nodoCola* lista) {
    int contador = 0;
    t_nodoCola* nodoActual = lista;

    while (nodoActual != NULL) {
        contador++;
        nodoActual = nodoActual->sgte;
    }

    return contador;
}

t_paquete* empaquetar(t_list* cabeza) {

    t_list_iterator* iterador = list_iterator_create(cabeza);
    t_paquete* paquete = crear_paquete_instrucciones();

    while (list_iterator_has_next(iterador)) {

    	char* siguiente = list_iterator_next(iterador);
    	int tamanio = (strlen(siguiente))+1;
    	agregar_a_paquete(paquete, siguiente,tamanio );

    }
    return paquete;
}

void serializarContexto(int unSocket){

	//VALORES DE PRUEBA, LO PASE ACA PORQUE PROBE YA DIRECTAMENTE QUE USEMOS EL PCB QUE NOS MANDA CONSOLA
	estadoEnEjecucion->programCounter = 0;
	strcpy(estadoEnEjecucion->registrosCpu.AX,"HOLA");
	strcpy(estadoEnEjecucion->registrosCpu.BX,"HOL");
	strcpy(estadoEnEjecucion->registrosCpu.CX,"HO");
	strcpy(estadoEnEjecucion->registrosCpu.DX,"H");
	strcpy(estadoEnEjecucion->registrosCpu.EAX,"HOLAHOLA");
	strcpy(estadoEnEjecucion->registrosCpu.EBX,"HOLAHOL");
	strcpy(estadoEnEjecucion->registrosCpu.ECX,"HOLAHO");
	strcpy(estadoEnEjecucion->registrosCpu.EDX,"HOLA");
	strcpy(estadoEnEjecucion->registrosCpu.RAX,"HOLAHOLAHOLAHOLA");
	strcpy(estadoEnEjecucion->registrosCpu.RBX,"HOLAHOLAHOLA");
	strcpy(estadoEnEjecucion->registrosCpu.RCX,"HOLAHOLA");
	strcpy(estadoEnEjecucion->registrosCpu.RDX,"HOLA");


//	list_add_all(contexto.listaInstrucciones, estadoEnEjecucion.listaInstrucciones);


//	t_list_iterator* iterador = list_iterator_create(contexto.listaInstrucciones);
//	t_list_iterator* iterador2 = list_iterator_create(contexto.listaInstrucciones);

//	contextoPRUEBA.listaInstrucciones->head->data; //CHAR*
//	contextoPRUEBA.listaInstrucciones->head->next; //PUNTERO A LA PROXIMA
//	contextoPRUEBA.listaInstrucciones->elements_count; //INDICE DE LA INSTRUCCION

	//BUFFER

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(int) + sizeof(estadoEnEjecucion->registrosCpu.AX) * 4 + sizeof(estadoEnEjecucion->registrosCpu.EAX) *4 + sizeof(estadoEnEjecucion->registrosCpu.RAX)*4;

	//	while (list_iterator_has_next(iterador)) {
	//
	//		char* siguiente = list_iterator_next(iterador);
	//		int tamanio = (strlen(siguiente))+1;
	//		list_add(contextoPRUEBA.listaInstrucciones_length, tamanio);
	//		buffer->size += tamanio + sizeof(int);
	//	}

	void* stream = malloc(buffer->size);
	int offset = 0; //desplazamiento

	memcpy(stream + offset, &estadoEnEjecucion->programCounter, sizeof(int));
	offset += sizeof(int); //No tiene sentido seguir calculando el desplazamiento, ya ocupamos el buffer completo

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

//	while (list_iterator_has_next(iterador2)) {
//		int indice = 0;
//		char* siguiente = list_iterator_next(iterador2);
//		memcpy(stream + offset, list_get(contextoPRUEBA.listaInstrucciones_length, indice), sizeof(int));
//		offset += sizeof(int);
//		memcpy(stream + offset, list_get(contextoPRUEBA.listaInstrucciones, indice), strlen(list_get(contextoPRUEBA.listaInstrucciones, indice)) + 1);
//		offset += sizeof(strlen(list_get(contextoPRUEBA.listaInstrucciones, indice)) + 1);
//		indice++;
//	}


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
	//free(contextoPRUEBA.instruccion);
	// Liberamos la memoria
	free(a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);

	return;
}


char* recibir_handshake(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	//log_info(logger, "Me llego el mensaje %s", buffer);
	return buffer;
}


