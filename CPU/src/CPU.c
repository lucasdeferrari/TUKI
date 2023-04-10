#include "CPU.h"

t_config* config;

int main(void) {
    logger = log_create("CPU.log", "CPU", 1, LOG_LEVEL_DEBUG);



    config = iniciar_config();
    config = config_create("/home/utnso/tp/tp-2023-1c-Los-operadores/CPU/CPU.config");

    if (config == NULL) {
        printf("No se pudo crear el config.");
        exit(5);
    }

//    ip_cpu = config_get_string_value(config, "IP_CPU");
//    puerto_cpu = config_get_string_value(config, "PUERTO_CPU");
//    ip_memoria = config_get_string_value(config, "IP_MEMORIA");
//    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
//    ip_filesystem = config_get_string_value(config, "IP_FILESYSTEM");
//    puerto_filesystem = config_get_string_value(config, "PUERTO_FILESYSTEM");


    return EXIT_SUCCESS;
}



void* serverCPU(void* ptr){

    int server_fd = iniciar_servidor();
    log_info(logger, "CPU listo para recibir al cliente");
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
    		case -1:
    			log_error(logger, "el cliente se desconecto. Terminando servidor");
    			return EXIT_FAILURE;
    			default:
    			log_warning(logger,"Operacion desconocida. No quieras meter la pata");
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
