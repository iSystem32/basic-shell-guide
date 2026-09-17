#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_LINE 1024
#define MAX_ARGS 64
#define COLOR_CYAN "\033[1;36m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_RESET "\033[0m"

// Muestra el prompt interactivo de la shell con color ANSI
void print_prompt() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf(COLOR_CYAN "basic-shell:" COLOR_GREEN "%s" COLOR_RESET "$ ", cwd);
    } else {
        printf(COLOR_CYAN "basic-shell" COLOR_RESET "$ ");
    }
    fflush(stdout);
}

// Tokeniza la línea de entrada dividiéndola por espacios y tabulaciones
int parse_line(char *line, char **args) {
    int count = 0;
    char *token = strtok(line, " \t\n\r");
    while (token != NULL && count < MAX_ARGS - 1) {
        // Soporta expansión simple de variable de entorno $HOME
        if (strcmp(token, "$HOME") == 0) {
            args[count++] = getenv("HOME");
        } else {
            args[count++] = token;
        }
        token = strtok(NULL, " \t\n\r");
    }
    args[count] = NULL;
    return count;
}

// Ejecuta comandos integrados (built-in) como 'cd' o 'exit'
int handle_builtin(char **args) {
    if (args[0] == NULL) return 1;

    if (strcmp(args[0], "exit") == 0) {
        printf("Saliendo de la Shell básica...\n");
        exit(0);
    }

    if (strcmp(args[0], "cd") == 0) {
        const char *dir = args[1];
        if (dir == NULL || strcmp(dir, "~") == 0) {
            dir = getenv("HOME");
        }
        if (chdir(dir) != 0) {
            perror("cd falló");
        }
        return 1;
    }

    return 0; // No es comando integrado
}

// Involucra fork(), execvp() y waitpid() para ejecutar programas externos
void launch_process(char **args) {
    pid_t pid = fork();

    if (pid == 0) {
        // --- Proceso Hijo ---
        if (execvp(args[0], args) == -1) {
            perror("Error al ejecutar el comando");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("Error al bifurcar (fork)");
    } else {
        // --- Proceso Padre (Shell) ---
        int status;
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
}

int main(int argc, char *argv[]) {
    char line[MAX_LINE];
    char *args[MAX_ARGS];

    // Si se pasa la bandera -c, ejecuta un solo comando no interactivo
    if (argc > 2 && strcmp(argv[1], "-c") == 0) {
        strncpy(line, argv[2], sizeof(line) - 1);
        line[sizeof(line) - 1] = '\0';
        int arg_count = parse_line(line, args);
        if (arg_count > 0) {
            if (!handle_builtin(args)) {
                launch_process(args);
            }
        }
        return 0;
    }

    printf("=== Bienvenido a la Shell Básica Educativa ===\n");
    printf("Escribe 'exit' para salir o 'cd <dir>' para cambiar de directorio.\n\n");

    while (1) {
        print_prompt();

        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n");
            break; // EOF (Ctrl+D)
        }

        int arg_count = parse_line(line, args);
        if (arg_count == 0) continue; // Línea vacía

        if (!handle_builtin(args)) {
            launch_process(args);
        }
    }

    return 0;
}
