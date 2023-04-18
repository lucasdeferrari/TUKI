#include "consola.h"

#define PATH_CONFIG_GLOBALES "/home/utnso/Documents/tp-2023-1c-Los-operadores/configs.config"

int verificarConfig(char* path){
	if (config_create(path) != NULL) //Si existen las config devuelve un 1 sino un -1
		return 1;
	else
		return -1;
}

void copiarConfigs(char* path){

	if(verificarConfig(path) == -1){
		fprintf(stderr,"Error al abrir las configs\n");
		exit(1);
	} else{
		t_config* configConsola = config_create(path);
		config_save_in_file(configConsola, PATH_CONFIG_GLOBALES);
	}
}

int main(int argc, char *argv[]) {
	//Aqui se verifica si introducieron todos los archivos necesarios para consola
	// 0 -> mismo archivo
	// 1 -> configs
	// 2 -> pseudocodigo
	if(argc == 3){
		char* pathConfig = argv[1];
		char* pathCode = argv[2];
		copiarConfigs(pathConfig); //Copia las config pasadas a un archivo global

	} else
		exit(1);

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

	log_info(logger, "Ingrese sus mensajes para el kernel: ");

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
