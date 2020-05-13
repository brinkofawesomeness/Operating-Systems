// bd1.cpp
// Block routines 1
// Conner Brinkley
// 03.15.2020

#define USE_STUD_BLOCK_READ
#define USE_STUD_BLOCK_SUBMIT

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

#if defined(USE_STUD_BLOCK_READ)
u8 virtio_blk_read(VirtioBlkDevice *blkdev,
		           Process *p,
				   u8 *buffer,
				   u64 size,
				   u64 offset)
{
	// Check to make sure the block device is connected
	if ((void *) blkdev->addr == nullptr) return STATUS_UNKNOWN_ERROR;
	
	// Create a new request
	VirtioBlkRequest *rq = new VirtioBlkRequest;
	rq->header.type = VIRTIO_BLK_T_IN;
	rq->header.reserved = 0;
	rq->header.sector = offset / 512;
	rq->data = buffer;
	rq->watcher = p;
	rq->max_data_size = size;
	
	// Submit the request and return
	virtio_blk_submit(blkdev, rq);
	return 0;
}
#else
#warning "Using built-in virtio_blk_read, NOT yours!"
#endif

#if defined(USE_STUD_BLOCK_SUBMIT)
void virtio_blk_submit(VirtioBlkDevice *blkdev,
		               VirtioBlkRequest *rq)
{
	// Set the status to running and calculate the ring head
	rq->status = STATUS_RUNNING;
	rq->ring_head = (blkdev->desc_idx + 1) % blkdev->max_idx;

	// Set the first descriptor for the header
	u16 index = rq->ring_head;
	blkdev->vqueue.desc[index].addr = (u64) &(rq->header);
	blkdev->vqueue.desc[index].len = (u32) sizeof(rq->header);
	blkdev->vqueue.desc[index].flags = VIRTQ_DESC_F_NEXT;
	blkdev->vqueue.desc[index].next = (index + 1) % blkdev->max_idx;

	// Set the second descriptor for the data buffer
	index = blkdev->vqueue.desc[index].next;
	blkdev->vqueue.desc[index].addr = (u64) rq->data;
	blkdev->vqueue.desc[index].len = (u32) rq->max_data_size;
	blkdev->vqueue.desc[index].flags = VIRTQ_DESC_F_NEXT;
	blkdev->vqueue.desc[index].flags |= VIRTQ_DESC_F_WRITE;
	blkdev->vqueue.desc[index].next = (index + 1) % blkdev->max_idx;

	// Set the third descriptor for the status
	index = blkdev->vqueue.desc[index].next;
	blkdev->vqueue.desc[index].addr = (u64) &(rq->status);
	blkdev->vqueue.desc[index].len = (u32) sizeof(rq->status);
	blkdev->vqueue.desc[index].flags = VIRTQ_DESC_F_WRITE;
	blkdev->vqueue.desc[index].next = 0;

	// Reset the index, store ring head in first available ring, increment available ring, and add the request
	blkdev->desc_idx = index;
	blkdev->vqueue.avail.ring[blkdev->vqueue.avail.idx] = rq->ring_head;
	blkdev->vqueue.avail.idx = (blkdev->vqueue.avail.idx + 1) % blkdev->max_idx;
	blkdev->requests.add(rq);
	
	// Signal the device and set the process to waiting
	((volatile VirtioMMIODevice *) blkdev->addr)->queue_notify = 0;
	rq->watcher->state = ProcessState::WAITING;
}
#else
#warning "Using built-in virtio_blk_submit, NOT yours!"
#endif


// Typing test at the shell will invoke the following.
void test_process()
{
	// Test 1
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

	// Test 2
	buffer = new char[8192];
	bytes = fs_read(8, "/samples/fict.txt", buffer, 0, 8192);

	if (bytes > 0) {
		buffer[bytes] = 0;
		printf("Test read %d bytes.\n", bytes);
		printf("%s\n\n", buffer);
	}
	else {
		printf("Error reading fict.txt\n");
	}

	delete [] buffer;

	// Test 3
	buffer = new char[2048];
	bytes = fs_read(8, "/test/test.data", buffer, 0, 2048);

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

