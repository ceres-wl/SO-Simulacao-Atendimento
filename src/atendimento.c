#include <time.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
#include "fila.h"
#include "time.h"
#include "processo.h"

#define STACK_SIZE 1024 * 64
#define TIMER_ESCALONADOR_USEC 100000

#define THREAD_ATEND 0
#define THREAD_RECEP 1
#define THREAD_SCANN 2

ucontext_t ctx_main, ctx_thread[3];

// Fila de clientes
Fila* fila;

// Thread atual
int atual = 1;

// Variável independente que dá uma chance pra thread do scanner rodar
int timing_scanner = 0;

// Se a recepção acabou o trabalho, o escalonador não precisa fazer troca de contexto mais
int recep_finish = 0;

// "Cooldown" pra acordar o analista
int timing_analista = 0;

// Se N é 0 ou não, se não for não precisamos do scanner
int needs_scanner;

// Entradas
int N; float X;

// Tempo de inicio do programa
long long tempo_inicio;

pid_t pid_analista;

// Usuário pediu pra terminar
int user_finish = 0;

int atendidos = 0;
int satisfeitos = 0;

// Função só pra auxiliar nossas trocas manuais de contexto
void swap_threads(int to){
    if(to == atual) return;
    int antiga = atual;
    atual = to;
    swapcontext(&ctx_thread[antiga], &ctx_thread[atual]);
}

void escalonador(int n){
    if(user_finish == 1){
        swapcontext(&ctx_thread[atual], &ctx_main);
    }

    // HACK O ideal aqui seria deletar o timer pra essa função nem ser chamada mais, mas não sei fazer isso
    // E não vai ajudar mt na eficiencia do programa
    if(recep_finish == 1){
        if(atual != THREAD_ATEND){
            swap_threads(THREAD_ATEND);
        }

        return;
    };

    timing_scanner++;

    if(needs_scanner && timing_scanner > 2){
        swap_threads(THREAD_SCANN);
        timing_scanner = 0;
    }else if(atual == 2){
        swap_threads(THREAD_ATEND);
    }
}

void iniciar_timer(int microsegundos) {
    //inicializar mascara para operacoes na fila
    sigemptyset(&mascara_escalonador);
    sigaddset(&mascara_escalonador, SIGALRM);

    struct sigaction sa;
    struct itimerval timer;

    sa.sa_handler = &escalonador;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; 

    sigaction(SIGALRM, &sa, NULL);

    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = microsegundos;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = microsegundos;

    setitimer(ITIMER_REAL, &timer, NULL);
}

void scanner(){
    char input;
    do{
        scanf("%c", &input);
    }while(input != 's');

    kill(pid_analista, SIGCONT);
    int status;
    waitpid(pid_analista, &status, WUNTRACED);

    user_finish = 1;
}

void atendente(){
    sem_t *sem_atend = sem_open("/sem_atend", O_RDWR);
    sem_t *sem_block = sem_open("/sem_block", O_RDWR);

    // Não fazer nd se não tiver cliente na fila
    // O busy waiting não é ideal mas não é pra influenciar mt na eficiencia do programa, só vai gastar mais cpu doq precisa as vezes
    Cliente* c = dequeueCliente(fila);
    while(c != NULL || recep_finish == 0){
        // Se a fila estiver vazia mas a recepção ainda estiver ativa não podemos terminar
        if(c == NULL){
            swap_threads(THREAD_RECEP);

            c = dequeueCliente(fila);
            continue;
        }

        kill(c->PID, SIGCONT);

        // Esperando sem_atend abrir
        sem_wait(sem_atend);
        sem_post(sem_atend);

        sem_wait(sem_block);

        long long tempo_atual = timeInMilliseconds()-tempo_inicio;
        satisfeitos += (tempo_atual <= c->prazo);
        atendidos++;

        // Escrever PID do cliente no LNG
        FILE* lng = fopen("lng.txt", "a");
        fprintf(lng, "%d\n", c->PID);
        fclose(lng);

        // Abrir sem_block
        sem_post(sem_block);

        if(timing_analista == 10){
            kill(pid_analista, SIGCONT);
            timing_analista = 0;
        }
        timing_analista++;

        destroyCliente(c);

        c = dequeueCliente(fila);
    }

    // Caso o timing não tenha permitido o analista de printar tudo, chamar mais uma vez
    kill(pid_analista, SIGCONT);
    int status;
    waitpid(pid_analista, &status, WUNTRACED);

    sem_close(sem_atend);
    sem_close(sem_block);
}

