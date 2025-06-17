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


// global variables
int sample_count = 0;
int time_count = 0;
float Ip = 1.0;
float Imax = 50;
float sum_volt[3] = {0,0,0};
float Irms[3] = {0,0,0};

// OC Protection global variables
int is50[3] = {FALSE,FALSE, FALSE};
int is51[3] = {FALSE,FALSE, FALSE};
uint64_t t_limit[3] = {0,0,0};
int isPickUpEvent[3] = {FALSE, FALSE, FALSE};
uint64_t t_0[3] = {0,0,0};


float OC_function(float A, float B, float p, float M)
{
    return (A / (pow(M, p) - 1)) + B;
}

uint64_t
getTimeNano()
{
    struct timespec now;

    clock_gettime(CLOCK_MONOTONIC, &now);

    uint64_t nsTime = now.tv_sec * 1000000000UL;
    nsTime += now.tv_nsec;

    return nsTime;
}

void OC_protection(float A, float B, float p, int i)
{
    if ((isPickUpEvent[i] && Ip > Irms[i])) {
        isPickUpEvent[i] = FALSE;
        return;
    }

    if (Irms[i] >= Imax) {
        is50[i] = TRUE;
        return;
    }

    if (!isPickUpEvent[i] && Irms[i] >= Ip) {
        t_limit[i] = SECTONANO(OC_function(A,B,p,Irms[i]/Ip));
        t_0[i] = getTimeNano();
        isPickUpEvent[i] = TRUE;
    }

    if (isPickUpEvent[i]) {

        uint64_t timestamp_now_oc = getTimeNano();

        uint64_t t_n = SECTONANO(OC_function(A,B,p,Irms[i]/Ip));

        if (t_n < t_limit[i]) {
            t_limit[i] = t_n;
        }

        if ((timestamp_now_oc - t_0[i]) >= t_limit[i]) {
            is51[i] = TRUE;
        }
    }
}

int main()
{
    wiringPiSetup();
    mcp3004Setup(MY_PIN, SPI_CHAN);

    uint64_t timestamp_before = getTimeNano();
    uint64_t timestamp_initial = timestamp_before;
    uint64_t onehour = 3600UL * 1000000000UL;
    //uint64_t fifteenminutes = 900UL * 1000000000UL;
    //uint64_t oneminute = 60UL * 1000000000UL;

    // overprotection current values
    const float A = 0.052;
    const float B = 0.114;
    const float p = 0.02;

    // variables for time
    uint64_t timestampdifference;
    uint64_t maxInterval = 0;
    int numAmostragem = 0;
    uint64_t cutInterval = 3000000UL; // 3 ms

    for (;;) {

    	uint64_t timestamp_now = getTimeNano();

    	if ((timestamp_now-timestamp_before)>sample_polltime) {

            timestamp_before = timestamp_now;
            for(int i = 0; i < PHASES; i++) {
                float v = AMP * (VOLTREAD(MY_PIN+i));

                timestampdifference = getTimeNano() - timestamp_now;

                if (timestampdifference > cutInterval) {
                    numAmostragem++;
                    if (timestampdifference > maxInterval) {
                        maxInterval = timestampdifference;
                    }
                }

           	    sum_volt[i] += v*v;
                sample_count++;

            }

            if (sample_count >= AMOSTRAS) {
                for(int i = 0; i < 3; i++) {
              		Irms[i] = IRMS(sum_volt[i], AMOSTRAS);
                    sum_volt[i] = 0;

                    OC_protection(A, B, p, i);

                    if (is51[i]) {
                        break;
                    } else if (is50[i]) {
                        break;
                    }
                }
          		sample_count = 0;
            }
        }



        if ((timestamp_now-timestamp_initial)>onehour) {
            break;
        }

    }

    for (int i = 0; i < 3; i++) {
        if (is50[i]) {
            printf("Trip 50! Irms value: %.2f A\n", Irms[i]);
        } else if (is51[i]) {
            printf("Trip 51! Irms value: %.2f A\n", Irms[i]);
        }
    }

    printf("Numero de intervalo de amostragens maior que %d\n", numAmostragem);
    printf("Latencia máxima %" PRIu64 "\n", maxInterval);

    return 0;
}
