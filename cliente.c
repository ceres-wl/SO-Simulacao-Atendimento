#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>

int main(){
    int tempo;
    srand( (unsigned)time(NULL) );
    int x = rand()%10;

    if(x == 0)
    tempo = 15;
    else if(x > 0 && x <= 3)
    tempo = 5;
    else
    tempo = 1;

    FILE* demanda = fopen("demanda.txt", "w+");
    fprintf(demanda, "%d", tempo);
    fclose(demanda);

    // printf("Cliente %d desligando...\n", getpid());
    // fflush(stdout);
    
    raise(SIGSTOP);

    // printf("Cliente %d acordado\n", getpid());
    // fflush(stdout);

    sem_t *sem = sem_open("/sem_atend", O_RDWR);

    if(sem != SEM_FAILED) sem_wait(sem);

    // printf("%d: dormindo por %d ms...\n", getpid(), tempo);
    // fflush(stdout);
    usleep(tempo);

    if(sem != SEM_FAILED) sem_post(sem);

    // printf("Cliente saindo\n");
    // fflush(stdout);
    return 0;
}