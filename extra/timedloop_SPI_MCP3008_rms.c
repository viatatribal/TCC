/***************************************************
Leitura do MCP3008 usando SPI
No formato polling ou timed loop
sem interrupção
Prof. Rainer Zanghi, D.Sc.
Universidade Federal Fluminense
****************************************************/

// bibliotecas principais do C
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <inttypes.h>
#include <math.h>


//biblioteca para acesso do MCP3008 via SPI
#include <wiringPi.h>
#include <mcp3004.h>

// Relacionados ao acesso ao SPI
#define SPI_CHAN 1
#define MY_PIN 12345

// Define DEBUG para imprimir valores e timestamp de
// um grupo de samplecount * maxchannel medições
//#define DEBUG 1

// Define DEBUG2 para imprimir o intervalo médio entre amostras de
// um grupo de samplecount * maxchannel medições,
// as estatísticas das medições e seu valor rms
#define DEBUG2 1

//determina número de amostras do buffer
#define SAMPLECOUNT 2048
//define número de canais lidos do MCP3008
#define MAXCHANNEL 3
//cria matriz de dados de tempo e medições
uint64_t timestamp[SAMPLECOUNT];
int value[MAXCHANNEL][SAMPLECOUNT];
//estatísticas e cálculos das medições
unsigned int count = 0;
float avgsampletime=0.0;
float rmsvalue[MAXCHANNEL];
int avgvalue[MAXCHANNEL];
int maxvalue[MAXCHANNEL];
int minvalue[MAXCHANNEL];

//tempos em us
const uint64_t samplepolltime = 100;
uint64_t timestampbefore = 0;
uint64_t timestampnow = 0;

// função para pegar tempo atual em nanosegundos
uint64_t
getTimeNano()
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    return (uint64_t)now.tv_sec * 1000000000UL + (uint64_t)now.tv_nsec;
}

int main() {

    float k = 3.3/1024.0;

    //inicializando variáveis nos arrays
    for (int chan=0; chan<MAXCHANNEL; chan++) {
        rmsvalue[chan] = 0.0;
        avgvalue[chan] = 478;
        maxvalue[chan] = 0;
        minvalue[chan] = 1023;
    }

    // inicia o wiringPi e o mcp3008
    wiringPiSetup();
    mcp3004Setup(MY_PIN, SPI_CHAN);

    timestampbefore = getTimeNano();

    for (;;) {
        //atualiza tempo atual
        timestampnow = getTimeNano();
        //verifica se já pode realizar a leitura
        if ((timestampnow-timestampbefore)>samplepolltime) {
            //salva tempo atual para próxima leitura
            timestampbefore = timestampnow;
            //registra o timestamp antes da leitura do primeiro canal
            timestamp[count] = timestampnow;
            //lê todos os canais do MCP3008 e armazena na matriz
            for (int chan=0;chan<MAXCHANNEL;chan++) {
                value[chan][count] = k * (float)analogRead(MY_PIN + chan);
            }
            //atualiza valor máximo e mínimo de todos os canais
            for (int chan=0; chan<MAXCHANNEL; chan++) {

                if (value[chan][count]>maxvalue[chan])
                    maxvalue[chan] = value[chan][count];
                if (value[chan][count]<minvalue[chan])
                    minvalue[chan] = value[chan][count];
                //calcula o somatorio dos quadrados de todos os canais
                rmsvalue[chan] += (value[chan][count]-avgvalue[chan]) * (value[chan][count]-avgvalue[chan]);
            }

            //só totaliza o tempo de amostragem a partir da segunda medição
            if (count>0) {
                avgsampletime += (timestamp[count]-timestamp[count-1]);
            }
            else
                avgsampletime = 0.0;

            //prepara para nova amostra
            count++;
            //caso tenha chegado ao final da matriz de amostras
            if (count==SAMPLECOUNT) {
            //retorna ao início da matriz para sobrescrever os dados
            count = 0;
            //calcula o tempo médio
            avgsampletime = avgsampletime / (SAMPLECOUNT-1);
            for (int chan=0; chan<MAXCHANNEL; chan++) {
                    //calcula o rms de todos os canais considerando todas as amostras
                    rmsvalue[chan] = sqrt(rmsvalue[chan]/SAMPLECOUNT);
                    //atualiza valor médio para o valor rms de todos os canais
                    avgvalue[chan] = minvalue[chan] + ((maxvalue[chan]-minvalue[chan])/2);
                }
                /***************************************
                *     ATUALIZAÇÃO DA BASE DE DADOS     *
                ***************************************/
                //imprime as últimas leituras de todos os canais e seu timestamp inicial
                #ifdef DEBUG
                    for (int chan=0; chan<MAXCHANNEL; chan++) {
                        printf("%i\n", value[chan][SAMPLECOUNT-1]);
                    }
                    printf("%" PRIu64 "\n", timestamp[SAMPLECOUNT-1]);
                #endif
                //imprime o tempo médio entre amostras de cada canal,
                //as estatísticas (max, min, avg) dos valores lidos de cada canal e
                //o valor rms de cada canal.
                #ifdef DEBUG2
                    printf("Avg time: %2.2f\n", avgsampletime);
                    for (int chan=0; chan<MAXCHANNEL; chan++) {
                        printf("Min: %i ", minvalue[chan]);
                        printf("Max: %i ", maxvalue[chan]);
                        printf("Avg value: %i ", avgvalue[chan]);
                        printf("RMS: %2.2f \n", rmsvalue[chan]);
                    }
                    printf("\n");
                #endif

                //reinicia rms, min, max para cálculo com novos valores
                for (int chan=0; chan<MAXCHANNEL; chan++) {
                    rmsvalue[chan] = 0.0;
                    minvalue[chan] = 1023;
                    maxvalue[chan] = 0;
                }
            }
        }
    }
}
