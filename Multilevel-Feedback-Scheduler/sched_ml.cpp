// Lab 4
// ML / MLF Scheduler Lab
// COSC 361
// Conner Brinkley
// 02.16.2020

// Comment out the following to use MY code to schedule and
// see what your code should do.
//
// If this is uncommented, then the OS will use YOUR scheduler
// defined below.
#define USE_STUD

#include <config.h>
#include <sched.h>
#include <programs.h>
#include <printf.h>
#include <syscall.h>
#include <cpu.h>
#include <clint.h>
#include <timer.h>


/* HELPER FUNCTION TO INCREASE CODE REUSABILITY FOR RR AND ML */
int is_candidate(Process& p) { // originally passed by value, not by reference, which made this invalid

	// Check if the process is running or sleeping
	if (p.state == ProcessState::RUNNING) return 1;
	if (p.state == ProcessState::SLEEPING) {

		// If we can wake it up, it's a candidate
		if (p.sleep_until <= get_time()) {
			p.state = ProcessState::RUNNING;
			p.sleep_until = 0;
			return 1;
		}
	}

	// If we make it here, it's either dead, waiting, or not ready to be woken up
	return 0;
}


/* ROUND ROBIN SCHEDULING ALGORITHM */
static Process *sched_rr(const Process *previous) {
    
	// Search all the processes below the previous one until we find a candidate
	for (u32 i = previous->pid; i < MAX_PROCESSES; i++) {
		if (is_candidate(_processes[i]) == 1) return &_processes[i];
	}

	// If we didn't find a candidate, search above the previous process
	// The last one we check is the previously ran process
	for (u32 i = 0; i < previous->pid; i++) {
		if (is_candidate(_processes[i]) == 1) return &_processes[i];
	}

	// If it reaches this point, there is no available process to be scheduled
	return nullptr;
}


/* MULTILEVEL SCHEDULING ALGORITHM */
static Process *sched_ml(const Process *previous) {
	
	// Keeps track of process and level
	Process *p = nullptr;
	u16 max_priority = LOWEST_PRIORITY - 1;

	// Search all the processes below the previous one
	for (u32 i = previous->pid; i < MAX_PROCESSES; i++) {
		if (_processes[i].priority > max_priority) {
			
			// If the priority is greater, update the return val and level
			if (is_candidate(_processes[i]) == 1) {
				p = &_processes[i];
				max_priority = p->priority;
			}
		}
	}

	// Search all the processes above the previous one
	// The last one we check is the previously ran process
	for (u32 i = 0; i < previous->pid; i++) {
		if (_processes[i].priority > max_priority) {
			
			// If the priority is greater, update the return val and level
			if (is_candidate(_processes[i]) == 1) {
				p = &_processes[i];
				max_priority = p->priority;
			}
		}
	}

	// Return the first process at the highest priority we found
	return p;
}


/* MULTILEVEL FEEDBACK SCHEDULING ALGORITHM */
static Process *sched_mlf(const Process *previous) {
	
	Process *p = sched_ml(previous);
	
	// Check the bounds before updating the QM and priority
	if (p->qm < MAXIMUM_QM) p->qm += 1;
	if (p->priority > LOWEST_PRIORITY) p->priority -= 1;	
	
	return p;
}


#if defined(USE_STUD)
Process *schedule(int hart)
{
	Process *p = nullptr;
	Process *previous = current[hart];
	
	// Determine which scheduling algorithm to use
	switch (kconf("sched")) {
		case SCHED_RR:
			p = sched_rr(previous);
			break;
		case SCHED_ML:
			p = sched_ml(previous);
			break;
		case SCHED_MLF:
			p = sched_mlf(previous);
			break;
	}
	
	// This is needed for the multi-core CPU
	// When a candidate is chosen, you must run the following.
	if (p != nullptr) {
		p->on_cpu = hart;
		p->p_cpu = hart;
	}
	return p;
}
#else
#warning "USING BUILT-IN SCHEDULER, NOT YOURS!"
#endif


/* USED FOR TESTING BELOW THIS POINT */

void some_additional_process()
{
	int i = 0;
	do {
		syscall_sleep(TIMER_MSECS(1500));
	} while (i++ < 5);
}

void test_process()
{
	unsigned int quantum_multiplier = 1;
	unsigned int priority = 15;

	printf("Spawning test process.\n");
	// See sched.h for parameters to add_process.
	add_process(some_additional_process,
			    "my_process",
				quantum_multiplier,
				CPU_MODE_USER,
				priority
				);
}