void recepcao(){   
    iniciar_timer(TIMER_ESCALONADOR_USEC);

    // Só a criação dos semáforos, a recepção não usa
    sem_t* sem_atend = sem_open("/sem_atend", O_CREAT, 0644, 1);
    if(sem_atend == SEM_FAILED){
        perror("Falha na criação de sem_atend");
        exit(1);
    }

    sem_t* sem_block = sem_open("/sem_block", O_CREAT, 0644, 1);
    if(sem_block == SEM_FAILED){
        perror("Falha na criação de sem_block");
        exit(1);
    }

    int num_proc = 0;
    while(N == 0 || num_proc < N){
        if(N == 0 && fila->tamanho >= 100){
            swap_threads(THREAD_ATEND);
            continue;
        }

        // Criando o cliente
        char** argv;
        char** envp;
        pid_t pid = create_proc("build/cliente", argv, envp);

        int status;
        waitpid(pid, &status, WUNTRACED); // Esperar o cliente dar sigstop antes de seguir

        // Lendo a demanda do cliente
        char buff_atend[2];
        FILE* demanda = fopen("demanda.txt", "r");
        fgets(buff_atend, sizeof(buff_atend), demanda);
        int tempo_atendimento = atoi(buff_atend);
        fclose(demanda);

        remove("demanda.txt");

        int prioridade = rand() % 2;
        int paciencia = prioridade ? X / 2 : X;
        enqueueCliente(fila,
            createCliente(pid, prioridade, timeInMilliseconds()-tempo_inicio, paciencia, tempo_atendimento)
        );
        
        swap_threads(THREAD_ATEND);

        num_proc++;
    }

    sem_close(sem_atend);
    sem_close(sem_block);

    // Se esse cara acabar, ele passa a vez pro atendente
    recep_finish = 1;
    swap_threads(THREAD_ATEND);
}

int main(int argc, char** argv) {
    tempo_inicio = timeInMilliseconds();

    sem_unlink("/sem_atend");
    sem_unlink("/sem_block");

    if(argc != 3){
        printf("Numero incorreto de argumentos!\n");
        return 0;
    }

    N = atoi(argv[1]);

    needs_scanner = N == 0;
    X = atof(argv[2]);

    fila = createFila(N!=0?N:100);
    
    srand(time(NULL));

    getcontext(&ctx_thread[0]);
    getcontext(&ctx_thread[1]);
    getcontext(&ctx_thread[2]);

    ctx_thread[0].uc_stack.ss_sp = malloc(STACK_SIZE);
    ctx_thread[0].uc_stack.ss_size = STACK_SIZE;
    ctx_thread[0].uc_stack.ss_flags = 0;
    ctx_thread[0].uc_link = &ctx_main;

    ctx_thread[1].uc_stack.ss_sp = malloc(STACK_SIZE);
    ctx_thread[1].uc_stack.ss_size = STACK_SIZE;
    ctx_thread[1].uc_stack.ss_flags = 0;
    ctx_thread[1].uc_link = &ctx_main;

    ctx_thread[2].uc_stack.ss_sp = malloc(STACK_SIZE);
    ctx_thread[2].uc_stack.ss_size = STACK_SIZE;
    ctx_thread[2].uc_stack.ss_flags = 0;
    ctx_thread[2].uc_link = &ctx_main;

    makecontext(&ctx_thread[0], (void(*)(void))atendente, 0);
    makecontext(&ctx_thread[1], (void(*)(void))recepcao, 0);
    makecontext(&ctx_thread[2], (void(*)(void))scanner, 0);

    FILE* analista_f = fopen("pid_analista.txt", "r");
    if(analista_f != NULL){
        char str[10] = "";
        fgets(str, sizeof(str), analista_f);
        fclose(analista_f);
        pid_analista = atoi(str);
    
        swapcontext(&ctx_main, &ctx_thread[1]);
    }else{
        printf("Inicie o analista antes!\n");
    }

    free(ctx_thread[0].uc_stack.ss_sp);
    free(ctx_thread[1].uc_stack.ss_sp);
    free(ctx_thread[2].uc_stack.ss_sp);

    destroyFila(fila);

    printf("Taxa de satisfação: %f\n", (float)satisfeitos/atendidos);
    printf("Tempo de execução: %lldms\n", timeInMilliseconds()-tempo_inicio);
    
    return 0;
}