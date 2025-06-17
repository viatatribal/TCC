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


Timestamp iecTimestamp;

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

    while (running)
    {
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

                Timestamp_clearFlags(&iecTimestamp);
                Timestamp_setTimeInNanoseconds(&iecTimestamp, Hal_getTimeInNs());
                Timestamp_setLeapSecondKnown(&iecTimestamp, true);

                IedServer_lockDataModel(iedServer);

                IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_MET_MMXU1_A1_phsA_t, &iecTimestamp);
                IedServer_updateFloatAttributeValue(iedServer, IEDMODEL_MET_MMXU1_A1_phsA_instCVal_mag_f, Irms[0][irms_count]);
                IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_MET_MMXU1_A1_phsB_t, &iecTimestamp);
                IedServer_updateFloatAttributeValue(iedServer, IEDMODEL_MET_MMXU1_A1_phsB_instCVal_mag_f, Irms[1][irms_count]);
                IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_MET_MMXU1_A1_phsC_t, &iecTimestamp);
                IedServer_updateFloatAttributeValue(iedServer, IEDMODEL_MET_MMXU1_A1_phsC_instCVal_mag_f, Irms[2][irms_count]);

                IedServer_unlockDataModel(iedServer);

                irms_count++;
                sample_count = 0;

            }
        }

        if ((timestamp_now-timestamp_initial)>end_time) {
            break;
        }

        Thread_sleep(1);
    }

    /* stop MMS server - close TCP server socket and all client sockets */
    IedServer_stop(iedServer);

    /* Cleanup - free all resources */
    IedServer_destroy(iedServer);

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
} /* main() */
