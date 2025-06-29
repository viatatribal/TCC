#include <wiringPi.h>
#include <mcp3004.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

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
		buffer[i] = k * (float)x;
	}

	printf("[");
	for (int i = 0; i < amostras; i++) {
		printf("%2.3f, ", buffer[i]);
	}
	printf("]\n\n");
}
