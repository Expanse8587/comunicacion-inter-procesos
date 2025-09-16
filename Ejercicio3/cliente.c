#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

int main() {
	char buffer[256];

	int server_socket;
    struct sockaddr_un server_addr;

    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, "unix_socket");

	printf("cliente[%d]: me estoy conectando con el servidor...\n", getpid());
    server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error");
        exit(EXIT_FAILURE);
    }
	int resultado=NULL;
	do
	{	
		printf("Ingresar operacion a resolver:\n");
		scanf("%s",&buffer);
		send(server_socket,&buffer,sizeof(buffer),NULL);
		recv(server_socket,&resultado,sizeof(resultado),NULL);
		if(strcmp(buffer,"exit") == 0)
			break;
		printf("Se recibio %d como resultado desde el servidor\n",resultado);
	} while (strcmp(buffer,"exit")!=0);
	close(server_socket);
	
	exit(0);
}
