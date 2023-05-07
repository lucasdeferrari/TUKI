#ifndef CONSOLA_H_
#define CONSOLA_H_
#define PATH_CONFIG_GLOBALES "/home/utnso/tp-2023-1c-Los-operadores/configs.config"

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include "utils.h"

void paquete(int);
void terminar_programa(int, t_log*, t_config*);
short verificacionPseudoCodigo(char* path);

#endif /* CONSOLA_H_ */
