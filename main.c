#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <string.h>
#include "options.h"
#include <semaphore.h>
#include <omp.h>
#include <time.h>

#define getName(var)  #var
int SIZEA, SIZEB, SIZERES, MATRIX_SIZE, N_BLOCKS;
int VERBOSE = 0;
int MATCH = 2;
int MISMATCH = -1;
int GAP = -1;
int GAP_SEQ = -1;

typedef struct
{
    int value;
    int x;
    int y;
} Point;

Point BIGGER_POINT;

int Similarity(char first, char second, int *gap_seq_a, int *gap_seq_b)
{
    if(gap_seq_a != NULL)
    {
        if(first == '-')
        {
            if(*gap_seq_a)
            {
                return GAP_SEQ;
            }
            *gap_seq_a = *gap_seq_a + 1;

            return GAP + *gap_seq_a * GAP_SEQ;
        }
        else
        {
            *gap_seq_a = 0;
        }
    }

    if(gap_seq_b != NULL)
    {
        if(second == '-')
        {
            if(*gap_seq_b)
            {
                return GAP_SEQ;
            }
            *gap_seq_b = *gap_seq_b + 1;

            return GAP + *gap_seq_b * GAP_SEQ;
        }
        else
        {
            *gap_seq_b = 0;
        }
    }


    if(first == second)
        return MATCH;
    return MISMATCH;
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

int FunctionSimilarity(int* mat, char a, char b, int i, int j, char *pos)
{
    int v1=INT_MIN,v2=INT_MIN,v3=INT_MIN;
    int result = 0, gap_seq_a = 0, gap_seq_b = 0;

    if(i-1 >= 0 && j-1 >= 0)
        v1 = mat[(i-1)*SIZEB+j-1] + Similarity(a, b, NULL, NULL);
    if(i-1 >= 0)
        v2 = mat[(i-1)*SIZEB+j] + Similarity(a, '-', &gap_seq_a, &gap_seq_b);
    if(j-1 >= 0)
        v3 = mat[i*SIZEB+j-1] + Similarity('-', b, &gap_seq_a, &gap_seq_b);

    result = v2;
    *pos = 'V';
    if(v1 > result)
    {
        result = v1;
        *pos = 'D';
    }
    if(v3 > result)
    {
        result = v3;
        *pos = 'H';
    }

    return result;
}

void PrintMatrix(int *mat)
{
    for (int i = 0; i < MATRIX_SIZE; i++) {
        printf("%0*d | ", 3, mat[i]);
        if((i+1)%SIZEB == 0) printf("\n");
    }
    printf("\n");
}

int* InitializeMatrix()
{
    int* mat = (int*) malloc(MATRIX_SIZE * sizeof(int));
    for (int i = 0; i < MATRIX_SIZE; i++)
        mat[i] = INT_MIN;
    
    return mat;
}

void CalculateSimilarity(int *mat, char *vetA, char *vetB)
{
    int max_threads, block_line_size, block_column_size;
    double start_time, end_time, elapsed_time;
    char *pos = (char*) calloc(1, sizeof(char));
    max_threads = omp_get_max_threads();
    sem_t semaphore[N_BLOCKS][N_BLOCKS];

    block_line_size = SIZEA / N_BLOCKS;
    block_column_size = SIZEB / N_BLOCKS;

    // Inicializar os semáforos
    for (int i = 0; i < N_BLOCKS; i++) 
    {
        for (int j = 0; j < N_BLOCKS; j++) 
        {
            if (i==0)
                sem_init(&semaphore[i][j], 0, 1); // Inicia com 1 recurso disponível
            else
                sem_init(&semaphore[i][j], 0, 0); // Inicia com 0 recurso indisponível
        }
    }

    printf("block_line_size: %d\nblock_column_size: %d\n", block_line_size, block_column_size);

    start_time = omp_get_wtime();
    // Iniciar uma região paralela
    #pragma omp parallel for
    for (int block_line = 0; block_line < N_BLOCKS; block_line++) 
    {
        for (int block_column = 0; block_column < N_BLOCKS; block_column++)
        {
            // calcula posicao inicial e final de cada bloco
            int block_initial_pos = block_line * block_line_size;
            int block_final_pos = block_initial_pos + block_line_size;
        
            // ultima thread fica com o restante das linhas
            // caso não seja divisível pela quantidade de blocos
            if(block_line == N_BLOCKS -1 && N_BLOCKS > 1)
                block_final_pos = SIZEA % N_BLOCKS + block_final_pos;

            sem_wait(&semaphore[block_line][block_column]);
            for (int i=block_initial_pos; i<block_final_pos; i++) 
            {
                int block_initial_column = block_column * block_column_size;
                int block_final_column = block_initial_column + block_column_size;
                if (block_column == N_BLOCKS - 1 && N_BLOCKS > 1)
                    block_final_column = SIZEB % N_BLOCKS + block_final_column;

                for (int j=block_initial_column; j<block_final_column; j++) 
                {
                    // calcula
                    if(j == 0) 
                        mat[i*SIZEB+j] = i * -1;
                    else 
                        if(i == 0) 
                            mat[i*SIZEB+j] = j * -1;
                        else
                            mat[i*SIZEB+j] = FunctionSimilarity(mat, vetA[i], vetB[j], i, j, pos);
                }
            }
            if (block_line <  N_BLOCKS - 1)
                sem_post(&semaphore[block_line+1][block_column]);
        }
    }
    end_time = omp_get_wtime();

    elapsed_time = end_time - start_time;
    printf("omp_wtime elapsed time: %f seconds\n", elapsed_time);

    if(mat[MATRIX_SIZE-1] == INT_MIN)
        printf("Not Completed... %d\n", mat[MATRIX_SIZE-1]);

    // Destrua os semáforos quando não forem mais necessários
    for (int i = 0; i < N_BLOCKS; i++) 
    {
        for (int j = 0; j < N_BLOCKS; j++) 
        {
            sem_destroy(&semaphore[i][j]);
        }
    }

    free(pos);
}

void MountSequence(int *mat, char *vetA, char *vetB, char **vetResA, char **vetResB)
{
    int i,j,k,l;
    char *pos = (char*) calloc(1, sizeof(char));

    for (i=0; i < SIZERES; i++)
    {
        *(*(vetResA) + i) = ' ';
        *(*(vetResB) + i) = ' ';
    }

    i = SIZEA-1;
    j = SIZEB-1;

    k=SIZERES;
    l=SIZERES;

    if(VERBOSE)
        printf("Matrix back path: ");

    do
    {
        k--;
        l--;
        FunctionSimilarity(mat, vetA[i], vetB[j], i, j, pos);
        if(i==0 && j==0) break;

        if(VERBOSE)
            printf(" %c |", *pos);

        if(*pos == 'D')
        {
            *(*(vetResA) + k) = vetA[i];
            *(*(vetResB) + l) = vetB[j];
            i--;
            j--;
        }
        else if(*pos == 'V')
        {
            *(*(vetResA) + k)  = vetA[i];
            *(*(vetResB) + l) = '-';
            i--;
        }
        else
        {
            *(*(vetResA) + k)  = '-';
            *(*(vetResB) + l)= vetB[j];
            j--;
        }
    } while (i >= 0 && j >= 0);

    free(pos);
    printf("\n");
    return;
}

void PrintVector(char *vet, int size)
{
    int i;
    for (i=0; i < size; i++)
        printf("%c ", vet[i]);
    printf("\n");
}

int ReadFastaData(char **vet, char *path)
{
    FILE* file;
    char ch;
    int i = 1, size = 1, skipLine = 0;

    file = fopen(path, "r");
    if(file == NULL)
    {
        printf("\nCan't read the file!\n");
        return size;
    }

    do {
        ch = fgetc(file);

        if(skipLine == 1 && ch == '\n')
            skipLine = 0;

        if(ch == '>')
            skipLine = 1;

        if((ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z') && skipLine == 0)
            size++;
        
    
    } while(ch != EOF);

    printf("SIZE: %d\n", size);

    *vet = (char*) calloc(size, sizeof(char));

    fseek( file, 0, SEEK_SET );
    skipLine = 0;
    do {
        ch = fgetc(file);

        if(skipLine == 1 && ch == '\n')
            skipLine = 0;

        if(ch == '>')
            skipLine = 1;

        if(ch >= 'a' && ch <= 'z')
            ch -= 32;

        if(ch >= 'A' && ch <= 'Z' && skipLine == 0)
        {
            *((*vet) + i) = ch;
            i++;
        }
    } while (ch != EOF);
    
    fclose(file);
    return size;
}

void ReadData(char **vetA, char **vetB, char *path)
{
    FILE* file;
    char ch;
    int next = 1, i, j;
    SIZEA = 1;
    SIZEB = 1;

    file = fopen(path, "r");
    if(file == NULL)
    {
        printf("\nCan't read the file!\n");
        return;
    }

    do {
        ch = fgetc(file);
        // ch = (int)toupper(ch);
        // printf("%c", ch);
        // if(ch >= 'a' && ch <= 'z')
        //     ch = ch - ' ';

        if(ch == '\n')
        {
            next = 0;
        }
        if(next == 1)
        {
            if(ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z')
            {
                SIZEA++;
            }
        }
        else
        {
            if(ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z')
            {
                SIZEB++;   
            }
        }
    
    } while(ch != EOF);

    printf("SIZEA: %d\n", SIZEA);
    printf("SIZEB: %d\n", SIZEB);

    *vetA = (char*) calloc(SIZEA, sizeof(char));
    *vetB = (char*) calloc(SIZEB, sizeof(char));

    fseek( file, 0, SEEK_SET );

    next = 1;
    i=1;
    j=1;
    do {
        ch = fgetc(file);

        if(ch == '\n')
            next = 0;

        if(next)
        {
            if(ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z')
            {
                *((*vetA) + i) = ch;
                i++;
            }
        }
        else
        {
            if(ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z') 
            {
                *((*vetB) + j) = ch;
                j++;
            }
        }
    } while (ch != EOF);
    
    fclose(file);
}

void PrintResults(char *vetA, char *vetB, int size)
{
    int i, hits=0, misses=0, gaps=0;
    char a, b;
    for(i=0; i < size; i++)
    {
        a = vetA[i];
        b = vetB[i];

        if(a >= 'A' && a <= 'Z' && b >= 'A' && b <= 'Z')
        {
            if(a == b)
            {
                hits++;
                continue;
            }
            else
            {
                misses++;
                continue;
            }
        }

        if(a == '-')
            gaps++;

        if(b == '-')
            gaps++;

    }

    printf("======Results======\n");
    printf("Hits: %d \nMisses: %d \nGaps: %d \nAlignmentSize: %d \n", hits, misses, gaps, size);
}

int main(int argc, char *argv[7])
{
    char *vetA, *vetB;
    int *mat;
    int i=0,j=0, res;
    char *vetResA;
    char *vetResB;
    int num_options = 0;

    if(argv[1] == NULL || argv[2] == NULL)
    {
        printf("\n\nUse: %s [File Path 1] [File Path 2]\n", argv[0]);
        printf("Options:\n");
        printf("--match | INT\n");
        printf("--mismatch | INT\n");
        printf("--gap | INT\n");
        printf("--gap_seq | INT\n");
        printf("-verbose | 1=TRUE 0=FALSE\n\n");
        return 0;
    }

    Option* options = process_options(argc, argv, &num_options);

    MATCH = options[0].value;
    MISMATCH = options[1].value;
    GAP = options[2].value;
    GAP_SEQ = options[3].value;
    VERBOSE = options[4].value;
    N_BLOCKS = options[5].value;

    if(VERBOSE)
        for(i=0; i < num_options; i++)
        {
            printf("%s = %d\n", options[i].option, options[i].value);
        }

    printf("Reading Files...\n");
    SIZEA = ReadFastaData(&vetA, argv[1]);
    SIZEB = ReadFastaData(&vetB, argv[2]);
    SIZERES = SIZEA + SIZEB;
    MATRIX_SIZE = SIZEA * SIZEB;

    vetResA = (char*) calloc(SIZERES, sizeof(char));
    vetResB = (char*) calloc(SIZERES, sizeof(char));

    if(VERBOSE)
    {
        PrintVector(vetA, SIZEA);
        PrintVector(vetB, SIZEB);

        PrintVector(vetResA, SIZERES);
        PrintVector(vetResB, SIZERES);
    }

    printf("Initialize Matrix...\n");
    mat = InitializeMatrix();
    printf("Calculate Matrix...\n");
    CalculateSimilarity(mat, vetA, vetB);
    if(VERBOSE)
        PrintMatrix(mat);
    
    printf("Mount Sequence...\n");
    MountSequence(mat, vetA, vetB, &vetResA, &vetResB);

    if(VERBOSE)
    {
        PrintVector(vetResA, SIZERES);
        PrintVector(vetResB, SIZERES);
    }

    printf("Calculate Results...\n");
    PrintResults(vetResA, vetResB, SIZERES);

    free(mat);
    free(vetA);
    free(vetB);
    free(vetResA);
    free(vetResB);
    return 0;
}