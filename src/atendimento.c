#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

typedef struct VecInt{
    int size;
    int* vec;
} VecInt;

// Alocar vetor de inteiros
VecInt allocVecInt(int size){
    VecInt v;
    v.size = size;

    v.vec = (int*)malloc(sizeof(int)*size);
    return v;
}

int main(int argc, char** argv){
    // TODO tratamento melhor de entrada
    if(argc != 3){
        printf("Numero incorreto de argumentos!");
        exit(0);
    }

    int N = atoi(argv[1]);
    // Não tenho certeza se ele vai usar float aqui, talvez seja só inteiro
    float X = atof(argv[2]);

    VecInt fila = allocVecInt(N!=0?N:100);

    return 0;
}