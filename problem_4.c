#include <stdio.h>
#include <pthread.h>  // Biblioteca para suporte a threads em C.
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>  // Biblioteca para manipulação de tempo.
#include <Windows.h>

#define NUM_CARROS 120 // Define o número de carros na simulação.
#define NUM_CAMINHOES 10 // Define o número de caminhões na simulação.
#define MAX_TEMPO_CHEGADA 3 // Define o tempo máximo de chegada dos veículos.
#define TEMPO_TRAVESSIA_CARRO 5 // Define o tempo de travessia de um carro.
#define TEMPO_TRAVESSIA_CAMINHAO 10 // Define o tempo de travessia de um caminhão.
#define TEMPO_TRAVESSIA_CARRO_ANTIGA 4 // Define o tempo de travessia de um carro na ponte antiga.
#define TEMPO_ESPACO 1 // Define o intervalo entre os veículos.
#define TEMPO_ESPACO_ANTIGA 2 // Define o intervalo entre os veículos na ponte antiga.
#define MAX_CARROS_PONTE 5 // Define o número máximo de carros que podem estar na ponte ao mesmo tempo.
#define MAX_CARROS_PONTE_ANTIGA 2 // Define o número máximo de carros que podem estar na ponte antiga ao mesmo tempo.
#define MAX_CAMINHAO_PONTE 1 // Define o número máximo de caminhões que podem estar na ponte ao mesmo tempo.

// Inicialização de mutex e variáveis de condição para controle de concorrência.
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_carro[2] = {PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER};
pthread_cond_t cond_caminhao[2] = {PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER};
pthread_cond_t cond_carro_antiga[2] = {PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER};

// Declaração de variáveis globais para controlar o estado da ponte e coletar estatísticas.
int carros_na_ponte = 0;
int caminhoes_na_ponte = 0;
int carros_na_ponte_antiga = 0;
int direcao_na_ponte = -1; // -1: ninguém na ponte, 0: direção 0, 1: direção 1
int direcao_na_ponte_antiga = -1; // -1: ninguém na ponte, 0: direção 0, 1: direção 1

// Variáveis para rastrear a quantidade de veículos antes de trocar o sentido.
int contador_carros_mesmo_sentido = 0;
int contador_caminhoes_mesmo_sentido = 0;
int contador_carros_mesmo_sentido_antiga = 0;

// Estatísticas de carros.
int tempo_max_espera_carros = 0;
int tempo_min_espera_carros = INT_MAX;
double tempo_total_espera_carros = 0;
int carros_por_direcao[2] = {0, 0};

// Estatísticas de caminhões.
int tempo_max_espera_caminhoes = 0;
int tempo_min_espera_caminhoes = INT_MAX;
double tempo_total_espera_caminhoes = 0;
int caminhoes_por_direcao[2] = {0, 0};

// Estatísticas da ponte.
int tempo_total_execucao;
int tempo_total_ocupado = 0;
int ponte_ocupada = -1;

// Estatísticas da ponte antiga.
int tempo_total_ocupado_antiga = 0;
int ponte_antiga_ocupada = -1;

