#include "cpu.h"
#include "memory.h"
#include "utils.h"
#include <stdio.h>

#define TST_ADDRESS 0x0100

u8 pack_flags(const i8080 state) {
  u8 flag = 0;
  if (state.Flag.s)
    flag |= 0x80; // bit 7
  if (state.Flag.z)
    flag |= 0x40; // bit 6
  if (state.Flag.ac)
    flag |= 0x10; // bit 4 (not bit 5!)
  if (state.Flag.p)
    flag |= 0x04; // bit 2
  if (state.Flag.cy)
    flag |= 0x01; // bit 0
  flag |= 0x02;   // bit 1 is always 1 on 8080
  return flag;
}

void test_run(struct i8080 *state, const char *test_file) {
  int result;
  if ((result = mem_load_file(test_file, TST_ADDRESS)) != 0) {
    printf("ERROR: ROM did not load! Result code: %d\n", result);
    return;
  }
  mem_write_byte(0x0000, 0xD3);
  mem_write_byte(0x0001, 0x00);

  mem_write_byte(0x0005, 0xD3);
  mem_write_byte(0x0006, 0x01);
  mem_write_byte(0x0007, 0xC9);

  state->Register.pc = TST_ADDRESS;

  for (;;) {
    /* u8 flag = pack_flags(*state); */

    /* printf("PC: %04X, AF: %04X, BC: %04X, DE: %04X, HL: %04X, SP: %04X, "
     */
    /*        "CYC: %ld (%02X %02X %02X %02X)\n", */
    /*        state->Register.pc, combine(state->Register.a, flag), */
    /*        state->Register.bc, state->Register.de, state->Register.hl, */
    /*        state->Register.sp, state->cycle, */
    /*        mem_read_byte(state->Register.pc), */
    /*        mem_read_byte(state->Register.pc + 1), */
    /*        mem_read_byte(state->Register.pc + 2), */
    /*        mem_read_byte(state->Register.pc + 3)); */

    if (state->Register.pc == 0x0005) {
      if (state->Register.c == 2) {
        printf("%c", state->Register.e);
      } else if (state->Register.c == 9) {
        u16 i = state->Register.de;
        u8 byte;
        while ((byte = mem_read_byte(i)) != '$') {
          printf("%c", byte);
          i++;
        }
      }
    }

    if (state->Register.pc == 0x0000) {

      printf("\nTest Finished.\n");
      break;
    }

    i8080_execute(state);
  }
}

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  struct i8080 state = i8080_init();

  test_run(&state, "roms/8080PRE.COM");
  test_run(&state, "roms/TST8080.COM");
  test_run(&state, "roms/CPUTEST.COM");
  test_run(&state, "roms/cpudiag.bin");
  /* test_run(&state, "roms/8080EXM.COM"); */

  return 0;
}
