/*
 *  server_example_basic_io.c
 *
 *  - How to use simple control models
 *  - How to serve analog measurement data
 *  - Using the IedServerConfig object to configure stack features
 */
#include <wiringPi.h>
#include <mcp3004.h>

#include "iec61850_server.h"
#include "hal_thread.h"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <inttypes.h>

#include "static_model.h"

/* libiec61850 */
static int running = 0;
static IedServer iedServer = NULL;

void
sigint_handler(int signalId)
{
    running = 0;
}


static void
connectionHandler (IedServer self, ClientConnection connection, bool connected, void* parameter)
{
    if (connected)
        printf("Connection opened\n");
    else
        printf("Connection closed\n");
}

/* orange pi */

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
int is50[3] = {false,false, false};
int is51[3] = {false,false, false};
uint64_t t_limit[3] = {0,0,0};
int isPickUpEvent[3] = {false, false, false};
uint64_t t_0[3] = {0,0,0};

Timestamp iecTimestamp;

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


void OC_protection(float A, float B, float p, int i,
    DataAttribute* FltA51, DataAttribute* FltA51_t,
    DataAttribute* Str51, DataAttribute* Str51_t)
{
    if (isPickUpEvent[i] && Ip > Irms[i]) {
        Timestamp_clearFlags(&iecTimestamp);
        Timestamp_setTimeInNanoseconds(&iecTimestamp, Hal_getTimeInNs());
        Timestamp_setLeapSecondKnown(&iecTimestamp, true);

        IedServer_lockDataModel(iedServer);
        IedServer_updateTimestampAttributeValue(iedServer, FltA51_t, &iecTimestamp);
        IedServer_updateFloatAttributeValue(iedServer, FltA51, Irms[i]);

        IedServer_updateTimestampAttributeValue(iedServer, Str51_t, &iecTimestamp);
        IedServer_updateBooleanAttributeValue(iedServer, Str51, false);
        IedServer_unlockDataModel(iedServer);

        isPickUpEvent[i] = false;
        return;
    }

    if (Irms[i] >= Imax) {
        is50[i] = true;
        return;
    }

    if (!isPickUpEvent[i] && Irms[i] >= Ip) {
        Timestamp_clearFlags(&iecTimestamp);
        Timestamp_setTimeInNanoseconds(&iecTimestamp, Hal_getTimeInNs());
        Timestamp_setLeapSecondKnown(&iecTimestamp, true);

        IedServer_lockDataModel(iedServer);
        IedServer_updateTimestampAttributeValue(iedServer, FltA51_t, &iecTimestamp);
        IedServer_updateFloatAttributeValue(iedServer, FltA51, Irms[i]);

        IedServer_updateTimestampAttributeValue(iedServer, Str51_t, &iecTimestamp);
        IedServer_updateBooleanAttributeValue(iedServer, Str51, true);
        IedServer_unlockDataModel(iedServer);

        t_limit[i] = SECTONANO(OC_function(A,B,p,Irms[i]/Ip));
        t_0[i] = getTimeNano();
        isPickUpEvent[i] = true;
    }

    if (isPickUpEvent[i]) {

        uint64_t timestamp_now_oc = getTimeNano();

        uint64_t t_n = SECTONANO(OC_function(A,B,p,Irms[i]/Ip));

        if (t_n < t_limit[i]) {
            t_limit[i] = t_n;
        }

        if ((timestamp_now_oc - t_0[i]) >= t_limit[i]) {
            is51[i] = true;
        }
    }
}

