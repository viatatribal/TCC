/***************************************************
Teste de velocidade de leitura do MCP3008 usando SPI
Prof. Rainer Zanghi, D.Sc.
Universidade Federal Fluminense

Adaptado do exemplo simpletest de
Author: Carter Nelson
License: Public Domain
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
// um grupo de samplecount * maxchannel medições e o valor rms
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
float rmsvalue = 0.0;
int avgvalue = 478;
int maxvalue = 0;
int minvalue = 1023;

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
  // inicia o wiringPi e o mcp3008
  wiringPiSetup();
  mcp3004Setup(MY_PIN, SPI_CHAN);

  for (;;) {
    //registra o timestamp antes da leitura do primeiro canal
    timestamp[count] = getTimeNano();
    //lê todos os canais do MCP3008 e armazena na matriz
    for (int i = 0; i < MAXCHANNEL; i++) {
        value[i][count] = k * (float)analogRead(MY_PIN + i);
    }
    //atualiza valor máximo e mínimo do canal 0
    if (value[0][count]>maxvalue) maxvalue = value[0][count];
    if (value[0][count]<minvalue) minvalue = value[0][count];
    //calcula o somatorio dos quadrados do primeiro canal
    rmsvalue += (value[0][count]-avgvalue) * (value[0][count]-avgvalue);
    //só totaliza o tempo de amostragem a partir da segunda medição
    if (count>0) {
        avgsampletime += (timestamp[count]-timestamp[count-1]);
    }
    else
        avgsampletime = 0.0;
    //prepara para nova amostra
    count++;
    //caso tenha chegado ao final da matriz
    if (count==SAMPLECOUNT) {
        //retorna ao início da matriz para sobrescrever os dados
        count = 0;
        //calcula o tempo médio
        avgsampletime = avgsampletime / ((SAMPLECOUNT-1)*MAXCHANNEL);
        //calcula o rms do primeiro canal considerando todas as amostras
        rmsvalue = sqrt(rmsvalue/SAMPLECOUNT);
        //atualiza valor médio para o valor rms do canal 0
        avgvalue = minvalue+((maxvalue-minvalue)/2);
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
        //as estatísticas (max, min, avg) dos valores lidos do canal 0 e
        //o valor rms do canal 0.
        #ifdef DEBUG2
        printf("Avg time: %2.2f\n", avgsampletime);
        printf("Min: %i\n", minvalue);
        printf("Max: %i\n", maxvalue);
        printf("Avg value: %i\n", avgvalue);
        printf("RMS: %2.2f\n", rmsvalue);
        #endif
        //reinicia rms, min, max para cálculo com novos valores
        rmsvalue = 0.0;
        minvalue = 1023;
        maxvalue = 0;
    }
  }
}
