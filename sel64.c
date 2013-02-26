#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/resource.h>

#include <nmmintrin.h>
#include <vector>
#include <iostream>

using namespace std;

class stop_watch
{
    private:
        rusage m_ruse1, m_ruse2;
        timeval m_timeOfDay1, m_timeOfDay2;
        static timeval m_first_t;
        static rusage m_first_r;
    public:

        stop_watch() : m_ruse1(), m_ruse2(), m_timeOfDay1(), m_timeOfDay2() {
            timeval t;
            t.tv_sec = 0; t.tv_usec = 0;
            m_ruse1.ru_utime = t; m_ruse1.ru_stime = t; // init m_ruse1
            m_ruse2.ru_utime = t; m_ruse2.ru_stime = t; // init m_ruse2
            m_timeOfDay1 = t; m_timeOfDay2 = t;
            if (m_first_t.tv_sec == 0) {
                gettimeofday(&m_first_t, 0);
            }
            if (m_first_r.ru_utime.tv_sec == 0 and m_first_r.ru_utime.tv_usec ==0) {
                getrusage(RUSAGE_SELF, &m_first_r);
            }
        }
        //! Start the stopwatch.
        /*! \sa stop
         */
        void start() {
            gettimeofday(&m_timeOfDay1, 0);
            getrusage(RUSAGE_SELF, &m_ruse1);
        };

        //! Stop the stopwatch.
        /*! \sa start
         */
        void stop() {
            getrusage(RUSAGE_SELF, &m_ruse2);
            gettimeofday(&m_timeOfDay2, 0);
        };

        //! Get the elapsed user time in milliseconds between start and stop.
        /*! \sa start, stop, get_real_time, get_sys_time
         */
        double get_user_time() {
            timeval t1, t2;
            t1 = m_ruse1.ru_utime;
            t2 = m_ruse2.ru_utime;
            return ((double)(t2.tv_sec*1000000 + t2.tv_usec - (t1.tv_sec*1000000 + t1.tv_usec)))/1000.0;
        };

        //! Get the elapsed system time in milliseconds between start and stop.
        /*! \sa start, stop, get_real_time, get_user_time
         */
        double get_sys_time() {
            timeval t1, t2;
            t1 = m_ruse1.ru_stime;
            t2 = m_ruse2.ru_stime;
            return ((double)(t2.tv_sec*1000000 + t2.tv_usec - (t1.tv_sec*1000000 + t1.tv_usec)))/1000.0;
        };

        //! Get the elapsed real time in milliseconds between start and stop.
        /*! \sa start, stop, get_sys_time, get_user_time
         */
        double get_real_time() {
            double result = ((double)((m_timeOfDay2.tv_sec*1000000 + m_timeOfDay2.tv_usec)-(m_timeOfDay1.tv_sec*1000000 + m_timeOfDay1.tv_usec)))/1000.0;
            if (result < get_sys_time() + get_user_time())
                return get_sys_time()+get_user_time();
            return result;
        };
};

timeval stop_watch::m_first_t = {0,0};
rusage stop_watch::m_first_r = {{0,0},{0,0}};

#define ONES_STEP_4 ( 0x1111111111111111ULL )
#define ONES_STEP_8 ( 0x0101010101010101ULL )
#define ONES_STEP_9 ( 1ULL << 0 | 1ULL << 9 | 1ULL << 18 | 1ULL << 27 | 1ULL << 36 | 1ULL << 45 | 1ULL << 54 )
#define ONES_STEP_16 ( 1ULL << 0 | 1ULL << 16 | 1ULL << 32 | 1ULL << 48 )
#define MSBS_STEP_4 ( 0x8ULL * ONES_STEP_4 )
#define MSBS_STEP_8 ( 0x80ULL * ONES_STEP_8 )
#define MSBS_STEP_9 ( 0x100ULL * ONES_STEP_9 )
#define MSBS_STEP_16 ( 0x8000ULL * ONES_STEP_16 )
#define INCR_STEP_8 ( 0x80ULL << 56 | 0x40ULL << 48 | 0x20ULL << 40 | 0x10ULL << 32 | 0x8ULL << 24 | 0x4ULL << 16 | 0x2ULL << 8 | 0x1 )

#define ONES_STEP_32 ( 0x0000000100000001ULL )
#define MSBS_STEP_32 ( 0x8000000080000000ULL )

#define COMPARE_STEP_8(x,y) ( ( ( ( ( (x) | MSBS_STEP_8 ) - ( (y) & ~MSBS_STEP_8 ) ) ^ (x) ^ ~(y) ) & MSBS_STEP_8 ) >> 7 )
#define LEQ_STEP_8(x,y) ( ( ( ( ( (y) | MSBS_STEP_8 ) - ( (x) & ~MSBS_STEP_8 ) ) ^ (x) ^ (y) ) & MSBS_STEP_8 ) >> 7 )

