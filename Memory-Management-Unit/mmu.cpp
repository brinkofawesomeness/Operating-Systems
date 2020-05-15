// mmu.cpp
// MMU routines
// Conner Brinkley
// 04.05.2020

// If these are UNCOMMENTED, then compiling will use the code in this file.
// If one of these are commented, then it will compile with the built-in init, get, or put
// functions

//#define USE_STUD_MAP
//#define USE_STUD_UNMAP
#define USE_STUD_VTP

#include <types.h>
#include <mmu.h>
#include <slab.h>
#include <sched.h>
#include <syscall.h>
#include <cpu.h>
#include <programs.h>
#include <printf.h>

#if defined(USE_STUD_MAP)
// MmuTable is defined in mmu.h
void map(MmuTable **table, u64 vaddress, u64 paddress, u64 bits)
{
	// map() is given a double pointer to a table.
	// This table the address of a pointer so that your map function
	// is able to create a root table.

	// This map() function assumes the SV39 MMU system (39-bit virtual address)
	// using 4096kb pages.
	// No other pages are necessary.
	// 
	// vaddress is the virtual address that you need
	// to map to the paddress, which is the physical address

	// The bits are the URWX bits. You are responsible for setting
	// these for a leaf. Also, make sure you set the valid bit!

	// Use kmalloc(size, alignment) to allocate memory
	// Size is the number of bytes of memory you need
	// Alignment is the power of 2 to align it.
	//   So, alignment=12 would make the memory address a multiple
	//   of 4096 (2^12 = 4096).
}
#else
#warning "Using BUILT-IN map(), not yours"
#endif

#if defined(USE_STUD_UNMAP)
void unmap(MmuTable *table)
{
	// Unmap needs to unmap all memory starting at the root
	// table. So, essentially, your unmap() function needs
	// to follow any branches to a leaf. When it hits a leaf
	// free the branch that the leaf is on.
	// Keep doing this and eventually free the root.

	// kfree() is a function that works just like free()


}
#else
#warning "Using BUILT-IN unmap(), not yours"
#endif

#if defined(USE_STUD_VTP)
u64 virt_to_phys(MmuTable *table, u64 vaddress)
{
	// Decompose the virtual address
	u64 vpn[MAX_LEVEL + 1];
	for (int level = 0; level <= MAX_LEVEL; level++) {
		vpn[level] = ((vaddress >> (level * 9 + 12)) & 0x1ff);
	}

	u64 page_offset = vaddress & 0xfff;
	u64 ppn, entry, paddress;
	bool leaf = false;
	int level = MAX_LEVEL;

	// Walk down the page table levels
	while (level >= 0) {
		
		// Add VPN[x] to level x page table address
		entry = table->entry[vpn[level]].value;
		
		// Check if it's a valid entry and if it's a branch or leaf
		if ( !(entry & MEB_VALID) ) return 0;
		if (entry & MEB_RWX) leaf = true;
		else if (level == 0) return 0;

		// Grab the PPN, find the next page table, and decrement the level
		ppn = ((entry & ~0x3ff) << 2);
		table = (MmuTable *) ppn;
		if (leaf) break;
		level--;
	}

	// Make the physical address
	paddress = (ppn >> (level * 9 + 12));
	while (level > 0) {
		paddress = (paddress << 9);
		paddress += vpn[level];
		level--;
	}
	paddress = (paddress << 12);
	paddress += page_offset;
	return paddress;
}
#else
#warning "Using BUILT-IN virt_to_phys(), not yours"
#endif

static void my_mmu_process()
{
	// NOTE: printf() won't work in here unless we map everything
	// it needs (which is a lot!)
	// So, don't use it.

	const char output[] = "\r\nHello World\r\n";
	for (int i = 0;output[i] != '\0';i++) {
		syscall_put_char(output[i]);
		syscall_sleep(1000000);
	}
	// Unmap is automatically called when this process terminates.
}

// Typing test at the shell will invoke the following.
void test_process()
{
	printf("Adding MyMMU process\n");
	Process *p = add_process(
			my_mmu_process,
			"MyMMU",
			1,
			CPU_MODE_USER,
			0,
			PROCESS_DEFAULT_STACK_SIZE,
			0,
			false
	);

	// This maps memory addresses 0x2000_0000 through 0x2000_9000
	// as identity maps.
	// These are the utilities, such as make_syscall, recover_process
	// and all other necessary function calls.
	for (int i = 0;i < 25;i++) {
		unsigned long addr = 0x20000000 + (i << 12);
		map(&p->page_map, addr, addr, MEB_USER | MEB_READ | MEB_WRITE | MEB_EXECUTE);
	}
	// We need to map the program so that it is executable:
	map(&p->page_map, (u64)my_mmu_process, (u64)my_mmu_process, MEB_USER | MEB_EXECUTE);
	map(&p->page_map, (u64)my_mmu_process + 4096, (u64)my_mmu_process + 4096, MEB_USER | MEB_EXECUTE);


	// To test to see if virtual addresses can translate to DIFFERENT
	// physical addresses, adjust the stack to a random number.
	// This cannot exceed 39-bits because we're using the Sv39 system.
	unsigned long stack_adj = 0xdcafe000;
	for (int i = 0;i <= 5;i++) {
		unsigned long addr = reinterpret_cast<unsigned long>(p->stack_top);
		addr += i << 12;
		map(&p->page_map, addr + stack_adj, addr, MEB_USER | MEB_READ | MEB_WRITE);
	}
	// We move the stack pointer up by a random amount just to test
	// virtual addresses versus physical addresses.
	// In this case, p->stack_top points to the PHYSICAL top of the stack.
	// Therefore, we can add any amount as long as we mapped it correctly above!
	p->context.regs[GpSp] = reinterpret_cast<unsigned long>(p->stack_top + 0x2000 + stack_adj);
	printf("- Adjusted MyMMU's stack to be at %p\n", p->context.regs[GpSp]);

	// Test virt_to_phys
	u64 to = (u64)p->stack_top;
	u64 from = to + stack_adj;
	u64 vtp_to = virt_to_phys(p->page_map, from);
	bool vtp_passed = to == vtp_to;
	printf("Manual virt-to-phys: %s\n", (vtp_passed ? "passed" : "failed"));
	printf(" -> Virtual: %p -> %p, should translate to %p\n",
			from,
			vtp_to,
			to
	      );

	// The following will turn the MMU off for this process
	// p->page_map = nullptr;

	if (p->page_map == nullptr) {
		printf("This test does NOT have the MMU turned on!\n");
	}
	// We set the process initially in the waiting state, otherwise we could
	// preempt THIS process (the one scheduling the MyMMU process) and prematurely
	// run the MyMMU process.
	p->state = RUNNING;
}

