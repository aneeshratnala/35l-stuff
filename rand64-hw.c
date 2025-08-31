// rand64-hw.c

#include <immintrin.h>
#include <cpuid.h>
#include "rand64-hw.h"

#define bit_RDRND (1 << 30)

struct cpuid { unsigned eax, ebx, ecx, edx; };

static struct cpuid cpuid(unsigned int leaf, unsigned int subleaf) {
    struct cpuid result;
    asm("cpuid"
        : "=a"(result.eax), "=b"(result.ebx), "=c"(result.ecx), "=d"(result.edx)
        : "a"(leaf), "c"(subleaf));
    return result;
}

_Bool rdrand_supported(void) {
    struct cpuid info = cpuid(1, 0);
    return (info.ecx & bit_RDRND) != 0;
}

void hardware_rand64_init(void) {}

unsigned long long hardware_rand64(void) {
    unsigned long long x = 0;
    while (!_rdrand64_step(&x))
        continue;
    return x;
}

void hardware_rand64_fini(void) {}
