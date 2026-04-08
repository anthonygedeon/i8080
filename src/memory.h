#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"
#include "utils.h"

#define MAX_MEMORY 0x10000

extern u8 mem[MAX_MEMORY];

u8 mem_read_byte(u16 val);
u8 mem_read_word(u16 val);

void mem_write_byte(u16 addr, u8 data);
void mem_write_word(u16 addr, u16 data);

int mem_load_file(const char *rom, const u16 address);

void mem_dump(void);

#endif
