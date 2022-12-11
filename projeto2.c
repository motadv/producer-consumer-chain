#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include </usr/include/semaphore.h>

#define BUFF_SIZE 5     /* total number of slots */
#define NP 1            /* total number of producers */
#define NCP1 5          /* total number of CP1*/
#define NCP2 4          /* total number of CP2*/
#define NCP3 3          /* total number of CP3*/
#define NC 1            /* total number of consumers */
#define NITERS 50       /* number of items produced/consumed */
#define MATRIX_ORDER 10 /*order of A, B and C*/

#define MAX_STRING_SIZE 100

typedef struct
{
    char nome[MAX_STRING_SIZE];
    double A[MATRIX_ORDER][MATRIX_ORDER];
    double B[MATRIX_ORDER][MATRIX_ORDER];
    double C[MATRIX_ORDER][MATRIX_ORDER];
    double V[MATRIX_ORDER];
    double E;
} S;

typedef struct
{
    S *buf[BUFF_SIZE]; /* shared var */

    sem_t full;  /* keep track of the number of full spots */
    sem_t empty; /* keep track of the number of empty spots */
    sem_t mutex; /* enforce mutual exclusion to shared data */

    int in;  /* buf[in%BUFF_SIZE] is the first empty slot */
    int out; /* buf[out%BUFF_SIZE] is the first full slot */
} sbuf_t;

void imprimeRecurso(S *recurso, FILE *saida);

sbuf_t shared[4];

void *Producer(void *arg)
{
    // Open list of inputs
    FILE *input_file = fopen("entrada.in", "r");
    if (input_file == NULL)
    {
        printf("Error: Producer unable to load input file\n");
        fflush(stdout);
        return NULL;
    }

    char line[MAX_STRING_SIZE];
    int i = 0;

    // Iterate over each entry
    for (i = 0; i < NITERS; i++)
    {
        fgets(line, MAX_STRING_SIZE, input_file);
        S *data = malloc(sizeof(S));

        // Set the name field in data to filename
        for (int j = 0; j < MAX_STRING_SIZE; j++)
        {
            data->nome[j] = line[j];
        }

        // Open input file
        strtok(line, "\n");
        FILE *file = fopen(line, "r");
        if (file == NULL)
        {
            printf("Error: Producer undable to load file %d\n", i);
            fflush(stdout);
            return NULL;
        }

        // Load matrix A
        for (int row = 0; row < MATRIX_ORDER; row++)
        {
            for (int column = 0; column < MATRIX_ORDER; column++)
            {
                fscanf(file, "%lf", &data->A[row][column]);
            }
        }

        // Load matrix B
        for (int row = 0; row < MATRIX_ORDER; row++)
        {
            for (int column = 0; column < MATRIX_ORDER; column++)
            {
                fscanf(file, "%lf", &data->B[row][column]);
            }
        }

        sem_wait(&shared[0].empty);

        sem_wait(&shared[0].mutex);

        shared[0].buf[shared[0].in] = data;
        printf("Produzido recurso: %s", shared[0].buf[shared[0].in]->nome);
        fflush(stdout);
        shared[0].in = (shared[0].in + 1) % BUFF_SIZE;

        sem_post(&shared[0].mutex);
        sem_post(&shared[0].full);
    }

    return NULL;
}

void *ConsumerProducer1(void *arg)
{
    int i;

    S *item;
    for (i = 0; i < NITERS; i++)
    {

        /* Prepare to read item from buf */

        /* If there are no filled slots, wait */
        sem_wait(&shared[0].full);
        /* If another thread uses the buffer, wait */
        sem_wait(&shared[0].mutex);
        item = shared[0].buf[shared[0].out];
        shared[0].out = (shared[0].out + 1) % BUFF_SIZE;
        /* Release the buffer */
        sem_post(&shared[0].mutex);
        /* Increment the number of empty slots */
        sem_post(&shared[0].empty);

        // Multiplication matrix into C
        for (int row = 0; row < MATRIX_ORDER; row++)
        {
            for (int column = 0; column < MATRIX_ORDER; column++)
            {
                item->C[row][column] = 0;
                for (int k = 0; k < MATRIX_ORDER; k++)
                {
                    item->C[row][column] += item->A[row][k] * item->B[k][column];
                }
            }
        }

        /* Prepare to write item to buf */

        /* If there are no empty slots, wait */
        sem_wait(&shared[1].empty);
        /* If another thread uses the buffer, wait */
        sem_wait(&shared[1].mutex);
        shared[1].buf[shared[1].in] = item;
        shared[1].in = (shared[1].in + 1) % BUFF_SIZE;
        /* Release the buffer */
        sem_post(&shared[1].mutex);
        /* Increment the number of full slots */
        sem_post(&shared[1].full);
    }
    return NULL;
}

