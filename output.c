// output.c
#include <stdio.h>
#include <limits.h>
#include "output.h"

bool writebytes(unsigned long long x, int nbytes) {
    do {
        if (putchar(x & 0xff) < 0)
            return false;
        x >>= CHAR_BIT;
        nbytes--;
    } while (nbytes > 0);

    return true;
}
