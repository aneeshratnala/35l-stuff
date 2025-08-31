// options.c
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include "options.h"

#include <stdbool.h>

bool parse_args(int argc, char **argv, struct options *opts) {
    // Set defaults
    opts->input = INPUT_RDRAND;
    opts->input_file = NULL;
    opts->output = OUTPUT_STDIO;
    opts->output_block_size = 0;
    opts->nbytes = -1;

    int opt;
    while ((opt = getopt(argc, argv, "i:o:")) != -1) {
        switch (opt) {
        case 'i':
            if (strcmp(optarg, "rdrand") == 0) {
                opts->input = INPUT_RDRAND;
            } else if (strcmp(optarg, "mrand48_r") == 0) {
                opts->input = INPUT_MRAND48;
            } else if (optarg[0] == '/') {
                opts->input = INPUT_FILE;
                opts->input_file = optarg;
            } else {
                fprintf(stderr, "Invalid input method: %s\n", optarg);
                return false;
            }
            break;

        case 'o':
            if (strcmp(optarg, "stdio") == 0) {
                opts->output = OUTPUT_STDIO;
            } else {
                char *endptr;
                long block_size = strtol(optarg, &endptr, 10);
                if (*endptr != '\0' || block_size <= 0) {
                    fprintf(stderr, "Invalid output block size: %s\n", optarg);
                    return false;
                }
                opts->output = OUTPUT_WRITE;
                opts->output_block_size = (int)block_size;
            }
            break;

        default:
            return false;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Missing NBYTES argument\n");
        return false;
    }

    char *endptr;
    errno = 0;
    opts->nbytes = strtoll(argv[optind], &endptr, 10);
    if (errno || *endptr != '\0' || opts->nbytes < 0) {
        fprintf(stderr, "Invalid NBYTES: %s\n", argv[optind]);
        return false;
    }

    return true;
}

bool parse_simple_args(int argc, char **argv, long long *nbytes) {
    bool valid = false;
    if (argc == 2) {
        char *endptr;
        errno = 0;
        *nbytes = strtoll(argv[1], &endptr, 10);
        if (errno)
            perror(argv[1]);
        else
            valid = !*endptr && 0 <= *nbytes;
    }
    return valid;
}


// // options.c
// #include <getopt.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <errno.h>
// #include <stdbool.h>
// #include "options.h"

// bool parse_options(int argc, char **argv, struct options *opts) {
//     // Set defaults
//     opts->input = INPUT_RDRAND;
//     opts->input_file = NULL;
//     opts->output = OUTPUT_STDIO;
//     opts->output_block_size = 0;
//     opts->nbytes = -1;

//     int opt;
//     while ((opt = getopt(argc, argv, "i:o:")) != -1) {
//         switch (opt) {
//         case 'i':
//             if (strcmp(optarg, "rdrand") == 0) {
//                 opts->input = INPUT_RDRAND;
//             } else if (strcmp(optarg, "mrand48_r") == 0) {
//                 opts->input = INPUT_MRAND48;
//             } else if (optarg[0] == '/') {
//                 opts->input = INPUT_FILE;
//                 opts->input_file = optarg;
//             } else {
//                 fprintf(stderr, "Invalid input method: %s\n", optarg);
//                 return false;
//             }
//             break;
//         case 'o':
//             if (strcmp(optarg, "stdio") == 0) {
//                 opts->output = OUTPUT_STDIO;
//             } else {
//                 char *endptr;
//                 long n = strtol(optarg, &endptr, 10);
//                 if (*endptr != '\0' || n <= 0) {
//                     fprintf(stderr, "Invalid output size: %s\n", optarg);
//                     return false;
//                 }
//                 opts->output = OUTPUT_WRITE;
//                 opts->output_block_size = (int)n;
//             }
//             break;
//         default:
//             return false;
//         }
//     }

//     if (optind >= argc) {
//         fprintf(stderr, "Missing NBYTES argument\n");
//         return false;
//     }

//     char *endptr;
//     errno = 0;
//     opts->nbytes = strtoll(argv[optind], &endptr, 10);
//     if (*endptr || errno || opts->nbytes < 0) {
//         fprintf(stderr, "Invalid NBYTES: %s\n", argv[optind]);
//         return false;
//     }

//     return true;
// }
