#include <dos.h>
#include <stdio.h>
#include <conio.h>
#include <io.h>

int msCounter = 0;

void interrupt far (*oldInt70h)(void);		// Указатель на старый обработчик прерывания ЧРВ								 
void interrupt far (*oldInt4Ah)(void);		// Указатель на старый обработчик прерывания будильника
void interrupt far newInt4Ahandler(void);	// Новый обработчик прерывания ЧРВ
void interrupt far newInt70handler(void);	// Новый обработчик прерывания будильника

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
// Ожидание освобождения часов
void wait(void) {
	do {								// Ожидание, пока часы заняты
		outp(0x70, 0x0A);
	} while (inp(0x71) & 0x80);			// 0x80 = 10000000 пока 7-й бит - 1, часы заняты
}
// Получение текущего значения времени
void getTime(void) {
	int value;

	wait();
	outp(0x70, 0x04);					// Текущий час
	value = inp(0x71); 
	printf("%d:", BCDToInteger(value)); 
	
	wait();
	outp(0x70, 0x02);					// Текущая минута
	value = inp(0x71); 
	printf("%d:", BCDToInteger(value)); 

	wait();
	outp(0x70, 0x00);					// Текущая секунда
	value = inp(0x71); 
	printf("%d   ", BCDToInteger(value)); 
	
	wait();
	outp(0x70, 0x07);					// Текущий день месяца
	value = inp(0x71); 
	printf("%d.", BCDToInteger(value)); 
	
	wait();
	outp(0x70, 0x08);					// Текущий месяц
	value = inp(0x71); 
	printf("%d.", BCDToInteger(value)); 
	
	wait();
	outp(0x70, 0x09);					// Текущий год
	value = inp(0x71); 
	printf("%d   ", BCDToInteger(value)); 
	
	wait();
	outp(0x70, 0x06);					// Текущий день недели
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
// Установка времени
void setTime(void) {
	int value;
	freeze();							// Запретить обновление часов

	do {
		printf("Enter hour: ");
		fflush(stdin);
		scanf("%d", &value);
	} while (value > 23 || value < 0);
	outp(0x70, 0x04);
	outp(0x71, IntToBCD(value));		// Значение в порт 71h в BCD-формате
 
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

	unfreeze();							// Разрешить обновление часов
}
// Установка будильника
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
	enable(); 			// 0xFE = 11111110, бит 0 в 0, чтобы разрешить прерывания от ЧРВ 
}
// Сброс будильника
void disableAlarm() {
	if (oldInt4Ah == NULL) {
		printf("Alarm is already disable or not set.\n");
		return;
	}
	_disable();
	setvect(0x4A, oldInt4Ah);
	outp(0xA1, (inp(0xA0) | 0x01));
	outp(0x70, 0x0B);
	outp(0xA1, inp(0x71) & 0xDF);		// 0xDF = 11011111, бит 0 в 1, бит 5 в 0, чтобы запретить прерывания от будильника
	_enable();
	printf("Alarm is disable.\n");
}
// Запретить обновление часов
void freeze(void) {
	unsigned char value;
	wait();								// Ожидание, пока часы заняты
	outp(0x70, 0x0B);
	value = inp(0x71);					// Читаем регистр состояния B
	value |= 0x80;						// Заменяем 7-й бит на 1 для запрещения обновления часов
	outp(0x70, 0x0B);
	outp(0x71, value);					// Записываем новое значение в регистр B, обновление часов запрещено 
}
// Разрешить обновление часов
void unfreeze(void) {
	unsigned char value;
	wait();								// Ожидание, пока часы заняты
	outp(0x70, 0x0B);
	value = inp(0x71);					// Читаем регистр состояния B
	value -= 0x80;						// Заменяем 7-й бит на 0 для разрешения обновления часов
	outp(0x70, 0x0B);
	outp(0x71, value);					// Записываем новое значение в регистр B, обновление часов разрешено
}
// Новый обработчик прерывания ЧРВ
void interrupt far newInt70handler(void) {
	msCounter++;						// Счётчик милисекунд
	outp(0x70, 0x0C);					// Если регистр C не будет прочитан после IRQ 8, тогда прерывание не случится снова
	inp(0x71);			 
	outp(0x20, 0x20);					// Посылаем контроллеру прерываний (master) сигнал EOI (end of interrupt)
	outp(0xA0, 0x20);					// Посылаем второму контроллеру прерываний (slave) сигнал EOI (end of interrupt) 
}
// Новый обработчик прерывания будильника
void interrupt far newInt4Ahandler(void) {
	write(1, "Good morning!", 13);
	disableAlarm();
}
// Задержка времени
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

	_disable();							// Запретить прерывания
	oldInt70h = getvect(0x70);
	setvect(0x70, newInt70handler);
	_enable();							// Разрешить прерывания

	// Размаскирование линии сигнала запроса от ЧРВ
	outp(0xA1, inp(0xA1) & 0xFE);			// 0xFE = 11111110, бит 0 в 0, чтобы разрешить прерывания от ЧРВ

	// Включение периодического прерывания
	outp(0x70, 0x0B);					// Выбираем регистр B
	outp(0x71, inp(0x71) | 0x40);			// 0x40 = 01000000, 6-й бит регистра B устанавливаем в 1 

	msCounter = 0;
	while (msCounter != delayPeriod);	// Задержка на заданное кол-во миллисекунд 
	printf("End delay of %d ms\n", msCounter);
	_disable();
	setvect(0x70, oldInt70h);			// Восстанавливаем старый обработчик
	_enable();
	printf("Time after delay: ");
	getTime();
}
// Перевод BCD в int
int BCDToInteger(int bcd) {
	return bcd % 16 + bcd / 16 * 10;
}
// Перевод int в BCD
unsigned char IntToBCD(int value) {
	return (unsigned char)((value / 10) << 4) | (value % 10);
}