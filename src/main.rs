use std::fs::File;
use std::io::Result;
use std::io::*;
use std::os::unix::prelude::FileExt;

const KB_64: usize = 65536;

struct Bus;

mod cpu {
    pub struct Cpu {
        pub register: Register,
        pub flags: Flag,
    }

    impl Cpu {
        pub fn new() -> Self {
            Self {
                register: Register::default(),
                flags: Flag::default(),
            }
        }
    }

    pub struct Flag {
        sign: u8,
        zero: u8,
        parity: u8,
        carry: u8,
        aux_carry: u8,
    }

    impl Default for Flag {
        fn default() -> Self {
            Self {
                sign: 0,
                zero: 0,
                parity: 0,
                carry: 0,
                aux_carry: 0,
            }
        }
    }

    pub struct Register {
        pub psw: u16,
        pub pc: u16,
        pub sp: u16,
        pub accumulator: u8,
        pub b: u8,
        pub c: u8,
        pub d: u8,
        pub e: u8,
        pub h: u8,
        pub l: u8,
    }

    impl Register {
        pub fn BC(&self) -> u16 {
            (self.b << 8 | self.c) as u16
        }
        pub fn DE(&self) -> u16 {
            (self.b << 8 | self.e) as u16
        }
        pub fn HL(&self) -> u16 {
            (self.h << 8 | self.l) as u16
        }
    }

    impl Default for Register {
        fn default() -> Self {
            Self {
                psw: 0,
                pc: 0,
                sp: 0,
                accumulator: 0,
                b: 0,
                c: 0,
                e: 0,
                d: 0,
                h: 0,
                l: 0,
            }
        }
    }
}

