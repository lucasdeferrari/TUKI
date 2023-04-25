#include "consola.h"

#define PATH_CONFIG_GLOBALES "/home/utnso/Documents/tp-2023-1c-Los-operadores/configs.config"
#define PATH_LOG "/home/utnso/Documents/tp-2023-1c-Los-operadores/Consola/consola.log"

#define F_READ "F_READ"
#define F_WRITE "F_WRITE"
#define SET "SET"
#define MOV_IN "MOV_IN"
#define MOV_OUT "MOV_OUT"
#define F_TRUNCATE "F_TRUNCATE"
#define F_SEEK "F_SEEK"
#define CREATE_SEGMENT "CREATE_SEGMENT"
#define I_O "I/O"
#define WAIT "WAIT"
#define SIGNAL "SIGNAL"
#define F_OPEN "F_OPEN"
#define F_CLOSE "F_CLOSE"
#define DELETE_SEGMENT "DELETE_SEGMENT"
#define EXIT "EXIT"
#define YIELD "YIELD"

struct Nodo {
    char* linea;
    struct Nodo* siguiente;
};typedef struct Nodo Nodo;

void insertar(Nodo** cabeza, char* linea) {
    Nodo* nuevo_nodo = (Nodo*)malloc(sizeof(Nodo));
    nuevo_nodo->linea = linea;
    nuevo_nodo->siguiente = NULL;

    Nodo* ultimo = *cabeza;

    if (*cabeza == NULL) {
        *cabeza = nuevo_nodo;
        return;
    }

    while (ultimo->siguiente != NULL) {
        ultimo = ultimo->siguiente;
    }

    ultimo->siguiente= nuevo_nodo;
    return;
}


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
	}else{
		t_config* configConsola = config_create(path);
		config_save_in_file(configConsola, PATH_CONFIG_GLOBALES);
	}
}

char** split(char* cadena, char delimitador, int* numSubcadenas) {
    int longitudCadena = strlen(cadena);
    int numDelimitadores = 0;
    for (int i = 0; i < longitudCadena; i++) {
        if (cadena[i] == delimitador) {
            numDelimitadores++; // Contar el número de delimitadores
        }
    }

    *numSubcadenas = numDelimitadores + 1;

    char** subcadenas = (char*)malloc((*numSubcadenas) * sizeof(char)); // Reservar memoria para el arreglo de subcadenas
    int inicioSubcadena = 0;
    int numCaracteresSubcadena = 0;
    int indiceSubcadena = 0;

	for (int i = 0; i < longitudCadena; i++) {
		 if (cadena[i] == delimitador || i == longitudCadena - 1) {
			 if (i == longitudCadena - 1 && cadena[i] != delimitador) {
				 numCaracteresSubcadena++; // Agregar el último caracter si no es un delimitador
				}
			 subcadenas[indiceSubcadena] = (char*)malloc((numCaracteresSubcadena + 1) * sizeof(char)); // Reservar memoria para la subcadena
			 strncpy(subcadenas[indiceSubcadena], cadena + inicioSubcadena, numCaracteresSubcadena); // Copiar la subcadena en el arreglo de subcadenas
			 subcadenas[indiceSubcadena][numCaracteresSubcadena] = '\0'; // Agregar el caracter nulo al final de la subcadena
			 inicioSubcadena = i + 1; // Actualizar el inicio de la siguiente subcadena
			 numCaracteresSubcadena = 0; // Reiniciar el contador de caracteres
			 indiceSubcadena++; // Incrementar el índice del arreglo de subcadenas
		 } else {
			 numCaracteresSubcadena++; // Contar el número de caracteres en la subcadena actual
		 }
	}
 return subcadenas;
}



char* nombreModulo(char* path){
	int numSubcadenas;
	char** subcadenas = split(path, '/', &numSubcadenas);
	char* file = subcadenas[numSubcadenas - 1];
	int numSubcadenas1;
	char** subcadenas1 = split(file, '.', &numSubcadenas1);

	char* modulo = subcadenas1[0];

	free(subcadenas1); // Liberar memoria para el arreglo de subcadenas
	free(subcadenas); // Liberar memoria para el arreglo de subcadenas

	return modulo;
}

t_paquete* empaquetar(Nodo* cabeza) {
    Nodo* actual = cabeza;
    t_paquete* paquete;

    paquete = crear_paquete();

    while (actual != NULL) {
    	agregar_a_paquete(paquete, actual->linea, strlen(actual->linea));
        actual = actual->siguiente;
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

char linea[200];
Nodo* cabeza = NULL;
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
config = config_create("../consola.config");

if (config == NULL) {
	printf("No se pudo crear el config.");
	exit(1);
}

ip_kernel = config_get_string_value(config, "IP_KERNEL");
puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");

// Creamos una conexión hacia el servidor
conexion_kernel = crear_conexion(ip_kernel, puerto_kernel);


//copiarConfigs(pathConfig); //Copia las config pasadas a un archivo global

printf("%s \n", nombreModulo(pathProgram));

archivo = fopen("/home/utnso/tp-2023-1c-Los-operadores/Consola/prueba.txt", "r");

if (archivo == NULL) {
	fprintf(stderr, "Error al abrir el archivo.\n");
	exit(1);
}

//char* ultima_linea = NULL;

while (fgets(linea, sizeof(linea), archivo) != NULL) {
		char* nueva_linea = (char*)malloc(strlen(linea));
		strcpy(nueva_linea, linea);
		insertar(&cabeza, nueva_linea);
	}

// Verificar si la última línea se ha agregado correctamente a la lista

//if (strlen(linea) > 0 && linea[strlen(linea)-1] != '\n') {
//    char* nueva_linea = (char*)malloc(strlen(linea) + 2);
//    strcpy(nueva_linea, linea);
//    strcat(nueva_linea, "\n");
//    insertar(&cabeza, nueva_linea);
//}

fclose(archivo);

//while (fgets(linea, sizeof(linea), archivo) != NULL) {
//		char* nueva_linea = (char*)malloc(strlen(linea) + 1);
//		strcpy(nueva_linea, linea);
//		insertar(&cabeza, nueva_linea);
//		if (feof(archivo)) {  // Verificar si la línea actual es la última
//			if (ultima_linea == NULL || strcmp(ultima_linea, nueva_linea) != 0) {
//				ultima_linea = nueva_linea;
//			} else {
//				free(nueva_linea);
//			}
//		}
//	}
//
//fclose(archivo);

// Agregar la última línea a la lista si no se ha agregado antes
//if (ultima_linea != NULL) {
//    insertar(&cabeza, ultima_linea);
//}

// CHEQUEAR SI ANDA EL EMPAQUETADO CON KERNEL
t_paquete* paquete = empaquetar(cabeza);
enviar_paquete(paquete, conexion_kernel);

//imprimir(cabeza);

eliminar_paquete(paquete);

liberarConexiones(conexion_kernel, logger, config);

//}
}


//void paquete(int conexion){
//	char* leido;
//	t_paquete* paquete;
//
//	paquete = crear_paquete();
//	leido = readline("> ");
//
//	while(strcmp(leido, "") != 0){
//		agregar_a_paquete(paquete, leido, strlen(leido));
//		leido = readline("> ");
//	}
//
//	enviar_paquete(paquete, conexion);
//
//	free(leido);
//	eliminar_paquete(paquete);
//}



void liberarConexiones(int conexion, t_log* logger, t_config* config){
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion);
}
