#include "memory.h"
#include "constants.h"
#include "types.h"
#include "utils.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

u8 mem[MAX_MEMORY];

u8 mem_read_byte(u16 val) { return mem[val]; }

u8 mem_read_word(u16 val) { return mem[val]; }

void mem_write_byte(u16 addr, u8 data) { mem[addr] = data; }

void mem_write_word(u16 addr, u16 data) {
    mem[addr] = get_lo(data);
    mem[addr + 1] = get_hi(data);
}

int mem_load_file(const char *rom, const u16 address) {
    FILE *fp = fopen(rom, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open file: %s (Error code: %d)\n",
                strerror(errno), errno);
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);

    if (address + file_size > 0x10000) {
        fprintf(stderr, "Error: ROM file too large to fit in memory.\n");
        fclose(fp);
        return 1;
    }

    size_t bytes_read = fread(&mem[address], 1, file_size, fp);
    fclose(fp);

    if (bytes_read != (size_t)file_size) {
        fprintf(stderr, "Error: Read %zu bytes, expected %ld.\n", bytes_read,
                file_size);
        return 1;
    }

    printf("Loaded %s to 0x%04X, Size: %ld bytes\n", rom, address, file_size);

    return 0;
}

void mem_dump(void) {
    printf("      00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");

    for (int i = 0; i < MAX_MEMORY; i += 16) {
        printf("%04X: ", i);
        for (int j = 0; j < 16; j++) {
            printf("%02X ", mem_read_byte(i + j));
        }
        printf("\n");
    }
}
