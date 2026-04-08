#include "cpu.h"
#include "memory.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    i8080 state = i8080_init();

    while (true) {
        i8080_execute(&state);
    }

    return 0;
}
