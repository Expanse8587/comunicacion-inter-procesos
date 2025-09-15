#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include "constants.h"

int generate_random_number(){
	return (rand() % 50);
}

enum { READ, WRITE };
enum { IDA, VUELTA };

int main(int argc, char **argv)
{	
	//Funcion para cargar nueva semilla para el numero aleatorio
	srand(time(NULL));

	int status, pid, n, start, buffer;
	n = atoi(argv[1]);
	buffer = atoi(argv[2]);
	start = atoi(argv[3]);

	if (argc != 4){ printf("Uso: anillo <n> <c> <s> \n"); exit(0);}
    
  	/* COMPLETAR */
    printf("Se crearán %i procesos, se enviará el caracter %i desde proceso %i \n", n, buffer, start);
    
	// Como comunicamos que el juego terminó y los hijos deben terminar su ejecucion y cerrar sus pipes
	// Matamos hijos desde el padre? Recorremos por ultima vez el anillo enviando EOF? Algo mas? 

	int pipe_mensajero[n][2];
	for (int i = 0;i < n; i++) {			//pipes circulares
		if(pipe(pipe_mensajero[i]) == -1){
			perror("buh");
			exit(1);
		}
	}
	int pipe_padre[2][2];					//Un pipe para comunicacion padre-start(setup)
	for (int i = 0;i < n; i++) {			//Otro para start-padre(finish)
		if(pipe(pipe_padre[i]) == -1){
			perror("buh");
			exit(1);
		}
	}

	pid_t pid_hijo;
	for(int i=0;i<n;i++){
		pid_hijo = fork();
		if(pid_hijo ==0){
			printf("Hijo %d creado\n",i);
			int anterior = i-1;
			int mensaje_inicial;
			if(i == 0)
				anterior = n-1;
			// Cada hijo cierra pipes de pipes_mensajero que no usa
			for(int j =0;j<n;j++){
				if(j != i && j != anterior){
					//Mantengo el read anterior y write actual
					close(pipe_mensajero[j][READ]);
					close(pipe_mensajero[j][WRITE]);
				}
				else if(j == i)
					close(pipe_mensajero[j][READ]);
				else if(j == anterior)
					close(pipe_mensajero[anterior][WRITE]);
			}
			// Setup de mensaje inicial
			if(i == start){
				int numero_secreto;
				read(pipe_padre[IDA][READ],&mensaje_inicial,sizeof(int));
				close(pipe_padre[IDA][READ]);
				close(pipe_padre[IDA][WRITE]);
				close(pipe_padre[VUELTA][READ]);
				do
				{
					numero_secreto = generate_random_number();
				} while (numero_secreto < mensaje_inicial);
				printf("Numero secreto %d generado\n",numero_secreto);
				printf("Mensaje inicial %d recibido\n",mensaje_inicial);
				mensaje_inicial++;
				write(pipe_mensajero[i][WRITE],&mensaje_inicial,sizeof(mensaje_inicial));
				while(1){
					read(pipe_mensajero[anterior][READ],&mensaje_inicial,sizeof(mensaje_inicial));
					printf("Hijo %d recibió %d\n",i,mensaje_inicial);
					if(mensaje_inicial >= numero_secreto)
						break;
					mensaje_inicial++;
					write(pipe_mensajero[i][WRITE],&mensaje_inicial,sizeof(mensaje_inicial));
				}
				//finish, broadcast al padre
				write(pipe_padre[VUELTA][WRITE],&mensaje_inicial,sizeof(mensaje_inicial));
				close(pipe_padre[VUELTA][WRITE]);
				close(pipe_mensajero[i][WRITE]);
				close(pipe_mensajero[anterior][READ]);
				exit(EXIT_SUCCESS);
			}
			else if(i != start){
				close(pipe_padre[IDA][READ]);
				close(pipe_padre[IDA][WRITE]);
				close(pipe_padre[VUELTA][READ]);
				close(pipe_padre[VUELTA][WRITE]);
				while(1){
					if(read(pipe_mensajero[anterior][READ],&mensaje_inicial,sizeof(mensaje_inicial)) == EOF){
						break;
					}
					printf("Hijo %d recibió %d\n",i,mensaje_inicial);
					mensaje_inicial++;
					write(pipe_mensajero[i][WRITE],&mensaje_inicial,sizeof(mensaje_inicial));
				}
				close(pipe_mensajero[i][WRITE]);
				close(pipe_mensajero[anterior][READ]);
				exit(EXIT_SUCCESS);
			}
		}
	}

	for(int i=0; i<n;i++){
		close(pipe_mensajero[i][READ]);
		close(pipe_mensajero[i][WRITE]);
	}

	close(pipe_padre[IDA][READ]);
	write(pipe_padre[IDA][WRITE],&buffer,sizeof(buffer));
	read(pipe_padre[VUELTA][READ],&buffer,sizeof(buffer));

	close(pipe_padre[IDA][WRITE]);
	close(pipe_padre[VUELTA][READ]);
	sleep(1);

	printf("Padre recibió finalmente el mensaje %d\n",buffer);
	exit(EXIT_SUCCESS);
    /* COMPLETAR */
}

