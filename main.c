#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

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
    int soma = 0;
    int number_process = 16;
    int block_size = 1;
    double start_time, end_time;
    double elapsed_time;

    if(argc > 3)
    {
        line = atoi(argv[1]);
        column = atoi(argv[2]);
        block_size = atoi(argv[3]);
    }

    int matrix_size = line * column;
    int *vector = (int*) malloc(sizeof(int) * matrix_size);

    if (vector == NULL) {
        printf("Erro na alocação de memória. %lu bytes\n", sizeof(int) * matrix_size);
        return 1;
    }

    // Inicializar o vetor com valores
    for (int i = 0; i < matrix_size; i++) {
        vector[i] = INT_MIN;
    }

    start_time = omp_get_wtime();
    // Iniciar uma região paralela
    #pragma omp parallel for
    for (int block = 0; block < line/block_size; block++) {
        
        int pid = omp_get_thread_num(); // Obtém o ID da thread

        for (int i=block*block_size; i<block*block_size+block_size; i++) {
            for (int j=0; j<column; j++) {
                //while((i-1)*column+j !=0 && i > 0 && vector[(i-1)*column+j] == 0);
                while(i > 0 && vector[(i-1)*column+j] == INT_MIN);

                // calcula
                if(j == 0) vector[i*column+j] = i * -1;
                else if(i == 0) vector[i*column+j] = j * -1;
                else vector[i*column+j] = Max(vector[i*column+j-1], vector[(i-1)*column+j-1], vector[(i-1)*column+j]) + 2;
            }
            //printf("Thread %d calculou a linha %d do bloco %d\n", pid, i, block);
        }
    }
    end_time = omp_get_wtime();
    // for (int i = 0; i < matrix_size; i++) {
    //     printf("%10d|", vector[i]);
    //     if((i+1)%column == 0) printf("\n");
    // }

    elapsed_time = end_time - start_time;
    printf("omp_wtime elapsed time: %f seconds\n", elapsed_time);

    if(vector[matrix_size-1] == INT_MIN)
        printf("Not Completed... %d\n", vector[matrix_size-1]);
    return 0;
}