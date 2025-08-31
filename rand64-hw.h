// rand64-hw.h
#ifndef RAND64_HW_H
#define RAND64_HW_H

#include <stdbool.h> // this is important for `bool` to be recognized

void hardware_rand64_init(void);
unsigned long long hardware_rand64(void);
void hardware_rand64_fini(void);
bool rdrand_supported(void);

#endif // RAND64_HW_H
