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
    int block_line_size, block_column_size;
    double start_time, end_time;
    double elapsed_time;
    int threads = omp_get_max_threads();
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
    sem_t semaphore[n_blocks][n_blocks];

    block_line_size = line/n_blocks;
    block_column_size = column/n_blocks;
    if (block_line_size < 1 || block_column_size < 1)
    {
        printf("Number of blocks can not be bigger than the lines number\n");
        return 0;
    }

    printf("block_line_size: %d\nblock_column_size: %d\n", block_line_size, block_column_size);

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
    for (int i = 0; i < n_blocks; i++) 
    {
        for (int j = 0; j < n_blocks; j++) 
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
    for (int block = 0; block < n_blocks; block++) 
    {
        int pid = omp_get_thread_num(); // Obtém o ID da thread

        for (int block_column = 0; block_column < n_blocks; block_column++)
        {
            // calcula posicao inicial e final de cada bloco
            int block_initial_pos = block * block_line_size;
            int block_final_pos = block_initial_pos + block_line_size;
            int i,j;
        
            // ultima thread fica com o restante das linhas
            // caso não seja divisível pela quantidade de blocos
            if (block == n_blocks - 1 && n_blocks > 1)
                block_final_pos = line % n_blocks + block_final_pos;

            sem_wait(&semaphore[block][block_column]);
            for (i=block_initial_pos; i<block_final_pos; i++) 
            {
                int block_initial_column = block_column * block_column_size;
                int block_final_column = block_initial_column + block_column_size;
                if (block_column == n_blocks - 1 && n_blocks > 1)
                    block_final_column = column % n_blocks + block_final_column;

                for (j=block_initial_column; j<block_final_column; j++) 
                {
                    // calcula
                    if(j == 0) 
                        vector[i*column+j] = i * -1;
                    else 
                        if(i == 0)
                            vector[i*column+j] = j * -1;
                        else
                            vector[i*column+j] = Max(vector[i*column+j-1], vector[(i-1)*column+j-1], vector[(i-1)*column+j]) + 2;
                    
                }
                //printf("Thread %d calculou a linha %d do bloco %d\n", pid, i, block);
            }
            if (block <  n_blocks - 1)
                sem_post(&semaphore[block+1][block_column]);
        }
    }
    end_time = omp_get_wtime();
    if(verbose)
        for (int i = 0; i < matrix_size; i++) 
        {
            printf("%10d|", vector[i]);
            if((i+1)%column == 0) printf("\n");
        }

    elapsed_time = end_time - start_time;
    printf("omp_wtime elapsed time: %f seconds\n", elapsed_time);

    if(vector[matrix_size-1] == INT_MIN)
        printf("Not Completed... %d\n", vector[matrix_size-1]);

    // Destrua os semáforos quando não forem mais necessários
    for (int i = 0; i < n_blocks; i++) 
    {
        for (int j = 0; j < n_blocks; j++) 
        {
            sem_destroy(&semaphore[i][j]);
        }
    }
    
    return 0;
}