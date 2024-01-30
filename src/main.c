#include <stdio.h>
// #include <errno.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "common.h"
#include "disassembler.h"

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

void unimplemented(void) {
	printf("instruction not implemented");
	exit(1);
}

void fetch(Cpu *cpu) {
	cpu->opcode = cpu->mem.ram[cpu->reg.pc];
}

uint8_t check_parity(const uint8_t byte) {
	uint8_t count;

	for (int i = 0; i < 8; i++) {
		if (((byte >> i) & 0x1) == 1) {
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

void cpu_set_zsp(Cpu *cpu, uint8_t result) {
	cpu->flag.zero = check_zero(result);
	cpu->flag.sign = check_sign(result);
	cpu->flag.parity = check_parity(result);
}

void cpu_sbb(Cpu *cpu, uint8_t reg) {
	uint8_t res = cpu->reg.a - (reg + cpu->flag.carry);
	cpu_set_zsp(cpu, res);
	cpu->flag.carry = ((((uint16_t)cpu->reg.a & 0xFF) 
		+ ((~((uint16_t)reg + cpu->flag.carry) + 1) & 0xFF)) >> 8) == 0;
	cpu->flag.aux = ((cpu->reg.a & 0x0F) + ((~reg + cpu->flag.carry & 0x0F)) & 0xF0) != 0;
	cpu->reg.a = res;
}

void cpu_ana(Cpu *cpu, uint8_t reg) {
	uint8_t res = cpu->reg.a & reg;
	cpu_set_zsp(cpu, res);
	cpu->flag.carry = 0;	
	cpu->flag.aux = ((cpu->reg.a | reg) & 0x8) != 0;
	cpu->reg.a = res;
}

void cpu_ora(Cpu *cpu, uint8_t reg) {
	uint8_t res = cpu->reg.a | reg;
	cpu_set_zsp(cpu, res);
	cpu->flag.carry = 0;	
	cpu->flag.aux = 0; 
	cpu->reg.a = res;
}

void cpu_sub(Cpu *cpu, uint8_t reg) {
	uint8_t res = cpu->reg.a - reg; 
	cpu_set_zsp(cpu, res);
	cpu->flag.carry = !(((uint16_t)cpu->reg.a + (uint16_t)~reg + 1) >> 8);
	cpu->flag.aux = (((cpu->reg.a & 0x0F) + ((~reg & 0x0F) + 1)) & 0x10) == 0x10;
	cpu->reg.a = res;
}

void cpu_xra(Cpu *cpu, uint8_t reg) {
	uint8_t res = cpu->reg.a ^ reg;
	cpu_set_zsp(cpu, res);
	cpu->flag.carry = 0;
	cpu->flag.aux = 0;
	cpu->reg.a = res;
}

void cpu_cmp(Cpu *cpu, uint8_t reg) {
	uint8_t res = cpu->reg.a - reg;
	cpu_set_zsp(cpu, res);
	cpu->flag.carry = cpu->reg.a < reg; 
	cpu->flag.aux = (((cpu->reg.a & 0x0F) + ((~reg & 0x0F) + 1)) & 0x10) == 0x10;
}

void cpu_add(Cpu *cpu, uint8_t reg) {
	uint8_t res = cpu->reg.a + reg;
	cpu_set_zsp(cpu, res);
	cpu->flag.carry = (uint16_t)(cpu->reg.a) + (uint16_t)(reg) > 0xFF;
	cpu->flag.aux =  ((cpu->reg.a & 0xF) + (reg & 0xF)) > 0xF;
	cpu->reg.a = res;
}

void cpu_adc(Cpu *cpu, uint8_t reg) {
	uint8_t res = cpu->reg.a + cpu->reg.a + cpu->flag.carry;
	cpu_set_zsp(cpu, res);
	cpu_add(cpu, reg);
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
		case 0x02: cpu->mem.ram[bc] = cpu->reg.a; break;
		case 0x03: {
			bc += 1;
			cpu->reg.b = bc >> 8;
			cpu->reg.c = bc & 0x00FF;
			break;
		}
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
		case 0x09: {
			uint32_t res = bc + hl;
			cpu->reg.h = res >> 8;
			cpu->reg.l = (res & 0x00FF);
			cpu->flag.carry = (res & 0xFFFF0000) != 0;
			break;
		}
		case 0x0A: cpu->reg.a = cpu->mem.ram[bc]; break;
		case 0x0B:
			bc -= 1;
			cpu->reg.b = bc >> 8;
			cpu->reg.c = bc & 0x00FF;
			break;
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
		case 0x12: cpu->mem.ram[de] = cpu->reg.a; break;
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
			de -= 1;
			cpu->reg.d = de >> 8;
			cpu->reg.e = de & 0x00FF;
			break;
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
		case 0x22: {
			uint16_t addr = (cpu->mem.ram[cpu->reg.pc + 1] << 8) | cpu->mem.ram[cpu->reg.pc];
			cpu->mem.ram[addr+1] = cpu->reg.h;
			cpu->mem.ram[addr] = cpu->reg.l;
			cpu->reg.pc += 2;
			break;
		}
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
		case 0x2A: {

			uint16_t addr = (cpu->mem.ram[cpu->reg.pc + 1] << 8) | cpu->mem.ram[cpu->reg.pc];
			cpu->reg.h = cpu->mem.ram[addr+1];
			cpu->reg.l = cpu->mem.ram[addr];
			cpu->reg.pc += 2;
			break;
				   }
		case 0x2B:
			hl -= 1;
			cpu->reg.h = hl >> 8;
			cpu->reg.l = hl & 0x00FF;
			break;
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
		case 0x2F: cpu->reg.a = ~cpu->reg.a; break;
		case 0x30:
			unimplemented();break;
		case 0x31:
			cpu->reg.sp = (cpu->mem.ram[cpu->reg.pc + 1] << 8) | cpu->mem.ram[cpu->reg.pc];
			cpu->reg.pc += 2;
			break;
		case 0x32:
			cpu->mem.ram[(cpu->mem.ram[cpu->reg.pc + 1] << 8) | cpu->mem.ram[cpu->reg.pc]] = cpu->reg.a;
			cpu->reg.pc += 2;
			break;
		case 0x33:
			unimplemented();break;
		case 0x34:
			cpu->mem.ram[hl]++;
			cpu_set_zsp(cpu, cpu->mem.ram[hl]);
			cpu->flag.aux = ((cpu->mem.ram[hl] & 0xF) + 1) > 0xF;
			break;
		case 0x35:
			cpu->mem.ram[hl]--;
			cpu_set_zsp(cpu, cpu->mem.ram[hl]);
			cpu->flag.aux = (((cpu->mem.ram[hl]  & 0x0F) + ((~0x01 & 0x0F) + 1)) & 0x10) == 0x10;
			break;
		case 0x36:
			cpu->mem.ram[hl] = cpu->mem.ram[cpu->reg.pc];
			cpu->reg.pc++;
			break;
		case 0x37: cpu->flag.carry = 1; break;
		case 0x38:
			unimplemented();break;
		case 0x39:
			unimplemented();break;
		case 0x3A: {
			uint16_t addr = (cpu->mem.ram[cpu->reg.pc + 1] << 8) | cpu->mem.ram[cpu->reg.pc];
			cpu->reg.a = cpu->mem.ram[addr]; 
			cpu->reg.pc += 2;
			break;
		}
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
		case 0x3F: cpu->flag.carry = !cpu->flag.carry; break;
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

		case 0x80: cpu_add(cpu, cpu->reg.b); break;
		case 0x81: cpu_add(cpu, cpu->reg.c); break;
		case 0x82: cpu_add(cpu, cpu->reg.d); break;
		case 0x83: cpu_add(cpu, cpu->reg.e); break;
		case 0x84: cpu_add(cpu, cpu->reg.h); break;
		case 0x85: cpu_add(cpu, cpu->reg.l); break;
		case 0x86: cpu_add(cpu, cpu->mem.ram[hl]); break;
		case 0x87: cpu_add(cpu, cpu->reg.a); break;
		case 0x88: cpu_adc(cpu, cpu->reg.b + cpu->flag.carry); break;
		case 0x89: cpu_adc(cpu, cpu->reg.c + cpu->flag.carry); break;
		case 0x8A: cpu_adc(cpu, cpu->reg.d + cpu->flag.carry); break;
		case 0x8B: cpu_adc(cpu, cpu->reg.e + cpu->flag.carry); break;
		case 0x8C: cpu_adc(cpu, cpu->reg.h + cpu->flag.carry); break;
		case 0x8D: cpu_adc(cpu, cpu->reg.l + cpu->flag.carry); break;
		case 0x8E: cpu_adc(cpu, cpu->mem.ram[hl] + cpu->flag.carry); break;
		case 0x8F: cpu_adc(cpu, cpu->reg.a + cpu->flag.carry); break;

		case 0x90: cpu_sub(cpu, cpu->reg.b); break;
		case 0x91: cpu_sub(cpu, cpu->reg.c); break;
		case 0x92: cpu_sub(cpu, cpu->reg.d); break;
		case 0x93: cpu_sub(cpu, cpu->reg.e); break;
		case 0x94: cpu_sub(cpu, cpu->reg.h); break;
		case 0x95: cpu_sub(cpu, cpu->reg.l); break;
		case 0x96: cpu_sub(cpu, cpu->mem.ram[hl]); break;
		case 0x97: cpu_sub(cpu, cpu->reg.a); break;
		case 0x98: cpu_sbb(cpu, cpu->reg.b); break;
		case 0x99: cpu_sbb(cpu, cpu->reg.c); break;
		case 0x9A: cpu_sbb(cpu, cpu->reg.d); break;
		case 0x9B: cpu_sbb(cpu, cpu->reg.e); break;
		case 0x9C: cpu_sbb(cpu, cpu->reg.h); break;
		case 0x9D: cpu_sbb(cpu, cpu->reg.l); break;
		case 0x9E: cpu_sbb(cpu, cpu->mem.ram[hl]); break;	
		case 0x9F: cpu_sbb(cpu, cpu->reg.a); break;
		
		case 0xA0: cpu_ana(cpu, cpu->reg.b); break;
		case 0xA1: cpu_ana(cpu, cpu->reg.c); break;
		case 0xA2: cpu_ana(cpu, cpu->reg.d); break;
		case 0xA3: cpu_ana(cpu, cpu->reg.e); break;
		case 0xA4: cpu_ana(cpu, cpu->reg.h); break;
		case 0xA5: cpu_ana(cpu, cpu->reg.l); break;
		case 0xA6: cpu_ana(cpu, cpu->mem.ram[hl]); break;
		case 0xA7: cpu_ana(cpu, cpu->reg.a); break;
		case 0xA8: cpu_xra(cpu, cpu->reg.b); break;
		case 0xA9: cpu_xra(cpu, cpu->reg.c); break;
		case 0xAA: cpu_xra(cpu, cpu->reg.d); break;
		case 0xAB: cpu_xra(cpu, cpu->reg.e); break;
		case 0xAC: cpu_xra(cpu, cpu->reg.h); break;
		case 0xAD: cpu_xra(cpu, cpu->reg.l); break;
		case 0xAE: cpu_xra(cpu, cpu->mem.ram[hl]); break;
		case 0xAF: cpu_xra(cpu, cpu->reg.a); break;

		case 0xB0: cpu_ora(cpu, cpu->reg.b); break;
		case 0xB1: cpu_ora(cpu, cpu->reg.c); break;
		case 0xB2: cpu_ora(cpu, cpu->reg.d); break;
		case 0xB3: cpu_ora(cpu, cpu->reg.e); break;
		case 0xB4: cpu_ora(cpu, cpu->reg.h); break;
		case 0xB5: cpu_ora(cpu, cpu->reg.l); break;
		case 0xB6: cpu_ora(cpu, cpu->mem.ram[hl]); break;
		case 0xB7: cpu_ora(cpu, cpu->reg.a); break;
		case 0xB8: cpu_cmp(cpu, cpu->reg.b); break;
		case 0xB9: cpu_cmp(cpu, cpu->reg.c); break;
		case 0xBA: cpu_cmp(cpu, cpu->reg.d); break;
		case 0xBB: cpu_cmp(cpu, cpu->reg.e); break;
		case 0xBC: cpu_cmp(cpu, cpu->reg.h); break;
		case 0xBD: cpu_cmp(cpu, cpu->reg.l); break;
		case 0xBE: cpu_cmp(cpu, cpu->mem.ram[hl]); break;
		case 0xBF: cpu_cmp(cpu, cpu->reg.a); break;

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
			// The 8080 AND instructions set `ac` to reflect the OR of bit 3 of the operands involved 
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
		load_memory(&cpu.mem, ARRAY_SIZE(bytes), bytes);
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
