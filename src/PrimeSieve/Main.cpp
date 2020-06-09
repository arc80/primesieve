/*------------------------------------
  ///\  Plywood C++ Framework
  \\\/  https://plywood.arc80.com/
------------------------------------*/

// This is a sample application that prints all the prime numbers that fit in a 32-bit integer (up
// to 4294967296) using a segmented prime sieve algorithm.
// https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes#Segmented_sieve
//
// It uses only 8192 bytes of heap space to do the sieve. The first 4096 bytes (divisorMask) is used
// to sieve all the primes up to 65536, and the other 4096 bytes (primeMask) is used to sieve all
// subsequent integers in blocks of 65536, using divisorMask to accelerate the process. Some tricks
// are used to reduce the number of writes. The algorithm is single-threaded, but could be
// multithreaded by using separate threads to sieve the higher blocks.
//
// Further optimization is possible, but it doesn't really matter since the application is
// overwhelmingly bottlenecked writing to stdout. This can be seen by disabling the PRINT_PRIME
// macro, in which case the RelWithDebInfo build currently completes in ~15 secs on an average PC.

#include <ply-runtime/Base.h>

#define PRINT_PRIME(n) sw << (n) << '\n'
//#define PRINT_PRIME(n)

using namespace ply;

PLY_INLINE bool isSet(ArrayView<u32> divisorMask, u32 index) {
    u32 bit = 1 << (index & 31);
    return (divisorMask[index >> 5] & bit) != 0;
}

PLY_INLINE void set(ArrayView<u32> divisorMask, u32 index) {
    u32 bit = 1 << (index & 31);
    divisorMask[index >> 5] |= bit;
}

int main() {
    StringWriter sw = StdOut::createStringWriter();

    // Every bit in divisorMask represents an odd integer < 65536:
    Array<u32> divisorMask;
    divisorMask.resize(1024);
    memset(divisorMask.get(), 0, divisorMask.sizeBytes());

    // First, sieve all the primes up to 65536 in divisorMask by marking all (odd) non-primes as
    // composite. For this to work, we only need to check prime factors up to 256.
    PRINT_PRIME(2);
    u32 p = 3;
    for (; p < 256; p += 2) {
        if (!isSet(divisorMask.view(), p >> 1)) {
            PRINT_PRIME(p);

            u32 sq = p * p;
            u32 j = p;
            u32 maxJ = min(sq, 65536 / p);
            u32 toSet_ = sq >> 1;
            while (j < maxJ) {
                // If j is already marked composite, and j < p * p, that means p * j was already
                // marked composite, so we don't have to do it again:
                if (!isSet(divisorMask.view(), j >> 1)) {
                    set(divisorMask.view(), toSet_);
                }
                j += 2;
                toSet_ += p;
            }

            // Mark all other multiples of p as composite unconditionally:
            while (toSet_ < 32768) {
                set(divisorMask.view(), toSet_);
                toSet_ += p;
            }
        }
    }

    // All multiples of primes up to 256 have now been marked composite. Therefore, all integers up
    // to 65536 that have not been marked composite must be prime.
    for (; p < 65536; p += 2) {
        if (!isSet(divisorMask.view(), p >> 1)) {
            PRINT_PRIME(p);
        }
    }

    // primeMask represents a block of odd integers > 65536. We sieve all remaining integers in
    // blocks of 65536.
    Array<u32> primeMask;
    primeMask.resize(1024);

    u32 base = 65536; // The block starts at this integer
    for (;;) {
        memset(primeMask.get(), 0, primeMask.sizeBytes());
        for (u32 p = 3; p < 65536; p += 2) {
            if (!isSet(divisorMask.view(), p >> 1)) {
                u32 sq = p * p;
                if (sq > base + 65535)
                    break; // Early out; no need to check higher divisors

                // Find the lowest j such that j * p >= base. That tells us where to start marking
                // multiples of p in this block:
                u32 j = p;
                u32 maxJ = min(65536u, sq);
                u32 toSet = sq;
                if (base > toSet) {
                    j = (base / p) + 1;
                    if ((j & 1) == 0) {
                        j++;
                    }
                    toSet = p * j;
                }
                u32 toSet_ = (toSet - base) >> 1;

                while (toSet_ < 32768 && j < maxJ) {
                    // If j is already marked composite, and j < p * p, that means p * j was already
                    // marked composite, so we don't have to do it again:
                    if (!isSet(divisorMask.view(), j >> 1)) {
                        set(primeMask.view(), toSet_);
                    }
                    j += 2;
                    toSet_ += p;
                }

                // Mark all other multiples of p as composite unconditionally:
                while (toSet_ < 32768) {
                    set(primeMask.view(), toSet_);
                    toSet_ += p;
                }
            }
        }

        // At this point, any integer in the block that has not been marked composite must be prime.
        for (u32 i = 1; i < 65536; i += 2) {
            if (!isSet(primeMask.view(), i >> 1)) {
                p = base + i;
                PRINT_PRIME(p);
            }
        }

        // Advance to the next block until base overflows:
        u32 newBase = base + 65536;
        if (newBase < base)
            break;
        base = newBase;
    }

    return 0;
}
