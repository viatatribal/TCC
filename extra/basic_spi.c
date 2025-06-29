#include <wiringPi.h>
#include <mcp3004.h>
#include <stdio.h>
#include <stdlib.h>

#define SPI_CHAN 1
#define MY_PIN 12345

int main()
{
	int x;
	float v, k = 3.3/1024.0;

	wiringPiSetup();
	mcp3004Setup(MY_PIN, SPI_CHAN);

	for (int n = 0; n < 3; n++) {
		for (int i = 0; i < 3; i++) {
			x = analogRead(MY_PIN + i);
			v = k * (float)x;
			printf("AD ch %i: = %4d\n", i, x);
			printf("AD vd %i: = %2.2f\n", i, v);
		}
		printf("\n\n");
	}

	return
}
