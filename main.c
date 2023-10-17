#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <semaphore.h>


double calcularPi(int total_points) {
    int points_inside_circle = 0;
    double x, y;

    srand(time(NULL));

    for (int i = 0; i < total_points; i++) {
        x = (double)rand() / RAND_MAX;
        y = (double)rand() / RAND_MAX;

        if (x * x + y * y <= 1) {
            points_inside_circle++;
        }
    }

    return 4.0 * points_inside_circle / total_points;
}

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
    int threads = omp_get_max_threads();
    sem_t semaphore[threads];

    if(argc > 3)
    {
        line = atoi(argv[1]);
        column = atoi(argv[2]);
        block_size = atoi(argv[3]);
    }

    printf("Threads %d\n", threads);
    printf("NThreads: %d of %d lines per thread\n", line/block_size, block_size);

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

    // Inicializar os semáforos
    for (int i = 0; i < threads; i++) {
        sem_init(&semaphore[i], 0, 1); // Inicia com 1 recurso disponível
    }

    start_time = omp_get_wtime();
    // Iniciar uma região paralela
    #pragma omp parallel for
    for (int block = 0; block < line/block_size; block++) {

        int pid = omp_get_thread_num(); // Obtém o ID da thread
        int lastPid = pid -1;
        if(lastPid == -1)
            lastPid = 15;

        for (int i=block*block_size; i<block*block_size+block_size; i++) {
            for (int j=0; j<column; j++) {
                //while(i > 0 && vector[(i-1)*column+j] == INT_MIN);
                if(i > 0 && vector[(i-1)*column+j] == INT_MIN)
                    sem_wait(&semaphore[lastPid]);

                // calcula
                if(j == 0) vector[i*column+j] = i * -1;
                else if(i == 0) vector[i*column+j] = j * -1;
                else vector[i*column+j] = Max(vector[i*column+j-1], vector[(i-1)*column+j-1], vector[(i-1)*column+j]) + 2;
                //calcularPi(1000000);
                sem_post(&semaphore[pid]);
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

    // Destrua os semáforos quando não forem mais necessários
    for (int i = 0; i < threads; i++) {
        sem_destroy(&semaphore[i]);
    }
    
    return 0;
}