#include "programa.h"

//Se ejecuta como "./algo.ansisop *Aqui va la direccion donde se encuentra para poder levantarlo*"
// ./algo.ansisop /home/utnso/Escritorio


int main(int argc, char **argv) {

	//Evita errores al no pasar el Path del .ansisop
	if (argc == 2)
	{
		printf("Necesita la direccion como segundo argumento\n");
		return EXIT_FAILURE;
	}

	//Se busca el .ansisop y se levanta el PUERTO+IP del Kernel
    char* path = buscarPath(argv[1],argv[2]);
    FILE* fp = abrirParaLeer(path);

    //Sentencia de prueba, pasa la dirección del .ansisop directamente
    //FILE* fp = abrirParaLeer("/home/utnso/facil.ansisop");

    //Se fija que exista el fichero antes de abrir cualquier cosa
    if (fp == NULL)
    {
    	printf("Ruta erronea\n");
    	return EXIT_FAILURE;
    }

    //genero el fichero a enviar
    tipoPaquete *paquete = inicializarPaquete(fp);
    recorrerScript(paquete);

    //Obtención de los campos del archivo de configuración
    CONFIG_PROGRAMA *configPrograma = malloc(sizeof(CONFIG_PROGRAMA));
    obtenerConfiguracion(configPrograma);

		//Se crea conexion con kernel
		int socketPrograma = conectarAlKernel(configPrograma);
		if (socketPrograma == -1) {
			printf("Error de conexion con el kernel\n");
    	return EXIT_FAILURE;
		}

		//Enviar handshake al kernel
		int handshake = handshakePrograma(socketPrograma);
		if (handshake == -1) {
			printf("Error de handshake con el kernel\n");
	   	return EXIT_FAILURE;
		}

		//Enviar codigo ansisop al kernel
		int envio = enviarCodigo(socketPrograma, paquete);
		if (envio == -1) {
			printf("Error al enviar el codigo ansisop al kernel\n");
	   	return EXIT_FAILURE;
		}
		//Recibir y procesar los resultados del kernel
		tipoPaqueteK paqueteK;
		recibirPaquete(socketPrograma, &paqueteK);
		//Mientra haya respuestas que procesar
		while (1){
			//Segun el tipo de paquete que nos envie el kernel procesamos
			switch(paqueteK.type){
				case IMPRIMIR:{
					printf("%s\n",paqueteK.respuesta);
					free(paqueteK.respuesta);
					continue;
				}
				case SIN_MEMORIA:{
					break;
				}
				case PROCESO_FINALIZADO:{
					break;
				}

				default: {
					break;
				}
			}//fin switch
		/*Despues de imprimir vuelvo a escuchar para recibir mas paquetes desde el kernel*/
		recibirPaquete(socketPrograma, &paqueteK);
		}//fin while

		liberarMemoria(paquete);
    free(configPrograma);

    //Recorro el archivo por ahora, para ver que devuelve bien las cosas y cierro
    recorrerFichero (fp);
    cerrarFile(fp);

    return EXIT_SUCCESS;
}

char* buscarPath(char* arg1,char* arg2){
		char* c = strrchr(arg1,'/');
		char* d = arg2;
		strcat(d,c);
		return d;
}

FILE* abrirParaLeer(char* path){
	FILE* fp;
	fp = fopen(path,"r");
	return fp;
}

void cerrarFile(FILE* fp){
	fclose(fp);
	return;
}

void cerrarConfig(CONFIG_PROGRAMA* config){
	free(config);
	return;
}

void recorrerFichero (FILE* fp){
	char c;
	while(!feof(fp))	{
	   c = fgetc(fp);
	   printf("%c",c);
	}
	return;
}

//Calcula el tamaño del fichero fp, incluyendo a EOF
int calcularTamFile(FILE *fp){
	fseek(fp,0L,SEEK_END);
	return (ftell(fp)+1);
}

