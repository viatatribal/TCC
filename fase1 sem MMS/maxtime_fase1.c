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

const uint64_t samplepolltime = 208333;
const uint64_t oc_currentpolltime = 0;

#define AMOSTRAS 80
#define CICLOS 3600
#define TOTAL_TIMESTAMP 288000

// MACROS
#define VOLTREAD(A)             (k * (float) analogRead((A))) - avg_volt
#define IRMS(VOLT,TOTAL)      sqrt((VOLT)/(TOTAL))
#define OC_PRO(A, B, p, M)      ((A) / (pow((M), (p)) - 1)) + (B)
#define SECTONANO(A)  (A)*1000000000UL
#define NANOTOSEC(A)  (A)/1000000000UL
#define AMP 1

// overprotection current values
const float A = 0.052;
const float B = 0.114;
const float p = 0.02;


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
    int amostra_count = 0;
    int tempo_count = 0;
    float sumVolt = 0;
    float Irms;

    float Ip = 1.0;
    float Imax = 50;

    int is50 = FALSE;
    int is51 = FALSE;


    wiringPiSetup();
    mcp3004Setup(MY_PIN, SPI_CHAN);

    uint64_t timestampbefore = getTimeNano();
    uint64_t timestampinitial = timestampbefore;
    uint64_t onehour = 3600UL * 1000000000UL;
    //uint64_t fifteenminutes = 900UL * 1000000000UL;
    //uint64_t oneminute = 60UL * 1000000000UL;

    // variables for debug
    uint64_t timeOC = 0;
    uint64_t timeDifference = 0;
    int cyclesInsideOC = 0;

    // variables for time
    uint64_t timestampdifference;
    uint64_t maxInterval = 0;
    int numAmostragem = 0;
    uint64_t cutInterval = 3000000UL; // 3 ms

    for (;;) {

    	uint64_t timestampnow = getTimeNano();

    	if ((timestampnow-timestampbefore)>samplepolltime) {

            //timestamp[tempo_count++] = timestampnow - timestampbefore;
            timestampbefore = timestampnow;
            float v = AMP * (VOLTREAD(MY_PIN));
            timestampdifference = getTimeNano() - timestampnow;

            if (timestampdifference > cutInterval) {
                numAmostragem++;
                if (timestampdifference > maxInterval) {
                    maxInterval = timestampdifference;
                }
            }

    	    sumVolt += v*v;
            amostra_count++;
        }

        if (amostra_count >= AMOSTRAS) {
       		Irms = IRMS(sumVolt, AMOSTRAS);
            sumVolt = 0;
      		amostra_count = 0;

            // OC PROTECTION
            if (Irms >= Imax) {
                is50 = TRUE;
                break;
            }

            if (Irms >= Ip) {

                uint64_t t0 = getTimeNano();
                uint64_t timestampOC = t0;
                uint64_t tLimite = SECTONANO((A / (pow(Irms/Ip, p) - 1)) + B);
		//tLimite = SECTONANO(tLimite);

                while (1) {
                    uint64_t timestampnow = getTimeNano();

                    if ((timestampnow-timestampbefore)>samplepolltime) {
                        timestampbefore = timestampnow;
                        float v = AMP * (VOLTREAD(MY_PIN));
                        sumVolt += v*v;
                        amostra_count++;
                    }

                    if (/*(timestampnow-timestampOC)>oc_currentpolltime ||*/
                        amostra_count >= AMOSTRAS) {
                        //timestampOC = timestampnow;
                        Irms = IRMS(sumVolt, amostra_count);
			cyclesInsideOC++;
                        sumVolt = 0;
                        amostra_count = 0;
                        uint64_t tN = SECTONANO((A / (pow(Irms/Ip, p) - 1)) + B);
			//tN = SECTONANO(tN);

                        if (tN < tLimite) {
                            tLimite = tN;
                        }
                    }

                    if (Ip > Irms) {
                        break;
                    }

                    if ((timestampnow - t0) >= tLimite) {
                        is51 = TRUE;
                        timeOC = tLimite;
                        timeDifference = timestampnow - t0;
                        //goto exit_loop;
                    }

                }
            }

	    }

        if ((timestampnow-timestampinitial)>onehour) {
            break;
	    }

    }
// OC PROTECTION
/*
exit_loop:
    if (is50) {
        printf("Trip 50! Irms value: %.2f\n", Irms);
    } else if (is51) {
        printf("Trip 51! Irms value: %.2f\n", Irms);
        printf("Trip 51! Time for OC: %.2f\n", NANOTOSEC(timeOC));
        printf("Trip 51! Time difference: %" PRIu64 "\n", NANOTOSEC(timeDifference));
	printf("Trip 51! Total cycles: %d\n", cyclesInsideOC);
    }
*/
    printf("Numero de intervalo de amostragens maior que %d\n", numAmostragem);
    printf("Latencia máxima %" PRIu64 "\n", maxInterval);

    return 0;
}
