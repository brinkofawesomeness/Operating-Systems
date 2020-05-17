// fs1.cpp
// File system routines lab 1
// Conner Brinkley
// 04.13.2020

// #define USE_STUD_FS_READ
// #define USE_STUD_FS_TRAVERSE_PATH

#include <types.h>
#include <fs.h>
#include <slab.h>
#include <vblock.h>
#include <syscall.h>
#include <printf.h>
#include <programs.h>
#include <util.h> // for strcmp and memcpy

#if defined(USE_STUD_FS_READ)
i32 fs_read(u32 fs_number,		// This is the block system 1 - 8
            const char *path,   // This is the file path can only be absolute
			char *buffer,		// This is the buffer to store the block data
			u32 starting_byte,  // This is the starting byte to read
			u32 max_size		// This is the maximum number of bytes that the buffer can take
			)
{
	i32 num_bytes_actually_read = 0;

	// The num_bytes_actually_read is a signed integer. If you return a negative number,
	// that will be considered an error with the number itself being the error code.

	return num_bytes_actually_read;
}
#else
#warning "Using BUILT-IN fs_read, not yours!"
#endif

#if defined(USE_STUD_FS_TRAVERSE_PATH)
InodeCache *fs_traverse_path(u32 fs_number,		// The cache 1-8
                             InodeCache *cwd,   // The current working directory
							 const char *path   // The path to traverse.
							 )
{
	InodeCache *ret = nullptr;

	// The cwd InodeCache pointer is irrelevant if path is absolute.

	// If you return nullptr, this tells the OS that the file or path was not found.
	// The InodeCache structure has the following fields:
	// Inode inode   				--This is a copy of the Inode structure
	// DirEntry this_entry;			--This relates the entry's name and Inode number (not structure)
	// List<InodeCache> entries;	--This contains all children entries (inodes)
	// InodeCache *parent;			--This is the parent to this directory (or nullptr if root)

	return ret;
}
#else
#warning "Using BUILT-IN fs_traverse_path, not yours!"
#endif


// Typing test at the shell will invoke the following.
void test_process()
{
	const int BTR = 4096;
	char *buf = new char[BTR];
	auto bytes = syscall_fs_read(8, "/samples/fict.txt", buf, 44000, BTR);
	printf("Got %d bytes, printed\n\"%.4096s\"\n", bytes, buf);
	delete [] buf;
}

