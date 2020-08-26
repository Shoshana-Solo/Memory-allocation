#include <glob.h>

typedef struct MemoryAllocator MemoryAllocator;

MemoryAllocator* memoryAllocatorInit(void* memoryPool,size_t size);

/* Returns a ptr to the memoryPool */
void* memoryAllocatorRelease(MemoryAllocator* allocator);

void* memoryAllocatorAllocate(MemoryAllocator* allocator,size_t size);

/* Merge the next adjacent block is free */
void memoryAllocatorFree(MemoryAllocator* allocator, void* ptr);

/* Merges all adjacent free blocks, and returns the size oflargest free block */
size_t memoryAllocatorOptimize(MemoryAllocator*allocator);