# Make x86-64 random byte generators.

# Copyright 2015, 2020, 2021 Paul Eggert

# This program is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.

# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

# Optimization level.  Change this -O2 to -Og or -O0 or whatever.
OPTIMIZE =

# The C compiler and its options.
CC = gcc
CFLAGS = $(OPTIMIZE) -g3 -Wall -Wextra -fanalyzer \
  -march=native -mtune=native -mrdrnd

# The archiver command, its options and filename extension.
TAR = tar
TARFLAGS = --gzip --transform 's,^,randall/,'
TAREXT = tgz

default: randall

# randall: randall.c rand64-hw.c rand64-sw.c
# 	$(CC) $(CFLAGS) $@.c -o $@

# randall: randall.c rand64-hw.c rand64-sw.c output.c options.c
# 	$(CC) $(CFLAGS) randall.c rand64-hw.c rand64-sw.c output.c options.c -o randall

OBJS = randall.o rand64-hw.o rand64-sw.o output.o options.o

randall: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o randall

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


	
assignment: randall-assignment.$(TAREXT)
assignment-files = COPYING Makefile randall.c
randall-assignment.$(TAREXT): $(assignment-files)
	$(TAR) $(TARFLAGS) -cf $@ $(assignment-files)

submission-tarball: randall-submission.$(TAREXT)
submission-files = $(assignment-files) \
  notes.txt options.c options.h output.c output.h rand64-hw.c rand64-hw.h \
  rand64-sw.c rand64-sw.h

randall-submission.$(TAREXT): $(submission-files)
	$(TAR) $(TARFLAGS) -cf $@ $(submission-files)

repository-tarball:
	$(TAR) -czf randall-git.tgz .git

.PHONY: default clean assignment submission-tarball repository-tarball

clean:
	rm -f *.o *.$(TAREXT) randall

# make check implementation: check that output of running ./randall 7 is 7 bytes long
check:
	@echo "Checking that ./randall 7 outputs 7 bytes..."
	@./randall 7 | wc -c | grep -q "^7$$" && echo "Check passed." || echo "Check failed."



# NOUVELLE:

# # Makefile for randall project

# # Optimization level. Change this -O2 to -Og or -O0 or whatever.
# OPTIMIZE = -O2

# # The C compiler and its options.
# CC = gcc
# # CRITICAL: Added -std=gnu11 and -Werror (optional but recommended)
# CFLAGS = $(OPTIMIZE) -g3 -Wall -Wextra -fanalyzer \
#          -std=gnu11 -Werror \
#          -march=native -mtune=native -mrdrnd

# # List of all source files (main program and modules)
# SOURCES = randall.c options.c output.c rand64-hw.c rand64-sw.c
# # Corresponding object files
# OBJS = $(SOURCES:.c=.o)

# # Executable name
# TARGET = randall

# # The archiver command, its options and filename extension.
# TAR = tar
# TARFLAGS = --gzip --transform 's,^,randall/,'
# TAREXT = tgz

# .PHONY: default all clean check submission-tarball repository-tarball

# default: $(TARGET)

# all: $(TARGET)

# # Rule to link the program
# $(TARGET): $(OBJS)
# 	$(CC) $(CFLAGS) $(OBJS) -o $@

# # Generic rule to compile .c files into .o files
# # Dependencies include the .c file itself and all .h files (using a wildcard for simplicity)
# %.o: %.c *.h
# 	$(CC) $(CFLAGS) -c $< -o $@

# clean:
# 	rm -f $(TARGET) $(OBJS) *.$(TAREXT)

# # Updated check target - you will expand this later
# check: $(TARGET)
# 	@echo "--- Running Basic Checks ---"
# 	@echo "Test 1: Requesting 10 bytes (default options)..."
# 	@if ./$(TARGET) 10 | wc -c | grep -q "^\s*10$$"; then \
# 	  echo "Test 1 PASSED"; \
# 	else \
# 	  echo "Test 1 FAILED"; exit 1; \
# 	fi
# 	@echo "Test 2: Requesting 0 bytes..."
# 	@if ./$(TARGET) 0 | wc -c | grep -q "^\s*0$$"; then \
# 	  echo "Test 2 PASSED"; \
# 	else \
# 	  echo "Test 2 FAILED"; exit 1; \
# 	fi
# 	@echo "Test 3: Invalid NBYTES..."
# 	@if ! ./$(TARGET) abc >/dev/null 2>&1; then \
# 	  echo "Test 3 PASSED (program exited with error as expected)"; \
# 	else \
# 	  echo "Test 3 FAILED (program did not error on invalid NBYTES)"; exit 1; \
# 	fi
# 	@echo "Test 4: Missing NBYTES..."
# 	@if ! ./$(TARGET) >/dev/null 2>&1; then \
# 	  echo "Test 4 PASSED (program exited with error as expected)"; \
# 	else \
# 	  echo "Test 4 FAILED (program did not error on missing NBYTES)"; exit 1; \
# 	fi
# 	@echo "Test 5: Using -o stdio explicitly..."
# 	@if ./$(TARGET) -o stdio 15 | wc -c | grep -q "^\s*15$$"; then \
# 	  echo "Test 5 PASSED"; \
# 	else \
# 	  echo "Test 5 FAILED"; exit 1; \
# 	fi
# 	@echo "--- Basic Checks Complete ---"


# # Files for the submission tarball (ensure all your .c and .h files are listed)
# SUBMISSION_FILES = COPYING Makefile notes.txt \
#                    randall.c rand64-hw.c rand64-hw.h rand64-sw.c rand64-sw.h \
#                    output.c output.h options.c options.h

# submission-tarball: randall-submission.$(TAREXT)
# randall-submission.$(TAREXT): $(SUBMISSION_FILES)
# 	$(TAR) $(TARFLAGS) -cf $@ $(SUBMISSION_FILES)

# repository-tarball:
# 	$(TAR) -czf randall-git.tgz .git --exclude='randall-git.tgz' --exclude='randall-submission.tgz' --exclude='*.o' --exclude='$(TARGET)'
