#include <stdio.h> 
#include <conio.h> 
#include <Windows.h> 
#include <math.h> 
#include <time.h> 

//3. Нахождение  суммы квадратов  всех элементов  матрицы. 

int main() {
	int times = 10000000;
	int matrix[8][8];
	int res[8][8];
	int result, sum;
	double time;
	LARGE_INTEGER timerFrequency, timerStart, timerStop;

	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
			matrix[i][j] = rand() % 99 + 1;

	printf("\nIninitial matrix:\n\n");
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++)
			printf("%d\t ", matrix[i][j]);
		printf("\n");
	}

	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
			res[i][j] = 0;

	QueryPerformanceFrequency(&timerFrequency);
	QueryPerformanceCounter(&timerStart);

	for (int k = 0; k < times; k++) {
		sum = 0;
		for (int i = 0; i < 8; i++)
			for (int j = 0; j < 8; j++) {
				result = 0;
				result = matrix[i][j] * matrix[i][j];
				res[i][j] = result;
				sum += result;
		}
	}

	QueryPerformanceCounter(&timerStop);
	time = (double)(timerStop.QuadPart - timerStart.QuadPart) / (double)timerFrequency.QuadPart;

	printf("\nC result matrix: \n\n");
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++)
			printf("%d\t ", res[i][j]);
		printf("\n");
	}

	printf("\nC sum result: %d\n ", sum);
	printf("\nC time: %f\n", time);
	
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
			res[i][j] = 0;

	QueryPerformanceFrequency(&timerFrequency);
	QueryPerformanceCounter(&timerStart);

	for (int k = 0; k < times; k++) {
		sum = 0;
		int col = 64;
		_asm {
			pusha
			xor esi, esi
			loop_1:
				mov eax, [matrix + esi]
				imul eax, [matrix + esi]
				mov [res + esi], eax
				add sum, eax
				add esi, 4
				sub col, 1
			jnz loop_1
				popa
		}
	}

	QueryPerformanceCounter(&timerStop);
	time = (double)(timerStop.QuadPart - timerStart.QuadPart) / (double)timerFrequency.QuadPart;

	printf("\nAssembler without MMX result matrix: \n\n");
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++)
			printf("%d\t ", res[i][j]);
		printf("\n");
	}

	printf("\nAssembler without MMX sum result: %d\n ", sum);
	printf("\nAssembler without MMX time: %f\n", time);
	
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)	
			res[i][j] = 0;

	QueryPerformanceFrequency(&timerFrequency);
	QueryPerformanceCounter(&timerStart);

	for (int k = 0; k < times; k++) {
		sum = 0;
		int col = 64;
		_asm {
			pusha
			xor esi, esi
			pxor MM7, MM7
			loop_1mmx :			
				movq MM0, [matrix + esi]
				pmaddwd MM0, [matrix + esi]
				movq [res + esi], MM0
				paddd MM7, MM0
				add esi, 8
				sub col, 2
			jnz loop_1mmx

				movq MM0, MM7
				psrlq MM7, 32
				paddd MM7, MM0
				movd sum, MM7
				emms
				popa
		}
	}

	QueryPerformanceCounter(&timerStop);
	time = (double)(timerStop.QuadPart - timerStart.QuadPart) / (double)timerFrequency.QuadPart;

	printf("\nAssembler with MMX:\n\n");

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++)
			printf("%d\t ", res[i][j]);
		printf("\n");
	}

	printf("\nAssembler with MMX sum result: %d\n ", sum);
	printf("\nAssembler with MMX time: %f \n", time);

	system("pause");
	return 0;
}