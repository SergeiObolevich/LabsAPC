#include <dos.h>
#include <stdio.h>
#include <conio.h>
#include <io.h>

int msCounter = 0;

void interrupt far (*oldInt70h)(void);		// ��������� �� ������ ���������� ���������� ���								 
void interrupt far (*oldInt4Ah)(void);		// ��������� �� ������ ���������� ���������� ����������
void interrupt far newInt4Ahandler(void);	// ����� ���������� ���������� ���
void interrupt far newInt70handler(void);	// ����� ���������� ���������� ����������

void freeze(void);
void unfreeze(void);

int BCDToInteger(int bcd);
unsigned char IntToBCD(int value);
void getTime(void);
void setTime(void);
void delay_time(void);
void wait(void);
void enableAlarm(void);
void disableAlarm(void);

void main() {
	char c, value;
	clrscr();
	printf("\n1. Show time\n2. Set time\n3. Delay time\n4. Enable alarm\n5. Disable alarm\n0. Exit\n\n");
	while (c != 0) {
		c = getch();
		switch (c) {
			case '1': getTime(); break;
			case '2': setTime(); break;
			case '3': delay_time(); break;
			case '4': enableAlarm(); break;
			case '5': disableAlarm(); break;
			case '0': return;
		}
	}
}
// �������� ������������ �����
void wait(void) {
	do {								// ��������, ���� ���� ������
		outp(0x70, 0x0A);
	} while (inp(0x71) & 0x80);			// 0x80 = 10000000 ���� 7-� ��� - 1, ���� ������
}
// ��������� �������� �������� �������
void getTime(void) {
	int value;

	wait();
	outp(0x70, 0x04);					// ������� ���
	value = inp(0x71); 
	printf("%d:", BCDToInteger(value)); 
	
	wait();
	outp(0x70, 0x02);					// ������� ������
	value = inp(0x71); 
	printf("%d:", BCDToInteger(value)); 

	wait();
	outp(0x70, 0x00);					// ������� �������
	value = inp(0x71); 
	printf("%d   ", BCDToInteger(value)); 
	
	wait();
	outp(0x70, 0x07);					// ������� ���� ������
	value = inp(0x71); 
	printf("%d.", BCDToInteger(value)); 
	
	wait();
	outp(0x70, 0x08);					// ������� �����
	value = inp(0x71); 
	printf("%d.", BCDToInteger(value)); 
	
	wait();
	outp(0x70, 0x09);					// ������� ���
	value = inp(0x71); 
	printf("%d   ", BCDToInteger(value)); 
	
	wait();
	outp(0x70, 0x06);					// ������� ���� ������
	value = inp(0x71);
	switch (BCDToInteger(value)) {
		case 2: printf("Monday\n"); break;
		case 3: printf("Tuesday\n"); break;
		case 4: printf("Wednesday\n"); break;
		case 5: printf("Thursday\n"); break;
		case 6: printf("Friday\n"); break;
		case 7: printf("Saturday\n"); break;
		case 1: printf("Sunday\n"); break;
		default: printf("Day of week is not set\n"); break;
	}
}
// ��������� �������
void setTime(void) {
	int value;
	freeze();							// ��������� ���������� �����

	do {
		printf("Enter hour: ");
		fflush(stdin);
		scanf("%d", &value);
	} while (value > 23 || value < 0);
	outp(0x70, 0x04);
	outp(0x71, IntToBCD(value));		// �������� � ���� 71h � BCD-�������
 
	do {
		printf("Enter minute: ");
		fflush(stdin);
		scanf("%d", &value);
	} while (value > 59 || value < 0);
	outp(0x70, 0x02);
	outp(0x71, IntToBCD(value));

	do {
		printf("Enter second: ");
		fflush(stdin);
		scanf("%d", &value);
	} while (value > 59 || value < 0);
	outp(0x70, 0x00);
	outp(0x71, IntToBCD(value));
 
	do {
		printf("Enter week day number: ");
		fflush(stdin);
		scanf("%d", &value);
	} while (value > 7 || value < 1);
	outp(0x70, 0x06);
	outp(0x71, IntToBCD(value));

	do {
		printf("Enter day of month: ");
		fflush(stdin);
		scanf("%d", &value);
	} while (value > 31 || value < 1);
	outp(0x70, 0x07);
	outp(0x71, IntToBCD(value));

	do {
		printf("Enter mounth: ");
		fflush(stdin);
		scanf("%d", &value);
	} while (value > 12 || value < 1);
	outp(0x70, 0x08);
	outp(0x71, IntToBCD(value));

	do {
		printf("Enter year: ");
		fflush(stdin);
		scanf("%d", &value);
	} while (value > 99 || value < 0);
	outp(0x70, 0x09);
	outp(0x71, IntToBCD(value));

	unfreeze();							// ��������� ���������� �����
}
// ��������� ����������
void enableAlarm(void) {
	int value, second, minute, hour;

	do {
		printf("Enter second: ");
		fflush(stdin);
		scanf("%d", &second);
	} while (second > 59 || second < 0);

	do{
		printf("Enter minute: ");
		fflush(stdin);
		scanf("%d", &minute);
	} while (minute > 59 || minute < 0);

	do {
		printf("Enter hour: ");
		fflush(stdin);
		scanf("%d", &hour);
	} while (hour > 23 || hour < 0);

	outp(0x70, 0x01);
	outp(0x71, IntToBCD(second));

	outp(0x70, 0x03);
	outp(0x71, IntToBCD(minute));

	outp(0x70, 0x05);
	outp(0x71, IntToBCD(hour));

	outp(0x70, 0xB);
	outp(0x71, inp(0x71) | 0x20);

	disable();
	oldInt4Ah = getvect(0x4A);
	setvect(0x4A, newInt4Ahandler);
	enable(); 			// 0xFE = 11111110, ��� 0 � 0, ����� ��������� ���������� �� ��� 
}
// ����� ����������
void disableAlarm() {
	if (oldInt4Ah == NULL) {
		printf("Alarm is already disable or not set.\n");
		return;
	}
	_disable();
	setvect(0x4A, oldInt4Ah);
	outp(0xA1, (inp(0xA0) | 0x01));
	outp(0x70, 0x0B);
	outp(0xA1, inp(0x71) & 0xDF);		// 0xDF = 11011111, ��� 0 � 1, ��� 5 � 0, ����� ��������� ���������� �� ����������
	_enable();
	printf("Alarm is disable.\n");
}
// ��������� ���������� �����
void freeze(void) {
	unsigned char value;
	wait();								// ��������, ���� ���� ������
	outp(0x70, 0x0B);
	value = inp(0x71);					// ������ ������� ��������� B
	value |= 0x80;						// �������� 7-� ��� �� 1 ��� ���������� ���������� �����
	outp(0x70, 0x0B);
	outp(0x71, value);					// ���������� ����� �������� � ������� B, ���������� ����� ��������� 
}
// ��������� ���������� �����
void unfreeze(void) {
	unsigned char value;
	wait();								// ��������, ���� ���� ������
	outp(0x70, 0x0B);
	value = inp(0x71);					// ������ ������� ��������� B
	value -= 0x80;						// �������� 7-� ��� �� 0 ��� ���������� ���������� �����
	outp(0x70, 0x0B);
	outp(0x71, value);					// ���������� ����� �������� � ������� B, ���������� ����� ���������
}
// ����� ���������� ���������� ���
void interrupt far newInt70handler(void) {
	msCounter++;						// ������� ����������
	outp(0x70, 0x0C);					// ���� ������� C �� ����� �������� ����� IRQ 8, ����� ���������� �� �������� �����
	inp(0x71);			 
	outp(0x20, 0x20);					// �������� ����������� ���������� (master) ������ EOI (end of interrupt)
	outp(0xA0, 0x20);					// �������� ������� ����������� ���������� (slave) ������ EOI (end of interrupt) 
}
// ����� ���������� ���������� ����������
void interrupt far newInt4Ahandler(void) {
	write(1, "Good morning!", 13);
	disableAlarm();
}
// �������� �������
void delay_time(void) {
	unsigned long delayPeriod;
	unsigned char value;

	do {
		printf("Enter delay time in milliseconds: ");
		fflush(stdin);
		scanf("%ld", &delayPeriod);
	} while (delayPeriod < 1);
	printf("Time before delay: ");
	getTime();
	printf("Delaying ...\n");

	_disable();							// ��������� ����������
	oldInt70h = getvect(0x70);
	setvect(0x70, newInt70handler);
	_enable();							// ��������� ����������

	// ��������������� ����� ������� ������� �� ���
	outp(0xA1, inp(0xA1) & 0xFE);			// 0xFE = 11111110, ��� 0 � 0, ����� ��������� ���������� �� ���

	// ��������� �������������� ����������
	outp(0x70, 0x0B);					// �������� ������� B
	outp(0x71, inp(0x71) | 0x40);			// 0x40 = 01000000, 6-� ��� �������� B ������������� � 1 

	msCounter = 0;
	while (msCounter != delayPeriod);	// �������� �� �������� ���-�� ����������� 
	printf("End delay of %d ms\n", msCounter);
	_disable();
	setvect(0x70, oldInt70h);			// ��������������� ������ ����������
	_enable();
	printf("Time after delay: ");
	getTime();
}
// ������� BCD � int
int BCDToInteger(int bcd) {
	return bcd % 16 + bcd / 16 * 10;
}
// ������� int � BCD
unsigned char IntToBCD(int value) {
	return (unsigned char)((value / 10) << 4) | (value % 10);
}