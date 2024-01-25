#include <stdio.h>
// #include <errno.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

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

	cpu->reg.psw |= cpu->flag.sign << 7;
	cpu->reg.psw |= cpu->flag.zero << 6;
	cpu->reg.psw |= cpu->flag.aux << 4;
	cpu->reg.psw |= cpu->flag.parity << 2;
	cpu->reg.psw |= 1 << 1;
	cpu->reg.psw |= cpu->flag.carry;

	printf("PC: %.4X AF: %.4X BC: %.4X DE: %.4X HL: %.4X SP: %.4X CYC: %d     (%02X %02X %02X %02X)", cpu->reg.pc, cpu->reg.psw, 
			bc, de, hl, cpu->reg.sp, cpu->cyc, cpu->mem.ram[cpu->reg.pc], cpu->mem.ram[cpu->reg.pc+1], cpu->mem.ram[cpu->reg.pc+2], cpu->mem.ram[cpu->reg.pc+3]);

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
			unimplemented();break;
		case 0x05:;
			// Flags: S Z A P
			uint8_t res = cpu->reg.b - 1;
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
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
			unimplemented();break;
		case 0x0D:
			unimplemented();break;
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
			unimplemented();break;
		case 0x15:
			unimplemented();break;
		case 0x16:
			unimplemented();break;
		case 0x17:
			unimplemented();break;
		case 0x18:
			unimplemented();break;
		case 0x19: {
			uint32_t res = de + de;
			cpu->reg.d = res >> 8;
			cpu->reg.e = (res & 0x00FF);
			cpu->flag.carry = (res & 0xFFFF0000) != 0;
			break;
		}
		case 0x1A:
			cpu->reg.a = cpu->mem.ram[(cpu->reg.d << 8) | cpu->reg.e];
			break;
		case 0x1B:
			unimplemented();break;
		case 0x1C:
			unimplemented();break;
		case 0x1D:
			unimplemented();break;
		case 0x1E:
			unimplemented();break;
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
			unimplemented();break;
		case 0x25:
			unimplemented();break;
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
			unimplemented();break;
		case 0x2D:
			unimplemented();break;
		case 0x2E:
			unimplemented();break;
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
			unimplemented();break;
		case 0x3D:
			unimplemented();break;
		case 0x3E:
			unimplemented();break;
		case 0x3F:
			unimplemented();break;
		case 0x40:
			unimplemented();break;
		case 0x41:
			unimplemented();break;
		case 0x42:
			unimplemented();break;
		case 0x43:
			unimplemented();break;
		case 0x44:
			unimplemented();break;
		case 0x45:
			unimplemented();break;
		case 0x46:
			unimplemented();break;
		case 0x47:
			unimplemented();break;
		case 0x48:
			unimplemented();break;
		case 0x49:
			unimplemented();break;
		case 0x4A:
			unimplemented();break;
		case 0x4B:
			unimplemented();break;
		case 0x4C:
			unimplemented();break;
		case 0x4D:
			unimplemented();break;
		case 0x4E:
			unimplemented();break;
		case 0x4F:
			unimplemented();break;
		case 0x50:
			unimplemented();break;
		case 0x51:
			unimplemented();break;
		case 0x52:
			unimplemented();break;
		case 0x53:
			unimplemented();break;
		case 0x54:
			unimplemented();break;
		case 0x55:
			unimplemented();break;
		case 0x56:
			unimplemented();break;
		case 0x57:
			unimplemented();break;
		case 0x58:
			unimplemented();break;
		case 0x59:
			unimplemented();break;
		case 0x5A:
			unimplemented();break;
		case 0x5B:
			unimplemented();break;
		case 0x5C:
			unimplemented();break;
		case 0x5D:
			unimplemented();break;
		case 0x5E:
			unimplemented();break;
		case 0x5F:
			unimplemented();break;
		case 0x60:
			unimplemented();break;
		case 0x61:
			unimplemented();break;
		case 0x62:
			unimplemented();break;
		case 0x63:
			unimplemented();break;
		case 0x64:
			unimplemented();break;
		case 0x65:
			unimplemented();break;
		case 0x66:
			unimplemented();break;
		case 0x67:
			unimplemented();break;
		case 0x68:
			unimplemented();break;
		case 0x69:
			unimplemented();break;
		case 0x6A:
			unimplemented();break;
		case 0x6B:
			unimplemented();break;
		case 0x6C:
			unimplemented();break;
		case 0x6D:
			unimplemented();break;
		case 0x6E:
			unimplemented();break;
		case 0x6F:
			cpu->reg.l = cpu->reg.a;
			break;
		case 0x70:
			unimplemented();break;
		case 0x71:
			unimplemented();break;
		case 0x72:
			unimplemented();break;
		case 0x73:
			unimplemented();break;
		case 0x74:
			unimplemented();break;
		case 0x75:
			unimplemented();break;
		case 0x76:
			unimplemented();break;
		case 0x77:
			cpu->mem.ram[(cpu->reg.h << 8) | cpu->reg.l] = cpu->reg.a;
			break;
		case 0x78:
			unimplemented();break;
		case 0x79:
			unimplemented();break;
		case 0x7A:
			unimplemented();break;
		case 0x7B:
			unimplemented();break;
		case 0x7C:
			cpu->reg.a = cpu->reg.h;
			break;
		case 0x7D:
			unimplemented();break;
		case 0x7E:
			unimplemented();break;
		case 0x7F:
			unimplemented();break;
		case 0x80:
			unimplemented();break;
		case 0x81:
			unimplemented();break;
		case 0x82:
			unimplemented();break;
		case 0x83:
			unimplemented();break;
		case 0x84:
			unimplemented();break;
		case 0x85:
			unimplemented();break;
		case 0x86:
			unimplemented();break;
		case 0x87:
			unimplemented();break;
		case 0x88:
			unimplemented();break;
		case 0x89:
			unimplemented();break;
		case 0x8A:
			unimplemented();break;
		case 0x8B:
			unimplemented();break;
		case 0x8C:
			unimplemented();break;
		case 0x8D:
			unimplemented();break;
		case 0x8E:
			unimplemented();break;
		case 0x8F:
			unimplemented();break;
		case 0x90:
			unimplemented();break;
		case 0x91:
			unimplemented();break;
		case 0x92:
			unimplemented();break;
		case 0x93:
			unimplemented();break;
		case 0x94:
			unimplemented();break;
		case 0x95:
			unimplemented();break;
		case 0x96:
			unimplemented();break;
		case 0x97:
			unimplemented();break;
		case 0x98:
			unimplemented();break;
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
		case 0xAF:
			unimplemented();break;
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
			unimplemented();break;
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
			unimplemented();break;
		case 0xC5:
			unimplemented();break;
		case 0xC6:
			unimplemented();break;
		case 0xC7:
			unimplemented();break;
		case 0xC8:
			unimplemented();break;
		case 0xC9:
			cpu->reg.pc = (cpu->mem.ram[cpu->reg.sp+1] << 8) | cpu->mem.ram[cpu->reg.sp];
			cpu->reg.sp += 2;
			break;
		case 0xCA:
			unimplemented();break;
		case 0xCB:
			unimplemented();break;
		case 0xCC:
			unimplemented();break;
		case 0xCD: {
			// CALL a16
			uint16_t ret_addr = cpu->reg.pc + 2;
			cpu->mem.ram[cpu->reg.sp-1] = (ret_addr >> 8);
			cpu->mem.ram[cpu->reg.sp-2] = ret_addr & 0xFF;
			cpu->reg.sp -= 2;
			cpu->reg.pc = (cpu->mem.ram[cpu->reg.pc + 1] << 8) | cpu->mem.ram[cpu->reg.pc];
			break;
		}
		case 0xCE:
			unimplemented();break;
		case 0xCF:
			unimplemented();break;
		case 0xD0:
			unimplemented();break;
		case 0xD1:
			unimplemented();break;
		case 0xD2:
			unimplemented();break;
		case 0xD3:
			exit(1);
			cpu->reg.pc++;
			break;
		case 0xD4:
			unimplemented();break;
		case 0xD5:
			cpu->mem.ram[cpu->reg.sp-1] = cpu->reg.d;
			cpu->mem.ram[cpu->reg.sp-2] = cpu->reg.e;
			cpu->reg.sp -= 2;
			break;
		case 0xD6:
			unimplemented();break;
		case 0xD7:
			unimplemented();break;
		case 0xD8:
			unimplemented();break;
		case 0xD9:
			unimplemented();break;
		case 0xDA:
			unimplemented();break;
		case 0xDB:
			unimplemented();break;
		case 0xDC:
			unimplemented();break;
		case 0xDD:
			unimplemented();break;
		case 0xDE:
			unimplemented();break;
		case 0xDF:
			unimplemented();break;
		case 0xE0:
			unimplemented();break;
		case 0xE1:
			cpu->reg.h = cpu->mem.ram[cpu->reg.sp + 1];
			cpu->reg.l = cpu->mem.ram[cpu->reg.sp];			
			cpu->reg.sp += 2;
			break;
		case 0xE2:
			unimplemented();break;
		case 0xE3:
			unimplemented();break;
		case 0xE4:
			unimplemented();break;
		case 0xE5:
			cpu->mem.ram[cpu->reg.sp-1] = cpu->reg.h;
			cpu->mem.ram[cpu->reg.sp-2] = cpu->reg.l;
			cpu->reg.sp -= 2;
			break;
		case 0xE6:
			unimplemented();break;
		case 0xE7:
			unimplemented();break;
		case 0xE8:
			unimplemented();break;
		case 0xE9:
			unimplemented();break;
		case 0xEA:
			unimplemented();break;
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
			unimplemented();break;
		case 0xED:
			unimplemented();break;
		case 0xEE:
			unimplemented();break;
		case 0xEF:
			unimplemented();break;
		case 0xF0:
			unimplemented();break;
		case 0xF1:
			unimplemented();break;
		case 0xF2:
			unimplemented();break;
		case 0xF3:
			unimplemented();break;
		case 0xF4:
			unimplemented();break;
		case 0xF5:
			unimplemented();break;
		case 0xF6:
			unimplemented();break;
		case 0xF7:
			unimplemented();break;
		case 0xF8:
			unimplemented();break;
		case 0xF9:
			unimplemented();break;
		case 0xFA:
			unimplemented();break;
		case 0xFB:
			unimplemented();break;
		case 0xFC:
			unimplemented();break;
		case 0xFD:
			unimplemented();break;
		case 0xFE: {
			uint8_t res = cpu->reg.a - cpu->mem.ram[cpu->reg.pc];
			cpu->flag.sign = check_sign(res);
			cpu->flag.zero = check_zero(res);
			cpu->flag.parity = check_parity(res);
			cpu->flag.carry = cpu->mem.ram[cpu->reg.pc] > cpu->reg.a;
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
	
	uint16_t i = 0;
	while (true) {
		fetch(&cpu);
		
		// decode(&cpu);

		execute(&cpu);

#ifndef TST8080
		 if (cpu.reg.pc == 5) {
			if (cpu.reg.c == 2) {
				printf("%c", cpu.reg.e);
			} else if (cpu.reg.c == 9) {
				uint16_t de = (cpu.reg.d << 8) | cpu.reg.e;
				i = de;
				while (cpu.mem.ram[i] != '$') {
					  printf("%c", cpu.mem.ram[i]);
					  i++;
				}
			}
		}
#endif


	}
	
	fclose(fp);

	return EXIT_SUCCESS;
}
