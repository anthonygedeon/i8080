#include <stdio.h>
// #include <errno.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "disassembler.h"

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof(x[0]))

#define ROM_ADDRESS  0x0000 
#define WRAM_ADDRESS 0x2000
#define VRAM_ADDRESS 0x2400

#define MAX_MEMORY 0x10000

long fsize(FILE *file) {
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);
	return size;
}

typedef struct Memory {
	uint8_t ram[MAX_MEMORY];

	int (*write)(struct Memory, size_t, const uint8_t*);
} Memory;

void load_memory(struct Memory *memory, size_t size, const uint8_t *data) {
	for (size_t i = ROM_ADDRESS; i < size;  i++) {
		memory->ram[i+0x100] = data[i];
	}
}

typedef struct Register {
	uint16_t pc;
	uint16_t sp;
	uint8_t psw;
	uint8_t a;
	uint8_t b;
	uint8_t c;
	uint8_t d;
	uint8_t h;
	uint8_t e;
	uint8_t l;
} Register;

typedef struct ConditionFlag {
	uint8_t sign;
	uint8_t zero;
	uint8_t aux;
	uint8_t parity;
	uint8_t carry;
} ConditionFlag;

typedef struct Cpu {
	int			  cyc;
	uint8_t       opcode;
	Memory		  mem;
	Register	  reg;	
	ConditionFlag flag;
} Cpu;

Cpu cpu_new(void) {
	Cpu cpu;
	
	memset(cpu.mem.ram, 0, MAX_MEMORY);
	// cpu.mem.write
		
	cpu.cyc = 0;
	cpu.opcode = 0;
	cpu.reg.pc = 0;
#ifndef TST8080
	cpu.reg.pc = 0x100;
#endif
	cpu.reg.sp = 0;

	cpu.reg.a = 0;
	cpu.reg.b = 0;
	cpu.reg.c = 0;
	cpu.reg.d = 0;
	cpu.reg.h = 0;
	cpu.reg.e = 0;
	cpu.reg.l = 0;
	
	cpu.flag.sign   = 0;
	cpu.flag.zero   = 0;
	cpu.flag.aux	= 0;
	cpu.flag.parity = 0;
	cpu.flag.carry  = 0;
	cpu.reg.psw = 0;

	// | | | |A| | | | |
	// |S|Z|0|C|0|P|1|C|
	// 
	// S  State of Sign bit
	// Z  State of Zero bit
	// 0  always 0
	// AC State of auxiliary Carry bit
	// 0  always 0
	// P  State of Parity bit
	// 1  always 1
	// C  State of Carry bit

	return cpu;
}

// int load_rom(const char *rom) {}

void unimplemented(void) {
	printf("instruction not implemented");
	exit(1);
}

void fetch(Cpu *cpu) {
	cpu->opcode = cpu->mem.ram[cpu->reg.pc];
}

// TODO: I would like to create a fetch-decode-execute cycle
// void decode(Cpu *cpu) {}

uint8_t check_parity(const uint8_t byte) {
	uint8_t count;

	for (int i = 0; i < 8; i++) {
		if (((byte >> i) & 0x1U) == 1) {
			count++;
		}
	}

	return count % 2 == 0;
}

uint8_t check_zero(const uint8_t byte) {
	return byte == 0;
}

uint8_t check_sign(const uint8_t byte) {
	return (byte & 0x80) == 0x80;
}

