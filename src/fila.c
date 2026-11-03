#include "fila.h"

Cliente* createCliente(pid_t PID, int prioridade, long long hora_chegada, int tempo_atendimento){
    Cliente* cliente = (Cliente*)malloc(sizeof(Cliente));
    cliente->PID = PID;
    cliente->prioridade = prioridade;
    cliente->hora_chegada = hora_chegada;
    cliente->tempo_atendimento = tempo_atendimento;

    return cliente;
}

Fila* createFila(int max_size){
    Fila* f = (Fila*)malloc(sizeof(Fila));
    f->start = -1;
    f->end = 0;
    f->max_size = max_size;

    f->vec = (Cliente**)malloc(sizeof(Cliente*)*max_size);
    return f;
}

void enqueueCliente(Fila *f, Cliente *c)
{
}

void dequeueCliente(Fila *f)
{
}

Cliente *topCliente(Fila *f)
{
    return NULL;
}

int isFull(Fila *f)
{
    return 0;
}

int isEmpty(Fila *f)
{
    return 0;
}
