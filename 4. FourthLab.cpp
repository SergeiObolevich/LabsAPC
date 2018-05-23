#include <stdio.h>
#include <conio.h>
#include <dos.h>

#define COUNT 17
//#define COUNT 8
#define DELAY 10
#define TIME_RUN 65536

void soundGeneration();
void divisionCoefficient();
void statusWord();
void randomNumber();

void main() {
	clrscr();
	char choice;
	do {
		printf("\n\n1. Sound generation.\n2. Coefficient of division.\n3. Status word.\n4. Random number.\n0. Exit.\n");
		fflush(stdin);
		printf("\nYour choise: ");
		scanf("%s", &choice);

		switch (choice) {
			case '1': soundGeneration(); break;
			case '2': divisionCoefficient(); break;
			case '3': statusWord(); break;
			case '4': randomNumber(); break;
		}
	} while (choice != '0');
}

void soundGeneration() {
	//int frequency[COUNT] = { 329, 246, 246, 261, 293, 329, 392, 349 };
	//int durability[COUNT] = { 400, 400, 400, 400, 400, 400, 400, 400 };
	int frequency[COUNT] = { 523, 523, 523, 659, 784, 523, 523, 523, 659, 784, 784, 884, 784, 687, 659, 579, 519 };
	int durability[COUNT] = { 200, 200, 200, 700, 500, 200, 200, 200, 700, 500, 400, 200, 200, 200, 200, 200, 400 };
	int delayCounter[COUNT] = { 100, 100, 100, 70, 50, 100, 100, 100, 70, 80, 40, 20, 20, 20, 20, 20, 40 };
	long unsigned base = 1193180;
	int frequencyCounter, kd;

	for (frequencyCounter = 0; frequencyCounter < COUNT; frequencyCounter++) {
		outp(0x43, 0xB6);
		kd = base / frequency[frequencyCounter];
		outp(0x42, kd % 256);
		kd /= 256;
		outp(0x42, kd);

		outp(0x61, inp(0x61) | 3);
		delay(durability[frequencyCounter]);
		outp(0x61, inp(0x61) & 0xFC);
		delay(delayCounter[frequencyCounter]);
		//delay(DELAY);
	}
}

void divisionCoefficient() {
	unsigned long j;
	int Channel;
	int ports[] = { 0x40, 0x41, 0x42 };
	int controlWords[] = { 0, 64, 128 };
	unsigned kdLow, kdHigh, kd, max;

	printf("\n\nCoefficient of division: \n");

	for (Channel = 0; Channel < 3; Channel++) {
		kd = 0;
		max = 0;

		if (Channel == 2) {
			outp(0x61, inp(0x61) | 3);
		}

		for (j = 0; j < TIME_RUN; j++) {
			outp(0x43, controlWords[Channel]);
			kdLow = inp(ports[Channel]);
			kdHigh = inp(ports[Channel]);
			kd = kdHigh * 256 + kdLow;

			if (kd > max) {
				max = kd;
			}
		}
		if (Channel == 2) {
			outp(0x61, inp(0x61) & 0xFC);
		}
		printf("\nChannel %d: %4X\n", Channel, max);
	}
}

void statusWord() {
	printf("\n\nStatus word: \n");
	char stateWord[8];
	int Channel, state;
	int ports[] = {0x40, 0x41, 0x42};
	int controlWords[] = {226, 228, 232};   //11 10 001 0, 11 10 010 0, 11 10 100 0

	for (Channel = 0; Channel < 3; Channel++) {
		outp(0x43, controlWords[Channel]);
		state = inp(ports[Channel]);
	
		for (int i = 7; i >= 0; i--) {
			stateWord[i] = state % 2 + '0';
			state /= 2;

		}
		printf("\nChannel %d: ", Channel);
		for (i = 0; i < 8;  i++) {
			printf("%c", stateWord[i]);
		}
		printf("\n");
	}
}

void randomNumber() {
	char choice;
	unsigned int limit = TIME_RUN - 1, numberLow, numberHigh, number;

	do {
		printf("\n\n1. Set a limit.\n2. Get a number.\n0. Exit\n");
		fflush(stdin);
		printf("\nYour choise: ");
		scanf("%s", &choice);

		switch (choice) {
		case '1':
			do {
				printf("\nEnter a limit in range [1...65635].\n");
				fflush(stdin);
			} while (!scanf("%d", &limit) || limit < 1);

			outp(0x43, 0xB4); 
			outp(0x42, limit % 256);
			limit /= 256;
			outp(0x42, limit);
			outp(0x61, inp(0x61) | 1);

			break;

		case '2':
			outp(0x43, 128);
			numberLow = inp(0x42);
			numberHigh = inp(0x42);
			number = numberHigh * 256 + numberLow;

			printf("\nRandom number: %u\n", number);

			break;
		}
	} while (choice != '0');
	outp(0x61, inp(0x61) & 0xFC);
}