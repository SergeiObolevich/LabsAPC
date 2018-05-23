#include <dos.h> 

struct VIDEO
{
	unsigned char symb;
	unsigned char attr;
};

int attr = 0x5e;

void print(int offset, int);
void get_reg();
/*IRQ0-7.*/
void interrupt(*Old08) (...);
void interrupt(*Old09) (...);
void interrupt(*Old0A) (...);
void interrupt(*Old0B) (...);
void interrupt(*Old0C) (...);
void interrupt(*Old0D) (...);
void interrupt(*Old0E) (...);
void interrupt(*Old0F) (...);
/*IRQ8-15.*/
void interrupt(*Old70) (...);
void interrupt(*Old71) (...);
void interrupt(*Old72) (...);
void interrupt(*Old73) (...);
void interrupt(*Old74) (...);
void interrupt(*Old75) (...);
void interrupt(*Old76) (...);
void interrupt(*Old77) (...);
/*New interrupt handlers.*/
void interrupt New08(...) { get_reg(); Old08(); }
void interrupt New09(...) { attr++;  get_reg(); Old09(); }
void interrupt New0A(...) { get_reg(); Old0A(); }
void interrupt New0B(...) { get_reg(); Old0B(); }
void interrupt New0C(...) { get_reg(); Old0C(); }
void interrupt New0D(...) { get_reg(); Old0D(); }
void interrupt New0E(...) { get_reg(); Old0E(); }
void interrupt New0F(...) { get_reg(); Old0F(); }
void interrupt New70(...) { get_reg(); Old70(); }
void interrupt New71(...) { get_reg(); Old71(); }
void interrupt New72(...) { get_reg(); Old72(); }
void interrupt New73(...) { get_reg(); Old73(); }
void interrupt New74(...) { get_reg(); Old74(); }
void interrupt New75(...) { get_reg(); Old75(); }
void interrupt New76(...) { get_reg(); Old76(); }
void interrupt New77(...) { get_reg(); Old77(); }
/*Fill videomemory pointert.*/
void print(int offset, int val) {
	char temp;
	int i;
	VIDEO far* screen = (VIDEO far *)MK_FP(0xB800, 0);
	screen += 12*80 + offset;
	for (i = 7; i >= 0; i--) {    //Find bits.
		temp = val % 2;
		val /= 2;                 //Fill the screen.
		screen->symb = temp + '0';
		screen->attr = attr;
		screen++;
	}
}
/*Get data from registers.*/
void get_reg() {
	int sub = 20;
	int reg;
	/*Leading controller.*/
	reg = inp(0x21);              //Masks.
	print(0 + sub, reg);

	outp(0x20, 0x0B);             //Is served.
	reg = inp(0x20);
	print(9 + sub, reg);

	outp(0x20, 0x0A);             //Requests.
	reg = inp(0x20);
	print(18 + sub, reg);

	reg = inp(0xA1);              //Masks.
	print(80 + sub, reg);

	outp(0xA0, 0x0B);             //Is served.
	reg = inp(0xA0);
	print(80 + 9 + sub, reg);

	outp(0xA0, 0x0A);             //Requests.
	reg = inp(0xA0);
	print(80 + 18 + sub, reg);
}

void init() {
	/*IRQ0-7.*/
	Old08 = getvect(0x08);        //Timer.
	Old09 = getvect(0x09);        //Keyboard.
	Old0A = getvect(0x0A);		  //Ega.
	Old0B = getvect(0x0B);		  //Unused.
	Old0C = getvect(0x0C);		  //RS-232 (COM2,4).
	Old0D = getvect(0x0D);        //Unused.
	Old0E = getvect(0x0E);        //Floppy.
	Old0F = getvect(0x0F);        //Printer (LPT).
	/*IRQ8-15.*/
	Old70 = getvect(0x70);
	Old71 = getvect(0x71);
	Old72 = getvect(0x72);
	Old73 = getvect(0x73);
	Old74 = getvect(0x74);
	Old75 = getvect(0x75);
	Old76 = getvect(0x76);
	Old77 = getvect(0x77);

	setvect(0x08, New08);
	setvect(0x09, New09);
	setvect(0x0A, New0A);
	setvect(0x0B, New0B);
	setvect(0x0C, New0C);
	setvect(0x0D, New0D);
	setvect(0x0E, New0E);
	setvect(0x0F, New0F);

	setvect(0x68, New70);
	setvect(0x69, New71);
	setvect(0x6A, New72);
	setvect(0x6B, New73);
	setvect(0x6C, New74);
	setvect(0x6D, New75);
	setvect(0x6E, New76);
	setvect(0x6F, New77);
	/*Disable interrupts handling (cli).*/
	_disable();
	/*Master.*/
	outp(0x20, 0x11);
	outp(0x21, 0x08);
	outp(0x21, 0x04);
	outp(0x21, 0x01);
	/*Slave.*/
	outp(0xA0, 0x11);
	outp(0xA1, 0x68);
	outp(0xA1, 0x02);
	outp(0xA1, 0x01);
	/*Enable interrupt handling (sti).*/
	_enable();
}

int main() {
	unsigned far *fp;
	init();
	/*Removing psp.*/
	FP_SEG(fp) = _psp; 
	FP_OFF(fp) = 0x2c;
	_dos_freemem(*fp);
	_dos_keep(0, (_DS - _CS) + (_SP / 16) + 1);
	return 0;
}