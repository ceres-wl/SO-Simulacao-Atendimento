#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>

#define STACK_SIZE 1024 * 64

ucontext_t ctx_main, ctx_thread[2];
int atual = 0;

/* 
Implementação incompleta de thread com escalonamento e contexto que o yuri fez na aula

Acho q dá pra aproveitar uma parte do código, só tem q usar um algoritmo de escalonamento melhor acho,
e generalizar mais pra gente conseguir usar no resto do programa
*/

void printA()
{
    while(1){
      printf("A");
      fflush(stdout);
      usleep(100000);
    }
}

void printB()
{
    while(1){
       printf("B");
       fflush(stdout);
       usleep(100000);
    }
}

int quantum = 0;
void escalonador() 
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

int main() {
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

    makecontext(&ctx_thread[0], printA, 0);
    makecontext(&ctx_thread[1], printB, 0);

    iniciar_timer(10);
    
    swapcontext(&ctx_main, &ctx_thread[0]);

    free(ctx_thread[0].uc_stack.ss_sp);
    free(ctx_thread[1].uc_stack.ss_sp);
    
    return 0;
}