fn main() -> Result<()> {
    // TODO: load rom into memory
    let mut memory = vec![0u8; KB_64];

    let file = File::open("cpu_tests/TST8080.COM")?;
    file.read_exact_at(&mut memory, 0);
    // .expect("failed to load rom into memory");

    let mut cpu = cpu::Cpu::new();

    loop {
        // TODO: handle pc impl in cpu mod
        let opcode: u8 = memory[cpu.register.pc as usize];

        let rl = memory[(cpu.register.pc + 1) as usize] as u16;
        // TODO: there is a better way of doing this
        if (cpu.register.pc + 2) >= u16::MAX {
            break;
        }
        let rh = memory[(cpu.register.pc + 2) as usize] as u16;
        let d16 = rh << 8 | rl;

        let addr = ((memory[(cpu.register.pc + 2) as usize] as u16) << 8)
            | memory[(cpu.register.pc + 1) as usize] as u16;

        print!("{:#06X} ", cpu.register.pc);

        // disassemble
        match opcode {
            0x00 | 0x08 | 0x10 | 0x18 | 0x20 | 0x28 | 0x30 | 0x38 => {
                println!("NOOP");
                cpu.register.pc += 1;
            }
            0x01 => {
                println!("LXI B, {:#06X}", d16);
                cpu.register.bc = d16;
                cpu.register.pc += 3;
            }
            0x02 => {
                println!("STAX B");
                memory[cpu.register.BC() as usize] = cpu.register.accumulator;
                cpu.register.pc += 1;
            }
            0x03 => {
                println!("INX B");
                cpu.register.b += 1;
                cpu.register.c += 1;

                cpu.register.pc += 1;
            }
            0x04 => {
                println!("INR B");
                cpu.register.bc += 1;
                cpu.register.pc += 1;
            }
            0x05 => {
                println!("DCR B");
                cpu.register.pc += 1;
            }
            0x06 => {
                let d8 = memory[(cpu.register.pc + 1) as usize] as u8;
                println!("MVI B, {:#06X}", d8);
                cpu.register.pc += 2;
            }
            0x07 => {
                println!("RLC");
                cpu.register.pc += 1;
            }
            0x09 => {
                println!("DAD B");
                cpu.register.pc += 1;
            }
            0x0a => {
                println!("LDAX B");
                cpu.register.pc += 1;
            }
            0x0b => {
                println!("DCX B");
                cpu.register.pc += 1;
            }
            0x0c => {
                println!("INR C");
                cpu.register.pc += 1;
            }
            0x0d => {
                println!("DCR C");
                cpu.register.pc += 1;
            }
            0x0e => {
                let d8 = memory[(cpu.register.pc + 1) as usize] as u8;
                println!("MVI C, {:#06X}", d8);
                cpu.register.pc += 2;
            }
            0x0f => {
                println!("RRC");
                cpu.register.pc += 1;
            }
            0x11 => {
                println!("LXI D, {:#06X}", d16);
                cpu.register.pc += 3;
            }
            0x12 => {
                println!("STAX D");
                cpu.register.pc += 1;
            }
            0x13 => {
                println!("INX D");
                cpu.register.pc += 1;
            }
            0x14 => {
                println!("INR D");
                cpu.register.pc += 1
            }
            0x15 => {
                println!("DCR D");
                cpu.register.pc += 1;
            }
            0x16 => {
                let d8 = memory[(cpu.register.pc + 1) as usize] as u8;
                println!("MVI D, {:#06X}", d8);
                cpu.register.pc += 2;
            }
            0x17 => {
                println!("RAL");
                cpu.register.pc += 1;
            }
            0x19 => {
                println!("DAD D");
                cpu.register.pc += 1;
            }
            0x1a => {
                println!("LDAX D");
                cpu.register.pc += 1;
            }
            0x1b => {
                println!("DCX D");
                cpu.register.pc += 1;
            }
            0x1c => {
                println!("INR E");
                cpu.register.pc += 1;
            }
            0x1d => {
                println!("DCR E");
                cpu.register.pc += 1;
            }
            0x1e => {
                let d8 = memory[(cpu.register.pc + 1) as usize] as u8;
                println!("MVI E, {:#06X}", d8);
                cpu.register.pc += 2;
            }
            0x1f => {
                println!("RAR");
                cpu.register.pc += 1;
            }
            0x21 => {
                println!("LXI H, {:#06X}", d16);
                cpu.register.pc += 3;
            }
            0x22 => {
                println!("SHLD {:#06X}", addr);
                cpu.register.pc += 3;
            }
            0x23 => {
                println!("INX H");
                cpu.register.pc += 1;
            }
            0x24 => {
                println!("INR H");
                cpu.register.pc += 1;
            }
            0x25 => {
                println!("DCR M");
                cpu.register.pc += 1;
            }
            0x26 => {
                let d8 = memory[(cpu.register.pc + 1) as usize] as u8;
                println!("MVI H, {:#06X}", d8);
                cpu.register.pc += 2;
            }
            0x27 => {
                println!("DAA");
                cpu.register.pc += 1;
            }
            0x29 => {
                println!("DAD H");
                cpu.register.pc += 1;
            }
            0x2a => {
                println!("LHLD {:#06X}", addr);
                cpu.register.pc += 3;
            }
            0x2b => {
                println!("DCX H");
                cpu.register.pc += 1;
            }
            0x2c => {
                println!("INR L");
                cpu.register.pc += 1;
            }
            0x2d => {
                println!("DCR L");
                cpu.register.pc += 1;
            }
            0x2e => {
                let d8 = memory[(cpu.register.pc + 1) as usize] as u8;
                println!("MOV L, {:#06X}", d8);
                cpu.register.pc += 2;
            }
            0x2f => {
                println!("CMA");
                cpu.register.pc += 1;
            }
            0x31 => {
                println!("LXI SP, {:#06X}", d16);
                cpu.register.pc += 3;
            }
            0x32 => {
                println!("STA {:#06X}", addr);
                cpu.register.pc += 3;
            }
            0x33 => {
                println!("INX SP");
                cpu.register.pc += 1;
            }
            0x34 => {
                println!("INR M");
                cpu.register.pc += 1;
            }
            0x35 => {
                println!("DCR M");
                cpu.register.pc += 1;
            }
            0x36 => {
                let d8 = memory[(cpu.register.pc + 1) as usize] as u8;
                println!("MVI M, {:#06X}", d8);
                cpu.register.pc += 2;
            }
            0x37 => {
                println!("STC");
                cpu.register.pc += 1;
            }
            0x39 => {
                println!("DAD SP");
                cpu.register.pc += 1;
            }
            0x3a => {
                println!("LDA {:#06X}", addr);
                cpu.register.pc += 3;
            }
            0x3b => {
                println!("DCX SP");
                cpu.register.pc += 1;
            }
            0x3c => {
                println!("INR A");
                cpu.register.pc += 1;
            }
            0x3d => {
                println!("DCR A");
                cpu.register.pc += 1;
            }
            0x3e => {
                let d8 = memory[(cpu.register.pc + 1) as usize] as u8;
                println!("MVI A, {:#06X}", d8);
                cpu.register.pc += 2;
            }
            0x3f => {
                println!("CMC");
                cpu.register.pc += 1;
            }
            0x40 => {
                println!("MOV B, B");
                cpu.register.pc += 1;
            }
            0x41 => {
                println!("MOV B, C");
                cpu.register.pc += 1
            }
            0x42 => {
                println!("MOV B, D");
                cpu.register.pc += 1;
            }
            0x43 => {
                println!("MOV B, E");
                cpu.register.pc += 1;
            }
            0x44 => {
                println!("MOV B, H");
                cpu.register.pc += 1;
            }
            0x45 => {
                println!("MOV B, L");
                cpu.register.pc += 1;
            }
            0x46 => {
                println!("MOV B,M");
                cpu.register.pc += 1;
            }
            0x47 => {
                println!("MOV B, A");
                cpu.register.pc += 1;
            }
            0x48 => {
                println!("MOV C, B");
                cpu.register.pc += 1;
            }
            0x49 => {
                println!("MOV C, C");
                cpu.register.pc += 1
            }
            0x4a => {
                println!("MOV C, D");
                cpu.register.pc += 1
            }
            0x4b => {
                println!("MOV C, E");
                cpu.register.pc += 1
            }
            0x4c => {
                println!("MOV C, H");
                cpu.register.pc += 1
            }
            0x4d => {
                println!("MOV C, L");
                cpu.register.pc += 1;
            }
            0x4e => {
                println!("MOV C, M");
                cpu.register.pc += 1;
            }
            0x4f => {
                println!("MOV C, A");
                cpu.register.pc += 1;
            }
            0x50 => {
                println!("MOV D, B");
                cpu.register.pc += 1
            }
            0x51 => {
                println!("MOV D, C");
                cpu.register.pc += 1
            }
            0x52 => {
                println!("MOV D, D");
                cpu.register.pc += 1
            }
            0x53 => {
                println!("MOV D, E");
                cpu.register.pc += 1
            }
            0x54 => {
                println!("MOV, D, H");
                cpu.register.pc += 1
            }
            0x55 => {
                println!("MOV, D, L");
                cpu.register.pc += 1
            }
            0x56 => {
                println!("MOV D, M");
                cpu.register.pc += 1
            }
            0x57 => {
                println!("MOV D, A");
                cpu.register.pc += 1
            }
            0x58 => {
                println!("MOV E, B");
                cpu.register.pc += 1
            }
            0x59 => {
                println!("MOV E, C");
                cpu.register.pc += 1
            }
            0x5a => {
                println!("MOV E, D");
                cpu.register.pc += 1
            }
            0x5b => {
                println!("MOV E, E");
                cpu.register.pc += 1
            }
            0x5c => {
                println!("MOV E, H");
                cpu.register.pc += 1
            }
            0x5d => {
                println!("MOV E, L");
                cpu.register.pc += 1
            }
            0x5e => {
                println!("MOV E, M");
                cpu.register.pc += 1
            }
            0x5f => {
                println!("MOV E, A");
                cpu.register.pc += 1
            }
            0x60 => {
                println!("MOV H, B");
                cpu.register.pc += 1
            }
            0x61 => {
                println!("MOV H, C");
                cpu.register.pc += 1
            }
            0x62 => {
                println!("MOV H, D");
                cpu.register.pc += 1
            }
            0x63 => {
                println!("MOV H, E");
                cpu.register.pc += 1
            }
            0x64 => {
                println!("MOV H, H");
                cpu.register.pc += 1
            }
            0x65 => {
                println!("MOV H, L");
                cpu.register.pc += 1
            }
            0x66 => {
                println!("MOV H, M");
                cpu.register.pc += 1
            }
            0x67 => {
                println!("MOV H, A");
                cpu.register.pc += 1
            }
            0x68 => {
                println!("MOV L, B");
                cpu.register.pc += 1
            }
            0x69 => {
                println!("MOV L, C");
                cpu.register.pc += 1
            }
            0x6a => {
                println!("MOV L, D");
                cpu.register.pc += 1
            }
            0x6b => {
                println!("MOV L, E");
                cpu.register.pc += 1
            }
            0x6c => {
                println!("MOV L, H");
                cpu.register.pc += 1
            }
            0x6d => {
                println!("MOV L, L");
                cpu.register.pc += 1
            }
            0x6e => {
                println!("MOV L, M");
                cpu.register.pc += 1
            }
            0x6f => {
                println!("MOV L, A");
                cpu.register.pc += 1
            }
            0x70 => {
                println!("MOV M, B");
                cpu.register.pc += 1
            }
            0x71 => {
                println!("MOV M, C");
                cpu.register.pc += 1
            }
            0x72 => {
                println!("MOV M, D");
                cpu.register.pc += 1
            }
            0x73 => {
                println!("MOV M, E");
                cpu.register.pc += 1
            }
            0x74 => {
                println!("MOV M, H");
                cpu.register.pc += 1
            }
            0x75 => {
                println!("MOV M, L");
                cpu.register.pc += 1
            }
            0x76 => {
                println!("HALT");
                cpu.register.pc += 1
            }
            0x77 => {
                println!("MOV M, A");
                cpu.register.pc += 1
            }
            0x78 => {
                println!("MOV A, B");
                cpu.register.pc += 1
            }
            0x79 => {
                println!("MOV A, C");
                cpu.register.pc += 1
            }
            0x7a => {
                println!("MOV A, D");
                cpu.register.pc += 1
            }
            0x7b => {
                println!("MOV A, E");
                cpu.register.pc += 1
            }
            0x7c => {
                println!("MOV A, H");
                cpu.register.pc += 1
            }
            0x7d => {
                println!("MOV A, L");
                cpu.register.pc += 1
            }
            0x7e => {
                println!("MOV A, M");
                cpu.register.pc += 1
            }
            0x7f => {
                println!("MOV A, A");
                cpu.register.pc += 1
            }
            0x80 => {
                println!("ADD B");
                cpu.register.pc += 1;
            }
            0x81 => {
                println!("ADD C");
                cpu.register.pc += 1;
            }
            0x82 => {
                println!("ADD D");
                cpu.register.pc += 1;
            }
            0x83 => {
                println!("ADD E");
                cpu.register.pc += 1;
            }
            0x84 => {
                println!("ADD H");
                cpu.register.pc += 1;
            }
            0x85 => {
                println!("ADD L");
                cpu.register.pc += 1;
            }
            0x86 => {
                println!("ADD M");
                cpu.register.pc += 1;
            }
            0x87 => {
                println!("ADD A");
                cpu.register.pc += 1;
            }
            0x88 => {
                println!("ADC B");
                cpu.register.pc += 1;
            }
            0x89 => {
                println!("ADC D");
                cpu.register.pc += 1;
            }
            0x8a => {
                println!("ADC D");
                cpu.register.pc += 1;
            }
            0x8b => {
                println!("ADC E");
                cpu.register.pc += 1;
            }
            0x8c => {
                println!("ADC H");
                cpu.register.pc += 1;
            }
            0x8d => {
                println!("ADC L");
                cpu.register.pc += 1;
            }
            0x8e => {
                println!("ADC M");
                cpu.register.pc += 1;
            }
            0x8f => {
                println!("ADC A");
                cpu.register.pc += 1;
            }
            0x90 => {
                println!("SUB B");
                cpu.register.pc += 1;
            }
            0x91 => {
                println!("SUB C");
                cpu.register.pc += 1;
            }
            0x92 => {
                println!("SUB D");
                cpu.register.pc += 1;
            }
            0x93 => {
                println!("SUB E");
                cpu.register.pc += 1;
            }
            0x94 => {
                println!("SUB H");
                cpu.register.pc += 1;
            }
            0x95 => {
                println!("SUB L");
                cpu.register.pc += 1;
            }
            0x96 => {
                println!("SUB M");
                cpu.register.pc += 1;
            }
            0x97 => {
                println!("SUB A");
                cpu.register.pc += 1;
            }
            0x98 => {
                println!("SBB B");
                cpu.register.pc += 1;
            }
            0x99 => {
                println!("SBB C");
                cpu.register.pc += 1;
            }
            0x9a => {
                println!("SBB D");
                cpu.register.pc += 1;
            }
            0x9b => {
                println!("SBB E");
                cpu.register.pc += 1;
            }
            0x9c => {
                println!("SBB H");
                cpu.register.pc += 1;
            }
            0x9d => {
                println!("SBB L");
                cpu.register.pc += 1;
            }
            0x9e => {
                println!("SBB M");
                cpu.register.pc += 1;
            }
            0x9f => {
                println!("SBB A");
                cpu.register.pc += 1;
            }
            0xa0 => {
                println!("ANA B");
                cpu.register.pc += 1;
            }
            0xa1 => {
                println!("ANA C");
                cpu.register.pc += 1;
            }
            0xa2 => {
                println!("ANA D");
                cpu.register.pc += 1;
            }
            0xa3 => {
                println!("ANA E");
                cpu.register.pc += 1;
            }
            0xa4 => {
                println!("ANA H");
                cpu.register.pc += 1;
            }
            0xa5 => {
                println!("ANA L");
                cpu.register.pc += 1;
            }
            0xa6 => {
                println!("ANA M");
                cpu.register.pc += 1;
            }
            0xa7 => {
                println!("ANA A");
                cpu.register.pc += 1;
            }
            0xa8 => {
                println!("XRA B");
                cpu.register.pc += 1;
            }
            0xa9 => {
                println!("XRA C");
                cpu.register.pc += 1;
            }
            0xaa => {
                println!("XRA D");
                cpu.register.pc += 1;
            }
            0xab => {
                println!("XRA E");
                cpu.register.pc += 1;
            }
            0xac => {
                println!("XRA H");
                cpu.register.pc += 1;
            }
            0xad => {
                println!("XRA L");
                cpu.register.pc += 1;
            }
            0xae => {
                println!("XRA M");
                cpu.register.pc += 1;
            }
            0xaf => {
                println!("XRA A");
                cpu.register.pc += 1;
            }
            0xb0 => {
                println!("ORA B");
                cpu.register.pc += 1;
            }
            0xb1 => {
                println!("ORA C");
                cpu.register.pc += 1;
            }
            0xb2 => {
                println!("ORA D");
                cpu.register.pc += 1;
            }
            0xb3 => {
                println!("ORA E");
                cpu.register.pc += 1;
            }
            0xb4 => {
                println!("ORA H");
                cpu.register.pc += 1;
            }
            0xb5 => {
                println!("ORA L");
                cpu.register.pc += 1;
            }
            0xb6 => {
                println!("ORA M");
                cpu.register.pc += 1;
            }
            0xb7 => {
                println!("ORA A");
                cpu.register.pc += 1;
            }
            0xb8 => {
                println!("CMP B");
                cpu.register.pc += 1;
            }
            0xb9 => {
                println!("CMP C");
                cpu.register.pc += 1;
            }
            0xba => {
                println!("CMP D");
                cpu.register.pc += 1;
            }
            0xbb => {
                println!("CMP E");
                cpu.register.pc += 1;
            }
            0xbc => {
                println!("CMP H");
                cpu.register.pc += 1;
            }
            0xbd => {
                println!("CMP L");
                cpu.register.pc += 1;
            }
            0xbe => {
                println!("CMP M");
                cpu.register.pc += 1;
            }
            0xbf => {
                println!("CMP A");
                cpu.register.pc += 1;
            }
            0xc0 => {
                println!("RNZ");
                cpu.register.pc += 1;
            }
            0xc1 => {
                println!("POP B");
                cpu.register.pc += 1;
            }
            0xc2 => {
                // println!("JNZ {}", addr);
                cpu.register.pc += 3;
            }
            0xc3 | 0xcb => {
                println!("JMP {}", addr);
                cpu.register.pc += 3;
            }
            0xc4 => {
                println!("CNZ {}", addr);
                cpu.register.pc += 3;
            }
            0xc5 => {
                println!("PUSH B");
                cpu.register.pc += 1
            }
            0xc6 => {
                let d8 = memory[(cpu.register.pc + 1) as usize] as u8;
                println!("ADI {}", d8);
                cpu.register.pc += 2;
            }
            0xc7 => {
                println!("RST 0");
                cpu.register.pc += 1;
            }
            0xc8 => {
                println!("RZ");
                cpu.register.pc += 1;
            }
            0xc9 | 0xd9 => {
                println!("RET");
                cpu.register.pc += 1;
            }
            0xca => {
                println!("JZ {}", addr);
                cpu.register.pc += 3;
            }
            0xcc => {
                println!("CZ {}", addr);
                cpu.register.pc += 3;
            }
            0xcd | 0xdd | 0xed | 0xfd => {
                println!("CALL {}", addr);
                cpu.register.pc += 1;
            }
            0xce => {
                let d8 = memory[(cpu.register.pc + 1) as usize] as u8;
                println!("ACI {}", d8);
                cpu.register.pc += 2;
            }
            0xcf => {
                println!("RST 1");
                cpu.register.pc += 1;
            }
            0xd0 => {
                println!("RNC");
                cpu.register.pc += 1;
            }
            0xd1 => {
                println!("POP D");
                cpu.register.pc += 1;
            }
            0xd2 => {
                println!("JNC {}", addr);
                cpu.register.pc += 3;
            }
            0xd3 => {
                let d8 = memory[(cpu.register.pc + 1) as usize] as u8;
                println!("OUT {}", d8);
                cpu.register.pc += 2;
            }
            0xd4 => {
                println!("CNC {}", addr);
                cpu.register.pc += 3;
            }
            0xd5 => {
                println!("PUSH D");
                cpu.register.pc += 1;
            }
            0xd6 => {
                let d8 = memory[(cpu.register.pc + 1) as usize] as u8;
                println!("SUI {}", d8);
                cpu.register.pc += 2;
            }
            0xd7 => {
                println!("RST 2");
                cpu.register.pc += 1;
            }
            0xd8 => {
                println!("RC");
                cpu.register.pc += 1;
            }
            0xda => {
                println!("JC {}", addr);
                cpu.register.pc += 3;
            }
            0xdb => {
                let d8 = memory[(cpu.register.pc + 1) as usize] as u8;
                println!("IN {}", d8);
                cpu.register.pc += 2;
            }
            0xdc => {
                println!("CC {}", addr);
                cpu.register.pc += 3;
            }
            0xde => {
                let d8 = memory[(cpu.register.pc + 1) as usize] as u8;
                println!("SBI {}", d8);
                cpu.register.pc += 2;
            }
            0xdf => {
                println!("RST 3");
                cpu.register.pc += 1;
            }
            0xe0 => {
                println!("RPO");
                cpu.register.pc += 1;
            }
            0xe1 => {
                println!("POP H");
                cpu.register.pc += 1;
            }
            0xe2 => {
                println!("JPO {}", addr);
                cpu.register.pc += 3;
            }
            0xe3 => {
                println!("XTHL");
                cpu.register.pc += 1;
            }
            0xe4 => {
                println!("CPO {}", addr);
                cpu.register.pc += 3;
            }
            0xe5 => {
                println!("PUSH H");
                cpu.register.pc += 1;
            }
            0xe6 => {
                let d8 = memory[(cpu.register.pc + 1) as usize] as u8;
                println!("ANI {}", d8);
                cpu.register.pc += 2;
            }
            0xe7 => {
                println!("RST 4");
                cpu.register.pc += 1;
            }
            0xe8 => {
                println!("RPE");
                cpu.register.pc += 1;
            }
            0xe9 => {
                println!("PCHL");
                cpu.register.pc += 1;
            }
            0xea => {
                println!("JPE {}", addr);
                cpu.register.pc += 3;
            }
            0xeb => {
                println!("XCHG");
                cpu.register.pc += 1;
            }
            0xec => {
                println!("CPE {}", addr);
                cpu.register.pc += 3;
            }
            0xee => {
                let d8 = memory[(cpu.register.pc + 1) as usize] as u8;
                println!("XRI {}", d8);
                cpu.register.pc += 2;
            }
            0xef => {
                println!("RST 5");
                cpu.register.pc += 1;
            }
            0xf0 => {
                println!("RP");
                cpu.register.pc += 1;
            }
            0xf1 => {
                println!("POP PSW");
                cpu.register.pc += 1;
            }
            0xf2 => {
                println!("JP {}", addr);
                cpu.register.pc += 2;
            }
            0xf3 => {
                println!("DI");
                cpu.register.pc += 1;
            }
            0xf4 => {
                println!("CP {}", addr);
                cpu.register.pc += 3;
            }
            0xf5 => {
                println!("PUSH PSW");
                cpu.register.pc += 1;
            }
            0xf6 => {
                let d8 = memory[(cpu.register.pc + 1) as usize] as u8;
                println!("ORI {}", d8);
                cpu.register.pc += 2;
            }
            0xf7 => {
                println!("RST 6");
                cpu.register.pc += 1;
            }
            0xf8 => {
                println!("RM");
                cpu.register.pc += 1;
            }
            0xf9 => {
                println!("SPHL");
                cpu.register.pc += 1;
            }
            0xfa => {
                println!("JM {}", addr);
                cpu.register.pc += 3;
            }
            0xfb => {
                println!("EI");
                cpu.register.pc += 1;
            }
            0xfc => {
                println!("CM {}", addr);
                cpu.register.pc += 3;
            }
            0xfe => {
                let d8 = memory[(cpu.register.pc + 1) as usize] as u8;
                println!("CPI {}", d8);
                cpu.register.pc += 2;
            }
            0xff => {
                println!("RST 7");
                cpu.register.pc += 1;
            }
            _ => unreachable!(),
        }
    }

    Ok(())
}
