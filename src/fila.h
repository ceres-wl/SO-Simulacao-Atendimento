#include <stdio.h>
#include <stdlib.h>

typedef struct Cliente{
    pid_t PID;
    // hora_chegada, não sei que estrutura representa tempo em c 
    int prioridade; // 0: baixa, 1: alta
    // tempo_atendimento, não sei que estrutura representa tempo em c 
} Cliente;

Cliente* createCliente(pid_t PID, int prioridade);

typedef struct Fila{
    // Teoricamente a gente não precisa guardar isso, já que o max é 100, mas tá aí
    int size;
    
    int start;
    int end;
    
    Cliente** vec;
} Fila;

Fila* createFila(int max_size);
void enqueueCliente(Fila* f, Cliente* c);
void dequeueCliente(Fila* f);
Cliente* topCliente(Fila* f);
int isFull(Fila* f);