// Função que será executada por cada thread (carro).
void *carro(void *arg) {
    int id = *((int *)arg);
    int minha_direcao = id % 2; // A direção é determinada pelo id do carro.

    // O carro espera um tempo aleatório antes de tentar atravessar a ponte.
    sleep(rand() % MAX_TEMPO_CHEGADA + 1);

    pthread_mutex_lock(&mutex); // Adquire o lock antes de manipular os recursos compartilhados.

    int inicio_espera = time(NULL); // Registra o tempo de início da espera.

    // Tentar a ponte principal.
    while (direcao_na_ponte != -1 && (direcao_na_ponte != minha_direcao || carros_na_ponte == MAX_CARROS_PONTE || contador_carros_mesmo_sentido == MAX_CARROS_PONTE)) {
        // Se não for possível usar a ponte principal, tentar a ponte antiga.
        if (direcao_na_ponte_antiga == -1 || direcao_na_ponte_antiga == minha_direcao && carros_na_ponte_antiga < MAX_CARROS_PONTE_ANTIGA && contador_carros_mesmo_sentido_antiga < MAX_CARROS_PONTE_ANTIGA) {
            if (direcao_na_ponte_antiga == -1 || contador_carros_mesmo_sentido_antiga == MAX_CARROS_PONTE_ANTIGA) {
                direcao_na_ponte_antiga = minha_direcao;
                contador_carros_mesmo_sentido_antiga = 0;
                if (ponte_antiga_ocupada == -1) {
                    ponte_antiga_ocupada = time(NULL); // Registra o tempo em que a ponte antiga começou a ser ocupada.
                }
            }
            // Atualiza o estado da ponte antiga.
            carros_na_ponte_antiga++;
            contador_carros_mesmo_sentido_antiga++;
            carros_por_direcao[minha_direcao]++;

            printf("Carro %d está atravessando na direção %d na ponte antiga\n", id, minha_direcao); // Imprime que o carro começou a atravessar a ponte antiga.

            sleep(TEMPO_ESPACO_ANTIGA);  // Adicionado espaço entre carros na ponte antiga.

            pthread_mutex_unlock(&mutex); // Libera o lock após manipular os recursos compartilhados.

            // Simula o tempo que o carro leva para atravessar a ponte antiga.
            sleep(TEMPO_TRAVESSIA_CARRO_ANTIGA);

            pthread_mutex_lock(&mutex); // Adquire o lock novamente para atualizar o estado da ponte.

            // Imprime que o carro terminou de atravessar a ponte antiga.
            printf("Carro %d terminou de atravessar na direção %d na ponte antiga\n", id, minha_direcao);

            carros_na_ponte_antiga--; // Decrementa o número de carros na ponte antiga.

            // Caso a ponte antiga esteja vazia, notifica os carros.
            if (carros_na_ponte_antiga == 0) {
                direcao_na_ponte_antiga = -1;
                tempo_total_ocupado_antiga += time(NULL) - ponte_antiga_ocupada;
                ponte_antiga_ocupada = -1;
                pthread_cond_broadcast(&cond_carro_antiga[!minha_direcao]);
                pthread_cond_broadcast(&cond_carro[!minha_direcao]);
            } else {
                pthread_cond_signal(&cond_carro_antiga[minha_direcao]);
                pthread_cond_signal(&cond_carro[minha_direcao]);
            }

            // Libera o lock e a memória alocada para o carro que atravessou na ponte antiga.
            pthread_mutex_unlock(&mutex);
            free(arg);
            return NULL;
        } else {
            pthread_cond_wait(&cond_carro[minha_direcao], &mutex); // Caso não consiga passar na antiga, aguarda na principal.
        }
    }


    int fim_espera = time(NULL); // Registra o tempo de fim da espera.
    int tempo_espera = fim_espera - inicio_espera; // Calcula o tempo total de espera.

    // Atualiza as estatísticas de tempo de espera.
    tempo_min_espera_carros = tempo_espera < tempo_min_espera_carros ? tempo_espera : tempo_min_espera_carros;
    tempo_max_espera_carros = tempo_espera > tempo_max_espera_carros ? tempo_espera : tempo_max_espera_carros;
    tempo_total_espera_carros += tempo_espera;

    // Se 5 carros já passaram e a ponte estiver vazia, define a direção do tráfego.
    if (direcao_na_ponte == -1 || contador_carros_mesmo_sentido == MAX_CARROS_PONTE) {
        direcao_na_ponte = minha_direcao;
        contador_carros_mesmo_sentido = 0;
        if (ponte_ocupada == -1) {
            ponte_ocupada = time(NULL); // Registra o tempo em que a ponte começou a ser ocupada.
        }
    }

    // Atualiza o estado da ponte.
    carros_na_ponte++;
    contador_carros_mesmo_sentido++;
    carros_por_direcao[minha_direcao]++;

    printf("Carro %d está atravessando na direção %d na ponte principal\n", id, minha_direcao); // Imprime que o carro começou a atravessar a ponte.

    sleep(TEMPO_ESPACO);  // Espera um intervalo antes de permitir o próximo carro na ponte principal.

    pthread_mutex_unlock(&mutex); // Libera o lock após manipular os recursos compartilhados.

    // Simula o tempo que o carro leva para atravessar a ponte principal.
    sleep(TEMPO_TRAVESSIA_CARRO);

    pthread_mutex_lock(&mutex); // Adquire o lock novamente para atualizar o estado da ponte.

    // Imprime que o carro terminou de atravessar a ponte principal.
    printf("Carro %d terminou de atravessar na direção %d na ponte principal\n", id, minha_direcao);

    carros_na_ponte--; // Decrementa o número de carros na ponte principal.

    // Se a ponte estiver vazia, notifica veículos da direção oposta. Caso contrário, notifica veículos da mesma direção.
    if (carros_na_ponte == 0 && caminhoes_na_ponte == 0) {
        direcao_na_ponte = -1;
        tempo_total_ocupado += time(NULL) - ponte_ocupada;
        ponte_ocupada = -1;
        pthread_cond_broadcast(&cond_carro[!minha_direcao]);
        pthread_cond_broadcast(&cond_caminhao[!minha_direcao]);
    } else {
        pthread_cond_signal(&cond_carro[minha_direcao]);
        pthread_cond_signal(&cond_caminhao[minha_direcao]);
    }

    // Libera o lock e a memória alocada para o id do carro.
    pthread_mutex_unlock(&mutex);
    free(arg);

    return NULL;
}

