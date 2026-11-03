#include <stdio.h>
#include <stdlib.h>

typedef struct Cliente{
    pid_t PID;
    long long hora_chegada;
    int prioridade; // 0: baixa, 1: alta
    int tempo_atendimento;
} Cliente;

Cliente* createCliente(pid_t PID, int prioridade, long long hora_chegada, int tempo_atendimento);

typedef struct Fila{
    // Teoricamente a gente não precisa guardar isso, já que o max é 100, mas tá aí
    int max_size;
    int cur_size;
    
    int start;
    int end;
    
    Cliente** vec;
} Fila;

Fila* createFila(int max_size);
void enqueueCliente(Fila* f, Cliente* c);
void dequeueCliente(Fila* f);
Cliente* topCliente(Fila* f);
int isFull(Fila* f);
int isEmpty(Fila* f);