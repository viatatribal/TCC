#include <wiringPi.h>
#include <mcp3004.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <inttypes.h>
#include <math.h>

#define SPI_CHAN 1
#define MY_PIN 12345

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
#define AMP                     2


// global variables
int sample_count = 0;
int time_count = 0;
float Ip = 1.0;
float Imax = 50;
float sum_volt[3] = {0,0,0};
float Irms[3] = {0,0,0};

// OC Protection global variables
int is50[3] = {FALSE,FALSE,FALSE};
int is51[3] = {FALSE,FALSE,FALSE};
uint64_t t_limit[3] = {0,0,0};
int isPickUpEvent[3] = {FALSE,FALSE,FALSE};
uint64_t t_0[3] = {0,0,0};

// global variables for test1
float Irms_pickup = 0;
float Irms_final = 0;
struct timespec pickup_event_time;
struct timespec activated_time;


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
        Irms_pickup = Irms[i];
        clock_gettime(CLOCK_REALTIME, &pickup_event_time);
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
    uint64_t one_minute = 60UL * 1000000000UL;


    // overprotection current values
    const float A = 0.052;
    const float B = 0.114;
    const float p = 0.02;

    // variaveis para teste 1
    // variaveis para teste 2
    float amp = 1.0;
    uint64_t T1 = 10UL * 1000000000UL; // 10 seconds
    float total_time = 0;
    int isT1 = TRUE;

    for (;;) {

    	uint64_t timestamp_now = getTimeNano();

    	if ((timestamp_now-timestamp_before)>sample_polltime) {

            timestamp_before = timestamp_now;
            for(int i = 0; i < 1; i++) {
                float v = amp * (VOLTREAD(MY_PIN+i));
           	    sum_volt[i] += v*v;
                sample_count++;
            }

            if (sample_count >= AMOSTRAS) {
                for(int i = 0; i < 1; i++) {
              		Irms[i] = IRMS(sum_volt[i], AMOSTRAS);
                    sum_volt[i] = 0;
                }
          		sample_count = 0;

                OC_protection(A, B, p, 0);

                if (is51[0]) {
                    clock_gettime(CLOCK_REALTIME, &activated_time);
                    Irms_final = Irms[0];
                    Irms[0] = 0;
                    total_time = NANOTOSEC(timestamp_now - timestamp_initial);
                    break;
                } else if (is50[0]) {
                    break;
                }
            }
        }

        if (isT1 && (timestamp_now-timestamp_initial)>T1) {
            sum_volt[0] = 0;
            sample_count = 0;
            amp = 10;
            isT1 = FALSE;
        }

    }

    printf("Teste do trip:\n");
    printTime(pickup_event_time);
    printf(" %f A Proteção Sobrecorrente Temporizada Fase A Pickup\n", Irms_pickup);
    printTime(activated_time);
    printf(" %f A Proteção Sobrecorrente Temporizada Fase A Operada \n", Irms_final);
    printf("Tempo total do teste: %.2f s\n", total_time);

        return 0;
}
