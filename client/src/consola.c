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

	logger = iniciar_logger();

	logger = log_create("/home/utnso/tp/tp-2023-1c-Los-operadores/tp/client/src/consola.log", "Consola", true, LOG_LEVEL_INFO);

	// Usando el logger creado previamente
	// Escribi: "Hola! Soy un log"

	//log_info(logger, "Hola! Soy un Log");

	/* ---------------- ARCHIVOS DE CONFIGURACION ---------------- */

	config = iniciar_config();

	config = config_create("/home/utnso/tp/tp-2023-1c-Los-operadores/tp/client/consola.config");

//	// Usando el config creado previamente, leemos los valores del config y los
//	// dejamos en las variables 'ip', 'puerto' y 'valor'
//
	if (config == NULL) {
		printf("No se pudo crear el config.");
		exit(5);
	}

	ip = config_get_string_value(config, "IP_KERNEL");
	//printf("El ip es: %s", ip);
	puerto = config_get_string_value(config, "PUERTO_KERNEL");
	printf("El puerto es: %s", puerto);


	// Creamos una conexión hacia el servidor

	log_info(logger,"antes de conexion");

	conexion = crear_conexion(ip, puerto);

	log_info(logger, "despues de conexion");


	// Armamos y enviamos el paquete
	paquete(conexion);

	terminar_programa(conexion, logger, config);



	/*---------------------------------------------------PARTE 5-------------------------------------------------------------*/
	// Proximamente
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

	paquete = crear_paquete();

	// Leemos y esta vez agregamos las lineas al paquete
	leido = readline("> ");

	while(strcmp(leido, "") != 0){
		agregar_a_paquete(paquete, leido, strlen(leido));
		leido = readline("> ");
	}

	enviar_paquete(paquete, conexion);

	// ¡No te olvides de liberar las líneas y el paquete antes de regresar!

	free(leido);
	eliminar_paquete(paquete);

}

void terminar_programa(int conexion, t_log* logger, t_config* config)
{https://docs.utnso.com.ar/guias/herramientas/debugger
	/* Y por ultimo, hay que liberar lo que utilizamos (conexion, log y config)
	  con las funciones de las commons y del TP mencionadas en el enunciado */

	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion);
	exit(95);
}
