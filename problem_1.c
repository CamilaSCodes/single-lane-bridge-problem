#include <stdio.h>
#include <pthread.h>  // Biblioteca para suporte a threads em C.
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>  // Biblioteca para manipulação de tempo.
#include <Windows.h>

#define NUM_CARROS 100  // Define o número de carros na simulação.
#define MAX_TEMPO_CHEGADA 3  // Define o tempo máximo de chegada dos carros.
#define TEMPO_TRAVESSIA 5  // Define o tempo de travessia de um carro.
#define TEMPO_ESPACO 1  // Define o intervalo entre os carros.
#define MAX_CARROS_PONTE 5  // Define o número máximo de carros que podem estar na ponte ao mesmo tempo.

// Inicialização de mutex e variável de condição para controle de concorrência.
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// Declaração de variáveis globais para controlar o estado da ponte e coletar estatísticas.
int carros_na_ponte = 0;
int direcao_na_ponte = -1;

int carros_por_direcao[2] = {0, 0};
int tempo_max_espera = 0;
int tempo_min_espera = INT_MAX;
double tempo_total_espera = 0;
int tempo_total_execucao;
int tempo_total_ocupado = 0;
int ponte_ocupada = -1;

// Função que será executada por cada thread (carro).
void *carro(void *arg) {
    int id = *((int *)arg);
    int minha_direcao = id % 2;  // A direção é determinada pelo id do carro.

    // O carro espera um tempo aleatório antes de tentar atravessar a ponte.
    sleep(rand() % MAX_TEMPO_CHEGADA + 1);

    pthread_mutex_lock(&mutex);  // Adquire o lock antes de manipular os recursos compartilhados.

    int inicio_espera = time(NULL);  // Registra o tempo de início da espera.

    // Espera até que a ponte esteja vazia ou carros estejam indo na mesma direção e haja espaço suficiente na ponte.
    while (direcao_na_ponte != -1 && (direcao_na_ponte != minha_direcao || carros_na_ponte == MAX_CARROS_PONTE)) {
        pthread_cond_wait(&cond, &mutex);  // Aguarda até que seja notificado para prosseguir.
    }

    int fim_espera = time(NULL);  // Registra o tempo de fim da espera.
    int tempo_espera = fim_espera - inicio_espera;  // Calcula o tempo total de espera.

    // Atualiza as estatísticas de tempo de espera.
    tempo_min_espera = tempo_espera < tempo_min_espera ? tempo_espera : tempo_min_espera;
    tempo_max_espera = tempo_espera > tempo_max_espera ? tempo_espera : tempo_max_espera;
    tempo_total_espera += tempo_espera;

    // Se a ponte estiver vazia, define a direção do tráfego.
    if (direcao_na_ponte == -1) {
        direcao_na_ponte = minha_direcao;
        if (ponte_ocupada == -1) {
            ponte_ocupada = time(NULL);  // Registra o tempo em que a ponte começou a ser ocupada.
        }
    }

    // Atualiza o estado da ponte.
    carros_na_ponte++;
    carros_por_direcao[minha_direcao]++;
    printf("Carro %d está atravessando na direção %d\n", id, minha_direcao);  // Imprime que o carro começou a atravessar a ponte.

    sleep(TEMPO_ESPACO);  // Espera um intervalo antes de permitir o próximo carro.

    pthread_mutex_unlock(&mutex);  // Libera o lock após manipular os recursos compartilhados.

    // Simula o tempo que o carro leva para atravessar a ponte.
    sleep(TEMPO_TRAVESSIA);

    pthread_mutex_lock(&mutex);  // Adquire o lock novamente para atualizar o estado da ponte.

    // Imprime que o carro terminou de atravessar a ponte.
    printf("Carro %d terminou de atravessar na direção %d\n", id, minha_direcao);

    carros_na_ponte--;  // Decrementa o número de carros na ponte.

    // Se a ponte estiver vazia, atualiza o tempo total que a ponte foi ocupada.
    if (carros_na_ponte == 0) {
        direcao_na_ponte = -1;
        tempo_total_ocupado += time(NULL) - ponte_ocupada;
        ponte_ocupada = -1;
    }

    // Notifica todas as outras threads que a ponte está vazia.
    pthread_cond_broadcast(&cond);

    pthread_mutex_unlock(&mutex);  // Libera o lock.

    free(arg);  // Libera a memória alocada para o id do carro.
    return NULL;
}

// Função principal que inicializa e gerencia a simulação.
int main() {
    SetConsoleOutputCP(CP_UTF8);  // Define o conjunto de caracteres de saída do console para UTF-8.
    pthread_t threads[NUM_CARROS];  // Declara um array de threads para cada carro.

    int inicio_execucao = time(NULL);  // Registra o tempo de início da execução.

    // Cria as threads de carros.
    for (int i = 0; i < NUM_CARROS; i++) {
        int *id = malloc(sizeof(int));  // Aloca memória para o id do carro.
        *id = i;
        pthread_create(&threads[i], NULL, carro, id);  // Cria a thread.
    }

    // Espera todas as threads de carros terminarem.
    for (int i = 0; i < NUM_CARROS; i++) {
        pthread_join(threads[i], NULL);
    }

    int fim_execucao = time(NULL);  // Registra o tempo de fim da execução.

    // Calcula o tempo total de execução.
    tempo_total_execucao = fim_execucao - inicio_execucao;

    // Imprime as estatísticas coletadas.
    printf("\n-------------------------------------------------------------");
    printf("\n|                      Estatísticas                         |\n");
    printf("|                                                           |");
    printf("\n| 🚗 Quantidade de Carros que cruzaram cada sentido: %d, %d |\n", carros_por_direcao[0], carros_por_direcao[1]);
    printf("| 🚗 Tempo mínimo de espera para Carros: %d s                |\n", tempo_min_espera);
    printf("| 🚗 Tempo máximo de espera para Carros: %d s              |\n", tempo_max_espera);
    printf("| 🚗 Tempo médio de espera para Carros: %0.2f s             |\n", tempo_total_espera / NUM_CARROS);
    printf("|                                                           |");
    printf("\n| ⏰ Eficiência da ponte: %0.2f%%                            |\n", ((double)tempo_total_ocupado / tempo_total_execucao) * 100);
    printf("|                                                           |\n");
    printf("-------------------------------------------------------------\n");
    return 0;
}
