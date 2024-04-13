#pragma once

#include <stddef.h>
#include <stdint.h>
//#include <uchar.h> only needed for char16_t

typedef uint8_t      u8;
typedef int8_t       i8;
typedef uint16_t    u16;
typedef int16_t     i16;
//typedef char16_t    c16; windows thing
typedef int32_t     b32;
typedef int32_t     i32;
typedef uint32_t    u32;
typedef uint64_t    u64;
typedef float       f32;
typedef double      f64;
typedef uintptr_t  uptr;
typedef intptr_t   iptr;
typedef char       byte;
typedef ptrdiff_t  size;
typedef size_t    usize;

#define szeof(x)      (size)sizeof(x) //TODO: replace unsigned size of in main program and use thisn one
#define algnof(x)     (size)_Alignof(x)
#define countof(a)    (size)(szeof(a) / szeof(*(a)))
#define lengthof(s)   (countof(s) - 1)
#define new(a, t, n)  (t *)alloc(a, sizeof(t), _Alignof(t), n)

#define asert(c)     while(!(c)) __builtin_unreachable()


//Composite types
typedef struct {
    i32 *data;
    size  len;
    size  cap;
} i32s;