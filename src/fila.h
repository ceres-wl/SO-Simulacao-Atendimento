#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>

typedef struct Cliente{
    pid_t PID;
    long long hora_chegada;
    int prioridade; // 0: baixa, 1: alta
    int tempo_atendimento;
    long long prazo; //criterio de prioridade dos clientes

    struct Cliente *proximo;
} Cliente;

Cliente* createCliente(pid_t PID, int prioridade, long long hora_chegada, int paciencia, int tempo_atendimento);
void destroyCliente(Cliente* c);

//2 filas: uma pra prioridade alta, outra pra prioridade baixa
typedef struct Fila {
    Cliente *inicioAlta, *fimAlta;
    Cliente *inicioBaixa, *fimBaixa;

    int tamanho, capacidade;
} Fila;

Fila* createFila(int max_size);
void destroyFila(Fila* f);
int enqueueCliente(Fila* f, Cliente* c);
Cliente* dequeueCliente(Fila* f);
Cliente* topCliente(Fila* f);
int isFull(Fila* f);
int isEmpty(Fila* f);

extern sigset_t mascara_escalonador;