void *ConsumerProducer2(void *arg)
{
    int i;

    S *item;
    for (i = 0; i < NITERS; i++)
    {

        /* Prepare to read item from buf */

        /* If there are no filled slots, wait */
        sem_wait(&shared[1].full);
        /* If another thread uses the buffer, wait */
        sem_wait(&shared[1].mutex);
        item = shared[1].buf[shared[1].out];
        shared[1].out = (shared[1].out + 1) % BUFF_SIZE;
        /* Release the buffer */
        sem_post(&shared[1].mutex);
        /* Increment the number of empty slots */
        sem_post(&shared[1].empty);

        // Sum columns into each element of V
        for (int column = 0; column < MATRIX_ORDER; column++)
        {
            item->V[column] = 0;
            for (int row = 0; row < MATRIX_ORDER; row++)
            {
                item->V[column] += item->C[row][column];
            }
        }

        /* Prepare to write item to buf */

        /* If there are no empty slots, wait */
        sem_wait(&shared[2].empty);
        /* If another thread uses the buffer, wait */
        sem_wait(&shared[2].mutex);
        shared[2].buf[shared[2].in] = item;
        shared[2].in = (shared[2].in + 1) % BUFF_SIZE;
        /* Release the buffer */
        sem_post(&shared[2].mutex);
        /* Increment the number of full slots */
        sem_post(&shared[2].full);
    }
    return NULL;
}

void *ConsumerProducer3(void *arg)
{
    int i;

    S *item;
    for (i = 0; i < NITERS; i++)
    {

        /* Prepare to read item from buf */

        /* If there are no filled slots, wait */
        sem_wait(&shared[2].full);
        /* If another thread uses the buffer, wait */
        sem_wait(&shared[2].mutex);
        item = shared[2].buf[shared[2].out];
        shared[2].out = (shared[2].out + 1) % BUFF_SIZE;
        /* Release the buffer */
        sem_post(&shared[2].mutex);
        /* Increment the number of empty slots */
        sem_post(&shared[2].empty);

        // Sum elements of V into E
        item->E = 0;
        for (int contador = 0; contador < MATRIX_ORDER; contador++)
        {
            item->E += item->V[contador];
        }

        /* Prepare to write item to buf */

        /* If there are no empty slots, wait */
        sem_wait(&shared[3].empty);
        /* If another thread uses the buffer, wait */
        sem_wait(&shared[3].mutex);
        shared[3].buf[shared[3].in] = item;
        shared[3].in = (shared[3].in + 1) % BUFF_SIZE;
        /* Release the buffer */
        sem_post(&shared[3].mutex);
        /* Increment the number of full slots */
        sem_post(&shared[3].full);
    }
    return NULL;
}

void *Consumer(void *arg)
{
    int i;
    S *item;
    FILE *saida = fopen("saida.out", "w");

    for (i = 0; i < NITERS; i++)
    {

        /* Prepare to read item from buf */

        /* If there are no filled slots, wait */
        sem_wait(&shared[3].full);
        /* If another thread uses the buffer, wait */
        sem_wait(&shared[3].mutex);
        item = shared[3].buf[shared[3].out];
        shared[3].out = (shared[3].out + 1) % BUFF_SIZE;
        /* Release the buffer */
        sem_post(&shared[3].mutex);
        /* Increment the number of empty slots */
        sem_post(&shared[3].empty);

        imprimeRecurso(item, saida);
        fflush(saida);
    }

    fclose(saida);
    return NULL;
}