int
main(int argc, char** argv)
{
    wiringPiSetup();
    mcp3004Setup(MY_PIN, SPI_CHAN);

    int tcpPort = 102;

    if (argc > 1) {
        tcpPort = atoi(argv[1]);
    }

    printf("Using libIEC61850 version %s\n", LibIEC61850_getVersionString());

    /* Create new server configuration object */
    IedServerConfig config = IedServerConfig_create();

    /* Set buffer size for buffered report control blocks to 200000 bytes */
    IedServerConfig_setReportBufferSize(config, 200000);

    /* Set stack compliance to a specific edition of the standard (WARNING: data model has also to be checked for compliance) */
    IedServerConfig_setEdition(config, IEC_61850_EDITION_2);

    /* Set the base path for the MMS file services */
    IedServerConfig_setFileServiceBasePath(config, "./vmd-filestore/");

    /* disable MMS file service */
    IedServerConfig_enableFileService(config, false);

    /* enable dynamic data set service */
    IedServerConfig_enableDynamicDataSetService(config, true);

    /* disable log service */
    IedServerConfig_enableLogService(config, false);

    /* set maximum number of clients */
    IedServerConfig_setMaxMmsConnections(config, 2);

    /* Create a new IEC 61850 server instance */
    iedServer = IedServer_createWithConfig(&iedModel, NULL, config);

    /* configuration object is no longer required */
    IedServerConfig_destroy(config);

    /* set the identity values for MMS identify service */
    IedServer_setServerIdentity(iedServer, "UFF", "TCC", "1.0.0");

    //IedServer_setRCBEventHandler(iedServer, rcbEventHandler, NULL);

    /* By default access to variables with FC=DC and FC=CF is not allowed.
     * This allow to write to simpleIOGenericIO/GGIO1.NamPlt.vendor variable used
     * by iec61850_client_example1.
     */
    IedServer_setWriteAccessPolicy(iedServer, IEC61850_FC_DC, ACCESS_POLICY_ALLOW);

    /* MMS server will be instructed to start listening for client connections. */
    IedServer_start(iedServer, tcpPort);

    if (!IedServer_isRunning(iedServer))
    {
        printf("Starting server failed (maybe need root permissions or another server is already using the port)! Exit.\n");
        IedServer_destroy(iedServer);
        exit(-1);
    }

    running = 1;

    signal(SIGINT, sigint_handler);

    IedServer_lockDataModel(iedServer);
    IedServer_updateFloatAttributeValue(iedServer, IEDMODEL_PRO_PTOC1_StrVal_setMag_f, Ip);
    IedServer_updateFloatAttributeValue(iedServer, IEDMODEL_PRO_PTOC4_StrVal_setMag_f, Ip);
    IedServer_updateFloatAttributeValue(iedServer, IEDMODEL_PRO_PTOC7_StrVal_setMag_f, Ip);

    IedServer_updateFloatAttributeValue(iedServer, IEDMODEL_PRO_PIOC1_StrVal_setMag_f, Imax);
    IedServer_updateFloatAttributeValue(iedServer, IEDMODEL_PRO_PIOC1_StrVal_setMag_f, Imax);
    IedServer_updateFloatAttributeValue(iedServer, IEDMODEL_PRO_PIOC1_StrVal_setMag_f, Imax);
    IedServer_unlockDataModel(iedServer);

    uint64_t timestamp_before = getTimeNano();
    uint64_t timestamp_initial = timestamp_before;
    uint64_t onehour = 3600UL * 1000000000UL;


    // overprotection current values
    const float A = 0.052;
    const float B = 0.114;
    const float p = 0.02;

    // variables for time
    uint64_t timestampdifference;
    uint64_t maxInterval = 0;
    int numAmostragem = 0;
    uint64_t cutInterval = 3000000UL; // 3 ms

    while (running)
    {
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
                for(int i = 0; i < PHASES; i++) {
              		Irms[i] = IRMS(sum_volt[i], AMOSTRAS);
                    sum_volt[i] = 0;
                }

                Timestamp_clearFlags(&iecTimestamp);
                Timestamp_setTimeInNanoseconds(&iecTimestamp, Hal_getTimeInNs());
                Timestamp_setLeapSecondKnown(&iecTimestamp, true);

                IedServer_lockDataModel(iedServer);

                IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_MET_MMXU1_A1_phsA_t, &iecTimestamp);
                IedServer_updateFloatAttributeValue(iedServer, IEDMODEL_MET_MMXU1_A1_phsA_instCVal_mag_f, Irms[0]);
                IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_MET_MMXU1_A1_phsB_t, &iecTimestamp);
                IedServer_updateFloatAttributeValue(iedServer, IEDMODEL_MET_MMXU1_A1_phsB_instCVal_mag_f, Irms[1]);
                IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_MET_MMXU1_A1_phsC_t, &iecTimestamp);
                IedServer_updateFloatAttributeValue(iedServer, IEDMODEL_MET_MMXU1_A1_phsC_instCVal_mag_f, Irms[2]);

                IedServer_unlockDataModel(iedServer);

                OC_protection(A, B, p, 0,
                    IEDMODEL_PRO_PTOC1_FltA_phsA_instCVal_mag_f, IEDMODEL_PRO_PTOC1_FltA_phsA_t,
                    IEDMODEL_PRO_PTOC1_Str_general, IEDMODEL_PRO_PTOC1_Str_t);

                if (is51[0]) {
                    IedServer_lockDataModel(iedServer);
                    IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_PRO_PTOC1_FltA_phsA_t, &iecTimestamp);
                    IedServer_updateFloatAttributeValue(iedServer, IEDMODEL_PRO_PTOC1_FltA_phsA_instCVal_mag_f, Irms[0]);
                    IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_PRO_PTOC1_Op_t, &iecTimestamp);
                    IedServer_updateBooleanAttributeValue(iedServer, IEDMODEL_PRO_PTOC1_Op_general, true);
                    IedServer_unlockDataModel(iedServer);
                    is51[0] = false;
                    //reset_everything();
                    //continue;
                } else if (is50[0]) {
                    IedServer_lockDataModel(iedServer);
                    IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_PRO_PIOC1_FltA_phsA_t, &iecTimestamp);
                    IedServer_updateFloatAttributeValue(iedServer, IEDMODEL_PRO_PIOC1_FltA_phsA_instCVal_mag_f, Irms[0]);
                    IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_PRO_PIOC1_Op_t, &iecTimestamp);
                    IedServer_updateBooleanAttributeValue(iedServer, IEDMODEL_PRO_PIOC1_Op_general, true);
                    IedServer_unlockDataModel(iedServer);
                    is50[0] = false;

                    //reset_everything();
                    //continue;
                }

                OC_protection(A, B, p, 1,
                    IEDMODEL_PRO_PTOC4_FltA_phsA_instCVal_mag_f, IEDMODEL_PRO_PTOC4_FltA_phsA_t,
                    IEDMODEL_PRO_PTOC4_Str_general, IEDMODEL_PRO_PTOC4_Str_t);

                if (is51[1]) {
                    IedServer_lockDataModel(iedServer);
                    IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_PRO_PTOC4_FltA_phsA_t, &iecTimestamp);
                    IedServer_updateFloatAttributeValue(iedServer, IEDMODEL_PRO_PTOC4_FltA_phsA_instCVal_mag_f, Irms[1]);
                    IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_PRO_PTOC4_Op_t, &iecTimestamp);
                    IedServer_updateBooleanAttributeValue(iedServer, IEDMODEL_PRO_PTOC4_Op_general, true);
                    IedServer_unlockDataModel(iedServer);
                    is51[1] = false;
                    //reset_everything();
                    //continue;
                } else if (is50[1]) {
                    IedServer_lockDataModel(iedServer);
                    IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_PRO_PIOC4_FltA_phsA_t, &iecTimestamp);
                    IedServer_updateFloatAttributeValue(iedServer, IEDMODEL_PRO_PIOC4_FltA_phsA_instCVal_mag_f, Irms[1]);
                    IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_PRO_PIOC4_Op_t, &iecTimestamp);
                    IedServer_updateBooleanAttributeValue(iedServer, IEDMODEL_PRO_PIOC4_Op_general, true);
                    IedServer_unlockDataModel(iedServer);
                    is50[1] = false;

                    //reset_everything();
                    //continue;
                }

                OC_protection(A, B, p, 2,
                    IEDMODEL_PRO_PTOC7_FltA_phsA_instCVal_mag_f, IEDMODEL_PRO_PTOC7_FltA_phsA_t,
                    IEDMODEL_PRO_PTOC7_Str_general, IEDMODEL_PRO_PTOC7_Str_t);

                if (is51[2]) {
                    IedServer_lockDataModel(iedServer);
                    IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_PRO_PTOC7_FltA_phsA_t, &iecTimestamp);
                    IedServer_updateFloatAttributeValue(iedServer, IEDMODEL_PRO_PTOC7_FltA_phsA_instCVal_mag_f, Irms[2]);
                    IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_PRO_PTOC7_Op_t, &iecTimestamp);
                    IedServer_updateBooleanAttributeValue(iedServer, IEDMODEL_PRO_PTOC7_Op_general, true);
                    IedServer_unlockDataModel(iedServer);
                    is51[2] = false;
                    //reset_everything();
                    //continue;
                } else if (is50[2]) {
                    IedServer_lockDataModel(iedServer);
                    IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_PRO_PIOC7_FltA_phsA_t, &iecTimestamp);
                    IedServer_updateFloatAttributeValue(iedServer, IEDMODEL_PRO_PIOC7_FltA_phsA_instCVal_mag_f, Irms[2]);
                    IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_PRO_PIOC7_Op_t, &iecTimestamp);
                    IedServer_updateBooleanAttributeValue(iedServer, IEDMODEL_PRO_PIOC7_Op_general, true);
                    IedServer_unlockDataModel(iedServer);
                    is50[2] = false;

                    //reset_everything();
                    //continue;
                }

                sample_count = 0;

            }
        }

        if ((timestamp_now-timestamp_initial)>onehour) {
            break;
        }

        Thread_sleep(1);
    }

    /* stop MMS server - close TCP server socket and all client sockets */
    IedServer_stop(iedServer);

    /* Cleanup - free all resources */
    IedServer_destroy(iedServer);

    printf("Numero de intervalo de amostragens maior que %d\n", numAmostragem);
    printf("Latencia máxima %" PRIu64 "\n", maxInterval);

    return 0;
} /* main() */
