
// options.h
#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>

enum input_method {
    INPUT_RDRAND,
    INPUT_MRAND48,
    INPUT_FILE
};

enum output_method {
    OUTPUT_STDIO,
    OUTPUT_WRITE
};

struct options {
    enum input_method input;
    const char *input_file;           // if input == INPUT_FILE

    enum output_method output;
    int output_block_size;            // if output == OUTPUT_WRITE

    long long nbytes;
};

bool parse_args(int argc, char **argv, struct options *opts);
bool parse_simple_args(int argc, char **argv, long long *nbytes);

#endif // OPTIONS_H