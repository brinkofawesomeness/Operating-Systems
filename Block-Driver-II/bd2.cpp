// bd2.cpp
// Block routines 2
// Conner Brinkley
// 03.29.2020

#define USE_STUD_BLOCK_PENDING

#include <types.h>
#include <vblock.h>
#include <virtio.h>
#include <vqueue.h>
#include <fs.h>
#include <slab.h>
#include <sched.h>
#include <syscall.h>
#include <printf.h>
#include <programs.h>

#if defined(USE_STUD_BLOCK_PENDING)
void virtio_blk_pending(int num)
{
	// Error check the device's number and address
	if (num < 1 || num > BLOCK_DEVICES) return;
	if (g_VirtioBlockDevices[num - 1].addr == 0) return;

	// Acknowledge the interrupt
	VirtioBlkDevice *dev = (VirtioBlkDevice *) &g_VirtioBlockDevices[num - 1];
	VirtioMMIODevice *mmio = (VirtioMMIODevice *) dev->addr;
	mmio->interrupt_ack = mmio->interrupt_status;

	// Handle the interrupt
	ListElem <VirtioBlkRequest> *req;
	while (dev->used_idx != dev->vqueue.used.idx) {
	
		// Get the descriptor index
		u32 index = dev->vqueue.used.ring[dev->used_idx].id;

		// Find the ID in the requests
		if (!dev->requests.empty()) {
			req = dev->requests.head;
			while (req != nullptr) {
			
				// Once we find it, set the watcher's status and state before removing the request
				if (req->data->ring_head == index) {
					req->data->watcher->context.regs[GpA0] = req->data->status;
					req->data->watcher->state = ProcessState::RUNNING;
					dev->requests.remove(req->data);
					delete req->data;
					break;
				}
				req = req->next;
			}
		}

		// Onto the next
		dev->used_idx = (dev->used_idx + 1) % dev->max_idx;
	}
}
#else
#warning "Using built-in virtio_blk_pending, NOT yours!"
#endif



// Typing test at the shell will invoke the following.
void test_process()
{
	char *buffer = new char[65];
	i32 bytes;

	bytes = fs_read(8, "/cosc361/hello.txt", buffer, 0, 64);

	if (bytes > 0) {
		printf("Test read %d bytes\n", bytes);
		printf("%13s\n", buffer);
	}
	else {
		printf("Error reading README.txt\n");
	}

	delete [] buffer;

	buffer = new char[8192];
	bytes = fs_read(8, "/samples/fict.txt", buffer, 0, 8192);

	if (bytes > 0) {
		buffer[bytes] = 0;
		printf("Test read %d bytes.\n", bytes);
		printf("%s\n", buffer);
	}
	else {
		printf("Error reading fict.txt\n");
	}

	delete [] buffer;
}

