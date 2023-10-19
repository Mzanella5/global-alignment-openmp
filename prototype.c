#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <semaphore.h>
#include <limits.h>
#include <ctype.h>
#include <string.h>
#include "options.h"

int Max(int val1, int val2, int val3)
{
    int bigger = val1;
    if(val2 > bigger)
        bigger = val2;
    if(val3 > bigger)
        bigger = val3;

    return bigger;
}

int main(int argc, char *argv[]) {
    int line = 10; // Tamanho do vetor
    int column = 10; // Tamanho do vetor
    int block_size = 10;
    double start_time, end_time;
    double elapsed_time;
    int threads = omp_get_max_threads();
    sem_t semaphore[threads];
    int matrix_size;
    int *vector;
    int n_blocks = 1;
    int verbose = 0;

    if(argc > 3)
    {
        line = atoi(argv[1]);
        column = atoi(argv[2]);
        n_blocks = atoi(argv[3]);
        if(argc == 5)
            verbose = atoi(argv[4]);
    }

    block_size = line/n_blocks;
    if (block_size < 1)
    {
        printf("Number of blocks can not be bigger than the lines number\n");
        return 0;
    }

    printf("Threads %d\n", threads);
    printf("NThreads: %d of %d lines per thread\n", n_blocks, block_size);

    matrix_size = line * column;
    vector = (int*) malloc(sizeof(int) * matrix_size);

    if (vector == NULL) {
        printf("Erro na alocação de memória. %lu bytes\n", sizeof(int) * matrix_size);
        return 1;
    }

    // Inicializar o vetor com valores
    for (int i = 0; i < matrix_size; i++) {
        vector[i] = INT_MIN;
    }

    // Inicializar os semáforos
    for (int i = 0; i < threads; i++) {
        sem_init(&semaphore[i], 0, 1); // Inicia com 1 recurso disponível
    }

    start_time = omp_get_wtime();
    // Iniciar uma região paralela
    #pragma omp parallel for
    for (int block = 0; block < n_blocks; block++) {

        int pid = omp_get_thread_num(); // Obtém o ID da thread
        int lastPid = pid -1;
        if(lastPid == -1)
            lastPid = 15;

        // calcula posicao inicial e final de cada bloco
        int block_initial_pos = block * block_size;
        int block_final_pos = block_initial_pos + block_size;
    
        // ultima thread fica com o restante das linhas
        // caso não seja divisível pela quantidade de blocos
        if(block == n_blocks -1 && n_blocks > 1)
            block_final_pos = line % n_blocks + block_final_pos;

        for (int i=block_initial_pos; i<block_final_pos; i++) {
            for (int j=0; j<column; j++) {
                while(i > 0 && vector[(i-1)*column+j] == INT_MIN);
                // if(i > 0 && vector[(i-1)*column+j] == INT_MIN)
                //     sem_wait(&semaphore[lastPid]);

                // calcula
                if(j == 0) vector[i*column+j] = i * -1;
                else if(i == 0) vector[i*column+j] = j * -1;
                else{
                    vector[i*column+j] = Max(vector[i*column+j-1], vector[(i-1)*column+j-1], vector[(i-1)*column+j]) + 2;
                    //sem_post(&semaphore[pid]);
                }
            }
            //printf("Thread %d calculou a linha %d do bloco %d\n", pid, i, block);
        }
    }
    end_time = omp_get_wtime();
    if(verbose)
        for (int i = 0; i < matrix_size; i++) {
            printf("%10d|", vector[i]);
            if((i+1)%column == 0) printf("\n");
        }

    elapsed_time = end_time - start_time;
    printf("omp_wtime elapsed time: %f seconds\n", elapsed_time);

    if(vector[matrix_size-1] == INT_MIN)
        printf("Not Completed... %d\n", vector[matrix_size-1]);

    // Destrua os semáforos quando não forem mais necessários
    for (int i = 0; i < threads; i++) {
        sem_destroy(&semaphore[i]);
    }
    
    return 0;
}