// Função que será executada por cada thread (caminhão).
void *caminhao(void *arg) {
    int id = *((int *)arg);
    int minha_direcao = id % 2; // A direção é determinada pelo id do caminhão.

    // O caminhão espera um tempo aleatório antes de tentar atravessar a ponte.
    sleep(rand() % MAX_TEMPO_CHEGADA + 1);

    pthread_mutex_lock(&mutex); // Adquire o lock antes de manipular os recursos compartilhados.

    int inicio_espera = time(NULL); // Registra o tempo de início da espera.

    // Espera até que a ponte esteja vazia, ou a ponte esteja na mesma direção e haja espaço suficiente na ponte, ou até que 5 carros ou 1 caminhão atravessem.
    while (direcao_na_ponte != -1 && (direcao_na_ponte != minha_direcao || carros_na_ponte > 0 || caminhoes_na_ponte == MAX_CAMINHAO_PONTE || contador_caminhoes_mesmo_sentido == MAX_CAMINHAO_PONTE)) {
        pthread_cond_wait(&cond_caminhao[minha_direcao], &mutex);
    }

    int fim_espera = time(NULL); // Registra o tempo de fim da espera.
    int tempo_espera = fim_espera - inicio_espera; // Calcula o tempo total de espera.

    // Atualiza as estatísticas de tempo de espera.
    tempo_min_espera_caminhoes = tempo_espera < tempo_min_espera_caminhoes ? tempo_espera : tempo_min_espera_caminhoes;
    tempo_max_espera_caminhoes = tempo_espera > tempo_max_espera_caminhoes ? tempo_espera : tempo_max_espera_caminhoes;
    tempo_total_espera_caminhoes += tempo_espera;

    //Se 1 caminhão passou e a ponte estiver vazia, define a direção do tráfego.
    if (direcao_na_ponte == -1 || contador_caminhoes_mesmo_sentido == MAX_CAMINHAO_PONTE) {
        direcao_na_ponte = minha_direcao;
        contador_caminhoes_mesmo_sentido = 0;
        if (ponte_ocupada == -1) {
            ponte_ocupada = time(NULL); // Registra o tempo em que a ponte começou a ser ocupada.
        }
    }

    // Atualiza o estado da ponte.
    caminhoes_na_ponte++;
    contador_caminhoes_mesmo_sentido++;
    caminhoes_por_direcao[minha_direcao]++;

    printf("Caminhao %d está atravessando na direção %d\n", id, minha_direcao); // Imprime que o caminhão começou a atravessar a ponte.

    sleep(TEMPO_ESPACO); // Espera um intervalo antes de permitir o próximo caminhão.

    pthread_mutex_unlock(&mutex); // Libera o lock após manipular os recursos compartilhados.

    // Simula o tempo que o caminhão leva para atravessar a ponte.
    sleep(TEMPO_TRAVESSIA_CAMINHAO);

    pthread_mutex_lock(&mutex); // Adquire o lock novamente para atualizar o estado da ponte.

    // Imprime que o caminhão terminou de atravessar a ponte.
    printf("Caminhao %d terminou de atravessar na direção %d\n", id, minha_direcao);

    caminhoes_na_ponte--; // Decrementa o número de caminhões na ponte.

    // Se a ponte estiver vazia, notifica veículos da direção oposta. Caso contrário, notifica veículos da mesma direção.
    if (carros_na_ponte == 0 && caminhoes_na_ponte == 0) {
        direcao_na_ponte = -1;
        tempo_total_ocupado += time(NULL) - ponte_ocupada; // Atualiza o tempo total que a ponte principal foi ocupada.
        ponte_ocupada = -1;
        pthread_cond_broadcast(&cond_carro[!minha_direcao]);
        pthread_cond_broadcast(&cond_caminhao[!minha_direcao]);
    } else {
        pthread_cond_signal(&cond_caminhao[minha_direcao]);
        pthread_cond_signal(&cond_carro[minha_direcao]);
    }

    // Libera o lock e a memória alocada para o id do caminhão.
    pthread_mutex_unlock(&mutex);
    free(arg);

    return NULL;
}

