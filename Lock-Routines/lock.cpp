// lock.cpp
// Lock routines
// Conner Brinkley
// 02.23.2020

#define USE_STUD_SEM_LOCK
#define USE_STUD_SEM_UNLOCK
#define USE_STUD_MUTEX_LOCK
#define USE_STUD_MUTEX_UNLOCK
#define USE_STUD_BARRIER_WAIT

#include <cpu.h>
#include <sched.h>
#include <clint.h>
#include <lock.h>
#include <atomic.h>
#include <syscall.h>
#include <programs.h>
#include <printf.h>
#include <timer.h>

#if defined(USE_STUD_SEM_LOCK)
Semaphore semaphore_lock(SemaphoreRef s)
{
	// Block the process if the semaphore is 0
	while (s <= 0) {
		syscall_sleep(TIMER_MSECS(100));
	}

	// Once a spot is open, decrement the semaphore
	return atomic_add(&s, -1);
}
#else
#warning "Using BUILT-IN semaphore_lock(), not yours"
#endif

#if defined(USE_STUD_SEM_UNLOCK)
Semaphore semaphore_unlock(SemaphoreRef s)
{
	// Increment the semaphore once a spot opens up
	return atomic_add(&s, 1);
}
#else
#warning "Using BUILT-IN semaphore_unlock(), not yours"
#endif

#if defined(USE_STUD_MUTEX_LOCK)
Mutex mutex_lock(MutexRef m)
{
	// Block the process while the mutex is LOCKED
	Mutex prevState;
	while((prevState = atomic_swap(&m, LOCKED)) == LOCKED) {
		syscall_sleep(TIMER_MSECS(100));
	}

	// Once the spot opens up, lock it again
	return atomic_lock(&m);
}
#else
#warning "Using BUILT-IN mutex_lock(), not yours"
#endif

#if defined(USE_STUD_MUTEX_UNLOCK)
Mutex mutex_unlock(MutexRef m)
{
	// Unlock the mutex once the process is done
	return atomic_unlock(&m);
}
#else
#warning "Using BUILT-IN mutex_unlock(), not yours"
#endif

#if defined(USE_STUD_BARRIER_WAIT)
void barrier_wait(BarrierRef b)
{
	// Decrement the barrier
	atomic_add(&b, -1);

	// Block the process until everything else has caught up
	while (b > 0) {
		syscall_sleep(TIMER_MSECS(100));
	}
}
#else
#warning "Using BUILT-IN mutex_unlock(), not yours"
#endif

/////////////////////////////////////////////////////
//
// EVERYTHING BELOW HERE IS USED FOR TESTING.
// CHANGE AS YOU SEE FIT.
//
/////////////////////////////////////////////////////

#define GET_PARAM(r) int r; asm volatile("mv %0, a0" : "=r"(r));

// Unlocked is zero, locked is non-zero
Mutex my_mutex = 0;
// Test a common mutex (my_mutex above). Only one
// mutex must be in the critical section at a time.
static void mutex_proc()
{
	GET_PARAM(r);
	mutex_lock(my_mutex);
	printf("Mutex %lu: Acquired my_mutex.\n", r);
	printf("Mutex %lu: IN CRITICAL SECTION.\n", r);
	int i = 0;
	do {
		syscall_sleep(TIMER_MSECS(10));
	} while (i++ < 100);
	mutex_unlock(my_mutex);
	printf("Mutex %lu: Released my_mutex.\n", r);
}

// Generate a random number from an HPET
static long randint(long min, long max)
{
	unsigned long tm = get_time();
	// Get some randomization so all don't sleep the same amount of time.
	tm %= 10000;
	tm *= 10000000;
	tm += get_time() % 123456789L;
	return min + tm % (max - min + 1);
}

// Set up the barrier to wait for 5 processes
Barrier barrier = 5;
static void barrier_proc()
{
	GET_PARAM(r);
	unsigned long tm = (unsigned long)randint(10000000, 50000000);
	printf("Barrier %lu: Sleeping a random amount of time: %lu.\n", r, tm);
	syscall_sleep(tm);
	// Processes should stop here until all are waiting.
	printf("Barrier %lu: Sleep done, waiting on barrier (%d spots left).\n", r, barrier-1);
	barrier_wait(barrier);
	// You will notice that all processes will run these nearly immediately
	// after the barrier is freed.
	printf("Barrier %lu: Continuing after barrier.\n", r);
}

// Semaphore test that causes a deadlock
static void deadlock_sem()
{
	Semaphore s = 1;
	printf("Locking semaphore.\n");
	Semaphore prev = semaphore_lock(s);
	printf("Semaphore locked previous: %d, now %d.\nUnlocking semaphore.\n", prev, s);
	prev = semaphore_unlock(s);
	printf("Semaphore unlocked previous: %d, now %d.\n", prev, s);

	printf("Trying deadlock semaphore...\n");
	prev = semaphore_lock(s);
	// We lock here again, but the semaphore is already held, thus creating a deadlock.
	prev = semaphore_lock(s);
	// We should never get here, since we've locked a semaphore twice so it would be
	// waiting.
	printf("Semaphore unlocked previous: %d, now %d.\n", prev, s);
}

// Typing test at the shell will invoke the following.
void test_process()
{
	// Unlock the mutex
	my_mutex = 0; 
	// Set barrier to 5 members.
	barrier = 5;

	// This process shows what happens if we're in a deadlock. It will spin inside of semaphore_lock().
	// Since no other process up()s the semaphore, this will never be able to acquire the lock.
	add_process(deadlock_sem, "deadlock_sem");

	// Create 5 mutex running processes that compete for the same mutex. You should see one
	// mutex process lock, run, and then unlock. As soon as it unlocks, the next should grab
	// the mutex and run. If the mutex is locking properly, no two mutices will enter their
	// critical sections simultaneously.
	for (int i = 0;i < 5;i++) {
		char s[] = "mutexX";
		s[5] = i + '0';
		add_process(mutex_proc, s, 1, CPU_MODE_USER, 0, PROCESS_DEFAULT_STACK_SIZE, i);
	}

	// Create five processes to wait on a barrier. The barrier is initialized to 5, so all
	// processes will sleep for a random amount of time. When they all reach the barrier, they
	// will all continue processing.
	for (int i = 0;i < 5;i++) {
		char s[] = "barrierX";
		s[7] = i + '0';
		add_process(barrier_proc, s, 1, CPU_MODE_USER, 0, PROCESS_DEFAULT_STACK_SIZE, i);
	}
}

