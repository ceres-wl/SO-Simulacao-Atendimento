#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

// O ideal seria que retornar um erro ao invés de quitar do programa direto mas é oq temos
pid_t create_proc(const char* path, char **argv, char **envp);