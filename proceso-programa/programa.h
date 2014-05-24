#ifndef INTERPRETE_H_
#define INTERPRETE_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<commons/config.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<errno.h>

typedef struct {
	char* IP_KERNEL;
	int PUERTO_KERNEL;
}CONFIG_PROGRAMA;

typedef struct {
	char *script;
	int tam;
}tipoPaquete;

typedef enum {
	IMPRIMIR,
	SIN_MEMORIA,
	PROCESO_FINALIZADO
}tipoRespuesta;

typedef struct{
	tipoRespuesta type;
	int tam;
	char *respuesta;
} tipoPaqueteK;

char* buscarPath(char* arg1,char* arg2);
FILE* abrirParaLeer(char* path);
void cerrarFile(FILE* fp);
void recorrerFichero (FILE* fp);

int calcularTamFile(FILE *fp);
tipoPaquete *inicializarPaquete(FILE *fp);
void liberarMemoria(tipoPaquete *paquete);
void recorrerScript(tipoPaquete *paquete);

void obtenerConfiguracion(CONFIG_PROGRAMA *confProg);
void conectarASocket(CONFIG_PROGRAMA *confProg);

int conectarAlKernel(CONFIG_PROGRAMA *confProg);
int handshakePrograma(int socketPrograma);
int enviarCodigo(int socketPrograma, tipoPaquete *codigo);
int recibirPaquete(int socketPrograma, tipoPaqueteK *paqueteK);

int32_t socket_leer (int nuevo_socket, char *buffer, size_t tamanio);
int32_t socket_escribir (int nuevo_socket, char *datos, size_t longitud);
#endif /* INTERPRETE_H_ */