//Copio todos los caracteres del .ansisop en la estructura que se va a enviar
tipoPaquete *inicializarPaquete(FILE *fp){
	int i=0;
	tipoPaquete *paquete = malloc(sizeof(tipoPaquete));

	paquete->tam = calcularTamFile(fp);//con hash
	paquete->script = malloc(paquete->tam);

	if (paquete->script == NULL){
		printf("No se pudo reservar memoria\n");
	return NULL;
	}

	//Recorro el archivo desde el principio copiando cada caracter, inclusive el EOF
	fseek(fp,0L,SEEK_SET);
	while(!feof(fp)){
    	paquete->script[i] = fgetc(fp);
    	i++;
    }
    paquete->script[i] = fgetc(fp);
return paquete;
}

void liberarMemoria(tipoPaquete *paquete){
	free(paquete->script);
	free(paquete);
return;
}


//Muestra el contenido del script
void recorrerScript(tipoPaquete *paquete){
	int i=0;
	printf("El contenido copiado es:\n");
	for (i = 0; i < (paquete->tam); ++i)
		printf("%c",paquete->script[i]);
	printf("\n");
	return;
}

void obtenerConfiguracion(CONFIG_PROGRAMA *confProg){
//Lectura de configuración
    //Path de pruebas de archivo de config
    char *pathConfig = malloc(strlen("/home/utnso/Escritorio/proceso programa"));
    strcpy(pathConfig,"/home/utnso/Escritorio/proceso programa");

    //Obtener la dirección del archivo desde ANSISOP_CONFIG
    //char *pathConfig = getenv("ANSISOP_CONFIG");

    printf("La dirección del archivo de configuración es: %s\n",pathConfig);
    t_config *configPrograma = config_create(pathConfig);
    confProg->PUERTO_KERNEL = config_get_int_value(configPrograma,"Puerto");
    confProg->IP_KERNEL = config_get_string_value(configPrograma,"IP");
    printf("¿esta Puerto presente? %s\n",(config_has_property(configPrograma,"Puerto"))?"true":"false");
    printf("¿esta IP presente? %s\n",(config_has_property(configPrograma,"IP"))?"true":"false");

    printf("El IP del kernel es:%s\n",confProg->IP_KERNEL);
    printf("El puerto del kernel es:%d\n",confProg ->PUERTO_KERNEL);
free(pathConfig);
return;
}

void conectarASocket(CONFIG_PROGRAMA *confProg){
	  int sockPrograma = socket(AF_INET,SOCK_STREAM,0);
	    struct sockaddr_in plp_addr;
	    plp_addr.sin_family = AF_INET;
	    plp_addr.sin_addr.s_addr = inet_addr(confProg->IP_KERNEL);
	    plp_addr.sin_port = htons(confProg->PUERTO_KERNEL);
	    memset(&(plp_addr.sin_zero),'\0',8);
	    connect(sockPrograma,(struct sockaddr *) &plp_addr,sizeof(struct sockaddr));
	    close(sockPrograma);
	    config_destroy((t_config*)confProg);
return;
}

int conectarAlKernel(CONFIG_PROGRAMA *confProg){
	//crear descriptor de socket
	int sockPrograma = socket(AF_INET,SOCK_STREAM,0);
	if (sockPrograma == -1){
		perror("socket");
		return -1;
	}
	//armar estructura para la conexion
	struct sockaddr_in plp_addr;
	plp_addr.sin_family = AF_INET;
	plp_addr.sin_addr.s_addr = inet_addr(confProg->IP_KERNEL);
	plp_addr.sin_port = htons(confProg->PUERTO_KERNEL);
	memset(&(plp_addr.sin_zero),'\0',8);
	//conectarse
	int sockNew = connect(sockPrograma,(struct sockaddr *) &plp_addr,sizeof(struct sockaddr));
	if (sockNew == -1){
		perror("connect");
		return -1;
	}
	//retorna descriptor de la nueva conexion
	return sockNew;
}