// Função principal que inicializa e gerencia a simulação.
int main() {
    SetConsoleOutputCP(CP_UTF8); // Define o conjunto de caracteres de saída do console para UTF-8.
    pthread_t threads_carros[NUM_CARROS]; // Declara um array de threads para cada carro.
    pthread_t threads_caminhoes[NUM_CAMINHOES]; // Declara um array de threads para cada caminhão.

    int inicio_execucao = time(NULL); // Registra o tempo de início da execução.

    // Cria as threads de carros.
    for (int i = 0; i < NUM_CARROS; i++) {
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&threads_carros[i], NULL, carro, id);
    }

    // Cria as threads de caminhões.
    for (int i = 0; i < NUM_CAMINHOES; i++) {
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&threads_caminhoes[i], NULL, caminhao, id);
    }

    // Espera todas as threads dos veículos terminarem.
    for (int i = 0; i < NUM_CARROS; i++) {
        pthread_join(threads_carros[i], NULL);
    }
    for (int i = 0; i < NUM_CAMINHOES; i++) {
        pthread_join(threads_caminhoes[i], NULL);
    }

    int fim_execucao = time(NULL); // Registra o tempo de fim da execução.

    // Calcula o tempo total de execução.
    tempo_total_execucao = fim_execucao - inicio_execucao;

    // Imprime as estatísticas coletadas.
    printf("\n----------------------------------------------------------------");
    printf("\n|                      Estatísticas                            |\n");
    printf("|                                                              |");
    printf("\n| 🚗 Quantidade de Carros que cruzaram cada sentido: %d, %d    |\n", carros_por_direcao[0], carros_por_direcao[1]);
    printf("| 🚗 Tempo mínimo de espera para Carros: %d s                   |\n", tempo_min_espera_carros);
    printf("| 🚗 Tempo máximo de espera para Carros: %d s                 |\n", tempo_max_espera_carros);
    printf("| 🚗 Tempo médio de espera para Carros: %0.2f s                |\n", tempo_total_espera_carros / NUM_CARROS);
    printf("|                                                              |");
    printf("\n| 🚚 Quantidade de Caminhões que cruzaram cada sentido: %d, %d   |\n", caminhoes_por_direcao[0], caminhoes_por_direcao[1]);
    printf("| 🚚 Tempo mínimo de espera para Caminhões: %02d s               |\n", tempo_min_espera_caminhoes);
    printf("| 🚚 Tempo máximo de espera para Caminhões: %d s              |\n", tempo_max_espera_caminhoes);
    printf("| 🚚 Tempo médio de espera para Caminhões: %0.2f s            |\n", tempo_total_espera_caminhoes / NUM_CAMINHOES);
    printf("|                                                              |");
    printf("\n| ⌚ Eficiência da ponte: %0.2f%%                               |\n", ((double)tempo_total_ocupado / tempo_total_execucao) * 100);
    printf("| ⌛ Eficiência da ponte antiga: %0.2f%%                        |\n", ((double)tempo_total_ocupado_antiga / tempo_total_execucao) * 100);
    printf("|                                                              |\n");
    printf("----------------------------------------------------------------\n");

    return 0;
}
