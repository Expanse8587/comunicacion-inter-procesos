#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

int calcular(const char *expresion) {
    int num1, num2, resultado;
    char operador;

    // Usamos sscanf para extraer los dos números y el operador de la expresión
    if (sscanf(expresion, "%d%c%d", &num1, &operador, &num2) != 3) {
        printf("Formato incorrecto\n");
        return 0;  // En caso de error, retornamos 0.
    }

    // Realizamos la operación según el operador
    switch (operador) {
        case '+':
            resultado = num1 + num2;
            break;
        case '-':
            resultado = num1 - num2;
            break;
        case '*':
            resultado = num1 * num2;
            break;
        case '/':
            if (num2 != 0) {
                resultado = num1 / num2;
            } else {
                printf("Error: División por cero\n");
                return 0;  // Si hay división por cero, retornamos 0.
            }
            break;
        default:
            printf("Operador no reconocido\n");
            return 0;  // Si el operador no es válido, retornamos 0.
    }

    return resultado;
}

int main() {
    
    char buffer[256];  // buffer donde recibir data del cliente
    int server_socket; // FD
    int client_socket; // FD
    struct sockaddr_un server_addr;
    struct sockaddr_un client_addr;
    u_int32_t slen = sizeof(server_addr);
    u_int32_t clen = sizeof(client_addr);

    server_socket = socket(AF_UNIX,SOCK_STREAM,0); // Se obtiene file descriptor para conexion local

    server_addr.sun_family = AF_UNIX;              // Conexion local
    strcpy(server_addr.sun_path, "unix_socket");   // Se asocia estructura con archivo en disco
    unlink(server_addr.sun_path);

    if (bind(server_socket, (struct sockaddr *) &server_addr, slen) == -1) {    // Se obtiene puerto
    perror("Error");                                                            // y se guarda en server_addr
    exit(EXIT_FAILURE);
    }
    if (listen(server_socket, 1) == -1) {                                       // Escuchando conexiones entrantes
        perror("Error");
        exit(EXIT_FAILURE);
    }

    printf("Servidor iniciado: esperando clientes\n");

    while(1){
        client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &clen);
        if (client_socket == -1) { // error handling
            perror("Error");
            exit(EXIT_FAILURE);
        }

        pid_t pid_hijo = fork();

        if(pid_hijo == 0){
            close(server_socket);                                   // Siendo hijo, cierro fd de servidor
            while(1){
                if(recv(client_socket,buffer,sizeof(buffer),NULL) == -1)// Recibimos stream de chars de cliente
                    exit(EXIT_FAILURE);
                if(strcmp(buffer,"exit")==0){                           // Si recibimos exit, terminamos la conexion
                    close(client_socket);                               // Fin de conexión
                    exit(EXIT_SUCCESS);
                }
                int resultado = calcular(buffer);                                // Calculamos operacion
                printf("Hijo %d atendió pedido de un cliente del que no sabemos ningun dato\n",getpid());
                if(send(client_socket,&resultado,sizeof(resultado),NULL) == -1)  // Comunicamos resultado
                    exit(EXIT_FAILURE);
            }
        }
        close(client_socket);                                       // Padre cierra fd cliente, delega a hijo
    }
    exit(EXIT_SUCCESS);
}

