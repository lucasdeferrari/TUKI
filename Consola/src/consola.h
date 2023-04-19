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


t_log* iniciar_logger(void);
t_config* iniciar_config(void);
void leer_consola(t_log*);
void paquete(int);
void terminar_programa(int, t_log*, t_config*);
int verificarConfig(char*);
void copiarConfigs(char*);

#endif /* CONSOLA_H_ */
