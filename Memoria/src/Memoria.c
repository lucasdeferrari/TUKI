//Crear un archivo global de funciones comunes
#include "Memoria.h"

char* ip_memoria;
char* puerto_memoria;
int server_fd;

void iterator(char* value) {
	log_info(logger,"%s", value);
}

int main(void) {

	logger = log_create("memoria.log", "Memoria", 1, LOG_LEVEL_DEBUG);
	server_fd = iniciar_servidor();
	log_info(logger, "Memoria lista para recibir al cliente\n");

	while(1){
		iniciarHiloServer();
		pthread_join(serverMemoria_thread, NULL);
	}

	return EXIT_SUCCESS;

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
    	     //printf("\nEl hilo de la conexión se creo correctamente.\n");

}


void* serverMemoria(void* ptr){

	//sem_wait(&semKernelClientFileSystem);

	//int server_fd = iniciar_servidor();
    int cliente_fd = esperar_cliente(server_fd);

    t_list* lista;
    while (1) {
    	int cod_op = recibir_operacion(cliente_fd);
    	switch (cod_op) {
    		case MENSAJE:

    			char* handshake = recibir_handshake(cliente_fd);

    			if (strcmp(handshake, "kernel") == 0) {
    				//cosas de kernel
    				log_info(logger, "se conecto el kernel");
    			}
    			if (strcmp(handshake, "CPU") == 0) {
    				log_info(logger, "se conecto la cpu");
    				//cosas de cpu
    			}
    			if (strcmp(handshake, "filesystem") == 0) {
    				log_info(logger, "se conecto el filesystem");
    				//cosas de fs
    			}
    			free(handshake);
    			break;
    		case PAQUETE:
    			lista = recibir_paquete(cliente_fd);
    			log_info(logger, "Me llegaron los siguientes valores:");
    			list_iterate(lista, (void*) iterator);
    			break;
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

char* recibir_handshake(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	//log_info(logger, "Me llego el mensaje %s", buffer);
	return buffer;
}

