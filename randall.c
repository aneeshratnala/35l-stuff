/* Generate N bytes of random output.  */


/* When generating output this program uses the x86-64 RDRAND
  instruction if available to generate random numbers, falling back
  on /dev/random and stdio otherwise.


  This program is not portable.  Compile it with gcc -mrdrnd for a
  x86-64 machine.


  Copyright 2015, 2017, 2020 Paul Eggert


  This program is free software: you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.


  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.


  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.  */


  #include <cpuid.h>
  #include <errno.h>
  #include <immintrin.h>
  #include <limits.h>
  #include <stdbool.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <time.h>    // for time()
  #include "rand64-hw.h"
  #include "rand64-sw.h"
  #include "output.h"
  #include "options.h"
  #include <fcntl.h>    // open()
  #include <unistd.h>   // read(), close()

  
  
  /* Hardware implementation.  */
  
  
  // /* Description of the current CPU.  */
  // struct cpuid { unsigned eax, ebx, ecx, edx; };
  
  
  // /* Return information about the CPU.  See <http://wiki.osdev.org/CPUID>.  */
  // static struct cpuid
  // cpuid (unsigned int leaf, unsigned int subleaf)
  // {
  //   struct cpuid result;
  //   asm ("cpuid"
  //        : "=a" (result.eax), "=b" (result.ebx),
  //   "=c" (result.ecx), "=d" (result.edx)
  //        : "a" (leaf), "c" (subleaf));
  //   return result;
  // }
  
  
  // /* Return true if the CPU supports the RDRAND instruction.  */
  // static _Bool
  // rdrand_supported (void)
  // {
  //   struct cpuid extended = cpuid (1, 0);
  //   return (extended.ecx & bit_RDRND) != 0;
  // }
  
  
  // /* Initialize the hardware rand64 implementation.  */
  // static void
  // hardware_rand64_init (void)
  // {
  // }
  
  
  // /* Return a random value, using hardware operations.  */
  // static unsigned long long
  // hardware_rand64 (void)
  // {
  //   unsigned long long int x;
  
  
  //   /* Work around GCC bug 107565
  //      <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107565>.  */
  //   x = 0;
  
  
  //   while (! _rdrand64_step (&x))
  //     continue;
  //   return x;
  // }
  
  
  // /* Finalize the hardware rand64 implementation.  */
  // static void
  // hardware_rand64_fini (void)
  // {
  // }
  
  
  
  
  
  
  /* Software implementation.  */
  
  
  // /* Input stream containing random bytes.  */
  // static FILE *urandstream;
  
  
  // /* Initialize the software rand64 implementation.  */
  // static void
  // software_rand64_init (void)
  // {
  //   urandstream = fopen ("/dev/random", "r");
  //   if (! urandstream)
  //     abort ();
  // }
  
  
  // /* Return a random value, using software operations.  */
  // static unsigned long long
  // software_rand64 (void)
  // {
  //   unsigned long long int x;
  //   if (fread (&x, sizeof x, 1, urandstream) != 1)
  //     abort ();
  //   return x;
  // }
  
  
  // /* Finalize the software rand64 implementation.  */
  // static void
  // software_rand64_fini (void)
  // {
  //   fclose (urandstream);
  // }
  
  
  // in output.c/h:
  
  
  // static bool
  // writebytes (unsigned long long x, int nbytes)
  // {
  //   do
  //     {
  //       if (putchar (x) < 0)
  //  return false;
  //       x >>= CHAR_BIT;
  //       nbytes--;
  //     }
  //   while (0 < nbytes);
  
  
  //   return true;
  // }
  
// #include <errno.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <limits.h>
// #include <unistd.h>
// #include "rand64-hw.h"
// #include "rand64-sw.h"
// #include "output.h"
// #include "options.h"

// for mrand48_r
static struct drand48_data rand48_buf;

static void rand48_init(void) {
    srand48_r(time(NULL), &rand48_buf);  // seed with current time
}

