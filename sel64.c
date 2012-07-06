
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

//#include <wmmintrin.h>
#include <nmmintrin.h>

#include <sdsl/bitmagic.hpp>
#include <sdsl/util.hpp>
#include <sdsl/testutils.hpp>
#include <sdsl/int_vector.hpp>

using namespace sdsl;
using namespace std;

#define ONES_STEP_4 ( 0x1111111111111111ULL )
#define ONES_STEP_8 ( 0x0101010101010101ULL )
#define ONES_STEP_16 ( 1ULL << 0 | 1ULL << 16 | 1ULL << 32 | 1ULL << 48 )
#define MSBS_STEP_4 ( 0x8ULL * ONES_STEP_4 )
#define MSBS_STEP_8 ( 0x80ULL * ONES_STEP_8 )
#define MSBS_STEP_16 ( 0x8000ULL * ONES_STEP_16 )
#define INCR_STEP_8 ( 0x80ULL << 56 | 0x40ULL << 48 | 0x20ULL << 40 | 0x10ULL << 32 | 0x8ULL << 24 | 0x4ULL << 16 | 0x2ULL << 8 | 0x1 )
#define LEQ_STEP_8(x,y) ( ( ( ( ( (y) | MSBS_STEP_8 ) - ( (x) & ~MSBS_STEP_8 ) ) ^ (x) ^ (y) ) & MSBS_STEP_8 ) >> 7 )
#define ZCOMPARE_STEP_8(x) ( ( ( x | ( ( x | MSBS_STEP_8 ) - ONES_STEP_8 ) ) & MSBS_STEP_8 ) >> 7 )

/* the overflow table. this table is used to overflow the commulative byte sums for a specfic i */
const uint64_t PsOverflow1[] = {
    0x8080808080808080ULL,
    0x7f7f7f7f7f7f7f7fULL,
    0x7e7e7e7e7e7e7e7eULL,
    0x7d7d7d7d7d7d7d7dULL,
    0x7c7c7c7c7c7c7c7cULL,
    0x7b7b7b7b7b7b7b7bULL,
    0x7a7a7a7a7a7a7a7aULL,
    0x7979797979797979ULL,
    0x7878787878787878ULL,
    0x7777777777777777ULL,
    0x7676767676767676ULL,
    0x7575757575757575ULL,
    0x7474747474747474ULL,
    0x7373737373737373ULL,
    0x7272727272727272ULL,
    0x7171717171717171ULL,
    0x7070707070707070ULL,
    0x6f6f6f6f6f6f6f6fULL,
    0x6e6e6e6e6e6e6e6eULL,
    0x6d6d6d6d6d6d6d6dULL,
    0x6c6c6c6c6c6c6c6cULL,
    0x6b6b6b6b6b6b6b6bULL,
    0x6a6a6a6a6a6a6a6aULL,
    0x6969696969696969ULL,
    0x6868686868686868ULL,
    0x6767676767676767ULL,
    0x6666666666666666ULL,
    0x6565656565656565ULL,
    0x6464646464646464ULL,
    0x6363636363636363ULL,
    0x6262626262626262ULL,
    0x6161616161616161ULL,
    0x6060606060606060ULL,
    0x5f5f5f5f5f5f5f5fULL,
    0x5e5e5e5e5e5e5e5eULL,
    0x5d5d5d5d5d5d5d5dULL,
    0x5c5c5c5c5c5c5c5cULL,
    0x5b5b5b5b5b5b5b5bULL,
    0x5a5a5a5a5a5a5a5aULL,
    0x5959595959595959ULL,
    0x5858585858585858ULL,
    0x5757575757575757ULL,
    0x5656565656565656ULL,
    0x5555555555555555ULL,
    0x5454545454545454ULL,
    0x5353535353535353ULL,
    0x5252525252525252ULL,
    0x5151515151515151ULL,
    0x5050505050505050ULL,
    0x4f4f4f4f4f4f4f4fULL,
    0x4e4e4e4e4e4e4e4eULL,
    0x4d4d4d4d4d4d4d4dULL,
    0x4c4c4c4c4c4c4c4cULL,
    0x4b4b4b4b4b4b4b4bULL,
    0x4a4a4a4a4a4a4a4aULL,
    0x4949494949494949ULL,
    0x4848484848484848ULL,
    0x4747474747474747ULL,
    0x4646464646464646ULL,
    0x4545454545454545ULL,
    0x4444444444444444ULL,
    0x4343434343434343ULL,
    0x4242424242424242ULL,
    0x4141414141414141ULL,
    0x4040404040404040ULL
};

