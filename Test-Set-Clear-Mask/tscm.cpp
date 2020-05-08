// tscm.cpp
// Test, Set, Clear, Mask routines
// Conner Brinkley
// 01.26.2020


// If these are UNCOMMENTED, then compiling will use the code in this file.
// If one of these are commented, then it will compile with the built-in init, get, or put
// functions

#define USE_STUD_TEST
#define USE_STUD_SET
#define USE_STUD_CLEAR
#define USE_STUD_MASK

#include <util.h>
#include <programs.h>
#include <printf.h>
#include <types.h>

#if defined(USE_STUD_TEST)
bool bit_test(u64 value, u8 bit) {
	
	// Did not originally use 1UL, and therefore it 
	// cut off bits past 32
	if (value & (1UL << bit)) return true;
	return false;
}
#else
#warning "Using BUILT-IN test(), not yours"
#endif

#if defined(USE_STUD_SET)
u64 bit_set(u64 value, u8 bit) {

	// Did not originally use 1UL, and therefore it
	// cut off bits past 32
	value |= (1UL << bit);
	return value;
}
#else
#warning "Using BUILT-IN set(), not yours"
#endif

#if defined(USE_STUD_CLEAR)
u64 bit_clear(u64 value, u8 bit) {
	
	// Did not originally use 1UL, and therefore it 
	// cut off bits past 32
	value &= ~(1UL << bit);
	return value;
}
#else
#warning "Using BUILT-IN clear(), not yours"
#endif

#if defined(USE_STUD_MASK)
u64 bit_mask(u8 start, u8 end) {
	
	// Variables
	u64 mask;
	u8 low, high;
	
	// Find the low and high values
	if (start >= end) {
		low = end;
		high = start;
	} else {
		low = start;
		high = end;
	}

	// Left shift 1 by the difference
	// then subtract 1 to capture lower bits
	mask = high - low;
	mask = 1UL << mask;
	mask -= 1;
	
	// Left shift by the lowest number and set the highest bit
	mask <<= low;
	mask |= (1UL << high);
	
	return mask;
}
#else
#warning "Using BUILT-IN mask(), not yours"
#endif



// Type 'test' in the shell to invoke the following process.
void test_process()
{
	int failed = 0;
	u64 val;

	val = bit_set(0, 1);
	if (2 != val) {
		printf("Failed set #1, got %lu\n", val);
		failed++;
	}
	val = bit_set(val, 4);
	if (18 != val) {
		printf("Failed set #2, got %lu\n", val);
		failed++;
	}
	val = bit_set(val, 16);
	if (65554 != val) {
		printf("Failed set #3, got %lu\n", val);
		failed++;
	}
	val = bit_clear(val, 0);
	if (65554 != val) {
		printf("Failed clear #1, got %lu\n", val);
		failed++;
	}
	val = bit_clear(val, 4);
	if (65538 != val) {
		printf("Failed clear #2, got %lu\n", val);
		failed++;
	}
	val = bit_clear(val, 1);
	if (65536 != val) {
		printf("Failed clear #3, got %lu\n", val);
		failed++;
	}
	val = bit_clear(val, 16);
	if (0 != val) {
		printf("Failed clear #4, got %lu\n", val);
		failed++;
	}

	val = bit_mask(3, 0);
	if (15 != val) {
		printf("Failed mask #1, got %lu\n", val);
		failed++;
	}

	val = bit_mask(0, 3);
	if (15 != val) {
		printf("Failed mask #2, got %lu\n", val);
		failed++;
	}

	val = bit_mask(9, 7);
	if (896 != val) {
		printf("Failed mask #3, got %lu\n", val);
		failed++;
	}

	val = bit_mask(31, 31);
	if ((1UL << 31) != val) {
		printf("Failed mask #4, got %lu\n", val);
		failed++;
	}

	val = bit_mask(31, 0);
	if (0xffffffff != val) {
		printf("Failed mask #5, got %lu\n", val);
		failed++;
	}

	if (true != bit_test(17, 0)) {
		printf("Failed test #1\n");
		failed++;
	}

	if (false != bit_test(17, 1)) {
		printf("Failed test #2\n");
		failed++;
	}

	if (true != bit_test(74, 3)) {
		printf("Failed test #3\n");
		failed++;
	}

	if (0 == failed) {
		printf("Passed all tests!\n");
	}
	else {
		printf("Failed %d tests\n", failed);
	}

}

