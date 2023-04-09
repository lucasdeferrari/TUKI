#include "consola.h"

#include <readline/readline.h>

int main(void)
{
	int conexion;
	char* ip;
	char* puerto;
	char* valor;

	t_log* logger;
	t_config* config;

	logger = log_create("../consola.log", "Consola", true, LOG_LEVEL_INFO);

	config = config_create("./consola.config");

	if (config == NULL) {
		printf("No se pudo crear el config.");
		exit(1);
	}

	ip = config_get_string_value(config, "IP");
	puerto = config_get_string_value(config, "PUERTO");
	log_info(logger, "El puerto es %s", puerto);

	// Creamos una conexión hacia el servidor

	log_info(logger,"");

	conexion = crear_conexion(ip, puerto);

	log_info(logger, "despues de conexion");

	paquete(conexion);

	terminar_programa(conexion, logger, config);
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

void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion);
	exit(95);
}
