#include <time.h>
#include <unistd.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include <semaphore.h>
#include <fcntl.h>
#include "fila.h"
#include "time.h"

#define STACK_SIZE 1024 * 64

ucontext_t ctx_main, ctx_thread[2];
int atual = 0;

/* 
O comportamento da recepção é diferente o suficiente quando N é = 0 pra justificar criar uma lógica separada pra esse caso,
usando um signal pra acordar a recepção quando precisa ou algo assim,
enquanto nos outros casos basta deixar a recepção criando cliente até acabar
*/

void atendente(Fila* fila){
    // TODO fazer
    while(1){
        printf("atend: %d\n", timeInMilliseconds(NULL));
        fflush(stdout);
        usleep(100000);
    }
}

void recepcao(Fila* fila, int N){
    // sem_t* sem_atend = sem_open("/sem_atend", O_CREAT);
    // sem_t* sem_block = sem_open("/sem_block", O_CREAT);

    int num_proc = 0;
    while(N == 0 || num_proc == N-1){
        // Criando o cliente
        pid_t pid = fork();
        if(pid < 0){
            perror("Erro no fork");
            exit(1);
        }
        if(pid == 0){
            char* argv[8];
            char* envp[8];
            // não sei se tem forma melhor de passar o caminho aqui
            execve("/home/ceres/Desktop/SO-Simulacao-Atendimento/build/cliente", argv, envp);
            perror("Erro no execve");
            exit(1); // Não sei se ele sai automaticamente, imagino q nn
        }

        enqueueCliente(fila,
            createCliente(pid, rand()%2)
        );

        // pra testar
        printf("recep: %d\n", timeInMilliseconds());
        fflush(stdout);
        
        num_proc++;
    }
}

int quantum = 0;
void escalonador(int n) 
{
    quantum++;
    
    if(atual == 0)
    {
       if(quantum == 5)
       {
          atual = 1 - atual;
          swapcontext(&ctx_thread[1 - atual], &ctx_thread[atual]);
          quantum = 0;
       }
    }
    else
    {
       atual = 1 - atual;
       swapcontext(&ctx_thread[1 - atual], &ctx_thread[atual]);
       quantum = 0;
    }
}

void iniciar_timer(int milissegundos) {
    struct sigaction sa;
    struct itimerval timer;

    sa.sa_handler = &escalonador;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; 

    sigaction(SIGALRM, &sa, NULL);

    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = milissegundos * 1000;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = milissegundos * 1000;

    setitimer(ITIMER_REAL, &timer, NULL);
}

int main(int argc, char** argv) {
    // TODO tratamento melhor de entrada
    if(argc != 3){
        printf("Numero incorreto de argumentos!\n");
        exit(0);
    }

    int N = atoi(argv[1]);
    // Não tenho certeza se ele vai usar float aqui, talvez seja só inteiro
    float X = atof(argv[2]);

    Fila* fila = createFila(N!=0?N:100);
    
    srand(time(NULL));

    // A partir daqui é aquele código do yuri, modificado um pouquinho só

    getcontext(&ctx_thread[0]);
    getcontext(&ctx_thread[1]);

    ctx_thread[0].uc_stack.ss_sp = malloc(STACK_SIZE);
    ctx_thread[0].uc_stack.ss_size = STACK_SIZE;
    ctx_thread[0].uc_stack.ss_flags = 0;
    ctx_thread[0].uc_link = &ctx_main;

    ctx_thread[1].uc_stack.ss_sp = malloc(STACK_SIZE);
    ctx_thread[1].uc_stack.ss_size = STACK_SIZE;
    ctx_thread[1].uc_stack.ss_flags = 0;
    ctx_thread[1].uc_link = &ctx_main;

    makecontext(&ctx_thread[0], (void(*)(void))atendente, 1, fila);
    makecontext(&ctx_thread[1], (void(*)(void))recepcao, 2, fila, N);

    iniciar_timer(10);
    
    swapcontext(&ctx_main, &ctx_thread[0]);

    free(ctx_thread[0].uc_stack.ss_sp);
    free(ctx_thread[1].uc_stack.ss_sp);
    
    return 0;
}