/* select table for the last byte */
const uint8_t Select2561[] = {
    0,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
    4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
    5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
    4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
    6,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
    4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
    5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
    4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
    7,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
    4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
    5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
    4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
    6,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
    4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
    5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
    4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,

    0,0,0,1,0,2,2,1,0,3,3,1,3,2,2,1,
    0,4,4,1,4,2,2,1,4,3,3,1,3,2,2,1,
    0,5,5,1,5,2,2,1,5,3,3,1,3,2,2,1,
    5,4,4,1,4,2,2,1,4,3,3,1,3,2,2,1,
    0,6,6,1,6,2,2,1,6,3,3,1,3,2,2,1,
    6,4,4,1,4,2,2,1,4,3,3,1,3,2,2,1,
    6,5,5,1,5,2,2,1,5,3,3,1,3,2,2,1,
    5,4,4,1,4,2,2,1,4,3,3,1,3,2,2,1,
    0,7,7,1,7,2,2,1,7,3,3,1,3,2,2,1,
    7,4,4,1,4,2,2,1,4,3,3,1,3,2,2,1,
    7,5,5,1,5,2,2,1,5,3,3,1,3,2,2,1,
    5,4,4,1,4,2,2,1,4,3,3,1,3,2,2,1,
    7,6,6,1,6,2,2,1,6,3,3,1,3,2,2,1,
    6,4,4,1,4,2,2,1,4,3,3,1,3,2,2,1,
    6,5,5,1,5,2,2,1,5,3,3,1,3,2,2,1,
    5,4,4,1,4,2,2,1,4,3,3,1,3,2,2,1,

    0,0,0,0,0,0,0,2,0,0,0,3,0,3,3,2,
    0,0,0,4,0,4,4,2,0,4,4,3,4,3,3,2,
    0,0,0,5,0,5,5,2,0,5,5,3,5,3,3,2,
    0,5,5,4,5,4,4,2,5,4,4,3,4,3,3,2,
    0,0,0,6,0,6,6,2,0,6,6,3,6,3,3,2,
    0,6,6,4,6,4,4,2,6,4,4,3,4,3,3,2,
    0,6,6,5,6,5,5,2,6,5,5,3,5,3,3,2,
    6,5,5,4,5,4,4,2,5,4,4,3,4,3,3,2,
    0,0,0,7,0,7,7,2,0,7,7,3,7,3,3,2,
    0,7,7,4,7,4,4,2,7,4,4,3,4,3,3,2,
    0,7,7,5,7,5,5,2,7,5,5,3,5,3,3,2,
    7,5,5,4,5,4,4,2,5,4,4,3,4,3,3,2,
    0,7,7,6,7,6,6,2,7,6,6,3,6,3,3,2,
    7,6,6,4,6,4,4,2,6,4,4,3,4,3,3,2,
    7,6,6,5,6,5,5,2,6,5,5,3,5,3,3,2,
    6,5,5,4,5,4,4,2,5,4,4,3,4,3,3,2,

    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,
    0,0,0,0,0,0,0,4,0,0,0,4,0,4,4,3,
    0,0,0,0,0,0,0,5,0,0,0,5,0,5,5,3,
    0,0,0,5,0,5,5,4,0,5,5,4,5,4,4,3,
    0,0,0,0,0,0,0,6,0,0,0,6,0,6,6,3,
    0,0,0,6,0,6,6,4,0,6,6,4,6,4,4,3,
    0,0,0,6,0,6,6,5,0,6,6,5,6,5,5,3,
    0,6,6,5,6,5,5,4,6,5,5,4,5,4,4,3,
    0,0,0,0,0,0,0,7,0,0,0,7,0,7,7,3,
    0,0,0,7,0,7,7,4,0,7,7,4,7,4,4,3,
    0,0,0,7,0,7,7,5,0,7,7,5,7,5,5,3,
    0,7,7,5,7,5,5,4,7,5,5,4,5,4,4,3,
    0,0,0,7,0,7,7,6,0,7,7,6,7,6,6,3,
    0,7,7,6,7,6,6,4,7,6,6,4,6,4,4,3,
    0,7,7,6,7,6,6,5,7,6,6,5,6,5,5,3,
    7,6,6,5,6,5,5,4,6,5,5,4,5,4,4,3,

    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,
    0,0,0,0,0,0,0,5,0,0,0,5,0,5,5,4,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,
    0,0,0,0,0,0,0,6,0,0,0,6,0,6,6,4,
    0,0,0,0,0,0,0,6,0,0,0,6,0,6,6,5,
    0,0,0,6,0,6,6,5,0,6,6,5,6,5,5,4,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,
    0,0,0,0,0,0,0,7,0,0,0,7,0,7,7,4,
    0,0,0,0,0,0,0,7,0,0,0,7,0,7,7,5,
    0,0,0,7,0,7,7,5,0,7,7,5,7,5,5,4,
    0,0,0,0,0,0,0,7,0,0,0,7,0,7,7,6,
    0,0,0,7,0,7,7,6,0,7,7,6,7,6,6,4,
    0,0,0,7,0,7,7,6,0,7,7,6,7,6,6,5,
    0,7,7,6,7,6,6,5,7,6,6,5,6,5,5,4,

    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,
    0,0,0,0,0,0,0,6,0,0,0,6,0,6,6,5,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,
    0,0,0,0,0,0,0,7,0,0,0,7,0,7,7,5,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,
    0,0,0,0,0,0,0,7,0,0,0,7,0,7,7,6,
    0,0,0,0,0,0,0,7,0,0,0,7,0,7,7,6,
    0,0,0,7,0,7,7,6,0,7,7,6,7,6,6,5,

    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,
    0,0,0,0,0,0,0,7,0,0,0,7,0,7,7,6,

    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7
};

