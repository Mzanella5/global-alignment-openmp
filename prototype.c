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
int NO_AFFINE = 0;

typedef struct
{
    int value;
    int x;
    int y;
} Point;

Point BIGGER_POINT;

int Similarity(char first, char second, int *gap_seq_a, int *gap_seq_b)
{
    if(NO_AFFINE == 1)
    {
        // Simple gap penalty
        if(first == '-' || second == '-')
        {
            return GAP;
        }
    }
    else // Aplies affine gap
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

int FunctionSimilarity(int **mat, char a, char b, int i, int j, char *pos, int *gap_seq_a, int *gap_seq_b)
{
    int v1=INT_MIN,v2=INT_MIN,v3=INT_MIN;
    int result = 0;

    if(i-1 >= 0 && j-1 >= 0)
        v1 = mat[i-1][j-1] + Similarity(a, b, NULL, NULL);
    if(i-1 >= 0)
        v2 = mat[i-1][j] + Similarity(a, '-', gap_seq_a, gap_seq_b);
    if(j-1 >= 0)
        v3 = mat[i][j-1] + Similarity('-', b, gap_seq_a, gap_seq_b);

    result = v1;
    *pos = 'D';
    if(v2 > result)
    {
        result = v2;
        *pos = 'V';
        // if(gap_seq_b != NULL)
        //     *gap_seq_b = *gap_seq_b + 1;
    }
    if(v3 > result)
    {
        result = v3;
        *pos = 'H';
        // if(gap_seq_a != NULL)
        //     *gap_seq_a = *gap_seq_a + 1;
    }

    return result;
}

void PrintMatrix(int **mat)
{
    printf("<Print Matrix>\n");
    for (int i = 0; i < SIZEA; i++)
    {
        for (int j = 0; j < SIZEB; j++)
            printf("%0*d | ", 3, mat[i][j]);
        printf("\n");
    }
    printf("\n");
}

void FreeMatrix(int **matrix)
{
    for (int i=0; i < SIZEA; i++)
        free(matrix[i]);
    free(matrix);
}

int** InitializeMatrix()
{
    int** mat = (int**) malloc(SIZEA * sizeof(int *));
    for (int i = 0; i < SIZEA; i++)
        mat[i] = (int*) malloc(SIZEB * sizeof(int));
    
    return mat;
}

double CalculateSimilarity(int **mat, char *vetA, char *vetB)
{
    int block_line_size, block_column_size;
    double start_time, end_time, elapsed_time;
    char *pos = (char*) calloc(1, sizeof(char));
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

    printf("block line size: %d block column size: %d\n", block_line_size, block_column_size);

    start_time = omp_get_wtime();
    // Iniciar uma região paralela
    #pragma omp parallel for
    for (int block_line = 0; block_line < N_BLOCKS; block_line++) 
    {
        int gap_seq_a=1, gap_seq_b=1;
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
                        mat[i][j] = i * -1;
                    else 
                        if(i == 0) 
                            mat[i][j] = j * -1;
                        else
                            mat[i][j] = FunctionSimilarity(mat, vetA[i], vetB[j], i, j, pos, &gap_seq_a, &gap_seq_b);
                }
            }
            if (block_line <  N_BLOCKS - 1)
                sem_post(&semaphore[block_line+1][block_column]);
        }
    }
    end_time = omp_get_wtime();
    elapsed_time = end_time - start_time;
    printf("Alignment time: %f seconds\n", elapsed_time);

    if(mat[SIZEA-1][SIZEB-1] == INT_MIN)
        printf("Not Completed... %d\n", mat[SIZEA-1][SIZEB-1]);

    // Destrua os semáforos quando não forem mais necessários
    for (int i = 0; i < N_BLOCKS; i++) 
    {
        for (int j = 0; j < N_BLOCKS; j++) 
        {
            sem_destroy(&semaphore[i][j]);
        }
    }

    free(pos);
    return elapsed_time;
}

