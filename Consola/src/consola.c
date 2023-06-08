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
	char* linea2 = string_new();
//	char* linea3 = string_new();
//	char* linea4 = string_new();
	t_list * lista = list_create();
	t_list * lista2 = list_create();
//	t_list * lista3 = list_create();
//	t_list * lista4 = list_create();
	FILE* archivo;
	FILE* archivo2;
//	FILE* archivo3;
//	FILE* archivo4;
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
	archivo2 = fopen("/home/utnso/tp-2023-1c-Los-operadores/Consola/prueba2.txt", "r");
//	archivo3 = fopen("/home/utnso/tp-2023-1c-Los-operadores/Consola/prueba3.txt", "r");
//	archivo4 = fopen("/home/utnso/tp-2023-1c-Los-operadores/Consola/prueba4.txt", "r");
	//"/home/utnso/tp-2023-1c-Los-operadores/Consola/prueba.txt"

	if (archivo == NULL) {
		fprintf(stderr, "Error al abrir el archivo.\n");
		exit(1);
	}
	if (archivo2 == NULL) {
			fprintf(stderr, "Error al abrir el archivo2.\n");
			exit(1);
		}


	while (fgets(linea, 99, archivo) != NULL) {
			char* nueva_linea = string_new();
			nueva_linea = string_duplicate(linea);
			list_add(lista,nueva_linea);
		}

	fclose(archivo);

	while (fgets(linea2, 99, archivo2) != NULL) {
			char* nueva_linea2 = string_new();
			nueva_linea2 = string_duplicate(linea2);
			list_add(lista2,nueva_linea2);
			}

	fclose(archivo2);

//	while (fgets(linea3, 99, archivo3) != NULL) {
//				char* nueva_linea3 = string_new();
//				nueva_linea3 = string_duplicate(linea3);
//				list_add(lista3,nueva_linea3);
//				}
//
//		fclose(archivo3);
//
//	while (fgets(linea4, 99, archivo4) != NULL) {
//				char* nueva_linea4 = string_new();
//				nueva_linea4 = string_duplicate(linea4);
//				list_add(lista4,nueva_linea4);
//				}
//
//		fclose(archivo4);

	t_list_iterator* iterador = list_iterator_create(lista);
	t_paquete* paquete = empaquetar(lista);

	t_list_iterator* iterador2 = list_iterator_create(lista2);
	t_paquete* paquete2 = empaquetar(lista2);

	//t_list_iterator* iterador3 = list_iterator_create(lista3);
//	t_paquete* paquete3 = empaquetar(lista3);
//
//	t_list_iterator* iterador4 = list_iterator_create(lista4);
//	t_paquete* paquete4 = empaquetar(lista4);

	enviar_mensaje("consola",conexion_kernel); //Envía el handshake
	enviar_paquete(paquete, conexion_kernel);
	enviar_paquete(paquete2, conexion_kernel);
	//enviar_paquete(paquete3, conexion_kernel);
//	enviar_paquete(paquete4, conexion_kernel);

	eliminar_paquete(paquete);
	eliminar_paquete(paquete2);
//	eliminar_paquete(paquete3);
//	eliminar_paquete(paquete4);

	liberarConexiones(conexion_kernel, logger, config);
 //}
}


void liberarConexiones(int conexion, t_log* logger, t_config* config){
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion);
}
