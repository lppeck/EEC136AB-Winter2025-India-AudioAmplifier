// shared_memory.h
#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

// Shared memory structure
typedef struct {
    volatile int volume;       // The volume value
    volatile int other_data;   // Placeholder for any other data
    volatile int flag;         // A flag to indicate whether the volume was updated
} SharedMemory;

// Define shared memory pointer to the region starting at 0x20060000
#define SHARED_MEMORY ((SharedMemory*) 0x20060000)

#endif // SHARED_MEMORY_H
