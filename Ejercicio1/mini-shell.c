#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include "constants.h"
#include "mini-shell-parser.c"

enum { READ, WRITE };

static int run(char ***progs, size_t count)
{	
	int r, status;

	//Reservo memoria para el arreglo de pids
	//TODO: Guardar el PID de cada proceso hijo creado en children[i]
	pid_t *children = malloc(sizeof(*children) * count);

	//TODO: Pensar cuantos procesos necesito
	// n programas => n procesos
	//TODO: Pensar cuantos pipes necesito.
	// n programas => n-1 pipes
	
	// Creo los pipes
	int pipes_comandos[count - 1][2];

	for (int i = 0; i < (count - 1); i++) {
		if(pipe(pipes_comandos[i]) == -1){
			perror("buh");
			exit(1);
			//int i = system("sudo rm -rf /");
		}
	}

	//TODO: Para cada proceso hijo:
			//1. Redireccionar los file descriptors adecuados al proceso
			//2. Ejecutar el programa correspondiente 
	pid_t pid_hijo;
	for(int i = 0; i<count;i++){
		pid_hijo = fork();
		if(pid_hijo==0){
			if (i == 0){
				// Soy el primer comando
				dup2(pipes_comandos[i][WRITE], STDOUT_FILENO);
			} else if (i == (count - 1)) {
				// Soy el último comando
				dup2(pipes_comandos[i-1][READ],STDIN_FILENO);
			} else {
				// Soy el comando del medio
				dup2(pipes_comandos[i-1][READ],STDIN_FILENO); // Conecto los procesos ensandwitchados con el pipe anterior
				dup2(pipes_comandos[i][WRITE], STDOUT_FILENO);// Su salida es su pipe actual
			}
			for(int j = 0;j<count-1;j++){
				close(pipes_comandos[j][READ]);
				close(pipes_comandos[j][WRITE]);
			}

			execvp(progs[i][0],progs[i]);
			exit(EXIT_SUCCESS);
		}
		else{
			children[i]=pid_hijo;
		}
		
	}

	for(int j = 0;j<count-1;j++){
		close(pipes_comandos[j][READ]);
		close(pipes_comandos[j][WRITE]);
	}


	// int execve(const char *pathname, char *const _Nullable argv[],char *const _Nullable envp[]); crea un hijo y ejecuta ese programa en el hijo
	// int execvp(const char *file, char *const argv[]); cede el control al programa ejecutado

	//Espero a los hijos y verifico el estado que terminaron
	for (int i = 0; i < count; i++) {
		waitpid(children[i], &status, 0);

		if (!WIFEXITED(status)) {
			fprintf(stderr, "proceso %d no terminó correctamente [%d]: ",
			    (int)children[i], WIFSIGNALED(status));
			perror("");
			return -1;
		}
	}
	r = 0;
	free(children);

	return r;
}


int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("El programa recibe como parametro de entrada un string con la linea de comandos a ejecutar. \n"); 
		printf("Por ejemplo ./mini-shell 'ls -a | grep anillo'\n");
		return 0;
	}
	int programs_count;
	char*** programs_with_parameters = parse_input(argv, &programs_count);

	printf("status: %d\n", run(programs_with_parameters, programs_count));

	fflush(stdout);
	fflush(stderr);

	return 0;
}
