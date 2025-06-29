#include <wiringPi.h>
#include <mcp3004.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

const float min_volt = 1.18;
const float max_volt = 1.90;
const float avg_volt = min_volt + ((max_volt - min_volt) / 2.0);


#define SPI_CHAN 1
#define MY_PIN 12345	


int main()
{
	int amostras = 2048;
	float buffer[amostras];

	int x;
	float k = 3.3/1024.0;

	wiringPiSetup();
	mcp3004Setup(MY_PIN, SPI_CHAN);


	for (int i = 0; i < amostras; i++) {
		x = analogRead(MY_PIN + 0);
		buffer[i] = (k * (float)x) - avg_volt;
	}

	printf("[");
	for (int i = 0; i < amostras; i++) {
		printf("%2.3f, ", buffer[i]);
	}
	printf("]\n\n");
}
