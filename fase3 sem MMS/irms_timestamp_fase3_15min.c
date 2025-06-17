#include <wiringPi.h>
#include <mcp3004.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <inttypes.h>
#include <math.h>

#define SPI_CHAN 1
#define MY_PIN 12345
#define PHASES 3

const float min_volt = 1.18;
const float max_volt = 1.90;
const float avg_volt = min_volt + ((max_volt - min_volt) / 2.0);
const float k = 3.3 / 1024.0;

const uint64_t sample_polltime = 208333;

#define AMOSTRAS 80

// MACROS
#define VOLTREAD(A)             (k * (float) analogRead((A))) - avg_volt
#define IRMS(VOLT,TOTAL)        sqrt((VOLT)/(TOTAL))
#define SECTONANO(A)            (A)*1000000000UL
#define AMP                     1

float Irms[3][54000];
uint64_t timestamp[3][4320000];

uint64_t
getTimeNano()
{
    struct timespec now;

    clock_gettime(CLOCK_MONOTONIC, &now);

    uint64_t nsTime = now.tv_sec * 1000000000UL;
    nsTime += now.tv_nsec;

    return nsTime;
}

int main()
{
    wiringPiSetup();
    mcp3004Setup(MY_PIN, SPI_CHAN);

    uint64_t timestamp_before = getTimeNano();
    uint64_t timestamp_initial = timestamp_before;
    uint64_t end_time = 900UL * 1000000000UL;
    int minutes_irms = 60*60*15;
    int minutes_timestamp = 80*60*60*15;

    int sample_count = 0;
    int time_count = 0;
    int irms_count = 0;
    float sum_volt[3] = {0,0,0};

    uint64_t times[3] = {0,0,0};
    uint64_t timestamp_difference = 0;


    for (;;) {

    	uint64_t timestamp_now = getTimeNano();

    	if ((timestamp_now-timestamp_before)>sample_polltime) {


            for(int i = 0; i < PHASES; i++) {
                float v = AMP * (VOLTREAD(MY_PIN+i));
                timestamp_difference =  getTimeNano() - timestamp_before;
                timestamp[i][time_count] = timestamp_difference;
           	    sum_volt[i] += v*v;
            }
            timestamp_before = timestamp_now;
            time_count++;
            sample_count++;

            if (sample_count >= AMOSTRAS) {
                for(int i = 0; i < PHASES; i++) {
              		Irms[i][irms_count] = IRMS(sum_volt[i], AMOSTRAS);
                    sum_volt[i] = 0;
                }

                irms_count++;
          		sample_count = 0;
            }
        }



        if ((timestamp_now-timestamp_initial)>end_time) {
            break;
        }

    }

    FILE *arquivo;
    arquivo = fopen("irms15min.csv", "w");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo!\n");
        return 1;
    }

    fprintf(arquivo, "Irms1,Irms2,Irms3,\n");
    for (int i = 0; i < minutes_irms; i++) {
        for (int j = 0; j < PHASES; j++) {
            fprintf(arquivo,"%f,", Irms[j][i]);
        }
        fprintf(arquivo, "\n");
    }

    fclose(arquivo);

    arquivo = fopen("timestamp15min.csv", "w");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo!\n");
        return 1;
    }

    fprintf(arquivo, "timestamp1,timestamp2,timestamp3,\n");
    for (int i = 0; i < minutes_timestamp; i++) {
        for (int j = 0; j < PHASES; j++) {
            fprintf(arquivo,"%" PRIu64 ",", timestamp[j][i]);
        }
        fprintf(arquivo, "\n");
    }

    fclose(arquivo);
    printf("Dados salvos com sucesso!\n");

    return 0;
}
