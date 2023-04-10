#include "consola.h"

int main(void)
{
	int conexion_kernel;
	char* ip_kernel;
	char* puerto_kernel;

	t_log* logger;
	t_config* config;

	logger = log_create("../consola.log", "Consola", true, LOG_LEVEL_INFO);

	config = config_create("./consola.config");

	if (config == NULL) {
		printf("No se pudo crear el config.");
		exit(1);
	}

	ip_kernel = config_get_string_value(config, "IP_KERNEL");
	puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");

	// Creamos una conexión hacia el servidor

	conexion_kernel = crear_conexion(ip_kernel, puerto_kernel);

	log_info(logger, "Ingrese sus mensajes: ");

	paquete(conexion_kernel);

	liberarConexiones(conexion_kernel, logger, config);
}

t_log* iniciar_logger(void)
{
	t_log* nuevo_logger;

	return nuevo_logger;
}

t_config* iniciar_config(void)
{
	t_config* nuevo_config;

	return nuevo_config;
}

void leer_consola(t_log* logger)
{
	char* leido;
	leido = readline("> ");

	while(strcmp(leido, "") != 0){
		log_info(logger, leido);
		leido = readline("> ");
	}

	free(leido);
}

void paquete(int conexion){
	char* leido;
	t_paquete* paquete;

	paquete = crear_paquete();

	leido = readline("> ");

	while(strcmp(leido, "") != 0){
		agregar_a_paquete(paquete, leido, strlen(leido));
		leido = readline("> ");
	}

	enviar_paquete(paquete, conexion);

	free(leido);
	eliminar_paquete(paquete);
}

void liberarConexiones(int conexion, t_log* logger, t_config* config)
{
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion);

}
