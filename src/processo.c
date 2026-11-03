#include "processo.h"

// O ideal seria que retornar um erro ao invés de quitar do programa direto mas é oq temos
pid_t create_proc(const char* path, char **argv, char **envp){
    pid_t pid = fork();
    if(pid < 0){
        perror("Erro no fork");
        exit(1);
    }
    if(pid == 0){
        execve(path, argv, envp);
        perror("Erro no execve");
        exit(1); // Não sei se ele sai automaticamente, imagino q nn
    }

    return pid;
}