#include "Memoria.h"

char* ip_memoria;
char* puerto_memoria;

void iterator(char* value) {
	log_info(logger,"%s", value);
}

int main(void) {

	t_list* lista;

	logger = log_create("memoria.log", "Memoria", 1, LOG_LEVEL_DEBUG);
	int server_fd = iniciar_servidor();


		log_info(logger, "Memoria lista para recibir al cliente");
		int cliente_fd = esperar_cliente(server_fd);

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
			case -1:
				log_error(logger, "el cliente se desconecto. Terminando servidor");

				return EXIT_FAILURE;
			default:
				log_warning(logger,"Operacion desconocida. No quieras meter la pata");
				break;
			}
		}


	return EXIT_SUCCESS;

}

//void iniciarHiloServer() {
//
//    int err = pthread_create( &serverMemoria_thread,	// puntero al thread
//    	            NULL,
//    	        	&serverMemoria, // le paso la def de la función que quiero que ejecute mientras viva
//    				NULL); // argumentos de la función
//
//    	     if (err != 0) {
//    	      printf("\nNo se pudo crear el hilo de la conexión.\n");
//    	      exit(7);
//    	     }
//    	     printf("\nEl hilo de la conexión se creo correctamente.\n");
//
//}
//
//
//void* serverMemoria(void* ptr){
//
//	//sem_wait(&semKernelClientFileSystem);
//
//    int server_fd = iniciar_servidor();
//    log_info(logger, "Memoria lista para recibir al cliente\n");
//    int cliente_fd = esperar_cliente(server_fd);
//
//    t_list* lista;
//    while (1) {
//    	int cod_op = recibir_operacion(cliente_fd);
//    	switch (cod_op) {
//    		case MENSAJE:
//    			recibir_mensaje(cliente_fd);
//    			break;
//    		case PAQUETE:
//    			lista = recibir_paquete(cliente_fd);
//    			log_info(logger, "Me llegaron los siguientes valores:\n");
//    			list_iterate(lista, (void*) iterator);
//    			break;
//    		case -1:
//    			log_error(logger, "\nel cliente se desconecto. Terminando servidor");
//    			return EXIT_FAILURE;
//    			default:
//    			log_warning(logger,"\nOperacion desconocida. No quieras meter la pata");
//    		break;
//    	}
//    }
//
//    //sem_post(&semKernelServer);
//
//	return NULL;
//}


