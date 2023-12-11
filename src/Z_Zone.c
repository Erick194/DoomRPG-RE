/* Z_Zone.c */

//-----------------------------------------------------------------------------
//
// DESCRIPTION:
//      Zone Memory Allocation. Neat.
//
// Based on the doom64 Ex code by Samuel Villarreal
// https://github.com/svkaiser/Doom64EX/blob/master/src/engine/zone/z_zone.cc
//-----------------------------------------------------------------------------

#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "DoomRPG.h"
#include "Z_Zone.h"

static SDL_malloc_func malloc_func;
static SDL_calloc_func calloc_func;
static SDL_realloc_func realloc_func;
static SDL_free_func free_func;

#define ZONEID    0x1d4a12

typedef struct memblock_s memblock_t;

struct memblock_s {
    int id; // = ZONEID
    int size;
    memblock_t* prev;
    memblock_t* next;
};

// Linked list of allocated blocks for each tag type

static memblock_t* allocated_blocks[1];

//
// Z_InsertBlock
// Add a block into the linked list for its type.
//

static void Z_InsertBlock(memblock_t* block) {
    block->prev = NULL;
    block->next = allocated_blocks[0];
    allocated_blocks[0] = block;

    if (block->next != NULL) {
        block->next->prev = block;
    }
}

//
// Z_RemoveBlock
// Remove a block from its linked list.
//

static void Z_RemoveBlock(memblock_t* block) {
    // Unlink from list
    if (block->prev == NULL) {
        allocated_blocks[0] = block->next;    // Start of list
    }
    else {
        block->prev->next = block->next;
    }

    if (block->next != NULL) {
        block->next->prev = block->prev;
    }
}

//
// Z_Init
//

void Z_Init(void) {
    SDL_memset(allocated_blocks, 0, sizeof(allocated_blocks));

    //SDL_GetMemoryFunctions(&malloc_func, &calloc_func, &realloc_func, &free_func);
    //SDL_SetMemoryFunctions(Z_Malloc, Z_Calloc, Z_Realloc, Z_Free);
}


//
// Z_Free
//

void SDLCALL Z_Free(void* ptr) {
    memblock_t* block;

    block = (memblock_t*)((byte*)ptr - sizeof(memblock_t));

    if (block->id != ZONEID) {
        DoomRPG_Error("Z_Free: freed a pointer without ZONEID");
    }

    Z_RemoveBlock(block);

    // Free back to system
    free_func(block);
}

//
// Z_Malloc
//

void* SDLCALL Z_Malloc(size_t size) {
    memblock_t* newblock;
    unsigned char* data;
    void* result;

    // Malloc a block of the required size
    newblock = NULL;

    newblock = (memblock_t*)malloc_func(sizeof(memblock_t) + size);

    if (!newblock) {
        DoomRPG_Error("Z_Malloc: failed on allocation of %u bytes", size);
        return NULL;
    }

    // Hook into the linked list for this tag type
    newblock->id = ZONEID;
    newblock->size = (int)size;

    Z_InsertBlock(newblock);

    data = (unsigned char*)newblock;
    result = data + sizeof(memblock_t);

    return result;
}

//
// Z_Realloc
//

void* SDLCALL Z_Realloc(void* ptr, size_t size) {
    memblock_t* block;
    memblock_t* newblock;
    size_t origsize;
    unsigned char* data;
    void* result;

    if (!ptr) {
        return Z_Malloc(size);
    }

    if (size == 0) {
        Z_Free(ptr);
        return NULL;
    }

    block = (memblock_t*)((byte*)ptr - sizeof(memblock_t));

    newblock = NULL;

    if (block->id != ZONEID) {
        DoomRPG_Error("Z_Realloc: Reallocated a pointer without ZONEID");
    }

    Z_RemoveBlock(block);

    origsize = block->size;
    block->next = NULL;
    block->prev = NULL;

    newblock = (memblock_t*)realloc_func(block, sizeof(memblock_t) + size);
    if (!newblock) {
        DoomRPG_Error("Z_Realloc: failed on allocation of %u bytes", size);
        return NULL;
    }

    newblock->id = ZONEID;
    newblock->size = size;

    Z_InsertBlock(newblock);

    data = (unsigned char*)newblock;
    result = data + sizeof(memblock_t);

    // zero out newly created memory
    if (origsize < size) {
        SDL_memset((char*)(result) + origsize, 0, size - origsize);
    }

    return result;
}

//
// Z_Calloc
//

void* SDLCALL Z_Calloc(size_t nmemb, size_t size) {
    return SDL_memset(Z_Malloc(nmemb *size), 0, nmemb *size);
}

//
// Z_FreeMemory
//

int Z_FreeMemory(void) {
    int bytes = 0;
    memblock_t* block;

    for (block = allocated_blocks[0]; block != NULL; block = block->next) {
        bytes += block->size;
    }
    return bytes;
}