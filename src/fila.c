#include "fila.h"

sigset_t mascara_escalonador;

//bloquear o escalonador para evitar troca de contexto em insercao / remocao
int bloquear_escalonador(sigset_t *mascara_anterior){
    //bloqueia e a mascara anterior recebe o valor da mascara do escalonador
    return sigprocmask(SIG_BLOCK, &mascara_escalonador, mascara_anterior);
}

int desbloquear_escalonador(const sigset_t *mascara_anterior){
    //desbloqueia recuperando a mascara anterior
    return sigprocmask(SIG_SETMASK, mascara_anterior, NULL);
}

Cliente* createCliente(pid_t PID, int prioridade, long long hora_chegada, int paciencia, int tempo_atendimento){
    Cliente *c = malloc(sizeof(*c));

    c->PID = PID;
    c->prioridade = prioridade;
    c->hora_chegada = hora_chegada;
    c->prazo = hora_chegada + paciencia;
    c->tempo_atendimento = tempo_atendimento;
    c->proximo = NULL;

    return c;
}

// TODO fazer isso direito
void destroyCliente(Cliente *c){
    free(c);
}

Fila *createFila(int capacidade_maxima){
    Fila *f = malloc(sizeof(*f));

    f->inicioAlta = NULL; f->fimAlta = NULL;
    f->inicioBaixa = NULL; f->fimBaixa = NULL;
    f->tamanho = 0;
    f->capacidade = capacidade_maxima;

    return f;
}

// TODO fazer isso direito
void destroyFila(Fila *f){
    free(f);
}

int isFull(Fila *f){
    return (f->tamanho >= f->capacidade);
}

int isEmpty(Fila *f){
    return (f->tamanho == 0);
}

int enqueueCliente(Fila *f, Cliente *c){
    sigset_t mascara_anterior;
    if(bloquear_escalonador(&mascara_anterior) == -1) return 0;

    if(isFull(f)){
        desbloquear_escalonador(&mascara_anterior);
        return 0;
    }

    int p = c->prioridade;
    c->proximo = NULL;

    if(p == 1){
        if(f->fimAlta != NULL) f->fimAlta->proximo = c;
        else f->inicioAlta = c;
        f->fimAlta = c;
    }
    else if(p == 0){
        if(f->fimBaixa != NULL) f->fimBaixa->proximo = c;
        else f->inicioBaixa = c;
        f->fimBaixa = c;
    }
    f->tamanho++;

    desbloquear_escalonador(&mascara_anterior);
    return 1;
}

//prazos ja estarao ordenados, basta comparar o inicio de ambas
Cliente* escolher_fila(const Fila *f){
    Cliente *alta = f->inicioAlta, *baixa = f->inicioBaixa;

    if(alta == NULL) return baixa;
    if(baixa == NULL) return alta;

    if(baixa->prazo < alta->prazo) return baixa;
    else return alta;
}

Cliente *dequeueCliente(Fila *f){
    sigset_t mascara_anterior;
    if(bloquear_escalonador(&mascara_anterior) == -1) return NULL;

    if(isEmpty(f)){
        desbloquear_escalonador(&mascara_anterior);
        return NULL;
    }

    Cliente *c = escolher_fila(f);

    if(c->prioridade == 1){
        f->inicioAlta = c->proximo;
        if(f->inicioAlta == NULL) f->fimAlta = NULL;
    }
    else{
        f->inicioBaixa = c->proximo;
        if(f->inicioBaixa == NULL) f->fimBaixa = NULL;
    }
    c->proximo = NULL;
    f->tamanho--;

    desbloquear_escalonador(&mascara_anterior);
    return c;
}

Cliente *topCliente(Fila *f){
    sigset_t mascara_anterior;
    if(bloquear_escalonador(&mascara_anterior) == -1) return NULL;
    
    if(isEmpty(f)){
        desbloquear_escalonador(&mascara_anterior);
        return 0;
    }

    Cliente *c = escolher_fila(f);

    desbloquear_escalonador(&mascara_anterior);
    return c;
}