void imprimeRecurso(S *recurso, FILE *saida)
{
    char delimeter[41] = "=======================================\n";
    char spacer[41] = "---------------------------------------";
    if (saida == NULL)
    {
        printf("Erro ao abrir arquivo de saída!\n");
        return;
    }

    fprintf(saida, "%s\n", delimeter);
    fprintf(saida, "Nome: %s", recurso->nome);
    fprintf(saida, "%s\n", spacer);
    fprintf(saida, "Matriz A:\n");
    for (int i = 0; i < MATRIX_ORDER; i++)
    {
        for (int j = 0; j < MATRIX_ORDER; j++)
        {
            fprintf(saida, "%f ", recurso->A[i][j]);
        }
        fprintf(saida, "\n");
    }
    fprintf(saida, "%s\n", spacer);

    fprintf(saida, "Matriz B:\n");
    for (int i = 0; i < MATRIX_ORDER; i++)
    {
        for (int j = 0; j < MATRIX_ORDER; j++)
        {
            fprintf(saida, "%f ", recurso->B[i][j]);
        }
        fprintf(saida, "\n");
    }
    fprintf(saida, "%s\n", spacer);

    fprintf(saida, "Matriz C:\n");
    for (int i = 0; i < MATRIX_ORDER; i++)
    {
        for (int j = 0; j < MATRIX_ORDER; j++)
        {
            fprintf(saida, "%f ", recurso->C[i][j]);
        }
        fprintf(saida, "\n");
    }
    fprintf(saida, "%s\n", spacer);

    fprintf(saida, "Vetor V:\n");
    for (int i = 0; i < MATRIX_ORDER; i++)
    {
        fprintf(saida, "%f\n", recurso->V[i]);
    }
    fprintf(saida, "%s\n", spacer);

    fprintf(saida, "Valor E: %f\n", recurso->E);
}

int main()
{
    printf("Programa Inicializado\n");
    fflush(stdout);

    pthread_t idP[NP], idCP1[NCP1], idCP2[NCP2], idCP3[NCP3], idC[NC];
    int index;
    int sP[NP], sCP1[NCP1], sCP2[NCP2], sCP3[NCP3], sC[NC];

    printf("Inicializando semaforos\n");
    fflush(stdout);
    for (index = 0; index < 4; index++)
    {
        sem_init(&shared[index].full, 0, 0);
        sem_init(&shared[index].empty, 0, BUFF_SIZE);
        sem_init(&shared[index].mutex, 0, 1);
    }

    printf("Inicializando Produtores\n");
    fflush(stdout);
    for (index = 0; index < NP; index++)
    {
        sP[index] = index;
        /* Create a new producer */
        pthread_create(&idP[index], NULL, Producer, &sP[index]);
    }

    printf("Inicializando CP1\n");
    fflush(stdout);
    for (index = 0; index < NCP1; index++)
    {
        sCP1[index] = index;
        /* Create a new producer-consumer */
        pthread_create(&idCP1[index], NULL, ConsumerProducer1, &sCP1[index]);
    }

    printf("Inicializando CP2\n");
    fflush(stdout);
    for (index = 0; index < NCP2; index++)
    {
        sCP2[index] = index;
        /* Create a new producer-consumer */
        pthread_create(&idCP2[index], NULL, ConsumerProducer2, &sCP2[index]);
    }

    printf("Inicializando CP3\n");
    fflush(stdout);
    for (index = 0; index < NCP3; index++)
    {
        sCP1[index] = index;
        /* Create a new producer-consumer */
        pthread_create(&idCP3[index], NULL, ConsumerProducer3, &sCP3[index]);
    }

    printf("Inicializando Consumidor\n");
    fflush(stdout);
    for (index = 0; index < NC; index++)
    {
        sC[index] = index;
        /* Create a new consumer */
        pthread_create(&idC[index], NULL, Consumer, &sC[index]);
        pthread_join(idC[index], NULL);
    }

    for (index = 0; index < NCP1; index++)
    {
        pthread_cancel(idCP1[index]);
    }

    for (index = 0; index < NCP2; index++)
    {
        pthread_cancel(idCP2[index]);
    }

    for (index = 0; index < NCP3; index++)
        for (index = 0; index < NCP3; index++)
        {
            pthread_cancel(idCP3[index]);
            pthread_cancel(idCP3[index]);
        }

    pthread_exit(NULL);
}