#define UCOMPARE_STEP_9(x,y) ( ( ( ( ( ( (x) | MSBS_STEP_9 ) - ( (y) & ~MSBS_STEP_9 ) ) | ( x ^ y ) ) ^ ( x | ~y ) ) & MSBS_STEP_9 ) >> 8 )
#define UCOMPARE_STEP_16(x,y) ( ( ( ( ( ( (x) | MSBS_STEP_16 ) - ( (y) & ~MSBS_STEP_16 ) ) | ( x ^ y ) ) ^ ( x | ~y ) ) & MSBS_STEP_16 ) >> 15 )
#define ULEQ_STEP_9(x,y) ( ( ( ( ( ( (y) | MSBS_STEP_9 ) - ( (x) & ~MSBS_STEP_9 ) ) | ( x ^ y ) ) ^ ( x & ~y ) ) & MSBS_STEP_9 ) >> 8 )
#define ULEQ_STEP_16(x,y) ( ( ( ( ( ( (y) | MSBS_STEP_16 ) - ( (x) & ~MSBS_STEP_16 ) ) | ( x ^ y ) ) ^ ( x & ~y ) ) & MSBS_STEP_16 ) >> 15 )
#define ZCOMPARE_STEP_8(x) ( ( ( x | ( ( x | MSBS_STEP_8 ) - ONES_STEP_8 ) ) & MSBS_STEP_8 ) >> 7 )

#define EASY_LEQ_STEP_8(x,y) ( ( ( ( ( (y) | MSBS_STEP_8 ) - ( x ) ) ) & MSBS_STEP_8 ) >> 7 )

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
const uint8_t Select256[] = {
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
inline uint32_t select64_sdsl(uint64_t x, uint32_t i)
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
    return (byte_nr << 3) + Select256[((i-1) << 8) + ((x>>(byte_nr<<3))&0xFFULL) ];
}


/* the default non sse version of sdsl */
inline uint32_t select64_sdslold(uint64_t x, uint32_t i)
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
                return    Select256[(x&0xFFULL) + i];
            else
                return 8 +Select256[(((x>>8)&0xFFULL)  + i - ((s&0xFFULL)<<8))&0x7FFULL];//byte 1;
        else//byte >1
            if (b&0x0000000000800000ULL) //byte <=2
                return 16+Select256[(((x>>16)&0xFFULL) + i - (s&0xFF00ULL))&0x7FFULL];//byte 2;
            else
                return 24+Select256[(((x>>24)&0xFFULL) + i - ((s>>8)&0xFF00ULL))&0x7FFULL];//byte 3;
    else//  byte > 3
        if (b&0x0000800000000000ULL) // byte <=5
            if (b&0x0000008000000000ULL) //byte <=4
                return 32+Select256[(((x>>32)&0xFFULL) + i - ((s>>16)&0xFF00ULL))&0x7FFULL];//byte 4;
            else
                return 40+Select256[(((x>>40)&0xFFULL) + i - ((s>>24)&0xFF00ULL))&0x7FFULL];//byte 5;
        else// byte >5
            if (b&0x0080000000000000ULL) //byte<=6
                return 48+Select256[(((x>>48)&0xFFULL) + i - ((s>>32)&0xFF00ULL))&0x7FFULL];//byte 6;
            else
                return 56+Select256[(((x>>56)&0xFFULL) + i - ((s>>40)&0xFF00ULL))&0x7FFULL];//byte 7;
    return 0;
}

/* a sse version */
inline uint32_t
select64_sse(uint64_t x,uint32_t i)
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
    return (pos<<3) + Select256[(x>>(pos<<3)&0xFFULL) + ((i-1-csum_8[7+pos])<<8)  ];
}

/* from vigna's website */
inline int select64_vnet(const uint64_t x, const int k)
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

inline int select64_review(const uint64_t x, const int k)
{
    uint64_t byte_sums = x - ((x & 0xaaaaaaaaaaaaaaaaULL) >> 1);
    byte_sums = (byte_sums & 0x3333333333333333ULL) + ((byte_sums >> 2) & 0x3333333333333333ULL);
    byte_sums = (byte_sums + (byte_sums >> 4)) & 0x0f0f0f0f0f0f0f0fULL;
    byte_sums *= 0x0101010101010101ULL;
    const uint64_t k_step_8 = k * 0x0101010101010101ULL;
    const int place = ((((k_step_8 | 0x8080808080808080ULL) - byte_sums)
                        & 0x8080808080808080ULL) >> 7) * 0x0101010101010101ULL >> 53 & ~0x7;
    return place + Select256[x >> place & 0xFF | k - ((byte_sums << 8) >> place & 0xFF) << 8];
}