void execute(Cpu *cpu) {
	uint16_t bc = (cpu->reg.b << 8) | cpu->reg.c;
	uint16_t de = (cpu->reg.d << 8) | cpu->reg.e;
	uint16_t hl = (cpu->reg.h << 8) | cpu->reg.l;
	
	cpu->reg.psw = 0;
	cpu->reg.psw |= cpu->flag.sign << 7;
	cpu->reg.psw |= cpu->flag.zero << 6;
	cpu->reg.psw |= cpu->flag.aux << 4;
	cpu->reg.psw |= cpu->flag.parity << 2;
	cpu->reg.psw |= 1 << 1;
	cpu->reg.psw |= cpu->flag.carry << 0;

	// printf("PC: %.4X, AF: %04X, BC: %.4X, DE: %.4X, HL: %.4X, SP: %.4X, CYC: %d     (%02X %02X %02X %02X)", 
	// 		cpu->reg.pc, cpu->reg.a << 8 | cpu->reg.psw, bc, de, hl, cpu->reg.sp, cpu->cyc, cpu->mem.ram[cpu->reg.pc], cpu->mem.ram[cpu->reg.pc+1], cpu->mem.ram[cpu->reg.pc+2], cpu->mem.ram[cpu->reg.pc+3]);
	//
	
	printf("%04X: %02X	", cpu->reg.pc, cpu->opcode);
	disassemble(cpu->mem.ram, cpu->reg.pc);
	printf("\n");
	printf("A=%02X, AF=%04X, BC=%04X, DE=%04X, HL=%04X, PC=%04X CYC: %d", cpu->reg.a, 
			cpu->reg.a<<8 | cpu->reg.psw, bc, de, hl, cpu->reg.pc, cpu->cyc);
	printf(" {%c%c%c%c%c} ",  
			cpu->flag.sign     ? 'S' : '.', 
			cpu->flag.zero     ? 'Z' : '.', 
			cpu->flag.aux == 1 ? 'A' : '.', 
			cpu->flag.parity   ? 'P' : '.', 
			cpu->flag.carry    ? 'C' : '.');
	printf("\n");
	printf("SP={$%04X stack:[", cpu->reg.sp);
	// TODO: pushed bytes should have number next to it to indicate how recent it is
	for (uint16_t i = 0x07BD; i >= cpu->reg.sp && cpu->reg.sp != 0; i--) {
		printf(" %02X ", cpu->mem.ram[i]);
	}
	printf("]}\n");

	cpu->reg.pc++;

	switch(cpu->opcode) {
		case 0x00: break;
		case 0x01:
			cpu->reg.b = cpu->mem.ram[cpu->reg.pc + 1];
			cpu->reg.c = cpu->mem.ram[cpu->reg.pc];
			cpu->reg.pc += 2;
			break;
		case 0x02:; 
			// uint16_t address = cpu->reg.c | (cpu->reg.b << 8);
			// cpu->mem.ram[address] = cpu->reg.a;
			unimplemented();break;
		case 0x03:
			unimplemented();break;
		case 0x04:
			cpu->reg.b++;
			cpu->flag.sign = check_sign(cpu->reg.b);
			cpu->flag.zero = check_zero(cpu->reg.b);
			cpu->flag.parity = check_parity(cpu->reg.b);
			cpu->flag.aux = ((cpu->reg.b & 0xF) + 1) > 0xF;
			break;
		case 0x05:;
			// Flags: S Z A P
			// TODO handle ac flag
			uint8_t res = cpu->reg.b - 1;
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.aux = (((cpu->reg.c & 0x0F) + ((~0x01 & 0x0F) + 1)) & 0x10) == 0x10;
			cpu->reg.b = res;
			break;
		case 0x06:
			cpu->reg.b = cpu->mem.ram[cpu->reg.pc];
			cpu->reg.pc++;
			break;
		case 0x07:
			unimplemented();break;
		case 0x08:
			unimplemented();break;
		case 0x09:
			unimplemented();break;
		case 0x0A:
			unimplemented();break;
		case 0x0B:
			unimplemented();break;
		case 0x0C:
			cpu->reg.c++;
			cpu->flag.sign = check_sign(cpu->reg.c);
			cpu->flag.zero = check_zero(cpu->reg.c);
			cpu->flag.parity = check_parity(cpu->reg.c);
			cpu->flag.aux = ((cpu->reg.c & 0xF) + 1) > 0xF;
			break;
		case 0x0D:
			cpu->reg.c--;
			cpu->flag.sign = check_sign(cpu->reg.c);
			cpu->flag.zero = check_zero(cpu->reg.c);
			cpu->flag.parity = check_parity(cpu->reg.c);
			cpu->flag.aux = (((cpu->reg.c & 0x0F) + ((~0x01 & 0x0F) + 1)) & 0x10) == 0x10;
			break;
		case 0x0E:
			cpu->reg.c = cpu->mem.ram[cpu->reg.pc];
			cpu->reg.pc++;
			break;
		case 0x0F:
			unimplemented();break;
		case 0x10:
			unimplemented();break;
		case 0x11:
			cpu->reg.d = cpu->mem.ram[cpu->reg.pc + 1];
			cpu->reg.e = cpu->mem.ram[cpu->reg.pc];
			cpu->reg.pc += 2;
			break;
		case 0x12:
			unimplemented();break;
		case 0x13:
			de += 1;
			cpu->reg.d = de >> 8;
			cpu->reg.e = (de & 0x00FF);
			break;
		case 0x14:
			cpu->reg.d++;
			cpu->flag.sign = check_sign(cpu->reg.d);
			cpu->flag.zero = check_zero(cpu->reg.d);
			cpu->flag.parity = check_parity(cpu->reg.d);
			cpu->flag.aux = ((cpu->reg.d & 0xF) + 1) > 0xF;
			break;
		case 0x15:
			cpu->reg.d--;
			cpu->flag.sign = check_sign(cpu->reg.d);
			cpu->flag.zero = check_zero(cpu->reg.d);
			cpu->flag.parity = check_parity(cpu->reg.d);
			cpu->flag.aux = (((cpu->reg.d & 0x0F) + ((~0x01 & 0x0F) + 1)) & 0x10) == 0x10;
			break;
		case 0x16:
			cpu->reg.d = cpu->mem.ram[cpu->reg.pc];
			cpu->reg.pc++;
			break;
		case 0x17:
			unimplemented();break;
		case 0x18:
			unimplemented();break;
		case 0x19: {
			uint32_t res = de + hl;
			cpu->reg.h = res >> 8;
			cpu->reg.l = (res & 0x00FF);
			cpu->flag.carry = (res & 0xFFFF0000) != 0;
			break;
		}
		case 0x1A:
			cpu->reg.a = cpu->mem.ram[(cpu->reg.d << 8) | cpu->reg.e];
			break;
		case 0x1B:
			unimplemented();break;
		case 0x1C:
			cpu->reg.e++;
			cpu->flag.sign = check_sign(cpu->reg.e);
			cpu->flag.zero = check_zero(cpu->reg.e);
			cpu->flag.parity = check_parity(cpu->reg.e);
			cpu->flag.aux = ((cpu->reg.d & 0xF) + 1) > 0xF;
			break;
		case 0x1D:
			cpu->reg.e--;
			cpu->flag.sign = check_sign(cpu->reg.e);
			cpu->flag.zero = check_zero(cpu->reg.e);
			cpu->flag.parity = check_parity(cpu->reg.e);
			cpu->flag.aux = (((cpu->reg.e & 0x0F) + ((~0x01 & 0x0F) + 1)) & 0x10) == 0x10;
			break;
		case 0x1E:
			cpu->reg.e = cpu->mem.ram[cpu->reg.pc];
			cpu->reg.pc++;
			break;
		case 0x1F:
			unimplemented();break;
		case 0x20:
			unimplemented();break;
		case 0x21:
			cpu->reg.h = cpu->mem.ram[cpu->reg.pc + 1];
			cpu->reg.l = cpu->mem.ram[cpu->reg.pc];
			cpu->reg.pc += 2;
			break;
		case 0x22:
			unimplemented();break;
		case 0x23:
			hl += 1;
			cpu->reg.h = hl >> 8;
			cpu->reg.l = (hl & 0x00FF);
			break;
		case 0x24:
			cpu->reg.h++;
			cpu->flag.sign = check_sign(cpu->reg.h);
			cpu->flag.zero = check_zero(cpu->reg.h);
			cpu->flag.parity = check_parity(cpu->reg.h);
			cpu->flag.aux = ((cpu->reg.h & 0xF) + 1) > 0xF;
			break;
		case 0x25:
			cpu->reg.h--;
			cpu->flag.sign = check_sign(cpu->reg.h);
			cpu->flag.zero = check_zero(cpu->reg.h);
			cpu->flag.parity = check_parity(cpu->reg.h);
			cpu->flag.aux = (((cpu->reg.h & 0x0F) + ((~0x01 & 0x0F) + 1)) & 0x10) == 0x10;
			break;
		case 0x26:
			cpu->reg.h = cpu->mem.ram[cpu->reg.pc];
			cpu->reg.pc++;
			break;
		case 0x27:
			unimplemented();break;
		case 0x28:
			unimplemented();break;
		case 0x29: {
			uint32_t res = hl + hl;
			cpu->reg.h = res >> 8;
			cpu->reg.l = (res & 0x00FF);
			cpu->flag.carry = (res & 0xFFFF0000) != 0;
			break;
		}
		case 0x2A:
			unimplemented();break;
		case 0x2B:
			unimplemented();break;
		case 0x2C:
			cpu->reg.l++;
			cpu->flag.sign = check_sign(cpu->reg.l);
			cpu->flag.zero = check_zero(cpu->reg.l);
			cpu->flag.parity = check_parity(cpu->reg.l);
			cpu->flag.aux = ((cpu->reg.l & 0xF) + 1) > 0xF;
			break;
		case 0x2D:
			cpu->reg.l--;
			cpu->flag.sign = check_sign(cpu->reg.l);
			cpu->flag.zero = check_zero(cpu->reg.l);
			cpu->flag.parity = check_parity(cpu->reg.l);
			cpu->flag.aux = (((cpu->reg.l & 0x0F) + ((~0x01 & 0x0F) + 1)) & 0x10) == 0x10;
			break;
		case 0x2E:
			cpu->reg.l = cpu->mem.ram[cpu->reg.pc];
			cpu->reg.pc++;
			break;
		case 0x2F:
			unimplemented();break;
		case 0x30:
			unimplemented();break;
		case 0x31:
			cpu->reg.sp = (cpu->mem.ram[cpu->reg.pc + 1] << 8) | cpu->mem.ram[cpu->reg.pc];
			cpu->reg.pc += 2;
			break;
		case 0x32:
			unimplemented();break;
		case 0x33:
			unimplemented();break;
		case 0x34:
			unimplemented();break;
		case 0x35:
			unimplemented();break;
		case 0x36:
			cpu->mem.ram[hl] = cpu->mem.ram[cpu->reg.pc];
			cpu->reg.pc++;
			break;
		case 0x37:
			unimplemented();break;
		case 0x38:
			unimplemented();break;
		case 0x39:
			unimplemented();break;
		case 0x3A:
			unimplemented();break;
		case 0x3B:
			unimplemented();break;
		case 0x3C:
			cpu->reg.a++;
			cpu->flag.sign = check_sign(cpu->reg.a);
			cpu->flag.zero = check_zero(cpu->reg.a);
			cpu->flag.parity = check_parity(cpu->reg.a);
			cpu->flag.aux = ((cpu->reg.a & 0xF) + (cpu->mem.ram[cpu->reg.pc] & 0xF)) > 0xF;
			break;
		case 0x3D:
			cpu->reg.a--;
			cpu->flag.sign = check_sign(cpu->reg.a);
			cpu->flag.zero = check_zero(cpu->reg.a);
			cpu->flag.parity = check_parity(cpu->reg.a);
			cpu->flag.aux = (((cpu->reg.a & 0x0F) + ((~0x01 & 0x0F) + 1)) & 0x10) == 0x10;
			break;
		case 0x3E:
			cpu->reg.a = cpu->mem.ram[cpu->reg.pc];
			cpu->reg.pc++;
			break;
		case 0x3F:
			unimplemented();break;
		case 0x40:
			cpu->reg.b = cpu->reg.b;
			break;
		case 0x41:
			cpu->reg.b = cpu->reg.c;
			break;
		case 0x42:
			cpu->reg.b = cpu->reg.d;
			break;
		case 0x43:
			cpu->reg.b = cpu->reg.e;
			break;
		case 0x44:
			cpu->reg.b = cpu->reg.h;
			break;
		case 0x45:
			cpu->reg.b = cpu->reg.l;
			break;
		case 0x46:
			cpu->reg.b = cpu->mem.ram[hl];
			break;
		case 0x47:
			cpu->reg.b = cpu->reg.a;
			break;
		case 0x48:
			cpu->reg.c = cpu->reg.b;
			break;
		case 0x49:
			cpu->reg.c = cpu->reg.c;
			break;
		case 0x4A:
			cpu->reg.c = cpu->reg.d;
			break;
		case 0x4B:
			cpu->reg.c = cpu->reg.e;
			break;
		case 0x4C:
			cpu->reg.c = cpu->reg.h;
			break;
		case 0x4D:
			cpu->reg.c = cpu->reg.l;
			break;
		case 0x4E:
			cpu->reg.c = cpu->mem.ram[hl];
			break;
		case 0x4F:
			cpu->reg.c = cpu->reg.a;
			break;
		case 0x50:
			cpu->reg.d = cpu->reg.b;
			break;
		case 0x51:
			cpu->reg.d = cpu->reg.c;
			break;
		case 0x52:
			cpu->reg.d = cpu->reg.d;
			break;
		case 0x53:
			cpu->reg.d = cpu->reg.e;
			break;
		case 0x54:
			cpu->reg.d = cpu->reg.h;
			break;
		case 0x55:
			cpu->reg.d = cpu->reg.l;
			break;
		case 0x56:
			cpu->reg.d = cpu->mem.ram[hl];
			break;
		case 0x57:
			cpu->reg.d = cpu->reg.a;
			break;
		case 0x58:
			cpu->reg.e = cpu->reg.b;
			break;
		case 0x59:
			cpu->reg.e = cpu->reg.c;
			break;
		case 0x5A:
			cpu->reg.e = cpu->reg.d;
			break;
		case 0x5B:
			cpu->reg.e = cpu->reg.e;
			break;
		case 0x5C:
			cpu->reg.e = cpu->reg.h;
			break;
		case 0x5D:
			cpu->reg.e = cpu->reg.l;
			break;
		case 0x5E:
			cpu->reg.e = cpu->mem.ram[hl];
			break;
		case 0x5F:
			cpu->reg.e = cpu->reg.a;
			break;
		case 0x60:
			cpu->reg.h = cpu->reg.b;
			break;
		case 0x61:
			cpu->reg.h = cpu->reg.c;
			break;
		case 0x62:
			cpu->reg.h = cpu->reg.d;
			break;
		case 0x63:
			cpu->reg.h = cpu->reg.e;
			break;
		case 0x64:
			cpu->reg.h = cpu->reg.h;
			break;
		case 0x65:
			cpu->reg.h = cpu->reg.l;
			break;
		case 0x66:
			cpu->reg.h = cpu->mem.ram[hl];
			break;
		case 0x67:
			cpu->reg.h = cpu->reg.a;
			break;
		case 0x68:
			cpu->reg.l = cpu->reg.b;
			break;
		case 0x69:
			cpu->reg.l = cpu->reg.c;
			break;
		case 0x6A:
			cpu->reg.l = cpu->reg.d;
			break;
		case 0x6B:
			cpu->reg.l = cpu->reg.e;
			break;
		case 0x6C:
			cpu->reg.l = cpu->reg.h;
			break;
		case 0x6D:
			cpu->reg.l = cpu->reg.l;
			break;
		case 0x6E:
			cpu->reg.l = cpu->mem.ram[hl]; 
			break;
		case 0x6F:
			cpu->reg.l = cpu->reg.a;
			break;
		case 0x70:
			cpu->mem.ram[hl] = cpu->reg.b;
			break;
		case 0x71:
			cpu->mem.ram[hl] = cpu->reg.c;
			break;
		case 0x72:
			cpu->mem.ram[hl] = cpu->reg.d;
			break;
		case 0x73:
			cpu->mem.ram[hl] = cpu->reg.e;
			break;
		case 0x74:
			cpu->mem.ram[hl] = cpu->reg.h;
			break;
		case 0x75:
			cpu->mem.ram[hl] = cpu->reg.l;
			break;
		case 0x76: // HALT
			break;
		case 0x77:
			cpu->mem.ram[(cpu->reg.h << 8) | cpu->reg.l] = cpu->reg.a;
			break;
		case 0x78:
			cpu->reg.a = cpu->reg.b;
			break;
		case 0x79:
			cpu->reg.a = cpu->reg.c;
			break;
		case 0x7A:
			cpu->reg.a = cpu->reg.d;
			break;
		case 0x7B:
			cpu->reg.a = cpu->reg.e;
			break;
		case 0x7C:
			cpu->reg.a = cpu->reg.h;
			break;
		case 0x7D:
			cpu->reg.a = cpu->reg.l;
			break;
		case 0x7E:
			cpu->reg.a = cpu->mem.ram[hl]; 
			break;
		case 0x7F:
			cpu->reg.a = cpu->reg.a;
			break;
		case 0x80: {
			uint8_t res = cpu->reg.a + cpu->reg.b;
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = (uint16_t)(cpu->reg.a) + (uint16_t)(cpu->reg.b) > 0xFF;
			cpu->flag.aux =  ((cpu->reg.a & 0xF) + (cpu->reg.b & 0xF)) > 0xF;
			cpu->reg.a = res;
			break;
		}
			break;
		case 0x81: {
			uint8_t res = cpu->reg.a + cpu->reg.c;
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = (uint16_t)(cpu->reg.a) + (uint16_t)(cpu->reg.c) > 0xFF;
			cpu->flag.aux =  ((cpu->reg.a & 0xF) + (cpu->reg.c & 0xF)) > 0xF;
			cpu->reg.a = res;
			break;
		}
		case 0x82: {
			uint8_t res = cpu->reg.a + cpu->reg.d;
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = (uint16_t)(cpu->reg.a) + (uint16_t)(cpu->reg.d) > 0xFF;
			cpu->flag.aux =  ((cpu->reg.a & 0xF) + (cpu->reg.d & 0xF)) > 0xF;
			cpu->reg.a = res;
			break;
		}
		case 0x83: {
			uint8_t res = cpu->reg.a + cpu->reg.e;
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = (uint16_t)(cpu->reg.a) + (uint16_t)(cpu->reg.e) > 0xFF;
			cpu->flag.aux =  ((cpu->reg.a & 0xF) + (cpu->reg.e & 0xF)) > 0xF;
			cpu->reg.a = res;
			break;
		}
		case 0x84: {
			uint8_t res = cpu->reg.a + cpu->reg.h;
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = (uint16_t)(cpu->reg.a) + (uint16_t)(cpu->reg.h) > 0xFF;
			cpu->flag.aux =  ((cpu->reg.a & 0xF) + (cpu->reg.h & 0xF)) > 0xF;
			cpu->reg.a = res;
			break;
		}
		case 0x85:{
			uint8_t res = cpu->reg.a + cpu->reg.l;
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = (uint16_t)(cpu->reg.a) + (uint16_t)(cpu->reg.l) > 0xFF;
			cpu->flag.aux =  ((cpu->reg.a & 0xF) + (cpu->reg.l & 0xF)) > 0xF;
			cpu->reg.a = res;
			break;
		}
		case 0x86: unimplemented();break;
		case 0x87:{
			uint8_t res = cpu->reg.a + cpu->reg.a;
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = (uint16_t)(cpu->reg.a) + (uint16_t)(cpu->reg.a) > 0xFF;
			cpu->flag.aux = ((cpu->reg.a & 0xF) + (cpu->reg.a & 0xF)) > 0xF;
			cpu->reg.a = res;
			break;
		}
		case 0x88:{
			uint8_t res = cpu->reg.a + cpu->reg.b + cpu->flag.carry;
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = (uint16_t)(cpu->reg.a) + (uint16_t)(cpu->reg.b)  > 0xFF;
			cpu->flag.aux =  ((cpu->reg.a & 0xF) + (cpu->reg.b & 0xF) ) > 0xF;
			cpu->reg.a = res;
			break;
		}
		case 0x89: {
			uint8_t res = cpu->reg.a + cpu->reg.c + cpu->flag.carry;
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = (uint16_t)(cpu->reg.a) + (uint16_t)(cpu->reg.c)  > 0xFF;
			cpu->flag.aux =  ((cpu->reg.a & 0xF) + (cpu->reg.c & 0xF) ) > 0xF;
			cpu->reg.a = res;
			break;
		}
		case 0x8A: {
			uint8_t res = cpu->reg.a + cpu->reg.d + cpu->flag.carry;
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = (uint16_t)(cpu->reg.a) + (uint16_t)(cpu->reg.d)  > 0xFF;
			cpu->flag.aux =  ((cpu->reg.a & 0xF) + (cpu->reg.d & 0xF) ) > 0xF;
			cpu->reg.a = res;
			break;
		}
		case 0x8B: {
			uint8_t res = cpu->reg.a + cpu->reg.e + cpu->flag.carry;
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = (uint16_t)(cpu->reg.a) + (uint16_t)(cpu->reg.e)  > 0xFF;
			cpu->flag.aux =  ((cpu->reg.a & 0xF) + (cpu->reg.e & 0xF) ) > 0xF;
			cpu->reg.a = res;
			break;
		}
		case 0x8C: {
			uint8_t res = cpu->reg.a + cpu->reg.h + cpu->flag.carry;
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = (uint16_t)(cpu->reg.a) + (uint16_t)(cpu->reg.h) > 0xFF;
			cpu->flag.aux =  ((cpu->reg.a & 0xF) + (cpu->reg.h & 0xF) ) > 0xF;
			cpu->reg.a = res;
			break;
		}
		case 0x8D: {
			uint8_t res = cpu->reg.a + cpu->reg.l + cpu->flag.carry;
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = (uint16_t)(cpu->reg.a) + (uint16_t)(cpu->reg.l) > 0xFF;
			cpu->flag.aux =  ((cpu->reg.a & 0xF) + (cpu->reg.l & 0xF) ) > 0xF;
			cpu->reg.a = res;
			break;
		}
		case 0x8E:
			unimplemented();break;
		case 0x8F: {
			uint8_t res = cpu->reg.a + cpu->reg.a + cpu->flag.carry;
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = (uint16_t)(cpu->reg.a) + (uint16_t)(cpu->reg.a) > 0xFF;
			cpu->flag.aux =  ((cpu->reg.a & 0xF) + (cpu->reg.a & 0xF) ) > 0xF;
			cpu->reg.a = res;
			break;
		}

		case 0x90: {
			uint8_t res = cpu->reg.a - cpu->reg.b;
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = !(((uint16_t)cpu->reg.a + (uint16_t)~cpu->reg.b + 1) >> 8);
			cpu->flag.aux = (((cpu->reg.a & 0x0F) + ((~cpu->reg.b & 0x0F) + 1)) & 0x10) == 0x10;
			cpu->reg.a = res;
			break;
		}
		case 0x91: {
			uint8_t res = cpu->reg.a - cpu->reg.c;
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = !(((uint16_t)cpu->reg.a + (uint16_t)~cpu->reg.c + 1) >> 8);
			cpu->flag.aux = (((cpu->reg.a & 0x0F) + ((~cpu->reg.c & 0x0F) + 1)) & 0x10) == 0x10;
			cpu->reg.a = res;
			break;
		}
		case 0x92: {
			uint8_t res = cpu->reg.a - cpu->reg.d;
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = !(((uint16_t)cpu->reg.a + (uint16_t)~cpu->reg.d + 1) >> 8);
			cpu->flag.aux = (((cpu->reg.a & 0x0F) + ((~cpu->reg.d & 0x0F) + 1)) & 0x10) == 0x10;
			cpu->reg.a = res;
			break;
		}
		case 0x93: {
			uint8_t res = cpu->reg.a - cpu->reg.e;
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = !(((uint16_t)cpu->reg.a + (uint16_t)~cpu->reg.e + 1) >> 8);
			cpu->flag.aux = (((cpu->reg.a & 0x0F) + ((~cpu->reg.e & 0x0F) + 1)) & 0x10) == 0x10;
			cpu->reg.a = res;
			break;
		}
		case 0x94: {
			uint8_t res = cpu->reg.a - cpu->reg.h;
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = !(((uint16_t)cpu->reg.a + (uint16_t)~cpu->reg.h + 1) >> 8);
			cpu->flag.aux = (((cpu->reg.a & 0x0F) + ((~cpu->reg.h & 0x0F) + 1)) & 0x10) == 0x10;
			cpu->reg.a = res;
			break;
		}
		case 0x95:{
			uint8_t res = cpu->reg.a - cpu->reg.l;
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = !(((uint16_t)cpu->reg.a + (uint16_t)~cpu->reg.l + 1) >> 8);
			cpu->flag.aux = (((cpu->reg.a & 0x0F) + ((~cpu->reg.l & 0x0F) + 1)) & 0x10) == 0x10;
			cpu->reg.a = res;
			break;
		}
		case 0x96:
			unimplemented();break;
		case 0x97: {
			uint8_t res = cpu->reg.a - cpu->reg.a;
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = !(((uint16_t)cpu->reg.a + (uint16_t)~cpu->reg.a + 1) >> 8);
			cpu->flag.aux = (((cpu->reg.a & 0x0F) + ((~cpu->reg.a & 0x0F) + 1)) & 0x10) == 0x10;
			cpu->reg.a = res;
			break;
		}
		case 0x98: {

			break;
		}
		case 0x99:
			unimplemented();break;
		case 0x9A:
			unimplemented();break;
		case 0x9B:
			unimplemented();break;
		case 0x9C:
			unimplemented();break;
		case 0x9D:
			unimplemented();break;
		case 0x9E:
			unimplemented();break;
		case 0x9F:
			unimplemented();break;
		case 0xA0:
			unimplemented();break;
		case 0xA1:
			unimplemented();break;
		case 0xA2:
			unimplemented();break;
		case 0xA3:
			unimplemented();break;
		case 0xA4:
			unimplemented();break;
		case 0xA5:
			unimplemented();break;
		case 0xA6:
			unimplemented();break;
		case 0xA7:
			unimplemented();break;
		case 0xA8:
			unimplemented();break;
		case 0xA9:
			unimplemented();break;
		case 0xAA:
			unimplemented();break;
		case 0xAB:
			unimplemented();break;
		case 0xAC:
			unimplemented();break;
		case 0xAD:
			unimplemented();break;
		case 0xAE:
			unimplemented();break;
		case 0xAF: {
			uint8_t res = cpu->reg.a ^ cpu->reg.a; 
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = 0;
			cpu->flag.aux = 0;
			cpu->reg.a = res;
			break;
		}
		case 0xB0:
			unimplemented();break;
		case 0xB1:
			unimplemented();break;
		case 0xB2:
			unimplemented();break;
		case 0xB3:
			unimplemented();break;
		case 0xB4:
			unimplemented();break;
		case 0xB5:
			unimplemented();break;
		case 0xB6:
			unimplemented();break;
		case 0xB7:
			unimplemented();break;
		case 0xB8:
			unimplemented();break;
		case 0xB9:
			unimplemented();break;
		case 0xBA:
			unimplemented();break;
		case 0xBB:
			unimplemented();break;
		case 0xBC:
			unimplemented();break;
		case 0xBD:
			unimplemented();break;
		case 0xBE:
			unimplemented();break;
		case 0xBF:
			unimplemented();break;
		case 0xC0:
			if (!cpu->flag.zero) {
				cpu->reg.pc = (cpu->mem.ram[cpu->reg.sp+1] << 8) | cpu->mem.ram[cpu->reg.sp];
				cpu->reg.sp += 2;
			}
			break;
		case 0xC1:
			unimplemented();break;
		case 0xC2:
			if (cpu->flag.zero == 0) {
				cpu->reg.pc = (cpu->mem.ram[cpu->reg.pc + 1] << 8) | cpu->mem.ram[cpu->reg.pc];
			} else {
				cpu->reg.pc += 2;
			}
			break;
		case 0xC3: 
			cpu->reg.pc = (cpu->mem.ram[cpu->reg.pc + 1] << 8) | cpu->mem.ram[cpu->reg.pc];
			break;
		case 0xC4:
			if (!cpu->flag.zero) {
				uint16_t ret_addr = cpu->reg.pc + 2;
				cpu->mem.ram[cpu->reg.sp-1] = (ret_addr >> 8);
				cpu->mem.ram[cpu->reg.sp-2] = ret_addr & 0xFF;
				cpu->reg.sp -= 2;
				cpu->reg.pc = (cpu->mem.ram[cpu->reg.pc + 1] << 8) | cpu->mem.ram[cpu->reg.pc];
			} else {
				cpu->reg.pc += 2;
			}
			break;
		case 0xC5:
			unimplemented();break;
		case 0xC6: {
			uint8_t res = cpu->reg.a + cpu->mem.ram[cpu->reg.pc];
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = (uint16_t)(cpu->reg.a) + (uint16_t)(cpu->mem.ram[cpu->reg.pc]) > 0xFF;
			cpu->flag.aux =  ((cpu->reg.a & 0xF) + (cpu->mem.ram[cpu->reg.pc] & 0xF)) > 0xF;
			cpu->reg.a = res;
			cpu->reg.pc++;
			break;
		}
		case 0xC7:
			unimplemented();break;
		case 0xC8:
			if (cpu->flag.zero) {
				cpu->reg.pc = (cpu->mem.ram[cpu->reg.sp+1] << 8) | cpu->mem.ram[cpu->reg.sp];
				cpu->reg.sp += 2;
			}
			break;
		case 0xC9:
			cpu->reg.pc = (cpu->mem.ram[cpu->reg.sp+1] << 8) | cpu->mem.ram[cpu->reg.sp];
			cpu->reg.sp += 2;
			break;
		case 0xCA:
			if (cpu->flag.zero) {
				cpu->reg.pc = (cpu->mem.ram[cpu->reg.pc + 1] << 8) | cpu->mem.ram[cpu->reg.pc];
			} else {
				cpu->reg.pc += 2;
			}
			break;
		case 0xCB:
			unimplemented();break;
		case 0xCC:
			if (cpu->flag.zero) {
				uint16_t ret_addr = cpu->reg.pc + 2;
				cpu->mem.ram[cpu->reg.sp-1] = (ret_addr >> 8);
				cpu->mem.ram[cpu->reg.sp-2] = ret_addr & 0xFF;
				cpu->reg.sp -= 2;
				cpu->reg.pc = (cpu->mem.ram[cpu->reg.pc + 1] << 8) | cpu->mem.ram[cpu->reg.pc];
			} else {
				cpu->reg.pc += 2;
			}
			break;
		case 0xCD: {
			// CALL a16
			uint16_t ret_addr = cpu->reg.pc + 2;
			cpu->mem.ram[cpu->reg.sp-1] = (ret_addr >> 8);
			cpu->mem.ram[cpu->reg.sp-2] = ret_addr & 0xFF;
			cpu->reg.sp -= 2;
			cpu->reg.pc = (cpu->mem.ram[cpu->reg.pc + 1] << 8) | cpu->mem.ram[cpu->reg.pc];
			break;
		}
		case 0xCE: {
			uint8_t res = cpu->reg.a + cpu->mem.ram[cpu->reg.pc] + cpu->flag.carry;
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = (uint16_t)(cpu->reg.a) + (uint16_t)(cpu->mem.ram[cpu->reg.pc] + cpu->flag.carry) > 0xFF;
			cpu->flag.aux =  ((cpu->reg.a & 0xF) + (cpu->mem.ram[cpu->reg.pc] & 0xF) + cpu->flag.carry) > 0xF;
			cpu->reg.a = res;
			cpu->reg.pc++;
			break;
		}
		case 0xCF:
			unimplemented();break;
		case 0xD0:
			if (!cpu->flag.carry) {
				cpu->reg.pc = (cpu->mem.ram[cpu->reg.sp+1] << 8) | cpu->mem.ram[cpu->reg.sp];
				cpu->reg.sp += 2;
			}
			break;
		case 0xD1:
			cpu->reg.d = cpu->mem.ram[cpu->reg.sp + 1];
			cpu->reg.e = cpu->mem.ram[cpu->reg.sp];			
			cpu->reg.sp += 2;
			break;
		case 0xD2:
			if (cpu->flag.carry == 0) {
				cpu->reg.pc = (cpu->mem.ram[cpu->reg.pc + 1] << 8) | cpu->mem.ram[cpu->reg.pc];
			} else {
				cpu->reg.pc += 2;
			}
			break;
		case 0xD3:
			cpu->reg.pc++;
			break;
		case 0xD4:
			if (!cpu->flag.carry) {
				uint16_t ret_addr = cpu->reg.pc + 2;
				cpu->mem.ram[cpu->reg.sp-1] = (ret_addr >> 8);
				cpu->mem.ram[cpu->reg.sp-2] = ret_addr & 0xFF;
				cpu->reg.sp -= 2;
				cpu->reg.pc = (cpu->mem.ram[cpu->reg.pc + 1] << 8) | cpu->mem.ram[cpu->reg.pc];
			} else {
				cpu->reg.pc += 2;
			}
			break;
		case 0xD5:
			cpu->mem.ram[cpu->reg.sp-1] = cpu->reg.d;
			cpu->mem.ram[cpu->reg.sp-2] = cpu->reg.e;
			cpu->reg.sp -= 2;
			break;
		case 0xD6: {
			uint8_t res = cpu->reg.a - cpu->mem.ram[cpu->reg.pc];
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = cpu->reg.a < cpu->mem.ram[cpu->reg.pc];
			cpu->flag.aux = (((cpu->reg.a & 0x0F) + ((~cpu->mem.ram[cpu->reg.pc] & 0x0F) + 1)) & 0x10) == 0x10;
			cpu->reg.a = res;
			cpu->reg.pc++;
			break;
		}
		case 0xD7:
			unimplemented();break;
		case 0xD8:
			if (cpu->flag.carry) {
				cpu->reg.pc = (cpu->mem.ram[cpu->reg.sp+1] << 8) | cpu->mem.ram[cpu->reg.sp];
				cpu->reg.sp += 2;
			}
			break;
		case 0xD9:
			unimplemented();break;
		case 0xDA:
			if (cpu->flag.carry) {
				cpu->reg.pc = (cpu->mem.ram[cpu->reg.pc + 1] << 8) | cpu->mem.ram[cpu->reg.pc];
			} else {
				cpu->reg.pc += 2;
			}
			break;
		case 0xDB:
			unimplemented();break;
		case 0xDC:
			if (cpu->flag.carry) {
				uint16_t ret_addr = cpu->reg.pc + 2;
				cpu->mem.ram[cpu->reg.sp-1] = (ret_addr >> 8);
				cpu->mem.ram[cpu->reg.sp-2] = ret_addr & 0xFF;
				cpu->reg.sp -= 2;
				cpu->reg.pc = (cpu->mem.ram[cpu->reg.pc + 1] << 8) | cpu->mem.ram[cpu->reg.pc];
			} else {
				cpu->reg.pc += 2;
			}
			break;
		case 0xDD:
			unimplemented();break;
		case 0xDE: {
			uint8_t res = cpu->reg.a - (cpu->flag.carry + cpu->mem.ram[cpu->reg.pc]);
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = cpu->reg.a < cpu->mem.ram[cpu->reg.pc];
			cpu->flag.aux = (((cpu->reg.a & 0x0F) + ((~cpu->mem.ram[cpu->reg.pc] & 0x0F) + 1)) & 0x10) == 0x10;
			cpu->reg.a = res;
			cpu->reg.pc++;
			break;
		}
		case 0xDF:
			unimplemented();break;
		case 0xE0:
			if (cpu->flag.parity == 0) {
				cpu->reg.pc = (cpu->mem.ram[cpu->reg.sp+1] << 8) | cpu->mem.ram[cpu->reg.sp];
				cpu->reg.sp += 2;
			}
			break;
		case 0xE1:
			cpu->reg.h = cpu->mem.ram[cpu->reg.sp + 1];
			cpu->reg.l = cpu->mem.ram[cpu->reg.sp];			
			cpu->reg.sp += 2;
			break;
		case 0xE2:
			if (cpu->flag.parity == 0) {
				cpu->reg.pc = (cpu->mem.ram[cpu->reg.pc + 1] << 8) | cpu->mem.ram[cpu->reg.pc];
			} else {
				cpu->reg.pc += 2;
			}
			break;
		case 0xE3:
			unimplemented();break;
		case 0xE4:
			if (!cpu->flag.parity) {
				uint16_t ret_addr = cpu->reg.pc + 2;
				cpu->mem.ram[cpu->reg.sp-1] = (ret_addr >> 8);
				cpu->mem.ram[cpu->reg.sp-2] = ret_addr & 0xFF;
				cpu->reg.sp -= 2;
				cpu->reg.pc = (cpu->mem.ram[cpu->reg.pc + 1] << 8) | cpu->mem.ram[cpu->reg.pc];
			} else {
				cpu->reg.pc += 2;
			}
			break;
		case 0xE5:
			cpu->mem.ram[cpu->reg.sp-1] = cpu->reg.h;
			cpu->mem.ram[cpu->reg.sp-2] = cpu->reg.l;
			cpu->reg.sp -= 2;
			break;
		case 0xE6: {
			uint8_t res = cpu->reg.a & cpu->mem.ram[cpu->reg.pc];
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = 0;
			cpu->flag.aux = ((cpu->reg.a | cpu->mem.ram[cpu->reg.pc]) & 0x80) != 0;
			cpu->reg.a = res;
			cpu->reg.pc++;
			break;
		}
		case 0xE7:
			unimplemented();break;
		case 0xE8:
			if (cpu->flag.parity) {
				cpu->reg.pc = (cpu->mem.ram[cpu->reg.sp+1] << 8) | cpu->mem.ram[cpu->reg.sp];
				cpu->reg.sp += 2;
			}
			break;
		case 0xE9:
			unimplemented();break;
		case 0xEA:
			if (cpu->flag.parity) {
				cpu->reg.pc = (cpu->mem.ram[cpu->reg.pc + 1] << 8) | cpu->mem.ram[cpu->reg.pc];
			} else {
				cpu->reg.pc += 2;
			}
			break;
		case 0xEB: {
			uint8_t tmp1 = cpu->reg.h;
			uint8_t tmp2 = cpu->reg.l;
			cpu->reg.h = cpu->reg.d;
			cpu->reg.l = cpu->reg.e;
			cpu->reg.d = tmp1;
			cpu->reg.e = tmp2;		
			break;
		}
		case 0xEC:
			if (cpu->flag.parity) {
				uint16_t ret_addr = cpu->reg.pc + 2;
				cpu->mem.ram[cpu->reg.sp-1] = (ret_addr >> 8);
				cpu->mem.ram[cpu->reg.sp-2] = ret_addr & 0xFF;
				cpu->reg.sp -= 2;
				cpu->reg.pc = (cpu->mem.ram[cpu->reg.pc + 1] << 8) | cpu->mem.ram[cpu->reg.pc];
			} else {
				cpu->reg.pc += 2;
			}
			break;
		case 0xED:
			unimplemented();break;
		case 0xEE: {
			uint8_t res = cpu->reg.a ^ cpu->mem.ram[cpu->reg.pc];
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = 0;
			cpu->flag.aux = 0;
			cpu->reg.a = res;
			cpu->reg.pc++;
			break;
		}
		case 0xEF:
			unimplemented();break;
		case 0xF0:
			if (!cpu->flag.sign) {
				cpu->reg.pc = (cpu->mem.ram[cpu->reg.sp+1] << 8) | cpu->mem.ram[cpu->reg.sp];
				cpu->reg.sp += 2;
			}
			break;
		case 0xF1:
			unimplemented();break;
		case 0xF2:
			if (cpu->flag.sign == 0) {
				cpu->reg.pc = (cpu->mem.ram[cpu->reg.pc + 1] << 8) | cpu->mem.ram[cpu->reg.pc];
			} else {
				cpu->reg.pc += 2;
			}
			break;
		case 0xF3:
			unimplemented();break;
		case 0xF4:
			if (!cpu->flag.sign) {
				uint16_t ret_addr = cpu->reg.pc + 2;
				cpu->mem.ram[cpu->reg.sp-1] = (ret_addr >> 8);
				cpu->mem.ram[cpu->reg.sp-2] = ret_addr & 0xFF;
				cpu->reg.sp -= 2;
				cpu->reg.pc = (cpu->mem.ram[cpu->reg.pc + 1] << 8) | cpu->mem.ram[cpu->reg.pc];
			} else {
				cpu->reg.pc += 2;
			}
			break;
		case 0xF5:
			unimplemented();break;
		case 0xF6: {
			uint8_t res = cpu->reg.a | cpu->mem.ram[cpu->reg.pc];
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = 0;
			cpu->flag.aux = 0;
			cpu->reg.a = res;
			cpu->reg.pc++;
			break;
		}
		case 0xF7:
			unimplemented();break;
		case 0xF8:
			if (cpu->flag.sign) {
				cpu->reg.pc = (cpu->mem.ram[cpu->reg.sp+1] << 8) | cpu->mem.ram[cpu->reg.sp];
				cpu->reg.sp += 2;
			}
			break;
		case 0xF9:
			unimplemented();break;
		case 0xFA:
			if (cpu->flag.sign) {
				cpu->reg.pc = (cpu->mem.ram[cpu->reg.pc + 1] << 8) | cpu->mem.ram[cpu->reg.pc];
			} else {
				cpu->reg.pc += 2;
			}
			break;
		case 0xFB:
			unimplemented();break;
		case 0xFC:
			if (cpu->flag.sign) {
				uint16_t ret_addr = cpu->reg.pc + 2;
				cpu->mem.ram[cpu->reg.sp-1] = (ret_addr >> 8);
				cpu->mem.ram[cpu->reg.sp-2] = ret_addr & 0xFF;
				cpu->reg.sp -= 2;
				cpu->reg.pc = (cpu->mem.ram[cpu->reg.pc + 1] << 8) | cpu->mem.ram[cpu->reg.pc];
			} else {
				cpu->reg.pc += 2;
			}
			break;
		case 0xFD:
			unimplemented();break;
		case 0xFE: {
			uint8_t res = cpu->reg.a - cpu->mem.ram[cpu->reg.pc];
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = cpu->reg.a < cpu->mem.ram[cpu->reg.pc];
			cpu->flag.aux = (((cpu->reg.a & 0x0F) + ((~cpu->mem.ram[cpu->reg.pc] & 0x0F) + 1)) & 0x10) == 0x10;
			cpu->reg.pc++;
			break;
		}
		case 0xFF:
			unimplemented();break;
	}

	printf("\n");
}