int MountSequence(int **mat, char *vetA, char *vetB, char **vetResA, char **vetResB)
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
        FunctionSimilarity(mat, vetA[i], vetB[j], i, j, pos, NULL, NULL);
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
        else if(*pos == 'H')
        {
            *(*(vetResA) + k)  = '-';
            *(*(vetResB) + l)= vetB[j];
            j--;
        }
    } while (i >= 0 && j >= 0);

    free(pos);
    if(VERBOSE)
        printf("\n");
    return 1;
}

void PrintVector(char *vet, int size)
{
    for (int i=0; i < size; i++)
        if (vet[i] >= 'A' && vet[i] <= 'Z' || vet[i] == '-')
            printf("%c ", vet[i]);
        
    printf("\n");
}

int CountFinalSequence(char *vet, int size)
{
    int i, count=0;
    for (i=0; i < size; i++)
        if (vet[i] >= 'A' && vet[i] <= 'Z' || vet[i] == '-')
            count++;
        
    return count;
}

read_data_result ReadFastaData(char **vet, char *path)
{
    FILE* file;
    char ch;
    int i, size = 1, skipLine = 0;
    read_data_result result;

    file = fopen(path, "r");
    if(file == NULL)
    {
        printf("\nCan't read the file!\n");
        return result;
    }

    for (i=0; i < 99; i++)
    {
        ch = fgetc(file);
        if (ch == EOF || ch == '\n')
            break;
        else
            result.sequence_name[i] = ch;
    }

    result.sequence_name[i+1] = '\0';
    printf("%s\n", result.sequence_name);

    fseek(file, 0, SEEK_SET);

    do {
        ch = fgetc(file);

        if(skipLine == 1 && ch == '\n')
            skipLine = 0;

        if(ch == '>')
            skipLine = 1;

        if((ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z') && skipLine == 0)
            size++;
        
    
    } while(ch != EOF);

    *vet = (char*) calloc(size, sizeof(char));

    fseek(file, 0, SEEK_SET);
    skipLine = 0;
    i=1;
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
    result.size = size;
    return result;
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

char* PrintResults(char *vetA, char *vetB, int size, int size_sequence, int **mat)
{
    int i, hits=0, misses=0, gaps=0;
    char a, b, *ret;

    ret = (char*) calloc(100, sizeof(char));

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

    sprintf(ret, "Score: %d Identities: %d Gaps: %d Misses: %d AlignmentSize: %d\n", mat[SIZEA-1][SIZEB-1], hits, gaps, misses, size_sequence);

    printf("==========Results==========\n");
    printf("Hits: %d \nMisses: %d \nGaps: %d \nAlignmentSize: %d\nScore: %d\n", hits, misses, gaps, size_sequence, mat[SIZEA-1][SIZEB-1]);
    printf("===========================\n");
    return ret;
}

int WriteFile(char *vetA, char *vetB, int size, char *metrics, double elapsed_time, read_data_result result_a, read_data_result result_b)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    FILE *file;
    int lineSize = 100, l=0;
    char datetime[50], seqA[lineSize+3], seqB[lineSize+3], identities[lineSize+3], a,b;

    sprintf(datetime, "out/%02d%02d%02d%02d%02d%02d.txt",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec);

    file = fopen(datetime, "w");
    if (file == NULL) {
        printf("\nErro ao criar o arquivo.\n");
        return 0;
    }

    fprintf(file, "Reference sequence: %s", result_a.sequence_name);
    fprintf(file, "Subject sequence: %s", result_b.sequence_name);
    fprintf(file, "%s", metrics);
    fprintf(file, "Alignment time: %f seconds\n\n", elapsed_time);

    for (int i=0; i < size; i++)
    {
        if (l < lineSize)
        {
            a = vetA[i];
            b = vetB[i];

            if(a >= 'A' && a <= 'Z' && b >= 'A' && b <= 'Z' || a == '-' || b == '-')
            {
                if(a == b)
                {
                    identities[l] = '|';
                }
                else
                if(a == '-' || b == '-')
                {
                    identities[l] = '&';
                }
                else identities[l] = '*';

                seqA[l] = a;
                seqB[l] = b;
                l++;
            }
        }
        else
        {
            seqA[l] = '\0';
            seqB[l] = '\0';
            identities[l] = '\0';
            fprintf(file, "%s\n", seqA);
            fprintf(file, "%s\n", identities);
            fprintf(file, "%s\n\n", seqB);
            l = 0;
        }
    }

    seqA[l] = '\0';
    seqB[l] = '\0';
    identities[l] = '\0';
    fprintf(file, "%s\n", seqA);
    fprintf(file, "%s\n", identities);
    fprintf(file, "%s\n", seqB);    

    fclose(file);
    return 1;
}

void PrintHelp(char* prog_name)
{
    printf("\n\nUse: %s [File Path 1] [File Path 2]\n", prog_name);
    printf("Options:\n");
    printf("--match | INT\n");
    printf("--mismatch | INT\n");
    printf("--gap | INT\n");
    printf("--gap_seq | INT\n");
    printf("-help\n");
    printf("-verbose\n");
    printf("-no_affine\n\n");
}

int main(int argc, char *argv[7])
{
    char *vetA, *vetB;
    int **mat;
    int i=0,j=0, res, alignment_size, num_options = 0;
    char *vetResA;
    char *vetResB;
    double elapsed_time;
    read_data_result result_a;
    read_data_result result_b;

    if(argv[1] == NULL || argv[2] == NULL)
    {
        PrintHelp(argv[0]);
        return 0;
    }

    Option* options = process_options(argc, argv, &num_options);

    MATCH = options[0].value;
    MISMATCH = options[1].value;
    GAP = options[2].value;
    GAP_SEQ = options[3].value;
    VERBOSE = options[4].value;
    N_BLOCKS = options[5].value;
    NO_AFFINE = options[6].value;

    if(options[7].value)
    {
        PrintHelp(argv[0]);
        return 0;
    }

    printf("<Reading Files>\n");
    printf("Reference Sequence: ");
    result_a = ReadFastaData(&vetA, argv[1]);
    printf("Subject Sequence: ");
    result_b = ReadFastaData(&vetB, argv[2]);
    SIZEA = result_a.size;
    SIZEB = result_b.size;

    printf("SIZE A: %d SIZE B: %d\n", SIZEA-1, SIZEB-1);
    
    SIZERES = SIZEA + SIZEB;
    MATRIX_SIZE = SIZEA * SIZEB;

    vetResA = (char*) calloc(SIZERES, sizeof(char));
    vetResB = (char*) calloc(SIZERES, sizeof(char));

    printf("<Initialize Matrix>\n");
    mat = InitializeMatrix();
    printf("<Calculate Matrix>\n");
    elapsed_time = CalculateSimilarity(mat, vetA, vetB);

    if(VERBOSE)
        PrintMatrix(mat);
    
    printf("<Mount Sequence>\n");
    MountSequence(mat, vetA, vetB, &vetResA, &vetResB);

    if(VERBOSE)
    {
        printf("Sequence A: ");
        PrintVector(vetResA, SIZERES);
        printf("Sequence B: ");
        PrintVector(vetResB, SIZERES);
    }

    printf("<Calculate Results>\n");
    alignment_size = CountFinalSequence(vetResA, SIZERES);
    char *result = PrintResults(vetResA, vetResB, SIZERES, alignment_size, mat);

    //printf("Writing file...\n");
    //WriteFile(vetResA, vetResB, SIZERES, result, elapsed_time, result_a, result_b);

    FreeMatrix(mat);
    free(vetA);
    free(vetB);
    free(vetResA);
    free(vetResB);
    return 0;
}