// Conner Brinkley
// Round Robin Scheduler
// 02.09.2020
// COSC361

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

#if defined(USE_STUD)
Process *schedule(int hart)
{
	// Variables
	Process *p = nullptr;
	Process *previous = current[hart];

	// Search all the processes below the previous one 
	for (u32 i = previous->pid; i < MAX_PROCESSES; i++) {
		
		// Check if the process is running
		if (_processes[i].state == ProcessState::RUNNING) {
			p = &_processes[i];
			break;
		}

		// If not, check if it's sleeping and if we can wake it up
		else if (_processes[i].state == ProcessState::SLEEPING) {
			if (_processes[i].sleep_until <= get_time()) {
				
				// Now wake it, and reset its sleep timer
				_processes[i].state = ProcessState::RUNNING;
				_processes[i].sleep_until = 0;
				p = &_processes[i];
				break;
			}
		}
	}

	// If we didn't find a candidate, search above the previous process
	// Same loop as before, but the last process we condsider is the one previously ran
	if (p == nullptr) {
		for (u32 i = 0; i < previous->pid; i++) {
			if (_processes[i].state == ProcessState::RUNNING) {
				p = &_processes[i];
				break;
			} else if (_processes[i].state == ProcessState::SLEEPING) {
				if (_processes[i].sleep_until <= get_time()) {
					_processes[i].state = ProcessState::RUNNING;
					_processes[i].sleep_until = 0;
					p = &_processes[i];
					break;
				}
			}
		}
	}
	
	// Cycle and return
	if (p != nullptr) {
		p->on_cpu = hart;
		p->p_cpu = hart;
	}
	
	return p;
}
#else
#warning "USING BUILT-IN SCHEDULER, NOT YOURS!"
#endif

void some_additional_process()
{
	int i = 0;
	do {
		syscall_sleep(TIMER_SECS(1));
	} while (i++ < 10);
}

void test_process()
{
	unsigned int quantum_multiplier = 1;
	unsigned int priority = 0;

	printf("Spawning test process.\n");
	// See sched.h for parameters to add_process.
	add_process(some_additional_process,
			    "my_process",
				quantum_multiplier,
				CPU_MODE_USER,
				priority
				);
}