int main(int argc, char *argv[]) {
	Cpu cpu = cpu_new();

	const char *rom = "../cpu_tests/TST8080.COM";

	FILE *fp = fopen(rom, "r");
	if (fp == NULL) {
		return EXIT_FAILURE;
	}

	long size = fsize(fp);

	uint8_t bytes[size];
	const size_t count = fread(bytes, sizeof(bytes[0]), size, fp);

	if (count == size) {
		load_memory(&cpu.mem, ARRAY_LENGTH(bytes), bytes);
	} else {
		if (feof(fp)) {
			printf("Error reading %s: unexpected eof\n", rom);
		}
	}

	// if (strcmp(argv[1], "-d") == 0) {
	// 	for (int i = 0; i < size; i++) {
	// 		disassemble(memory, &pc);
	// 	}
	// }
	
	cpu.mem.ram[0x0000] = 0xD3;
	cpu.mem.ram[0x0001] = 0x00;
	
	cpu.mem.ram[0x0005] = 0xD3;
	cpu.mem.ram[0x0006] = 0x01;
	cpu.mem.ram[0x0007] = 0xC9;

	uint16_t i = 0;
	while (true) {
		fetch(&cpu);
		
		// decode(&cpu);

		execute(&cpu);

// #ifndef TST8080
// 		 if (cpu.reg.pc == 5) {
// 			if (cpu.reg.c == 2) {
// 				printf("%c", cpu.reg.e);
// 			} else if (cpu.reg.c == 9) {
// 				uint16_t de = (cpu.reg.d << 8) | cpu.reg.e;
// 				i = de;
// 				while (cpu.mem.ram[i] != '$') {
// 					  printf("%c", cpu.mem.ram[i]);
// 					  i++;
// 				}
// 			}
// 		}
// #endif

	}
	
	fclose(fp);

	return EXIT_SUCCESS;
}
