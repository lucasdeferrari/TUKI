#include "consola.h"

#define PATH_CONFIG_GLOBALES "/home/utnso/Documents/tp-2023-1c-Los-operadores/configs.config"
#define PATH_LOG "/home/utnso/Documents/tp-2023-1c-Los-operadores/Consola/consola.log"

struct Nodo {
    char* linea;
    struct Nodo* siguiente;
};typedef struct Nodo Nodo;

t_paquete* empaquetar(t_list* cabeza) {

    t_list_iterator* iterador = list_iterator_create(cabeza);
    t_paquete* paquete = crear_paquete();

    while (list_iterator_has_next(iterador)) {

    	char* siguiente = list_iterator_next(iterador);
    	int tamanio = (strlen(siguiente))+1;
    	agregar_a_paquete(paquete, siguiente,tamanio );

    }
    return paquete;
}



void imprimir(Nodo* cabeza) {
    Nodo* actual = cabeza;

    while (actual != NULL) {
        printf("%s", actual->linea);
        actual = actual->siguiente;
    }
}

int main(int argc, char *argv[]) {

	//Aqui se verifica si introducieron todos los archivos necesarios para consola

	// 0 -> mismo archivo

	// 1 -> configs

	// 2 -> pseudocodigo

	char* linea = string_new();
	t_list * lista = list_create();
	FILE* archivo;
	int conexion_kernel;
	char* ip_kernel;
	char* puerto_kernel;

	t_log* logger;
	t_config* config;


	//if(argc == 3){
	char* pathProgram = argv[0];
	char* pathConfig = argv[1];
	char* pathCode = argv[2];

	logger = log_create("../consola.log", "Consola", true, LOG_LEVEL_INFO);
	//Cambiar la ruta del config por pathConfig
	config = config_create("../consola.config");
	//"../consola.config"

	if (config == NULL) {
		printf("No se pudo crear el config.");
		exit(1);
	}

	ip_kernel = config_get_string_value(config, "IP_KERNEL");
	puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");

	conexion_kernel = crear_conexion(ip_kernel, puerto_kernel);

	//Cambiar la ruta del archivo por pathCode
	archivo = fopen("/home/utnso/tp-2023-1c-Los-operadores/Consola/prueba.txt", "r");
	//"/home/utnso/tp-2023-1c-Los-operadores/Consola/prueba.txt"

	if (archivo == NULL) {
		fprintf(stderr, "Error al abrir el archivo.\n");
		exit(1);
	}


	while (fgets(linea, 99, archivo) != NULL) {
			char*nueva_linea = string_new();
			nueva_linea = string_duplicate(linea);
			list_add(lista,nueva_linea);
		}

	fclose(archivo);

	t_list_iterator* iterador = list_iterator_create(lista);
	t_paquete* paquete = empaquetar(lista);

	enviar_mensaje("consola",conexion_kernel); //Envía el handshake
	enviar_paquete(paquete, conexion_kernel);

	eliminar_paquete(paquete);

	liberarConexiones(conexion_kernel, logger, config);
 //}
}


void liberarConexiones(int conexion, t_log* logger, t_config* config){
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion);
}
