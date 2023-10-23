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

long long Max(long long val1, long long val2, long long val3)
{
    int bigger = val1;
    if(val2 > bigger)
        bigger = val2;
    if(val3 > bigger)
        bigger = val3;

    return bigger;
}

int main(int argc, char *argv[]) {
    long long line, column; // Tamanho do vetor
    long long block_line_size, block_column_size;
    double start_time, end_time;
    double elapsed_time;
    int threads = omp_get_max_threads();
    long long matrix_size;
    long long **vector;
    long long n_blocks_line = 1, n_blocks_column = 1;
    int verbose = 0;

    if(argc > 3)
    {
        line = atoi(argv[1]);
        column = atoi(argv[2]);
        n_blocks_line = atoi(argv[3]);
        n_blocks_column = atoi(argv[4]);
        if(argc == 6)
            verbose = atoi(argv[5]);
    }
    sem_t semaphore[n_blocks_line][n_blocks_column];

    block_line_size = line/n_blocks_line;
    block_column_size = column/n_blocks_column;
    if (block_line_size < 1 || block_column_size < 1)
    {
        printf("Number of blocks can not be bigger than the lines number\n");
        return 0;
    }

    printf("block_line_size: %lld\nblock_column_size: %lld\n", block_line_size, block_column_size);

    matrix_size = line * column;
    vector = (long long**) malloc(sizeof(long long *) * line);
    for (long long i=0; i < line; i++)
        vector[i] = (long long*) malloc(sizeof(long long) * column);


    if (vector == NULL) {
        printf("Erro na alocação de memória. %llu bytes\n", sizeof(long long) * matrix_size);
        return 1;
    }

    // Inicializar o vetor com valores
    for (long long i = 0; i < line; i++) {
        for (long long j = 0; j < column; j++)
            vector[i][j] = LLONG_MIN;
    }

    // Inicializar os semáforos
    for (int i = 0; i < n_blocks_line; i++) 
    {
        for (int j = 0; j < n_blocks_column; j++) 
        {
            if (i==0)
                sem_init(&semaphore[i][j], 0, 1); // Inicia com 1 recurso disponível
            else
                sem_init(&semaphore[i][j], 0, 0); // Inicia com 0 recurso indisponível
        }
    }

    start_time = omp_get_wtime();
    // Iniciar uma região paralela
    #pragma omp parallel for
    for (long long block = 0; block < n_blocks_line; block++) 
    {
        int pid = omp_get_thread_num(); // Obtém o ID da thread

        for (long long block_column = 0; block_column < n_blocks_column; block_column++)
        {
            // calcula posicao inicial e final de cada bloco
            long long block_initial_pos = block * block_line_size;
            long long block_final_pos = block_initial_pos + block_line_size;
        
            // ultima thread fica com o restante das linhas
            // caso não seja divisível pela quantidade de blocos
            if (block == n_blocks_line - 1 && n_blocks_line > 1)
                block_final_pos = line % n_blocks_line + block_final_pos;

            sem_wait(&semaphore[block][block_column]);
            for (long long i=block_initial_pos; i<block_final_pos; i++) 
            {
                long long block_initial_column = block_column * block_column_size;
                long long block_final_column = block_initial_column + block_column_size;
                if (block_column == n_blocks_column - 1 && n_blocks_column > 1)
                    block_final_column = column % n_blocks_column + block_final_column;

                for (long long j=block_initial_column; j<block_final_column; j++) 
                {
                    // calcula
                    if(j == 0) 
                        vector[i][j] = i * -1;
                    else 
                        if(i == 0)
                            vector[i][j] = j * -1;
                        else
                            vector[i][j] = Max(vector[i][j-1], vector[i-1][j-1], vector[i-1][j]) + 2;
                    
                }
                //printf("Thread %d calculou a linha %d do bloco %d\n", pid, i, block);
            }
            if (block <  n_blocks_line - 1)
                sem_post(&semaphore[block+1][block_column]);
        }
    }
    end_time = omp_get_wtime();
    if(verbose)
        for (int i = 0; i < line; i++) 
        {
            for (int j = 0; j < column; j++) 
                printf("%10lld|", vector[i][j]);

            printf("\n");
        }

    elapsed_time = end_time - start_time;
    printf("omp_wtime elapsed time: %f seconds\n", elapsed_time);

    if(vector[line-1][column-1] == LLONG_MIN)
        printf("Not Completed... %lld\n", vector[line-1][column-1]);

    // Destrua os semáforos quando não forem mais necessários
    for (int i = 0; i < n_blocks_line; i++) 
    {
        for (int j = 0; j < n_blocks_column; j++) 
        {
            sem_destroy(&semaphore[i][j]);
        }
    }

    for (int i=0; i < line; i++)
        free(vector[i]);
    free(vector);
    
    return 0;
}