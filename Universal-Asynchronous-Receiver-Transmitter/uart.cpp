// uart.cpp
// UART routines
// Conner Brinkley
// 03.01.2020

#define USE_STUD_INIT
#define USE_STUD_GET
#define USE_STUD_PUT

#include <uart.h>
#include <timer.h>  // For FREQ
#include <programs.h>
#include <printf.h>

// UART BASE ADDRESS FOR GET AND PUT FUNCS
volatile unsigned char *UART_BASE;

// UART REGISTER OFFSETS
enum UART_REGS {
	RBR = 0,
	THR = 0,
	IER = 1,
	IIR = 2,
	FCR = 2,
	LCR = 3,
	MCR = 4,
	LSR = 5,
	MSR = 6,
	SCR = 7,
	DLL = 0,
	DLM = 1
};

// UART initialization is to set the clock
// to the correct frequency and set the registers
// accordingly.
#if defined(USE_STUD_INIT)
void uart_init(unsigned long base_address)
{
	// Set the UART base address as a global
	UART_BASE = (unsigned char *) base_address;

	// Zero out all the registers
	for (int offset = 0; offset < 8; offset++) {
		UART_BASE[offset] = 0x00;
	}

	// Open the divisor latch access bit (DLAB)
	UART_BASE[LCR] |= (1 << 7);

	// Calculate the divisor and write it to the DLL and DLM
	unsigned short divisor = FREQ / (115200 * 16);
	UART_BASE[DLL] = divisor;
	UART_BASE[DLM] = (divisor >> 8);

	// Close the DLAB
	UART_BASE[LCR] &= ~(1 << 7);

	// Set the registers
	UART_BASE[LCR] |= 1;             // Set the word length to 8 bits
	UART_BASE[LCR] |= (1 << 1);
	UART_BASE[LCR] |= (1 << 2);      // Set the stop bits to two
	UART_BASE[FCR] |= 1;             // Enable the FIFO
	UART_BASE[IER] |= 1;             // Enable interrupts
}
#else
#warning "Using BUILT-IN init(), not yours"
#endif

#if defined(USE_STUD_GET)
char uart_get()
{
	// Check the data ready (DR) bit
	if (UART_BASE[LSR] & 1) {
		return UART_BASE[RBR];
	} else {
		return '\0';
	}
}
#else
#warning "Using BUILT-IN get(), not yours"
#endif

#if defined(USE_STUD_PUT)
void uart_put(char data)
{
	// Spin wait until the TEMT bit is set to 1 before transmitting data
	while ( !(UART_BASE[LSR] & (1 << 6)) ) {}
	UART_BASE[THR] = data;
}
#else
#warning "Using BUILT-IN put(), not yours"
#endif

// Typing test at the shell will invoke the following.
// You probably won't need this for the UART lab, but it's
// there for whatever you need.
void test_process()
{

}

