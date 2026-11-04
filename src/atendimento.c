#include <time.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include <semaphore.h>
#include <fcntl.h>
#include "fila.h"
#include "time.h"
#include "processo.h"

#define STACK_SIZE 1024 * 64

ucontext_t ctx_main, ctx_thread[2];

void atendente(Fila* fila, pid_t pid_analista){
    sem_t *sem_atend = sem_open("/sem_atend", O_RDWR);
    sem_t *sem_block = sem_open("/sem_block", O_RDWR);

    // Não fazer nd se não tiver cliente na fila
    // O busy waiting não é ideal mas não é pra influenciar mt na eficiencia do programa, só vai gastar mais cpu doq precisa as vezes
    while(!isEmpty(fila)){
        Cliente* cliente = topCliente(fila);

        kill(cliente->PID, SIGCONT);

        sem_wait(sem_atend);
        sem_wait(sem_block);

        // Fechar semáforo block(Acho q não precisa)

        // Escrever PID do cliente no LNG
        FILE* lng = fopen("lng.txt", "a");
        fprintf(lng, "%d\n", cliente->PID);
        fclose(lng);

        // Abrir sem_block
        sem_post(sem_block);

        dequeueCliente(fila);

        // TODO decidir condição
        if(1){
            kill(pid_analista, SIGCONT);
        }
    }
}

// Se a recepção acabou o trabalho, o escalonador não precisa fazer troca de contexto mais
int recep_finish = 0;

void recepcao(Fila* fila, int N, long long tempo_inicio){
    // Só a criação dos semáforos, a recepção não usa
    sem_open("/sem_atend", O_CREAT);
    sem_open("/sem_block", O_CREAT);

    int num_proc = 0;
    while(N == 0 || num_proc < N){
        // TODO Busy waiting indireto, nesse caso afeta a eficiencia do programa,
        // dá pra pensar numa solução melhor dps
        if(N == 0 && fila->cur_size >= 100) continue;

        // Criando o cliente
        char** argv;
        char** envp;
        pid_t pid = create_proc("build/cliente", argv, envp);

        // Checando se a demanda já foi escrita antes de continuar
        // TODO mudar o tempo desse sleep se necessário
        while(access("demanda.txt", F_OK) != 0) sleep(10);

        // Lendo a demanda do cliente
        char buff_atend[2];
        FILE* demanda = fopen("demanda.txt", "r");
        fgets(buff_atend, sizeof(buff_atend), demanda);
        int tempo_atendimento = atoi(buff_atend);
        fclose(demanda);

        enqueueCliente(fila,
            createCliente(pid, rand()%2, timeInMilliseconds()-tempo_inicio, tempo_atendimento)
        );

        remove("demanda.txt");
        
        num_proc++;
    }

    // Se esse cara acabar, ele passa a vez pro atendente
    recep_finish = 1;
    swapcontext(&ctx_main, &ctx_thread[0]);
}

int quantum = 0;
int atual = 0;
void escalonador(int n) 
{
    // HACK O ideal aqui seria deletar o timer pra essa função nem ser chamada, mas não sei fazer isso
    if(recep_finish == 1){
        swapcontext(&ctx_main, &ctx_thread[0]);
        return; // nem sei se precisa desse return
    };
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
        return 0;
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

    char** argv_analista;
    char** envp_analista;
    pid_t pid_analista = create_proc("build/analista", argv_analista, envp_analista);

    makecontext(&ctx_thread[0], (void(*)(void))atendente, 2, fila, pid_analista);
    makecontext(&ctx_thread[1], (void(*)(void))recepcao, 3, fila, N, timeInMilliseconds());

    iniciar_timer(10);
    
    swapcontext(&ctx_main, &ctx_thread[1]);

    free(ctx_thread[0].uc_stack.ss_sp);
    free(ctx_thread[1].uc_stack.ss_sp);
    
    return 0;
}