#include "Consola.h"

int main(int argc, char* argv[]) {
<<<<<<< HEAD
=======



>>>>>>> d2f70ef (Conexion inicial Consola-Kernel)
	int conexion;
	char* ip;
	char* puerto;
	char* valor;

	t_log* logger;
	t_config* config;

	logger = iniciar_logger();

	logger = log_create("tp.log", "Log", true, LOG_LEVEL_INFO);

	config = config_create("consola.config");

	if (config == NULL) {
		exit(1);
	}

	ip = config_get_string_value(config, "IP");
	puerto = config_get_string_value(config, "PUERTO");
	valor = config_get_string_value(config, "CLAVE");

	// Loggeamos el valor de config
	log_info(logger, valor);

	// Creamos una conexión hacia el servidor
	conexion = crear_conexion(ip, puerto);

	// Enviamos al servidor el valor de CLAVE como mensaje
	enviar_mensaje(valor, conexion);

	// Armamos y enviamos el paquete
	paquete(conexion);

<<<<<<< HEAD
=======

>>>>>>> d2f70ef (Conexion inicial Consola-Kernel)
	terminar_programa(conexion, logger, config);

	log_destroy(logger);
}

t_log* iniciar_logger(void)
{
	t_log* nuevo_logger = NULL;

	return nuevo_logger;
}

t_config* iniciar_config(void)
{
	t_config* nuevo_config = NULL;

	return nuevo_config;
}

void leer_consola(t_log* logger)
{
	char* leido;

	// La primera te la dejo de yapa
	leido = readline("> ");

	// El resto, las vamos leyendo y logueando hasta recibir un string vacío
	while(strcmp(leido, "") != 0){
		log_info(logger, leido);
		leido = readline("> ");
	}

	// ¡No te olvides de liberar las lineas antes de regresar!
	free(leido);
}

void paquete(int conexion)
{
	// Ahora toca lo divertido!
	char* leido;
	t_paquete* paquete;

	// Leemos y esta vez agregamos las lineas al paquete
	paquete = crear_paquete();
	leido = readline("> ");
	while(strcmp(leido, "") != 0){
		agregar_a_paquete(paquete, leido, sizeof(leido));
		leido = readline("> ");
	}
	enviar_paquete(paquete, conexion);

	// ¡No te olvides de liberar las líneas y el paquete antes de regresar!
	free(leido);
	free(paquete);
	
}

void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	/* Y por ultimo, hay que liberar lo que utilizamos (conexion, log y config) 
	  con las funciones de las commons y del TP mencionadas en el enunciado */
	free(1);
}
