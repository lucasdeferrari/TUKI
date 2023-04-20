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



char** split(char* cadena, char delimitador, int* numSubcadenas) {

    int longitudCadena = strlen(cadena);

    int numDelimitadores = 0;

    for (int i = 0; i < longitudCadena; i++) {

        if (cadena[i] == delimitador) {

            numDelimitadores++; // Contar el número de delimitadores

        }

    }



    *numSubcadenas = numDelimitadores + 1;

    char** subcadenas = (char**)malloc((*numSubcadenas) * sizeof(char*)); // Reservar memoria para el arreglo de subcadenas



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

char* imprimirPalabra(FILE *archivo) {
	char* line = NULL;
	char linea[100] ;
	char caracter;
	char *palabra = linea;

while ((caracter = fgetc(archivo)) != '\n') {
	*palabra += caracter;
}
//*palabra = '\0'; // se agrega el caracter nulo al final de la línea


//fscanf(archivo, "%s", palabra);
printf("%s", linea);
if (linea== EXIT ){
//funcion_exit();
	printf("entre a exit");
}
if (palabra == EOF) {
	return 'z';
}
return palabra;

}

short verificacionPseudoCodigo(char* path){

	// verificar el path

	char* linea[100];
	char** linea1 = linea;
	char *palabraLeida;
	FILE* archivo;
	archivo = fopen(path, "r");



	if (archivo == NULL) {

		fprintf(stderr, "Error al abrir el archivo.\n");
	    exit(1); }




palabraLeida = imprimirPalabra(archivo);
while(palabraLeida != 'z') {
	palabraLeida = imprimirPalabra(archivo);
	**linea1 += palabraLeida;
}

printf("%s",linea);

//			int words = 0;
//
//			char** instructions = split(line, ' ', words);
//
//			printf("La linea leida es %s\n", line);
//
//
//
//			for(int i = 0; i < words; i++)
//
//				printf("%s \n", instructions[i]);



			//char* instruction = instructions[0];

//			if (strcmp(instruction, F_READ) != 0)

//				if(words != 4)

//					exit(1);

//			if (strcmp(instruction, F_WRITE) != 0)

//				if(words != 4)

//					exit(1);

//			if (strcmp(instruction, SET) != 0)

//				if(words != 3)

//					exit(1);

//			if (strcmp(instruction, MOV_IN) != 0)

//				if(words != 3)

//					exit(1);

//			if (strcmp(instruction, MOV_OUT) != 0)

//				if(words != 3)

//					exit(1);

//			if (strcmp(instruction, F_TRUNCATE) != 0)

//				if(words != 3)

//					exit(1);

//			if (strcmp(instruction, F_SEEK) != 0)

//				if(words != 3)

//					exit(1);

//			if (strcmp(instruction, CREATE_SEGMENT) != 0)

//				if(words != 3)

//					exit(1);

//			if (strcmp(instruction, I_O) != 0)

//				if(words != 2)

//					exit(1);

//			if (strcmp(instruction, WAIT) != 0)

//				if(words != 2)

//					exit(1);

//			if (strcmp(instruction, SIGNAL) != 0)

//				if(words != 2)

//					exit(1);

//			if (strcmp(instruction, F_OPEN) != 0)

//				if(words != 2)

//					exit(1);

//			if (strcmp(instruction, F_CLOSE) != 0)

//				if(words != 2)

//					exit(1);

//			if (strcmp(instruction, DELETE_SEGMENT) != 0)

//				if(words != 2)

//					exit(1);

//			if (strcmp(instruction, EXIT) != 0)

//				if(words != 1)

//					exit(1);

//			if (strcmp(instruction, YIELD) != 0)

//				if(words != 1)

//					exit(1);

			//line = fgets(line, 1,f);

		}



//}



int main(int argc, char *argv[]) {

	//Aqui se verifica si introducieron todos los archivos necesarios para consola

	// 0 -> mismo archivo

	// 1 -> configs

	// 2 -> pseudocodigo

	if(argc == 3){

		char* pathProgram = argv[0];

		char* pathConfig = argv[1];

		char* pathCode = argv[2];

		copiarConfigs(pathConfig); //Copia las config pasadas a un archivo global

		printf("%s \n", nombreModulo(pathProgram));

		verificacionPseudoCodigo(pathCode);

	} else
		verificacionPseudoCodigo("prueba.txt");

		exit(1);



	int conexion_kernel;

	char* ip_kernel;

	char* puerto_kernel;



	t_log* logger;

	t_config* config;



	logger = log_create(PATH_LOG, "Consola", true, LOG_LEVEL_INFO);



	config = config_create(PATH_CONFIG_GLOBALES);



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
