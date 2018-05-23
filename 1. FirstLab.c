/* Вариант 18*/
/* Первая лабораторная по АПК = шестая лабораторная КПиЯП */
/* f(x) = sqrt(x) / (x^2+1) */

#include <conio.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

void main(void) {
	double entryPoint, exitPoint, step, x, result = 0;
	int concluding;
	clock_t start, end;

	printf ("Function for calculation:\n");
	printf ("f(x) = sqrt(x) / (x^2+1)\n");
	do {
		
		do {
			printf("\nThe function contains a square root, the start point must be greater than zero.\n");
			printf("Enter the starting point of the interval: ");
			fflush (stdin);
			scanf_s ("%lf", &entryPoint);
		} while (entryPoint < 0);
		
		do {
			printf("\nThe end point must be greater than start point.\n");
			printf("Enter the end point of the interval: ");
			fflush (stdin);
			scanf_s ("%lf", &exitPoint);
		} while (exitPoint <= entryPoint);

		do {
			printf("\nThe step must be greater than zero.\n");
			printf("Enter the step: ");
			fflush(stdin);
			scanf_s("%lf", &step);
		} while (step <= 0);
		
		x = entryPoint;
		result = 0;
		start = clock();

		_asm {
			finit
			fld exitPoint
			fld x
		loop_start :
			fcom
			fstsw ax
			and ah, 01000101b
			jz loop_end
			
			fld x
			fsqrt

			fld x
			fmul x
			fld1
			fadd
			fdiv

			fadd result
			fstp result
			fadd step
			fst x
			jmp loop_start
		loop_end:
			fwait
		}		 
		end = clock();

		printf ("\nAssembler result: %lf", result);
		printf ("\nAssembler time: %f", (double)(end - start) / CLK_TCK);

		x = entryPoint;
		result = 0;
		start = clock();

		while (x <= exitPoint) {
			result += (sqrt(x) / (x * x + 1));
			x += step;
		}
		end = clock();

		printf ("\nC result: %lf", result);
		printf ("\nC time: %f", (double)(end - start) / CLK_TCK);

		printf ("\n\nDo you want to repeat? (If yes: entet 1. Else enter 0.): ");
		scanf_s ("%d", &concluding);
	} while (concluding != 0);
}