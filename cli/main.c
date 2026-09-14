#include "nem/driver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s <input.nem> <output.c>\n", argv[0]);
        return 1;
    }

    return compile_source_file(argv[1], argv[2]);
}