int handshakePrograma(int socketPrograma){
	//enviar handshake
	char handshake[] = "PROGRAMA";
	int lengthHandshake = strlen(handshake) + 1;
	if (socket_escribir(socketPrograma, handshake, lengthHandshake) != lengthHandshake ){
		puts("Error al enviar el HANDSHAKE");
		return -1;
	}
	//esperar respuesta
	char respuesta[] = "KERNEL";
  int lengthRespuesta = strlen(respuesta) + 1;
	if (socket_leer (socketPrograma, respuesta, lengthRespuesta) != lengthRespuesta){
		puts("Error al recibir handshake RESPUESTA ");
		return -1;
	}
	//verificar respuesta del kernel
	if (strcmp("KERNEL", respuesta) == 0) {
	puts("handshake correcto");
	return 1;
	} else {
		puts("handshake fallido");
		return -1;
	}
}

int enviarCodigo(int socketPrograma, tipoPaquete *codigo){
	//Envio codigo y valido
	if (socket_escribir(socketPrograma, codigo->script, codigo->tam) != codigo->tam ){
	puts("Error al enviar el codigo al kernel");
	return -1;
	}
	return 1;
}

int recibirPaquete(int socketPrograma, tipoPaqueteK *paqueteK){
	return 1;
}

/*
* Escribe dato en el socket. Devuelve numero de bytes escritos, o -1 si hay error.
*/
int32_t socket_escribir (int nuevo_socket, char *datos, size_t longitud){
	size_t escrito = 0;
	size_t aux = 0;

	/*
	* Comprobacion de los parametros de entrada
	*/
	if ((nuevo_socket == -1) || (datos == NULL) || (longitud < 1))
		return -1;

	/*
	* Bucle hasta que hayamos escrito todos los caracteres que se indicaron.
	*/
	while (escrito < longitud){
		aux = send(nuevo_socket, datos + escrito, longitud - escrito, 0);
		if (aux > 0){
			/*
			* Si se consiguio escribir caracteres, se actualiza la variable escrito
			*/
			escrito = escrito + aux;
		}
		else{
			/*
			* Si se cerro el socket, devolvemos el numero de caracteres leidos.
			* Si hubo un error, devuelve -1
			*/
			if (aux == 0){
				return escrito;
			}
			else{
				return -1;
			}
		}
	}

	/*
	* Devolvemos el total de caracteres leidos
	*/
	return escrito;
}

int32_t socket_leer (int nuevo_socket, char *buffer, size_t tamanio){
	size_t leido = 0;
	size_t aux = 0;

	/*Comprobacion de que los parametros de entrada son correctos*/

	if ((nuevo_socket == -1) || (buffer == NULL) || (tamanio < 1))
		return -1;

	/* Mientras no hayamos leido todos los datos solicitados*/
	while (leido < tamanio){
		aux = recv(nuevo_socket, buffer + leido, tamanio - leido, 0);
		if (aux > 0){
			/*
			* En caso de leer datos, se incrementa la variable que contiene los datos leidos hasta el momento
			*/
			leido = leido + aux;
		}
		else{
			/*
			* Si read devuelve 0, es que se cerro el socket. Se devuelven los caracteres leidos hasta ese momento
			*/
			if (aux == 0){
				return leido;
			}
			if (aux == -1){
				/*
				* En caso de error:
				* EINTR se produce hubo una  interrupcion del sistema antes de leer ningun dato. No
				* es un error realmente.
				* EGAIN significa que el socket no esta disponible por el  momento.
				* Ambos errores se tratan con una espera de 100 microsegundos y se vuelve a intentar.
				* El resto de los posibles errores provocan que salgamos de la funcion con error.
				*/
				switch (errno){
					case EINTR:
					case EAGAIN:
						usleep (100);
						break;
					default:
						return -1;
				}
			}
		}
	}

	/*
	* Se devuelve el total de los caracteres leidos
	*/
	return leido;
}
