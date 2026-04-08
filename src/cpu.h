#ifndef CPU_H
#define CPU_H

#include "types.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_PORTS 255

enum Status { STOPPED, RUNNING };

typedef struct i8080 {
    size_t cycle;

    bool interrupt;

    u8 out[MAX_PORTS];
    u8 in[MAX_PORTS];

    enum Status status;

    struct {
        u16 pc;
        u16 sp;
        u8 f;
        u8 a;

        union {
            struct {
                u8 c;
                u8 b;
                u8 e;
                u8 d;
                u8 l;
                u8 h;
            };

            struct {
                u16 bc;
                u16 de;
                u16 hl;
            };
        };

    } Register;

    struct {
        u8 s;
        u8 z;
        u8 p;
        u8 ac;
        u8 cy;
    } Flag;

} i8080;

void flag_check_s(i8080 *state, const u16 reg);
void flag_check_z(i8080 *state, const u16 reg);
void flag_check_p(i8080 *state, const u16 reg);
void flag_check_ac(i8080 *state, const u8 reg, const u8 val);
void flag_check_cy(i8080 *state, const u8 reg);

i8080 i8080_init(void);
void i8080_dump(i8080 *state);
void i8080_reset(i8080 *state);
void i8080_step(i8080 *state);
u8 i8080_fetch(i8080 *state);
void i8080_decode(i8080 *state, u8 opcode);
void i8080_execute(i8080 *state);
void i8080_debug();

#endif
