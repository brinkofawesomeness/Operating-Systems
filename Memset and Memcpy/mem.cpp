// mem.cpp
// memset and memcpy routines
// Conner Brinkley
// 02.01.2020

#define USE_STUD_MEMCPY
#define USE_STUD_MEMSET

#include <util.h>
#include <programs.h>
#include <printf.h>
#include <types.h>

#if defined(USE_STUD_MEMCPY)
void *memcpy(void *dst, const void *src, unsigned int bytes)
{
	
	// Calculate how many multiples of 8 are in "bytes"
	unsigned int multiples = bytes / 8;

	// Copy the source to the destination, 8 bytes at a time
	unsigned long long *llFrom = (unsigned long long *) src;
	unsigned long long *llTo = (unsigned long long *) dst;
	for (unsigned int i = 0; i < multiples; i++) {
		llTo[i] = llFrom[i];
	}

	// Copy over the remainder byte by byte
	unsigned char *cFrom = (unsigned char *) llFrom;
	unsigned char *cTo = (unsigned char *) llTo;
	for (unsigned int i = (multiples * 8); i < bytes; i++) {
		cTo[i] = cFrom[i];
	}
	
	return dst;
}
#else
#warning "Using BUILT-IN memcpy(), not yours"
#endif

#if defined(USE_STUD_MEMSET)
void *memset(void *dst, const char value, unsigned int bytes)
{
	
	// Calculate how many multiples of 8 are in "bytes"
	unsigned int multiples = bytes / 8;
	
	// Set 8 bytes at a time
	unsigned char values[8];
	for (unsigned int i = 0; i < 8; i++) values[i] = value;
	unsigned long long llBuffer = *((unsigned long long *) values);

	// Copy over as many multiples of 8 as we can
	unsigned long long *llDst = (unsigned long long *) dst;
	for (unsigned int i = 0; i < multiples; i++) {
		llDst[i] = llBuffer;
	}

	// Copy over the remainder byte by byte
	unsigned char *cDst = (unsigned char *) llDst;
	for (unsigned int i = (multiples * 8); i < bytes; i++) {
		cDst[i] = value;
	}

	return dst;
}
#else
#warning "Using BUILT-IN memset(), not yours"
#endif



// Type 'test' in the shell to invoke the following process.
void test_process()
{

	char *v = new char[100];

	memset(v, 'a', 100);

	for (int i = 0; i < 100; i++) {
		printf("arr[%d] = %c\n", i, v[i]);
	}

	memset(v, 'A', 50);
	printf("v: %s\n", v);

	char *z = new char[100];

	memcpy(z, v, 100);
	printf("z: %s\n", z);
}

