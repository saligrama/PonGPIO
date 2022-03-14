#include "rpi.h"

// of all the code/data in a pi binary file.
extern char __heap_start__;

// track if initialized.
static int init_p;

// this is the minimum alignment: must always
// roundup to at least sizeof(union align)
union align {
        double d;
        void *p;
        void (*fp)(void);
};

static char *heap;
static char *heap_start;
static char *heap_end;

// some helpers
static inline uint32_t max_u32(uint32_t x, uint32_t y) {
    return x > y ? x : y;
}
static inline unsigned is_aligned(unsigned x, unsigned n) {
    return (((x) & ((n) - 1)) == 0);
}
static inline unsigned is_aligned_ptr(void *ptr, unsigned n) {
    return is_aligned((unsigned)ptr, n);
}
static inline unsigned is_pow2(unsigned x) {
    return (x & -x) == x;
}
static inline unsigned roundup(unsigned x, unsigned n) {
    assert(is_pow2(n));
    return (x+(n-1)) & (~(n-1));
}


// symbol created by libpi/memmap, placed at the end

/*
 * address of returned pointer should be a multiple of
 * alignment. 
 */
void *kmalloc_aligned(unsigned nbytes, unsigned alignment) {
    assert(nbytes);
    demand(init_p, calling before initialized);
    demand(is_pow2(alignment), assuming power of two);

    if(alignment <= 4)
        alignment = 4;
    
    char *ret = (char *) roundup((unsigned) heap, alignment);
    heap = ret + roundup(nbytes, 4);
    demand(is_aligned_ptr(ret, alignment), heap roundup is not aligned);
    demand(heap < heap_end, out of room);
    return ret;
}

/*
 * Return a memory block of at least size <nbytes>
 * Notes:
 *  - There is no free, so is trivial: should be just 
 *    a few lines of code.
 *  - The returned pointer should always be 4-byte aligned.  
 *    Easiest way is to make sure the heap pointer starts 4-byte
 *    and you always round up the number of bytes.  Make sure
 *    you put an assertion in.  
 */
void *kmalloc(unsigned nbytes) { 
    return kmalloc_aligned(nbytes, 4);
}

/*
 * alternative to <kmalloc_init>:  set the start 
 * of the heap to <addr>
 */
void kmalloc_init_set_start(unsigned _addr, unsigned max_nbytes) {
    demand(!init_p, already initialized);
    init_p = 1;
    
    heap = (char *) _addr;
    heap_start = heap;
    heap_end = heap + max_nbytes;
    demand(is_aligned_ptr(heap, 4), heap start is not four-byte aligned);
}

/*
 * One-time initialization, called before kmalloc 
 * to setup heap. 
 *    - should be just a few lines of code.
 *    - sets heap pointer to the location of 
 *      __heap_start__.   print this to make sure
 *      it makes sense!
 *    - set the max size to 2mb
 */
void kmalloc_init(void) {
    if(init_p)
        return;

    // call kmalloc_init_set_start w/ right values.
    heap_start = &__heap_start__;
    heap = &__heap_start__;
    demand(is_aligned_ptr(heap, 4), heap start is not four-byte aligned);

    kmalloc_init_set_start((uintptr_t) heap_start, 128 << 20);
}


/* 
 * free all allocated memory: reset the heap 
 * pointer back to the beginning.
 */
void kfree_all(void) {
    heap = heap_start;
}

// return pointer to the first free byte.
// for the current implementation: the address <addr> of any
// allocated block satisfies: 
//    assert(<addr> < kmalloc_heap_ptr());
// 
void *kmalloc_heap_ptr(void) {
    return heap;
}
