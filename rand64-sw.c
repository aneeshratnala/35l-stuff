// rand64-sw.c
#include <stdio.h>
#include <stdlib.h>
#include "rand64-sw.h"

static FILE *urandstream = NULL;

void software_rand64_init(void) {
    urandstream = fopen("/dev/random", "r");
    if (!urandstream)
        abort();
}

unsigned long long software_rand64(void) {
    unsigned long long x;
    if (fread(&x, sizeof x, 1, urandstream) != 1)
        abort();
    return x;
}

void software_rand64_fini(void) {
    if (urandstream)
        fclose(urandstream);
}
