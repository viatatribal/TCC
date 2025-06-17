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
#define NANOTOSEC(A)            (A)/1000000000.0


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

// global variables for test1
float Irms_pickup[3] = {0};
float Irms_final[3]= {0};
struct timespec pickup_event_time[3];
struct timespec activated_time[3];


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
    if (isPickUpEvent[i] && Ip > Irms[i]) {
        isPickUpEvent[i] = FALSE;
        return;
    }

    if (Irms[i] >= Imax) {
        is50[i] = TRUE;
        return;
    }

    if (!isPickUpEvent[i] && Irms[i] >= Ip) {
        Irms_pickup[i] = Irms[i];
        clock_gettime(CLOCK_REALTIME, &pickup_event_time[i]);
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


void printTime(struct timespec now) {

    struct tm tm;
    localtime_r(&now.tv_sec, &tm);

    int milliseconds = now.tv_nsec / 1000000;
    int microseconds = (now.tv_nsec / 1000) % 1000;
    int nanoseconds  = now.tv_nsec % 1000;

    printf("%02d:%02d:%02d.%03d%03d%03d",
           tm.tm_hour, tm.tm_min, tm.tm_sec,
           milliseconds, microseconds, nanoseconds);
}


int main()
{
    wiringPiSetup();
    mcp3004Setup(MY_PIN, SPI_CHAN);

    uint64_t timestamp_before = getTimeNano();
    uint64_t timestamp_initial = timestamp_before;


    // overprotection current values
    const float A = 0.052;
    const float B = 0.114;
    const float p = 0.02;

    // variaveis para teste 2
    float amp[3] = {1.0, 1.0, 1.0};
    uint64_t T1 = 10UL * 1000000000UL; // 10 seconds
    uint64_t T2 = 20UL * 1000000000UL; // 20 seconds
    uint64_t T3 = 30UL * 1000000000UL; // 30 seconds
    uint64_t T4 = 40UL * 1000000000UL; // 40 seconds
    float total_time = 0;
    int isT1 = TRUE;
    int isT2 = TRUE;
    int isT3 = TRUE;
    int isT4 = TRUE;

    char *phases[3] = {"Fase A", "Fase B", "Fase C"};

    for (;;) {

    	uint64_t timestamp_now = getTimeNano();

    	if ((timestamp_now-timestamp_before)>sample_polltime) {

            timestamp_before = timestamp_now;
            for(int i = 0; i < PHASES; i++) {
                float v = amp[i] * (VOLTREAD(MY_PIN+i));
           	    sum_volt[i] += v*v;
            }
            sample_count++;

            if (sample_count >= AMOSTRAS) {
                for(int i = 0; i < PHASES; i++) {
              		Irms[i] = IRMS(sum_volt[i], AMOSTRAS);
                    sum_volt[i] = 0;

                    OC_protection(A, B, p, i);

                    if (is51[i]) {
                        clock_gettime(CLOCK_REALTIME, &activated_time[i]);
                        is51[i] = FALSE;
                        Irms_final[i] = Irms[i];
                        amp[i] = 1;
                        Irms[0] = Irms[1] = Irms[2] = 0;
                        sum_volt[0] = sum_volt[1] = sum_volt[2] = 0;
                        isPickUpEvent[i] = FALSE;
                        break;
                    } else if (is50[i]) {
                        break;
                    }
                }
          		sample_count = 0;

            }
        }

        if (isT1 && (timestamp_now-timestamp_initial)>T1) {
            sum_volt[0] = sum_volt[1] = sum_volt[2] = 0;
            sample_count = 0;
            amp[0] = 10;
            isT1 = FALSE;
        }

        if (isT2 && (timestamp_now-timestamp_initial)>T2) {
            sum_volt[0] = sum_volt[1] = sum_volt[2] = 0;
            sample_count = 0;
            amp[1] = 15;
            isT2 = FALSE;

        }

        if (isT3 && (timestamp_now-timestamp_initial)>T3) {
            sum_volt[0] = sum_volt[1] = sum_volt[2] = 0;
            sample_count = 0;
            amp[2] = 20;
            isT3 = FALSE;
        }

        if (isT4 && (timestamp_now-timestamp_initial)>T4) {
            total_time = NANOTOSEC(timestamp_now-timestamp_initial);
            break;
        }

    }

    printf("Teste do trip:\n\n");
    for (int i = 0; i < PHASES; i++) {
        printTime(pickup_event_time[i]);
        printf(" %f A Proteção Sobrecorrente Temporizada %s Pickup\n", Irms_pickup[i], phases[i]);
        printTime(activated_time[i]);
        printf(" %f A Proteção Sobrecorrente Temporizada %s Operada \n\n", Irms_final[i], phases[i]);
    }
    printf("Tempo total do teste: %.2f s\n", total_time);

        return 0;
}