/* from sux0.8 select.h */
inline int select64_vsux8(const uint64_t x, const int rank)
{
    // Phase 1: sums by byte
    uint64_t byte_sums = x - ((x & 0xa * ONES_STEP_4) >> 1);
    byte_sums = (byte_sums & 3 * ONES_STEP_4) + ((byte_sums >> 2) & 3 * ONES_STEP_4);
    byte_sums = (byte_sums + (byte_sums >> 4)) & 0x0f * ONES_STEP_8;
    byte_sums *= ONES_STEP_8;
    // Phase 2: compare each byte sum with rank
    const uint64_t rank_plus_1_step_8 = (rank + 0) * ONES_STEP_8;
    const int place = (EASY_LEQ_STEP_8(byte_sums, rank_plus_1_step_8) * ONES_STEP_8 >> 53) & ~0x7;

    // Phase 3: Compute the rank in the relevant byte and use lookup table.
    const int byte_rank = (rank + 0) - ((byte_sums << 8) >> place & 0xFF);
    return place + Select256[ x >> place & 0xFF | byte_rank << 8 ];
}

void set_random_bits(uint64_t* data,size_t n, int seed)
{
    if (0 == seed) {
        srand48((int)time(NULL));
    } else
        srand48(seed);

    if (n) {
        *data = (((uint64_t)lrand48()&0xFFFFULL)<<48)
                |(((uint64_t)lrand48()&0xFFFFULL)<<32)
                |(((uint64_t)lrand48()&0xFFFFULL)<<16)
                |((uint64_t)lrand48()&0xFFFFULL);

        for (size_t i=1; i < n; ++i) {
            *(++data) = (((uint64_t)lrand48()&0xFFFFULL)<<48)
                        |(((uint64_t)lrand48()&0xFFFFULL)<<32)
                        |(((uint64_t)lrand48()&0xFFFFULL)<<16)
                        |((uint64_t)lrand48()&0xFFFFULL);
        }
    }
}

inline uint64_t b1Cnt(uint64_t x)
{
    return __builtin_popcountll(x);
}

int main(int argc,char** argv)
{
    size_t m = 20000000;
    size_t n = 10000000;
    size_t sum = 0;
    size_t i;

    uint64_t* vec = new uint64_t[m];
    set_random_bits(vec,m,4711);
    std::vector<uint64_t> ones(m);
    std::vector<uint64_t> qrys(n);

    /* generate queries */
    for (i=0; i<n; i++) {
        qrys[i] = (rand() % m);
    }

    for (i=0; i<m; i++) {
        ones[i] = 1 + (rand() % b1Cnt(vec[i]));
    }

    stop_watch sw;
    sw.start();
    sum = 0;
    for (i=0; i<n; i++) {
        uint64_t v = vec[qrys[i]];
        sum += select64_sse(v, ones[qrys[i]]);
    }
    sw.stop();
    std::cout << "sum = " << sum << " #1 select64_sse = " << sw.get_real_time() << endl;

    sw.start();
    sum = 0;
    for (i=0; i<n; i++) {
        uint64_t v = vec[qrys[i]];
        sum += select64_sdsl(v, ones[qrys[i]]);
    }
    sw.stop();
    std::cout << "sum = " << sum << " #2 select64_sdsl = " << sw.get_real_time() << endl;

    sw.start();
    sum = 0;
    for (i=0; i<n; i++) {
        uint64_t v = vec[qrys[i]];
        sum += select64_sdslold(v, ones[qrys[i]]);
    }
    sw.stop();
    std::cout << "sum = " << sum << " #3 select64_sdsl(old) = " << sw.get_real_time() << endl;

    sw.start();
    sum = 0;
    for (i=0; i<n; i++) {
        uint64_t v = vec[qrys[i]];
        sum += select64_vnet(v, ones[qrys[i]]-1);
    }
    sw.stop();
    std::cout << "sum = " << sum << " #4 select64_vigna(website) = " << sw.get_real_time() << endl;

    sw.start();
    sum = 0;
    for (i=0; i<n; i++) {
        uint64_t v = vec[qrys[i]];
        sum += select64_review(v, ones[qrys[i]]-1);
    }
    sw.stop();
    std::cout << "sum = " << sum << " #5 select64_review = " << sw.get_real_time() << endl;

    sw.start();
    sum = 0;
    for (i=0; i<n; i++) {
        uint64_t v = vec[qrys[i]];
        sum += select64_vsux8(v, ones[qrys[i]]-1);
    }
    sw.stop();
    std::cout << "sum = " << sum << " #5 select64_vigna(sux8) = " << sw.get_real_time() << endl;


    sw.start();
    sum = 0;
    for (i=0; i<n; i++) {
        uint64_t v = vec[qrys[i]];
        sum += b1Cnt(v);
    }
    sw.stop();
    cout << "sum = " << sum << " b1Cnt = " << sw.get_real_time() << endl;


    return EXIT_SUCCESS;
}
