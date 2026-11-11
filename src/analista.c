#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

// TODO decidir como vamos ler o pid do analista, podemos ler só no inicio do programa,
// mas daí tem obrigatoriamente que abrir o analista antes do atendimento

// TODO printar pid num arquivo quando iniciar
int main(){
    sem_t *sem_block = sem_open("/sem_block", O_RDWR);

    FILE* analista_f = fopen("pid_analista.txt", "w+");
    fprintf(analista_f, "%d", getpid());
    fclose(analista_f);

    while (1){
        raise(SIGSTOP);

        sem_wait(sem_block);

        FILE* lng = fopen("lng.txt", "r+");

        // PID no linux vai até 7 digitos acho, botei um extra pra garantir
        char str[10] = "";
        int i = 0;
        do{
            char* res = fgets(str, sizeof(str), lng);
            if(res == NULL) break;
            printf("%s", str);
            i++;
        }while(i < 10);

        long cur_pos = ftell(lng);
        fseek(lng, 0, SEEK_END);

        long size = ftell(lng) - cur_pos;
        char* remaining_content = malloc(sizeof(char)*size);
        
        fseek(lng, cur_pos, 0);
        fread(remaining_content, sizeof(char), size, lng);

        // Reescrevendo o resto dos PIDs que não lemos no lng
        // Eu poderia só reabrir o arquivo destruindo ele mas acho q é mais lento
        ftruncate(lng->_fileno, 0);
        
        fseek(lng, 0, 0);
        fprintf(lng, "%s", remaining_content);

        // Jogar os prints logo no terminal antes de continuar
        fflush(stdout);

        // Esse sleep era pra diminuir as chances do 
        // usleep(20);

        fclose(lng);
        sem_post(sem_block);
    }
}