/* the currently fastest sse enhanced select */
inline uint32_t select64_1(uint64_t x, uint32_t i)
{
    uint64_t s = x, b;
    s = s-((s>>1) & 0x5555555555555555ULL);
    s = (s & 0x3333333333333333ULL) + ((s >> 2) & 0x3333333333333333ULL);
    s = (s + (s >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
    s = 0x0101010101010101ULL*s;
    b = (s+PsOverflow1[i]) & 0x8080808080808080ULL;
    int  byte_nr = __builtin_ctzll(b) >> 3;
    s <<= 8;
    i -= (s >> (byte_nr<<3)) & 0xFFULL;
    return (byte_nr << 3) + Select2561[((i-1) << 8) + ((x>>(byte_nr<<3))&0xFFULL) ];
}

/* a different sse enhanced select */
inline uint32_t select64_3(uint64_t x, uint32_t i)
{
    uint64_t s=x,n,b;
    n = (s>>1) & 0x7777777777777777ULL;
    s = s-n;
    n = (n>>1) & 0x7777777777777777ULL;
    s = s-n;
    n = (n>>1) & 0x7777777777777777ULL;
    s = s-n;
    s = (s + (s >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
    s = 0x0101010101010101ULL*s;
    b = (s+PsOverflow1[i]) & 0x8080808080808080ULL;
    int  byte_nr = __builtin_ctzll(b) >> 3;
    s <<= 8;
    i -= (s >> (byte_nr<<3)) & 0xFFULL;
    return (byte_nr << 3) + Select2561[((i-1) << 8) + ((x>>(byte_nr<<3))&0xFFULL) ];
}

/* th edefault non sse version of sdsl */
inline uint32_t select64_2(uint64_t x, uint32_t i)
{
    /*register*/ uint64_t s = x, b;  // s = sum
    s = s-((s>>1) & 0x5555555555555555ULL);
    s = (s & 0x3333333333333333ULL) + ((s >> 2) & 0x3333333333333333ULL);
    s = (s + (s >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
    s = 0x0101010101010101ULL*s;
    b = (s+PsOverflow1[i]);
    i = (i-1)<<8;
    if (b&0x0000000080000000ULL) // byte <=3
        if (b&0x0000000000008000ULL) //byte <= 1
            if (b&0x0000000000000080ULL)
                return    Select2561[(x&0xFFULL) + i];
            else
                return 8 +Select2561[(((x>>8)&0xFFULL)  + i - ((s&0xFFULL)<<8))&0x7FFULL];//byte 1;
        else//byte >1
            if (b&0x0000000000800000ULL) //byte <=2
                return 16+Select2561[(((x>>16)&0xFFULL) + i - (s&0xFF00ULL))&0x7FFULL];//byte 2;
            else
                return 24+Select2561[(((x>>24)&0xFFULL) + i - ((s>>8)&0xFF00ULL))&0x7FFULL];//byte 3;
    else//  byte > 3
        if (b&0x0000800000000000ULL) // byte <=5
            if (b&0x0000008000000000ULL) //byte <=4
                return 32+Select2561[(((x>>32)&0xFFULL) + i - ((s>>16)&0xFF00ULL))&0x7FFULL];//byte 4;
            else
                return 40+Select2561[(((x>>40)&0xFFULL) + i - ((s>>24)&0xFF00ULL))&0x7FFULL];//byte 5;
        else// byte >5
            if (b&0x0080000000000000ULL) //byte<=6
                return 48+Select2561[(((x>>48)&0xFFULL) + i - ((s>>32)&0xFF00ULL))&0x7FFULL];//byte 6;
            else
                return 56+Select2561[(((x>>56)&0xFFULL) + i - ((s>>40)&0xFF00ULL))&0x7FFULL];//byte 7;
    return 0;
}

/* a sse version */
inline uint32_t
select64_4(uint64_t x,uint32_t i)
{
    __m128i v;
    v= _mm_insert_epi64(v,x,0);
    const __m128i mask_notnull = _mm_setr_epi8(1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0); /*_mm_set1_epi8(0x01);*/
    const __m128i mask_popcnt =  _mm_setr_epi8(0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4);
    const __m128i mask_lo = _mm_set1_epi8(0x0F);
    /* split into high and low nibbles */
    __m128i lo = _mm_and_si128(v, mask_lo);
    __m128i hi = _mm_and_si128(_mm_srli_epi16(v, 4), mask_lo);
    /* perform popcnt for high and low nibbles */
    lo = _mm_shuffle_epi8(mask_popcnt, lo);
    hi = _mm_shuffle_epi8(mask_popcnt, hi);
    /* add nibble popcnts to 8bit popcnts */
    __m128i sum8 = _mm_add_epi8(lo, hi);

    /* create cummulative sum TODO: how to do this in the xmm register */
    uint8_t csum_8[16] = {0};
    uint64_t* csum8 = (uint64_t*)(&csum_8)+1;
    *csum8 = _mm_extract_epi64(sum8,0)*0x0101010101010101;
    sum8 = _mm_insert_epi64(sum8,*csum8,0);
    sum8 = _mm_add_epi8(sum8, mask_notnull); /* add one to all block sums so strcmp does not
                                                stop at 0 block */
    /* calc pos of block */
    __m128i range = _mm_set1_epi8(64);
    range = _mm_insert_epi8(range,i+1,0);
    uint32_t pos = _mm_cmpistri(range,sum8,_SIDD_UBYTE_OPS |_SIDD_CMP_RANGES|_SIDD_BIT_MASK);

    /* process last block */
    return (pos<<3) + Select2561[(x>>(pos<<3)&0xFFULL) + ((i-1-csum_8[7+pos])<<8)  ];
}

/* taken from Sebastiano Vigna for comparison */
inline int select64_v(const uint64_t x, const int k)     /* k < 128; returns 72 if there are less than k ones in x. */
{

    // Phase 1: sums by byte
    register uint64_t byte_sums = x - ((x & 0xa * ONES_STEP_4) >> 1);
    byte_sums = (byte_sums & 3 * ONES_STEP_4) + ((byte_sums >> 2) & 3 * ONES_STEP_4);
    byte_sums = (byte_sums + (byte_sums >> 4)) & 0x0f * ONES_STEP_8;
    byte_sums *= ONES_STEP_8;

    // Phase 2: compare each byte sum with k
    const uint64_t k_step_8 = k * ONES_STEP_8;
    const uint64_t place = (LEQ_STEP_8(byte_sums, k_step_8) * ONES_STEP_8 >> 53) & ~0x7;

    // Phase 3: Locate the relevant byte and make 8 copies with incrental masks
    const int byte_rank = k - (((byte_sums << 8) >> place) & 0xFF);

    const uint64_t spread_bits = (x >> place & 0xFF) * ONES_STEP_8 & INCR_STEP_8;
    const uint64_t bit_sums = ZCOMPARE_STEP_8(spread_bits) * ONES_STEP_8;

    // Compute the inside-byte location and return the sum
    const uint64_t byte_rank_step_8 = byte_rank * ONES_STEP_8;

    return place + (LEQ_STEP_8(bit_sums, byte_rank_step_8) * ONES_STEP_8 >> 56);
}


int main(int argc,char** argv)
{
    size_t m = 20000000;
    size_t n = 10000000;
    size_t sum = 0;
    size_t i;

    fprintf(stdout,"select64_1(%lX,%u) = %u\n",0x123456789ABCDEF,1,select64_1(0x123456789ABCDEF,1));
    fprintf(stdout,"select64_2(%lX,%u) = %u\n",0x123456789ABCDEF,1,select64_2(0x123456789ABCDEF,1));
    fprintf(stdout,"select64_3(%lX,%u) = %u\n",0x123456789ABCDEF,1,select64_3(0x123456789ABCDEF,1));
    fprintf(stdout,"select64_4(%lX,%u) = %u\n",0x123456789ABCDEF,1,select64_4(0x123456789ABCDEF,1));
    fprintf(stdout,"bit_magic::i1BP(%lX,%u) = %u\n",0x123456789ABCDEF,1,bit_magic::i1BP(0x123456789ABCDEF,1));

    fprintf(stdout,"select64_1(%lX,%u) = %u\n",0x123456789ABCDEF,22,select64_1(0x123456789ABCDEF,22));
    fprintf(stdout,"select64_2(%lX,%u) = %u\n",0x123456789ABCDEF,22,select64_2(0x123456789ABCDEF,22));
    fprintf(stdout,"select64_3(%lX,%u) = %u\n",0x123456789ABCDEF,22,select64_3(0x123456789ABCDEF,22));
    fprintf(stdout,"select64_4(%lX,%u) = %u\n",0x123456789ABCDEF,22,select64_4(0x123456789ABCDEF,22));
    fprintf(stdout,"bit_magic::i1BP(%lX,%u) = %u\n",0x123456789ABCDEF,22,bit_magic::i1BP(0x123456789ABCDEF,22));

    size_t pcnt = bit_magic::b1Cnt(0x123456789ABCDEF);
    for (i=1; i<pcnt; i++) {
        fprintf(stdout,"%lX,%zu select64_1() = %u bit_magic::i1BP() = %u\n",0x123456789ABCDEF,i,
                select64_3(0x123456789ABCDEF,i),bit_magic::i1BP(0x123456789ABCDEF,i));
        fprintf(stdout,"%lX,%zu select64_2() = %u bit_magic::i1BP() = %u\n",0x123456789ABCDEF,i,
                select64_3(0x123456789ABCDEF,i),bit_magic::i1BP(0x123456789ABCDEF,i));
        fprintf(stdout,"%lX,%zu select64_3() = %u bit_magic::i1BP() = %u\n",0x123456789ABCDEF,i,
                select64_3(0x123456789ABCDEF,i),bit_magic::i1BP(0x123456789ABCDEF,i));
        fprintf(stdout,"%lX,%zu select64_4() = %u bit_magic::i1BP() = %u\n",0x123456789ABCDEF,i,
                select64_3(0x123456789ABCDEF,i),bit_magic::i1BP(0x123456789ABCDEF,i));
        fprintf(stdout,"%lX,%zu select64_v() = %u bit_magic::i1BP() = %u\n",0x123456789ABCDEF,i,
                select64_3(0x123456789ABCDEF,i),bit_magic::i1BP(0x123456789ABCDEF,i));
    }

    int_vector<64> vec(m);
    int_vector<64> ones(m);
    int_vector<64> qrys(n);

    util::set_random_bits(vec,4711);

    /* generate queries */
    for (i=0; i<n; i++) {
        qrys[i] = (rand() % m);
    }
    for (i=0; i<m; i++) {
        ones[i] = 1 + (rand() % bit_magic::b1Cnt(vec[i]));
    }

    stop_watch sw;
    sw.start();
    sum = 0;
    for (i=0; i<n; i++) {
        uint64_t v = vec[qrys[i]];
        sum += bit_magic::i1BP(v, ones[qrys[i]]);
    }
    sw.stop();
    cout << "sum = " << sum << " #1 bit_magic::i1BP = " << sw.get_real_time() << endl;

    sw.start();
    sum = 0;
    for (i=0; i<n; i++) {
        uint64_t v = vec[qrys[i]];
        sum += select64_1(v, ones[qrys[i]]);
    }
    sw.stop();
    cout << "sum = " << sum << " #2 select64_1 = " << sw.get_real_time() << endl;

    sw.start();
    sum = 0;
    for (i=0; i<n; i++) {
        uint64_t v = vec[qrys[i]];
        sum += select64_2(v, ones[qrys[i]]);
    }
    sw.stop();
    cout << "sum = " << sum << " #3 select64_2 = " << sw.get_real_time() << endl;

    sw.start();
    sum = 0;
    for (i=0; i<n; i++) {
        uint64_t v = vec[qrys[i]];
        sum += select64_3(v, ones[qrys[i]]);
    }
    sw.stop();
    cout << "sum = " << sum << " #1 select64_3 = " << sw.get_real_time() << endl;

    sw.start();
    sum = 0;
    for (i=0; i<n; i++) {
        uint64_t v = vec[qrys[i]];
        sum += select64_4(v, ones[qrys[i]]);
    }
    sw.stop();
    cout << "sum = " << sum << " #2 select64_4 = " << sw.get_real_time() << endl;

    sw.start();
    sum = 0;
    for (i=0; i<n; i++) {
        uint64_t v = vec[qrys[i]];
        sum += select64_v(v, ones[qrys[i]]);
    }
    sw.stop();
    cout << "sum = " << sum << " #2 select64_v = " << sw.get_real_time() << endl;

    sw.start();
    sum = 0;
    for (i=0; i<n; i++) {
        uint64_t v = vec[qrys[i]];
        sum += bit_magic::b1Cnt(v);
    }
    sw.stop();
    cout << "sum = " << sum << " b1Cnt = " << sw.get_real_time() << endl;


    return EXIT_SUCCESS;
}