static unsigned long long rand48_rand64(void) {
    long x1, x2;
    mrand48_r(&rand48_buf, &x1);
    mrand48_r(&rand48_buf, &x2);
    return ((unsigned long long)(unsigned int)x1 << 32) | (unsigned int)x2;
}

static void rand48_fini(void) {
    // no cleanup needed
}


// for /F
static int rand_fd;

static void file_rand64_init(const char *path) {
    rand_fd = open(path, O_RDONLY);
    if (rand_fd < 0) {
        perror(path);
        exit(1);
    }
}

static unsigned long long file_rand64(void) {
    unsigned long long x;
    ssize_t total = 0;

    while (total < sizeof x) {
        ssize_t n = read(rand_fd, ((char *)&x) + total, sizeof x - total);
        if (n <= 0) {
            perror("read");
            exit(1);
        }
        total += n;
    }

    return x;
}

static void file_rand64_fini(void) {
    close(rand_fd);
}


int main(int argc, char **argv) {
    struct options opts;
    if (!parse_args(argc, argv, &opts)) {
        fprintf(stderr, "usage: %s [-i input] [-o output] NBYTES\n", argv[0]);
        return 1;
    }

    long long nbytes = opts.nbytes;
    if (nbytes == 0)
        return 0;

    // rng fncn ptrs
    void (*initialize)(void);
    unsigned long long (*rand64)(void);
    void (*finalize)(void);

    switch (opts.input) {
        case INPUT_RDRAND:
            if (!rdrand_supported()) {
                fprintf(stderr, "Error: RDRAND not supported on this CPU.\n");
                return 1;
            }
            initialize = hardware_rand64_init;
            rand64 = hardware_rand64;
            finalize = hardware_rand64_fini;
            break;

        case INPUT_MRAND48:
            // fprintf(stderr, "Error: mrand48_r input not yet implemented.\n");
            // return 1;
            initialize = rand48_init;
            rand64 = rand48_rand64;
            finalize = rand48_fini;
            break;

        case INPUT_FILE:
            // fprintf(stderr, "Error: file input (%s) not yet implemented.\n", opts.input_file);
            // return 1;
            // initialize = () => file_rand64_init(opts.input_file);  // Not valid C so instead:
            initialize = NULL;  // placeholder
            rand64 = file_rand64;
            finalize = file_rand64_fini;
            break;
    }

    // Handle output method
    unsigned char *buffer = NULL;
    int block_size = opts.output_block_size;

    if (opts.output == OUTPUT_WRITE) {
        // fprintf(stderr, "Error: write() output with block size %d not yet implemented.\n", opts.output_block_size);
        // return 1;
        buffer = malloc(block_size);
        if (!buffer) {
            perror("malloc");
            finalize();
            return 1;
        }

    }

    // initialize();
    if (opts.input == INPUT_FILE)
        file_rand64_init(opts.input_file);
    else
        initialize();

    int wordsize = sizeof rand64();
    int output_errno = 0;

    // do {
    //     unsigned long long x = rand64();
    //     int outbytes = nbytes < wordsize ? nbytes : wordsize;
    //     if (!writebytes(x, outbytes)) {
    //         output_errno = errno;
    //         break;
    //     }
    //     nbytes -= outbytes;
    // } while (nbytes > 0);
    while (nbytes > 0) {
        if (opts.output == OUTPUT_STDIO) {
            unsigned long long x = rand64();
            int outbytes = nbytes < wordsize ? nbytes : wordsize;
            if (!writebytes(x, outbytes)) {
                output_errno = errno;
                break;
            }
            nbytes -= outbytes;
        } else {
            int chunk = nbytes < block_size ? nbytes : block_size;
            int filled = 0;
    
            while (filled < chunk) {
                unsigned long long x = rand64();
                int to_copy = (chunk - filled < wordsize) ? (chunk - filled) : wordsize;
    
                for (int i = 0; i < to_copy; ++i) {
                    buffer[filled + i] = x & 0xFF;
                    x >>= 8;
                }
                filled += to_copy;
            }
    
            int written = 0;
            while (written < chunk) {
                ssize_t w = write(STDOUT_FILENO, buffer + written, chunk - written);
                if (w <= 0) {
                    output_errno = errno;
                    goto cleanup;
                }
                written += w;
            }
    
            nbytes -= chunk;
        }
    }
    

    // if (fclose(stdout) != 0)
    //     output_errno = errno;

    // if (output_errno) {
    //     errno = output_errno;
    //     perror("output");
    // }
    cleanup:
    if (buffer)
        free(buffer);

    if (fclose(stdout) != 0)
        output_errno = errno;

    if (output_errno) {
        errno = output_errno;
        perror("output");
    }


    finalize();
    return !!output_errno;
}

  /* Main program, which outputs N bytes of random data.  */
  // int
  // main (int argc, char **argv)
  // {
  //  /* Check arguments.  */
  // //  bool valid = false;
  // //  long long nbytes;
  // //  if (argc == 2)
  // //    {
  // //      char *endptr;
  // //      errno = 0;
  // //      nbytes = strtoll (argv[1], &endptr, 10);
  // //      if (errno)
  // //  perror (argv[1]);
  // //      else
  // //  valid = !*endptr && 0 <= nbytes;
  // //    }
  // //  if (!valid)
  // //    {
  // //      fprintf (stderr, "%s: usage: %s NBYTES\n", argv[0], argv[0]);
  // //      return 1;
  // //    }
  //   long long nbytes;
  //   if (!parse_simple_args(argc, argv, &nbytes)) {
  //       fprintf(stderr, "%s: usage: %s NBYTES\n", argv[0], argv[0]);
  //       return 1;
  //   }

  //   // struct options opts;
  //   // if (!parse_options(argc, argv, &opts)) {
  //   //     fprintf(stderr, "usage: %s [-i input] [-o output] NBYTES\n", argv[0]);
  //   //     return 1;
  //   // }
  //   // long long nbytes = opts.nbytes;

  
  //  /* If there's no work to do, don't worry about which library to use.  */
  //  if (nbytes == 0)
  //    return 0;
  
  
  //  /* Now that we know we have work to do, arrange to use the
  //     appropriate library.  */
  //  void (*initialize) (void);
  //  unsigned long long (*rand64) (void);
  //  void (*finalize) (void);

  //  if (rdrand_supported ())
  //    {
  //      initialize = hardware_rand64_init; // nvm. - changes to call as functions
  //      rand64 = hardware_rand64;
  //      finalize = hardware_rand64_fini;
  //    }
  //  else
  //    {
  //      initialize = software_rand64_init;
  //      rand64 = software_rand64;
  //      finalize = software_rand64_fini;
  //    }
  // // switch (opts.input) {
  // //   case INPUT_RDRAND:
  // //     if (!rdrand_supported()) {
  // //       fprintf(stderr, "Error: RDRAND not supported on this CPU.\n");
  // //       return 1;
  // //     }
  // //     initialize = hardware_rand64_init;
  // //     rand64 = hardware_rand64;
  // //     finalize = hardware_rand64_fini;
  // //     break;
  
  // //   case INPUT_MRAND48:
  // //     fprintf(stderr, "Error: mrand48_r not implemented yet.\n");
  // //     return 1;
  
  // //   case INPUT_FILE:
  // //     fprintf(stderr, "Error: file input not implemented yet.\n");
  // //     return 1;
  // // }
  
  
  
  //  initialize ();
  //  int wordsize = sizeof rand64 ();
  //  int output_errno = 0;
  
  
  //  do
  //    {
  //      unsigned long long x = rand64 ();
  //      int outbytes = nbytes < wordsize ? nbytes : wordsize;
  //      if (!writebytes (x, outbytes))
  //  {
  //    output_errno = errno;
  //    break;
  //  }
  //      nbytes -= outbytes;
  //    }
  //  while (0 < nbytes);
  
  
  //  if (fclose (stdout) != 0)
  //    output_errno = errno;
  
  
  //  if (output_errno)
  //    {
  //      errno = output_errno;
  //      perror ("output");
  //    }
  
  
  //  finalize ();
  //  return !!output_errno;
  // }
  
  
  
  