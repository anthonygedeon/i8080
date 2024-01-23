#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof(x[0]))
#define MAX_MEMORY 0xFFFF

uint8_t memory[MAX_MEMORY];

void memory_write(uint8_t memory[], size_t size, const uint8_t *data) {
	if (memory == NULL) {
		printf("Error: invalid pointer");
	}
	
	for (size_t i = 0; i < size;  i++) {
		memory[i] = data[i];
	}

}

long fsize(FILE *file) {
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);
	return size;
}

// int load_rom(const char *rom) {}

// TODO: `low | (high << 8)` 

void disassemble(uint8_t memory[], uint16_t *pc) {
	uint8_t opcode = memory[*pc];

	printf("%.4X %.2X  ", *pc, opcode);
	
	switch(opcode) {
		case 0x00:
			printf("NOP");
			*pc += 1;
			break;
		case 0x01:
			printf("LXI B, %02X %02X", memory[*pc + 2], memory[*pc + 1]);
			*pc += 3;
			break;
		case 0x02:
			printf("STAX B");
			*pc += 1;
			break;
		case 0x03:
			printf("INX B");
			*pc += 1;
			break;
		case 0x04:
			printf("INR B");
			*pc += 1;
			break;
		case 0x05:
			printf("DCR B");
			*pc += 1;
			break;
		case 0x06:
			printf("MVI B, %02X", memory[*pc + 1]);
			*pc += 2;
			break;
		case 0x07:
			printf("RLC");
			*pc += 1;
			break;
		case 0x08:
			printf("NOP");
			*pc += 1;
			break;
		case 0x09:
			printf("DAD B");
			*pc += 1;
			break;
		case 0x0A:
			printf("LDAX B");
			*pc += 1;
			break;
		case 0x0B:
			printf("DCX B");
			*pc += 1;
			break;
		case 0x0C:
			printf("INC C");
			*pc += 1;
			break;
		case 0x0D:
			printf("DCR C");
			*pc += 1;
			break;
		case 0x0E:
			printf("MVI C, %02X", memory[*pc + 1]);
			*pc += 2;
			break;
		case 0x0F:
			printf("RRC");
			*pc += 1;
			break;
		case 0x10:
			printf("NOP");
			*pc += 1;
			break;
		case 0x11:
			printf("LXI D, %02X %02X", memory[*pc + 2], memory[*pc + 1]);
			*pc += 3;
			break;
		case 0x12:
			printf("STAX D");
			*pc += 1;
			break;
		case 0x13:
			printf("INX D");
			*pc += 1;
			break;
		case 0x14:
			printf("INR D");
			*pc += 1;
			break;
		case 0x15:
			printf("DCR D");
			*pc += 1;
			break;
		case 0x16:
			printf("MVI D, %02X", memory[*pc + 1]);
			*pc += 2;
			break;
		case 0x17:
			printf("RAL");
			*pc += 1;
			break;
		case 0x18:
			printf("NOP");
			*pc += 1;
			break;
		case 0x19:
			printf("DAD D");
			*pc += 1;
			break;
		case 0x1A:
			printf("LDAX D");
			*pc += 1;
			break;
		case 0x1B:
			printf("DCX D");
			*pc += 1;
			break;
		case 0x1C:
			printf("INR E");
			*pc += 1;
			break;
		case 0x1D:
			printf("DCR E");
			*pc += 1;
			break;
		case 0x1E:
			printf("MVI E, %02X", memory[*pc + 1]);
			*pc += 2;
			break;
		case 0x1F:
			printf("RAR");
			*pc += 1;
			break;
		case 0x20:
			printf("NOP");
			*pc += 1;
			break;
		case 0x21:
			printf("LXI H, %02X %02X", memory[*pc + 2], memory[*pc + 1]);
			*pc += 3;
			break;
		case 0x22:
			printf("SHLD $%02X%02X", memory[*pc + 2], memory[*pc + 1]);
			*pc += 3;
			break;
		case 0x23:
			printf("INX H");
			*pc += 1;
			break;
		case 0x24:
			printf("INR H");
			*pc += 1;
			break;
		case 0x25:
			printf("DCR H");
			*pc += 1;
			break;
		case 0x26:
			printf("MVI E, %02X", memory[*pc + 1]);
			*pc += 2;
			break;
		case 0x27:
			printf("DAA");
			*pc += 1;
			break;
		case 0x28:
			printf("NOP");
			*pc += 1;
			break;
		case 0x29:
			printf("DAD");
			*pc += 1;
			break;
		case 0x2A:
			printf("LHLD $%02X%02X", memory[*pc + 2], memory[*pc + 1]);
			*pc += 3;
			break;
		case 0x2B:
			printf("DCX H");
			*pc += 1;
			break;
		case 0x2C:
			printf("INR L");
			*pc += 1;
			break;
		case 0x2D:
			printf("DCR L");
			*pc += 1;
			break;
		case 0x2E:
			printf("MVI L, %02X", memory[*pc + 1]);
			*pc += 2;
			break;
		case 0x2F:
			printf("CMA");
			*pc += 1;
			break;
		case 0x30:
			printf("NOP");
			*pc += 1;
			break;
		case 0x31:
			printf("LXI SP, %02X %02X", memory[*pc + 2], memory[*pc + 1]);
			*pc += 3;
			break;
		case 0x32:
			printf("STA $%02X%02X", memory[*pc + 2], memory[*pc + 1]);
			*pc += 3;
			break;
		case 0x33:
			printf("INX SP");
			*pc += 1;
			break;
		case 0x34:
			printf("INR M");
			*pc += 1;
			break;
		case 0x35:
			printf("DCR M");
			*pc += 1;
			break;
		case 0x36:
			printf("MVI M, %02X", memory[*pc + 1]);
			*pc += 2;
			break;
		case 0x37:
			printf("STC");
			*pc += 1;
			break;
		case 0x38:
			printf("NOP");
			*pc += 1;
			break;
		case 0x39:
			printf("DAD SP");
			*pc += 1;
			break;
		case 0x3A:
			printf("LDA $%02X%02X", memory[*pc + 2], memory[*pc + 1]);
			*pc += 3;
			break;
		case 0x3B:
			printf("DCX SP");
			*pc += 1;
			break;
		case 0x3C:
			printf("INR A");
			*pc += 1;
			break;
		case 0x3D:
			printf("DCR A");
			*pc += 1;
			break;
		case 0x3E:
			printf("MVI A, %02X", memory[*pc + 1]);
			*pc += 2;
			break;
		case 0x3F:
			printf("CMC");
			*pc += 1;
			break;
		case 0x40:
			printf("MOV B, B");
			*pc += 1;
			break;
		case 0x41:
			printf("MOV B, C");
			*pc += 1;
			break;
		case 0x42:
			printf("MOV B, D");
			*pc += 1;
			break;
		case 0x43:
			printf("MOV B, E");
			*pc += 1;
			break;
		case 0x44:
			printf("MOV B, H");
			*pc += 1;
			break;
		case 0x45:
			printf("MOV B, L");
			*pc += 1;
			break;
		case 0x46:
			printf("MOV B, M");
			*pc += 1;
			break;
		case 0x47:
			printf("MOV B, A");
			*pc += 1;
			break;
		case 0x48:
			printf("MOV C, B");
			*pc += 1;
			break;
		case 0x49:
			printf("MOV C, C");
			*pc += 1;
			break;
		case 0x4A:
			printf("MOV C, D");
			*pc += 1;
			break;
		case 0x4B:
			printf("MOV C, E");
			*pc += 1;
			break;
		case 0x4C:
			printf("MOV C, H");
			*pc += 1;
			break;
		case 0x4D:
			printf("MOV C, L");
			*pc += 1;
			break;
		case 0x4E:
			printf("MOV C, M");
			*pc += 1;
			break;
		case 0x4F:
			printf("MOV C, A");
			*pc += 1;
			break;
		case 0x50:
			printf("MOV D, B");
			*pc += 1;
			break;
		case 0x51:
			printf("MOV D, C");
			*pc += 1;
			break;
		case 0x52:
			printf("MOV D, D");
			*pc += 1;
			break;
		case 0x53:
			printf("MOV D, E");
			*pc += 1;
			break;
		case 0x54:
			printf("MOV D, H");
			*pc += 1;
			break;
		case 0x55:
			printf("MOV D, L");
			*pc += 1;
			break;
		case 0x56:
			printf("MOV D, M");
			*pc += 1;
			break;
		case 0x57:
			printf("MOV D, A");
			*pc += 1;
			break;
		case 0x58:
			printf("MOV E, B");
			*pc += 1;
			break;
		case 0x59:
			printf("MOV E, C");
			*pc += 1;
			break;
		case 0x5A:
			printf("MOV E, D");
			*pc += 1;
			break;
		case 0x5B:
			printf("MOV E, E");
			*pc += 1;
			break;
		case 0x5C:
			printf("MOV E, H");
			*pc += 1;
			break;
		case 0x5D:
			printf("MOV E, L");
			*pc += 1;
			break;
		case 0x5E:
			printf("MOV E, M");
			*pc += 1;
			break;
		case 0x5F:
			printf("MOV E, A");
			*pc += 1;
			break;
		case 0x60:
			printf("MOV H, B");
			*pc += 1;
			break;
		case 0x61:
			printf("MOV H, C");
			*pc += 1;
			break;
		case 0x62:
			printf("MOV H, D");
			*pc += 1;
			break;
		case 0x63:
			printf("MOV H, E");
			*pc += 1;
			break;
		case 0x64:
			printf("MOV H, H");
			*pc += 1;
			break;
		case 0x65:
			printf("MOV H, L");
			*pc += 1;
			break;
		case 0x66:
			printf("MOV H, M");
			*pc += 1;
			break;
		case 0x67:
			printf("MOV H, A");
			*pc += 1;
			break;
		case 0x68:
			printf("MOV L, B");
			*pc += 1;
			break;
		case 0x69:
			printf("MOV L, C");
			*pc += 1;
			break;
		case 0x6A:
			printf("MOV L, D");
			*pc += 1;
			break;
		case 0x6B:
			printf("MOV L, E");
			*pc += 1;
			break;
		case 0x6C:
			printf("MOV L, H");
			*pc += 1;
			break;
		case 0x6D:
			printf("MOV L, L");
			*pc += 1;
			break;
		case 0x6E:
			printf("MOV L, M");
			*pc += 1;
			break;
		case 0x6F:
			printf("MOV E, A");
			*pc += 1;
			break;
		case 0x70:
			printf("MOV M, B");
			*pc += 1;
			break;
		case 0x71:
			printf("MOV M, C");
			*pc += 1;
			break;
		case 0x72:
			printf("MOV M, D");
			*pc += 1;
			break;
		case 0x73:
			printf("MOV M, E");
			*pc += 1;
			break;
		case 0x74:
			printf("MOV M, H");
			*pc += 1;
			break;
		case 0x75:
			printf("MOV M, L");
			*pc += 1;
			break;
		case 0x76:
			printf("HALT");
			*pc += 1;
			break;
		case 0x77:
			printf("MOV M, A");
			*pc += 1;
			break;
		case 0x78:
			printf("MOV A, B");
			*pc += 1;
			break;
		case 0x79:
			printf("MOV A, C");
			*pc += 1;
			break;
		case 0x7A:
			printf("MOV A, D");
			*pc += 1;
			break;
		case 0x7B:
			printf("MOV A, E");
			*pc += 1;
			break;
		case 0x7C:
			printf("MOV A, H");
			*pc += 1;
			break;
		case 0x7D:
			printf("MOV A, L");
			*pc += 1;
			break;
		case 0x7E:
			printf("MOV A, M");
			*pc += 1;
			break;
		case 0x7F:
			printf("MOV A, A");
			*pc += 1;
			break;
		case 0x80:
			printf("ADD B");
			*pc += 1;
			break;
		case 0x81:
			printf("ADD C");
			*pc += 1;
			break;
		case 0x82:
			printf("ADD D");
			*pc += 1;
			break;
		case 0x83:
			printf("ADD E");
			*pc += 1;
			break;
		case 0x84:
			printf("ADD H");
			*pc += 1;
			break;
		case 0x85:
			printf("ADD L");
			*pc += 1;
			break;
		case 0x86:
			printf("ADD M");
			*pc += 1;
			break;
		case 0x87:
			printf("ADD A");
			*pc += 1;
			break;
		case 0x88:
			printf("ADC B");
			*pc += 1;
			break;
		case 0x89:
			printf("ADC C");
			*pc += 1;
			break;
		case 0x8A:
			printf("ADC D");
			*pc += 1;
			break;
		case 0x8B:
			printf("ADC E");
			*pc += 1;
			break;
		case 0x8C:
			printf("ADC H");
			*pc += 1;
			break;
		case 0x8D:
			printf("ADC L");
			*pc += 1;
			break;
		case 0x8E:
			printf("ADC M");
			*pc += 1;
			break;
		case 0x8F:
			printf("ADC A");
			*pc += 1;
			break;
		case 0x90:
			printf("SUB B");
			*pc += 1;
			break;
		case 0x91:
			printf("SUB C");
			*pc += 1;
			break;
		case 0x92:
			printf("SUB D");
			*pc += 1;
			break;
		case 0x93:
			printf("SUB E");
			*pc += 1;
			break;
		case 0x94:
			printf("SUB H");
			*pc += 1;
			break;
		case 0x95:
			printf("SUB L");
			*pc += 1;
			break;
		case 0x96:
			printf("SUB M");
			*pc += 1;
			break;
		case 0x97:
			printf("SUB A");
			*pc += 1;
			break;
		case 0x98:
			printf("SBB B");
			*pc += 1;
			break;
		case 0x99:
			printf("SBB C");
			*pc += 1;
			break;
		case 0x9A:
			printf("SBB D");
			*pc += 1;
			break;
		case 0x9B:
			printf("SBB E");
			*pc += 1;
			break;
		case 0x9C:
			printf("SBB H");
			*pc += 1;
			break;
		case 0x9D:
			printf("SBB L");
			*pc += 1;
			break;
		case 0x9E:
			printf("SBB M");
			*pc += 1;
			break;
		case 0x9F:
			printf("SBB A");
			*pc += 1;
			break;
		case 0xA0:
			printf("ANA B");
			*pc += 1;
			break;
		case 0xA1:
			printf("ANA C");
			*pc += 1;
			break;
		case 0xA2:
			printf("ANA D");
			*pc += 1;
			break;
		case 0xA3:
			printf("ANA E");
			*pc += 1;
			break;
		case 0xA4:
			printf("ANA H");
			*pc += 1;
			break;
		case 0xA5:
			printf("ANA L");
			*pc += 1;
			break;
		case 0xA6:
			printf("ANA M");
			*pc += 1;
			break;
		case 0xA7:
			printf("ANA A");
			*pc += 1;
			break;
		case 0xA8:
			printf("XRA B");
			*pc += 1;
			break;
		case 0xA9:
			printf("XRA C");
			*pc += 1;
			break;
		case 0xAA:
			printf("XRA D");
			*pc += 1;
			break;
		case 0xAB:
			printf("XRA E");
			*pc += 1;
			break;
		case 0xAC:
			printf("XRA H");
			*pc += 1;
			break;
		case 0xAD:
			printf("XRA L");
			*pc += 1;
			break;
		case 0xAE:
			printf("XRA M");
			*pc += 1;
			break;
		case 0xAF:
			printf("XRA A");
			*pc += 1;
			break;
		case 0xB0:
			printf("ORA B");
			*pc += 1;
			break;
		case 0xB1:
			printf("ORA C");
			*pc += 1;
			break;
		case 0xB2:
			printf("ORA D");
			*pc += 1;
			break;
		case 0xB3:
			printf("ORA E");
			*pc += 1;
			break;
		case 0xB4:
			printf("ORA H");
			*pc += 1;
			break;
		case 0xB5:
			printf("ORA L");
			*pc += 1;
			break;
		case 0xB6:
			printf("ORA M");
			*pc += 1;
			break;
		case 0xB7:
			printf("ORA A");
			*pc += 1;
			break;
		case 0xB8:
			printf("CMP B");
			*pc += 1;
			break;
		case 0xB9:
			printf("CMP C");
			*pc += 1;
			break;
		case 0xBA:
			printf("CMP D");
			*pc += 1;
			break;
		case 0xBB:
			printf("CMP E");
			*pc += 1;
			break;
		case 0xBC:
			printf("CMP H");
			*pc += 1;
			break;
		case 0xBD:
			printf("CMP L");
			*pc += 1;
			break;
		case 0xBE:
			printf("CMP M");
			*pc += 1;
			break;
		case 0xBF:
			printf("CMP A");
			*pc += 1;
			break;
		case 0xC0:
			printf("RNZ A");
			*pc += 1;
			break;
		case 0xC1:
			printf("POP B");
			*pc += 1;
			break;
		case 0xC2:
			printf("JNZ $%02X%02X", memory[*pc + 2], memory[*pc + 1]);
			*pc += 3;
			break;
		case 0xC3:
			printf("JMP $%02X%02X", memory[*pc + 2], memory[*pc + 1]);
			*pc += 3;
			break;
		case 0xC4:
			printf("CNZ $%02X%02X", memory[*pc + 2], memory[*pc + 1]);
			*pc += 3;
			break;
		case 0xC5:
			printf("PUSH B");
			*pc += 1;
			break;
		case 0xC6:
			printf("ADI B, %02X", memory[*pc + 1]);
			*pc += 2;
			break;
		case 0xC7:
			printf("RST 0");
			*pc += 1;
			break;
		case 0xC8:
			printf("RZ");
			*pc += 1;
			break;
		case 0xC9:
			printf("RET");
			*pc += 1;
			break;
		case 0xCA:
			printf("JZ $%02X%02X", memory[*pc + 2], memory[*pc + 1]);
			*pc += 3;
			break;
		case 0xCB:
			printf("JMP $%02X%02X", memory[*pc + 2], memory[*pc + 1]);
			*pc += 3;
			break;
		case 0xCC:
			printf("CZ $%02X%02X", memory[*pc + 2], memory[*pc + 1]);
			*pc += 3;
			break;
		case 0xCD:
			printf("CALL $%02X%02X", memory[*pc + 2], memory[*pc + 1]);
			*pc += 3;
			break;
		case 0xCE:
			printf("ACI %02X", memory[*pc + 1]);
			*pc += 2;
			break;
		case 0xCF:
			printf("RST");
			*pc += 1;
			break;
		case 0xD0:
			printf("RNC %02X", memory[*pc + 1]);
			*pc += 1;
			break;
		case 0xD1:
			printf("POP D %02X", memory[*pc + 1]);
			*pc += 1;
			break;
		case 0xD2:
			printf("JNC $%02X%02X", memory[*pc + 2], memory[*pc + 1]);
			*pc += 3;
			break;
		case 0xD3:
			printf("OUT %02X", memory[*pc + 1]);
			*pc += 2;
			break;
		case 0xD4:
			printf("CNC $%02X%02X", memory[*pc + 2], memory[*pc + 1]);
			*pc += 3;
			break;
		case 0xD5:
			printf("PUSH D");
			*pc += 1;
			break;
		case 0xD6:
			printf("ADI %02X", memory[*pc + 1]);
			*pc += 2;
			break;
		case 0xD7:
			printf("RST 2");
			*pc += 1;
			break;
		case 0xD8:
			printf("RZ");
			*pc += 1;
			break;
		case 0xD9:
			printf("RET");
			*pc += 1;
			break;
		case 0xDA:
			printf("JC $%02X%02X", memory[*pc + 2], memory[*pc + 1]);
			*pc += 3;
			break;
		case 0xDB:
			printf("IN %02X", memory[*pc + 1]);
			*pc += 2;
			break;
		case 0xDC:
			printf("CC $%02X%02X", memory[*pc + 2], memory[*pc + 1]);
			*pc += 3;
			break;
		case 0xDD:
			printf("CALL $%02X%02X", memory[*pc + 2], memory[*pc + 1]);
			*pc += 3;
			break;
		case 0xDE:
			printf("SBI %02X", memory[*pc + 1]);
			*pc += 2;
			break;
		case 0xDF:
			printf("RST 3");
			*pc += 1;
			break;
		case 0xE0:
			printf("RPO %02X", memory[*pc + 1]);
			*pc += 1;
			break;
		case 0xE1:
			printf("POP H %02X", memory[*pc + 1]);
			*pc += 1;
			break;
		case 0xE2:
			printf("JPO $%02X%02X", memory[*pc + 2], memory[*pc + 1]);
			*pc += 3;
			break;
		case 0xE3:
			printf("XTHL");
			*pc += 1;
			break;
		case 0xE4:
			printf("CPO $%02X%02X", memory[*pc + 2], memory[*pc + 1]);
			*pc += 3;
			break;
		case 0xE5:
			printf("PUSH H");
			*pc += 1;
			break;
		case 0xE6:
			printf("ANI %02X", memory[*pc + 1]);
			*pc += 2;
			break;
		case 0xE7:
			printf("RST 4");
			*pc += 1;
			break;
		case 0xE8:
			printf("RPE");
			*pc += 1;
			break;
		case 0xE9:
			printf("PCHL");
			*pc += 1;
			break;
		case 0xEA:
			printf("JPE $%02X%02X", memory[*pc + 2], memory[*pc + 1]);
			*pc += 3;
			break;
		case 0xEB:
			printf("XCHG");
			*pc += 1;
			break;
		case 0xEC:
			printf("CPE $%02X%02X", memory[*pc + 2], memory[*pc + 1]);
			*pc += 3;
			break;
		case 0xED:
			printf("CALL $%02X%02X", memory[*pc + 2], memory[*pc + 1]);
			*pc += 3;
			break;
		case 0xEE:
			printf("XRI %02X", memory[*pc + 1]);
			*pc += 2;
			break;
		case 0xEF:
			printf("RST 5");
			*pc += 1;
			break;
		case 0xF0:
			printf("RP %02X", memory[*pc + 1]);
			*pc += 1;
			break;
		case 0xF1:
			printf("POP PSW");
			*pc += 1;
			break;
		case 0xF2:
			printf("JP $%02X%02X", memory[*pc + 2], memory[*pc + 1]);
			*pc += 3;
			break;
		case 0xF3:
			printf("DI");
			*pc += 1;
			break;
		case 0xF4:
			printf("CP $%02X%02X", memory[*pc + 2], memory[*pc + 1]);
			*pc += 3;
			break;
		case 0xF5:
			printf("PUSH PSW");
			*pc += 1;
			break;
		case 0xF6:
			printf("ORI %02X", memory[*pc + 1]);
			*pc += 2;
			break;
		case 0xF7:
			printf("RST 6");
			*pc += 1;
			break;
		case 0xF8:
			printf("RM");
			*pc += 1;
			break;
		case 0xF9:
			printf("SPHL");
			*pc += 1;
			break;
		case 0xFA:
			printf("JM $%02X%02X", memory[*pc + 2], memory[*pc + 1]);
			*pc += 3;
			break;
		case 0xFB:
			printf("EI");
			*pc += 1;
			break;
		case 0xFC:
			printf("CM $%02X%02X", memory[*pc + 2], memory[*pc + 1]);
			*pc += 3;
			break;
		case 0xFD:
			printf("CALL $%02X%02X", memory[*pc + 2], memory[*pc + 1]);
			*pc += 3;
			break;
		case 0xFE:
			printf("CPI %02X", memory[*pc + 1]);
			*pc += 2;
			break;
		case 0xFF:
			printf("RST 7");
			*pc += 1;
			break;
	}

	printf("\n");
}

int main(int argc, char *argv[]) {
	const char *rom = "../invaders";

	FILE *fp = fopen(rom, "r");
	if (fp == NULL) {
		return EXIT_FAILURE;
	}

	long size = fsize(fp);

	uint8_t bytes[size];
	const size_t count = fread(bytes, sizeof(bytes[0]), size, fp);

	if (count == size) {
		memory_write(memory, ARRAY_LENGTH(bytes), bytes);
	} else {
		if (feof(fp)) {
			printf("Error reading %s: unexpected eof\n", rom);
		}
	}

	uint16_t pc = 0;

	if (strcmp(argv[1], "-d") == 0) {
		for (int i = 0; i < size; i++) {
			disassemble(memory, &pc);
		}
	}

	fclose(fp);

	return EXIT_SUCCESS;
}
