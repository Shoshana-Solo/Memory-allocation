#include <stdlib.h>
#include "memory_allocation.h"
#include <stddef.h>
#include <assert.h>
#include <stdio.h>

#define ALIGNMENT 8

typedef struct MemoryAllocator
{
   size_t size;
   void *payLoad;

}MemoryAllocator;

/*--------------------Auxiliary Functions--------------------*/

void insertMataData(void* payLoad , size_t size)
{
    *(size_t*)(payLoad) = size;
}

size_t getAlignedSize(size_t size)
{
    return (size % ALIGNMENT) ? size + ALIGNMENT - (size % ALIGNMENT) : size;
}

size_t getNextBlockSize(void* memoryPool)
{
    return *(size_t*)((char*)memoryPool + *(size_t*)memoryPool);
}

void slice(void* tempPayLoad , size_t size)
{
    size_t extraSize = (*(size_t*)tempPayLoad - 1) - size - sizeof(size_t);
    insertMataData(tempPayLoad , size + 1);
    tempPayLoad += size + sizeof(size_t);
    insertMataData(tempPayLoad , extraSize);
}

void* findFreeBlock(void * tempPayLoad , size_t size)
{
    while (tempPayLoad)
    {
        if (*(size_t *) tempPayLoad & (size_t) 1 | *(size_t *) tempPayLoad < size)
        {
            tempPayLoad += getAlignedSize(sizeof(size_t) + (*(size_t *) tempPayLoad) - 1);
        }
    }

    return *(size_t*)tempPayLoad;
}


int isFree(void* payLoad)
{
    return !(*(size_t*)payLoad & (size_t)1);
}

size_t getMax(void* memoryPool, size_t max)
{
    return (*(size_t*)memoryPool > max) ? *(size_t*)memoryPool : max;
}

void tryToMerge(void* payLoad, size_t* max)
{
    if (isFree(getNextBlockSize(payLoad)) && (char*)payLoad + *(size_t*)payLoad)
    {
        *(size_t*)payLoad += getNextBlockSize(payLoad);
        *max = getMax(payLoad , max);
    }

    else
    {
        *max = getMax(payLoad , max);
        payLoad += *(size_t*)payLoad;
    }
}



/*-----------------------------------------------------------*/


/*-----------------------API Functions-----------------------*/

/* memoryPool is a ptr to an already-existing large memory block */
MemoryAllocator* memoryAllocatorInit(void* memoryPool , size_t size)
{
    assert(memoryPool);

    MemoryAllocator* allocator = malloc (sizeof(MemoryAllocator));
    allocator -> size = (size - (size % ALIGNMENT));
    allocator -> payLoad = memoryPool;
    insertMataData(allocator -> payLoad , allocator -> size);
    return allocator;
}

/* Returns a ptr to the memoryPool */
void* memoryAllocatorRelease(MemoryAllocator* allocator)
{
    void *memoryPool = allocator -> payLoad;

    if(!allocator)
    {
        return NULL;
    }

    free(allocator);
    return memoryPool;
}

void* memoryAllocatorAllocate(MemoryAllocator* allocator,size_t size)
{
    assert(allocator);

    void *tempPayLoad = allocator -> payLoad;
    memoryAllocatorOptimize(allocator);

    while (tempPayLoad)
    {
        if(*(size_t*)tempPayLoad & (size_t)1 | *(size_t*)tempPayLoad < size)
        {
            tempPayLoad += getAlignedSize(sizeof(size_t) + (*(size_t*)tempPayLoad) - 1);
        }

        else
        {
            if(*(size_t*)tempPayLoad == size)
            {
                insertMataData(tempPayLoad , size + 1);
            }

            else if(*(size_t*)tempPayLoad > size)
            {
                slice(tempPayLoad , size);
            }

            return tempPayLoad;
        }
    }

    return NULL;

}


/* Merge the next adjacent block is free */
void memoryAllocatorFree(MemoryAllocator* allocator, void* ptr)
{
    assert(allocator);

    *(size_t*)(ptr) -= 1;
    size_t nextSize = *((char*)ptr + *(size_t*)(ptr) + sizeof(size_t));
    if(!(nextSize & (size_t)1))
    {
        *(size_t*)(ptr) += nextSize + sizeof(size_t);
    }
}

/* Merges all adjacent free blocks, and returns the size of largest free block */
size_t memoryAllocatorOptimize(MemoryAllocator* allocator)
{
    assert(allocator);
    void *tempPayLoad = allocator -> payLoad;
    size_t max = 0;

    while (tempPayLoad)
    {
        if(isFree(tempPayLoad))
        {
            tryToMerge(tempPayLoad, &max);
        }

        else
        {
            tempPayLoad += *(size_t*)tempPayLoad;
        }
    }

    return max;
}


/*-----------------------------------------------------------*/
