#pragma once

#include <stddef.h>
#include <stdint.h>

//NOTE: LINUX Platform Includes
#include <stdlib.h>
#include <unistd.h>
//TODO: eliminate depedency on string.h
#include <string.h>

//#include <uchar.h> only needed for char16_t

typedef uint8_t      u8;
typedef int8_t       i8;
typedef uint16_t    u16;
typedef int16_t     i16;
//typedef char16_t    c16; windows string type
typedef int32_t     b32;
typedef int32_t     i32;
typedef uint32_t    u32;
typedef int64_t     i64;
typedef uint64_t    u64;
typedef float       f32;
typedef double      f64;
typedef uintptr_t  uptr;
typedef intptr_t   iptr;
typedef char       byte;
typedef ptrdiff_t  size;
typedef size_t    usize;
#define true  1
#define false 0

#define szeof(x)      (size)sizeof(x) //TODO: replace unsigned size of in main program and use this one
#define algnof(x)     (size)_Alignof(x)
#define countof(a)    (size)(szeof(a) / szeof(*(a)))
#define lengthof(s)   (countof(s) - 1) // Length of literal C String
#define new(...)            newx(__VA_ARGS__,new4,new3,new2)(__VA_ARGS__)
#define newx(a,b,c,d,e,...) e
#define new2(a, t)          (t *)alloc(a, szeof(t), algnof(t), 1, 0) //a is for arena
#define new3(a, t, n)       (t *)alloc(a, szeof(t), algnof(t), n, 0) //a is for arena
#define new4(a, t, n, f)    (t *)alloc(a, szeof(t), algnof(t), n, f) //a is for arena

#define asert(c)     while(!(c)) __builtin_unreachable() //NOTE: Only Linux

//Flags for core functions
#define NONE   0x0
#define NOZERO 0x1

//Core OS functions
static void osfail(void)
{
    _exit(1);
}

//NOTE: fd = 1 is stdout
static i32 osread(i32 fd, u8 *buf, i32 cap)
{
    return (i32)read(fd, buf, cap);
}

static b32 oswrite(i32 fd, u8 *buf, i32 len)
{
    for (i32 off = 0; off < len;) {
        i32 r = (i32)write(fd, buf+off, len-off);
        if (r < 1) {
            return 0;
        }
        off += r;
    }
    return true;
}

//Composite types

// Memory 
typedef struct {
    byte *beg;
    byte *end; //NOTE: the end is not an addressable byte
} arena;

static void oom(void) {
    static const char msg[] = "out of memory\n";
    oswrite(2, (u8 *)msg, lengthof(msg));
    osfail();
}

static arena newarena(size cap) {
    assert(cap >= 0);
    arena a = {0}; //NOTE: this function defaults to 0
    a.beg = malloc((usize) cap);
    a.end = a.beg ? a.beg + cap : 0;
    return a;
}

static void freearena(arena a) {
    assert(a.beg != NULL);
    free(a.beg);
}

// Linear Allocator
// TODO: remove commented out version, kept as backup for alloc understanding
/* __attribute((malloc, alloc_size(2, 4), alloc_align(3))) // malloc for guarentee no alias + optimizations
static void *alloc(arena *a, size objsize, size align, size count, u32 flags)
{
    assert(count >= 0);
    size avail = a->end - a->beg;
    size padding = -(uptr)a->beg & (align - 1); // align is pow of 2, doing mod on remainder of memory by the object alignment
    if (count > (avail - padding) / objsize) {
        oom();
    }
    size total = count * objsize;
    void *p = a->beg + padding;
    a->beg += padding + total;
    return flags & NOZERO ? p : memset(p, 0, (usize) total);
} */

__attribute((malloc, alloc_size(2, 4), alloc_align(3))) // malloc for guarentee no alias + optimizations
static byte *alloc(arena *a, size objsize, size align, size count, u32 flags)
{
    assert(count >= 0);
    size pad = (uptr)a->end & (align - 1); // align is pow of 2, doing mod on remainder of memory by the object alignment
    assert(count < (a->end - a->beg - pad) / objsize);  // oom TODO: call oom for non debug builds
    a->end -= objsize*count + pad;
    return flags & NOZERO ? a->end : memset(a->end, 0, (usize) count * objsize);
}

static arena afromarena(arena *a, size cap) { //TODO: add flag support?
    assert(cap > 0);
    arena n;
    n.beg = alloc(a, cap, algnof(u8), 1, NONE); //NOTE: is my new arena byte aligned?
    n.end = n.beg + cap;
    return n;
}

// UTF-8 Strings
#define s8(s) (s8){(u8 *)s, lengthof(s)} // takes a literal C string and returns a pointer u8 string
typedef struct {
    u8  *data;
    size len;
} s8;

static s8   s8span(u8 *, u8 *);
static b32  s8equals(s8, s8);
static size s8compare(s8, s8);
static u64  s8hash(s8);
static s8   s8trim(s8);
static s8   s8clone(s8, arena *);


//Arena backed arrays
typedef struct {
    s8   *data;
    size len;
    size cap;  
} s8a;

typedef struct {
    i32 *data;
    size  len;
    size  cap;
} i32a;