
#ifndef INTS_DOT_H
#define INTS_DOT_H

    //
    // ints.h - just some ints that are nice to have
    //
    // Author   - Fletcher M
    //
    // Created  - 21/03/2025
    // Modified - 11/07/2025
    //

    #include <stdint.h>

    typedef uint64_t   u64;
    typedef uint32_t   u32;
    typedef uint16_t   u16;
    typedef uint8_t    u8;

    typedef int64_t    s64;
    typedef int32_t    s32;
    typedef int16_t    s16;
    typedef int8_t     s8;

    typedef u32        bool32;

    typedef float      f32;
    typedef double     f64;

    #define True       (0 == 0)
    #define False      (0 != 0)

#endif // INTS_DOT_H
