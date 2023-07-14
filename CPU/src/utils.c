#include"utils.h"

t_log* logger;

int iniciar_servidor(char* PUERTO)
{
	// Quitar esta línea cuando hayamos terminado de implementar la funcion
	// assert(!"no implementado!");

	int socket_servidor;

	struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;


	getaddrinfo(NULL, PUERTO, &hints, &servinfo);

	socket_servidor = socket(servinfo->ai_family,
	                         servinfo->ai_socktype,
	                         servinfo->ai_protocol);

	// Asociamos el socket a un puerto
	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

	// Escuchamos las conexiones entrantes
	listen(socket_servidor, SOMAXCONN);

	//int socket_cliente;
	//esperar_cliente(socket_servidor);
	freeaddrinfo(servinfo);

	return socket_servidor;
}

int esperar_cliente(int socket_servidor)
{
	int socket_cliente;
	socket_cliente = accept(socket_servidor, NULL, NULL);

	//log_info(logger, "Se conecto un cliente");

	return socket_cliente;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}


t_contextoEjecucion* recibir_contexto(int socket_cliente){

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));


	// Recibimos el buffer.
	//Recibimos el tamaño del buffer
	recv(socket_cliente, &(paquete->buffer->size), sizeof(int), 0);

	//Recibimos el contenido del buffer
	paquete->buffer->stream = malloc(paquete->buffer->size);
	recv(socket_cliente, paquete->buffer->stream, paquete->buffer->size, 0);

	//Desserializamos el contenido

	//Inicializamos estructura de contexto
	t_contextoEjecucion* contextoPRUEBA;
	contextoPRUEBA = malloc(sizeof(t_contextoEjecucion));
	contextoPRUEBA->listaInstrucciones = list_create();

	contextoPRUEBA->instruccion_length = 0;
	contextoPRUEBA->programCounter = 0;
	contextoPRUEBA->pid = 0;
	contextoPRUEBA->tiempoBloqueado = 0;

	for (int i = 0; i < sizeof(contextoPRUEBA->registrosCpu.AX); i++) {
		contextoPRUEBA->registrosCpu.AX[i] = '\0';
	}
	for (int i = 0; i < sizeof(contextoPRUEBA->registrosCpu.BX); i++) {
		contextoPRUEBA->registrosCpu.BX[i] = '\0';
	}
	for (int i = 0; i < sizeof(contextoPRUEBA->registrosCpu.CX); i++) {
		contextoPRUEBA->registrosCpu.CX[i] = '\0';
	}
	for (int i = 0; i < sizeof(contextoPRUEBA->registrosCpu.DX ); i++) {
		contextoPRUEBA->registrosCpu.DX[i] = '\0';
	}
	for (int i = 0; i < sizeof(contextoPRUEBA->registrosCpu.EAX ); i++) {
		contextoPRUEBA->registrosCpu.EAX[i] = '\0';
	}
	for (int i = 0; i < sizeof(contextoPRUEBA->registrosCpu.EBX ); i++) {
		contextoPRUEBA->registrosCpu.EBX[i] = '\0';
	}
	for (int i = 0; i < sizeof(contextoPRUEBA->registrosCpu.ECX ); i++) {
		contextoPRUEBA->registrosCpu.ECX[i] = '\0';
	}
	for (int i = 0; i < sizeof(contextoPRUEBA->registrosCpu.EDX ); i++) {
		contextoPRUEBA->registrosCpu.EDX[i] = '\0';
	}
	for (int i = 0; i < sizeof(contextoPRUEBA->registrosCpu.RAX ); i++) {
		contextoPRUEBA->registrosCpu.RAX[i] = '\0';
	}
	for (int i = 0; i < sizeof(contextoPRUEBA->registrosCpu.RBX ); i++) {
		contextoPRUEBA->registrosCpu.RBX[i] = '\0';
	}
	for (int i = 0; i < sizeof(contextoPRUEBA->registrosCpu.RCX ); i++) {
		contextoPRUEBA->registrosCpu.RCX[i] = '\0';
	}
	for (int i = 0; i < sizeof(contextoPRUEBA->registrosCpu.RDX ); i++) {
		contextoPRUEBA->registrosCpu.RDX[i] = '\0';
	}

	///
	void* stream = paquete->buffer->stream;

	// Deserializamos los campos que tenemos en el buffer
	 memcpy(&(contextoPRUEBA->pid), stream, sizeof(int));
	 stream += sizeof(int);

	 memcpy(&(contextoPRUEBA->programCounter), stream, sizeof(int));
	 stream += sizeof(int);

	 memcpy(&(contextoPRUEBA->registrosCpu.AX), stream, sizeof(contextoPRUEBA->registrosCpu.AX));
	 stream += sizeof(contextoPRUEBA->registrosCpu.AX);

	 memcpy(&(contextoPRUEBA->registrosCpu.BX), stream, sizeof(contextoPRUEBA->registrosCpu.BX));
	 stream += sizeof(contextoPRUEBA->registrosCpu.BX);

	 memcpy(&(contextoPRUEBA->registrosCpu.CX), stream, sizeof(contextoPRUEBA->registrosCpu.CX));
	 stream += sizeof(contextoPRUEBA->registrosCpu.CX);

	 memcpy(&(contextoPRUEBA->registrosCpu.DX), stream, sizeof(contextoPRUEBA->registrosCpu.DX));
	 stream += sizeof(contextoPRUEBA->registrosCpu.DX);

	 memcpy(&(contextoPRUEBA->registrosCpu.EAX), stream, sizeof(contextoPRUEBA->registrosCpu.EAX));
	 stream += sizeof(contextoPRUEBA->registrosCpu.EAX);

	 memcpy(&(contextoPRUEBA->registrosCpu.EBX), stream, sizeof(contextoPRUEBA->registrosCpu.EBX));
	 stream += sizeof(contextoPRUEBA->registrosCpu.EBX);

	 memcpy(&(contextoPRUEBA->registrosCpu.ECX), stream, sizeof(contextoPRUEBA->registrosCpu.ECX));
	 stream += sizeof(contextoPRUEBA->registrosCpu.ECX);

	 memcpy(&(contextoPRUEBA->registrosCpu.EDX), stream, sizeof(contextoPRUEBA->registrosCpu.EDX));
	 stream += sizeof(contextoPRUEBA->registrosCpu.EDX);

	 memcpy(&(contextoPRUEBA->registrosCpu.RAX), stream, sizeof(contextoPRUEBA->registrosCpu.RAX));
	 stream += sizeof(contextoPRUEBA->registrosCpu.RAX);

	 memcpy(&(contextoPRUEBA->registrosCpu.RBX), stream, sizeof(contextoPRUEBA->registrosCpu.RBX));
	 stream += sizeof(contextoPRUEBA->registrosCpu.RBX);

	 memcpy(&(contextoPRUEBA->registrosCpu.RCX), stream, sizeof(contextoPRUEBA->registrosCpu.RCX));
	 stream += sizeof(contextoPRUEBA->registrosCpu.RCX);

	 memcpy(&(contextoPRUEBA->registrosCpu.RDX), stream, sizeof(contextoPRUEBA->registrosCpu.RDX));
	 stream += sizeof(contextoPRUEBA->registrosCpu.RDX);

	 eliminar_paquete(paquete);

	 return contextoPRUEBA;

}


void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}

char* recibir_handshake(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	//log_info(logger, "Me llego el mensaje %s", buffer);
	return buffer;
}

t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
	free(valores);
}

//client

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

int crear_conexion(char *ip, char* puerto)
{
	//log_info(logger,"estoy adentro de crear conexion");

	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	// Ahora vamos a crear el socket.
	int socket_cliente = socket(server_info->ai_family,
								server_info->ai_socktype,
								server_info->ai_protocol);


	// Ahora que tenemos el socket, vamos a conectarlo
	connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);

	//log_info(logger, "Me estoy conectando!");

	freeaddrinfo(server_info);

	return socket_cliente;
}

void enviar_mensaje(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}


void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_super_paquete(void)
{
	//me falta un malloc!
	t_paquete* paquete;

	//descomentar despues de arreglar
	//paquete->codigo_operacion = PAQUETE;
	//crear_buffer(paquete);
	return paquete;
}

t_paquete* crear_paquete(void)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE;
	crear_buffer(paquete);
	return paquete;
}

t_paquete* crear_paquete_cod_operacion(int cod_operacion)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = cod_operacion;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}
