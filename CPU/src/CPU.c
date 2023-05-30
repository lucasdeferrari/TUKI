#include "CPU.h"
t_config* config;

void iniciar_ejecucion(){

	//MIENTRAS LA INSTRUCCION NO SEA EXIT, I/O, WAIT, YIELD.   ( O UN ERROR, F_*, *_SEGMENT )
	//HAY QUE SEGUIR EJECUTANDO LAS INSTRUCCIONES Y ACTUALIZANDO EL CONTEXTO PARA ENVIARLO DESPUÉS A KERNEL

	int continuarLeyendo = 1;

	while(continuarLeyendo>0){

		//list_get retorna el contenido de una posicion determianda de la lista
		char* proximaInstruccion = list_get(contexto->listaInstrucciones, contexto->programCounter);

		printf("INSTRUCCION A EJECUTAR: %s\n", proximaInstruccion );

		//ejecutarFuncion: ejecuta la función que corresponde y retorna un int para saber si debe seguir ejecutando
		continuarLeyendo = ejecutarFuncion(proximaInstruccion);

	}

	//Una vez que no se deba seguir ejecutando va a serializar el contexto actualizado y lo va a enviar
	// serializarContexto()

	return;
}

int ejecutarFuncion(char* proximaInstruccion){

	int continuarLeyendo = 0; //valor de prueba por ahora
//char**  string_split(char * text, char * separator);
	char** arrayInstruccion = string_split(proximaInstruccion, " ");
	char* nombreInstruccion = arrayInstruccion[0];
	printf("%s", nombreInstruccion);
	switch (nombreInstruccion) {
	    		case SET:

	    		case YIELD:

	    			break;
	    		case EXIT:

	    			break;
	    		case CONTEXTO:

	    			break;
	    		case -1:
	    			log_error(logger, "\nel kernel se desconecto. Terminando servidor");
	    			return EXIT_FAILURE;
				default:
					log_warning(logger,"\nOperacion desconocida. No quieras meter la pata");
					break;
	    	}
	    }
	//Reconoce la funcion (con split y head) y  despues sus parámetros (que se pasarian directo a la funcion)
	//Ejecuta la función actualizando el contexto
	//Dependiendo del tipo de funcion va a asignarle el valor correspondiente a continuarLeyendo

	return continuarLeyendo;
}


int main(void) {

	sem_init(&semCPUServer,0,1);
	sem_init(&semCPUClientMemoria,0,0);

    logger = log_create("CPU.log", "CPU", 1, LOG_LEVEL_DEBUG);

    config = config_create("/home/utnso/tp-2023-1c-Los-operadores/CPU/CPU.config");

    if (config == NULL) {
        printf("No se pudo crear el config.");
        exit(5);
    }

    ip_memoria= config_get_string_value(config, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");

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
	printf("\nNo se pudo crear el hilo del cliente Memoria del CPU.");
	exit(7);
	}
	printf("El hilo cliente de la Memoria se creo correctamente.");

}

void* clientMemoria(void* ptr) {
	int config = 1;
    int conexion_Memoria;
    char buffer[1024] = {0};

    conexion_Memoria = crear_conexion(ip_memoria, puerto_memoria);
    enviar_mensaje("CPU",conexion_Memoria);
    log_info(logger, "Ingrese sus mensajes para la Memoria: ");
    paquete(conexion_Memoria);
    int cod_op = recibir_operacion(conexion_Memoria);
    printf("codigo de operacion: %i\n", cod_op);
    recibir_mensaje(conexion_Memoria);
//    int valread = read(conexion_Memoria, buffer, 1024);
//    printf("Respuesta del servidor: %s\n", buffer);
    liberar_conexion(conexion_Memoria);


    sem_post(&semCPUClientMemoria);
	return NULL;
}

void iniciarHiloServer() {

    int err = pthread_create( 	&serverCPU_thread,	// puntero al thread
    	            			NULL,
								&serverCPU, // le paso la def de la función que quiero que ejecute mientras viva
								NULL); // argumentos de la función

	 if (err != 0) {
	  printf("\nNo se pudo crear el hilo de la conexión kernel-CPU \n");
	  exit(7);
	 }
	 printf("\nEl hilo de la conexión kernel-CPU se creo correctamente.\n");

}

void* serverCPU(void* ptr){

	sem_wait(&semCPUClientMemoria);

    int server_fd = iniciar_servidor();
    log_info(logger, "CPU lista para recibir al Kernel");
    int cliente_fd = esperar_cliente(server_fd);

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
    			break;
    		case INSTRUCCIONES:
    			contexto->listaInstrucciones = recibir_paquete(cliente_fd);
    			log_info(logger, "Instrucciones recibidas de Kernel:\n");
    			list_iterate(contexto->listaInstrucciones, (void*) iterator);
    			iniciar_ejecucion(); //Por ahora de prueba
    			break;
    		case CONTEXTO:
    			contexto = recibir_contexto(cliente_fd);
    			printf("\nprogramCounter recibido de Kernel = %d\n",contexto->programCounter);
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

    			break;
    		case -1:
    			log_error(logger, "\nel kernel se desconecto. Terminando servidor");
    			return EXIT_FAILURE;
			default:
				log_warning(logger,"\nOperacion desconocida. No quieras meter la pata");
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

// FUNCIONES INSTRUCCIONES


// REVISAR FUNCION SET
// SET: (Registro, Valor): Asigna al registro el valor pasado como parámetro.
void set_tp(char* reg, char* valor){
	strcpy(reg, valor);
	//delay 1000ml
}

//COMPLETAT FUNCIONES


// YIELD: Esta instrucción desaloja voluntariamente el proceso de la CPU.
void yield_tp(){

	//strcpy(contexto->instruccion, "yield");
	contexto->programCounter++;

    return;
}

// EXIT: Esta instrucción representa la syscall de finalización del proceso.
void exit_tp(){

	//strcpy(contexto->instruccion, "exit");
	contexto->programCounter++;

	return;
}


//I/O (Tiempo): Esta instrucción representa una syscall de I/O bloqueante.
//Se deberá devolver el Contexto de Ejecución actualizado al Kernel junto a la cantidad de unidades de tiempo que va a bloquearse el proceso
void i_o_tp() {

	//strcpy(contexto->instruccion, "i/o");
	contexto->programCounter++;
	//contexto->tiempoBloqueado = 1000;

	return;
}

////WAIT (Recurso): Esta instrucción solicita al Kernel que se asigne una instancia del recurso indicado por parámetro.
void wait_tp() {

		contexto->programCounter++;
		//strcpy(contexto->instruccion, "wait");


		//strcpy(contexto->recursoSolicitado, recurso);


		return;
}

//SIGNAL (Recurso): Esta instrucción solicita al Kernel que se libere una instancia del recurso indicado por parámetro.
void signal_tp() {

	contexto->programCounter++;
	//strcpy(contexto->instruccion, "signal");


	return;
}
