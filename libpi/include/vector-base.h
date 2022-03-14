#ifndef __VECTOR_BASE_SET_H__
#define __VECTOR_BASE_SET_H__
#include "libc/bit-support.h"
#include "asm-helpers.h"

/*
 * vector base address register:
 *   3-121 --- let's us control where the exception jump table is!
 *
 * defines: 
 *  - vector_base_set  
 *  - vector_base_get
 */

static inline void *vector_base_get(void) {
    void *vbase;
    asm volatile ("MRC p15, 0, %0, c12, c0, 0" : "=r" (vbase) :);
    return vbase;
}

// set the vector register to point to <vector_base>.
// must: 
//    - check that it satisfies the alignment restriction.
static inline void vector_base_set(void *vector_base) {
    if ((unsigned) vector_base & 0b11111)
        panic("Vector base must be 32-byte aligned\n");
    asm volatile ("MCR p15, 0, %0, c12, c0, 0" :: "r" (vector_base));
    assert(vector_base_get() == vector_base);
}
#endif
