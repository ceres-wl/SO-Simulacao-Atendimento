#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>


// TODO printar pid num arquivo quando iniciar
// TODO forma de avisar esse cara q o programa principal acabou, pra ele acabar tbm
int main(){
    sem_t *sem_block = sem_open("/sem_block", O_RDWR);

    while (1){
        raise(SIGSTOP);

        sem_wait(sem_block);

        FILE* lng = fopen("lng.txt", "r+");

        // PID no linux vai até 7 digitos acho, botei um extra pra garantir
        char str[8] = "";
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
        ftruncate(lng->_fileno, 0); // Isso aqui não devia tá funcionando, mas por algum motivo tá funcionando então eu vou aceitar ok? ok
        fseek(lng, 0, 0);
        fprintf(lng, "%s", remaining_content);

        fflush(stdout);

        usleep(20);

        sem_post(sem_block);
    }

    return 0;
}