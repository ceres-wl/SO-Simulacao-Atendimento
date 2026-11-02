#include "fila.h"

Cliente* createCliente(pid_t PID, int prioridade){
    Cliente* cliente = (Cliente*)malloc(sizeof(Cliente));
    cliente->PID = PID;
    cliente->prioridade = prioridade;

    return cliente;
}

Fila* createFila(int max_size){
    Fila* f = (Fila*)malloc(sizeof(Fila));
    f->start = -1;
    f->end = 0;
    f->size = max_size;

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
