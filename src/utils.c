#include "types.h"

/// Combine (r1, r2) 8-bit registers to make a 16-bit register
/// Example:
///	```
///	uint8_t a = 0xAE;
///	uint8_t b = 0x04;
///	uint16_t result = combine(a, b);
///	printf("%04X", result) => 0xAE04
///	```
u16 combine(const u8 r1, const u8 r2) { return (r1 << 8) | r2; }

/// Remove the lower byte of a 16-bit value and preserve the
/// high byte
/// Example:
///	```
///	uint8_t a = 0xAE12;
///	uint16_t result = get_hi(a);
///	printf("%04X", result) => 0x00AE
///	```
u8 get_hi(const u16 val) { return val >> 8; }

/// Remove the higher byte of a 16-bit value and preserve the
/// low byte
/// Example:
///	```
///	uint8_t a = 0xAE12;
///	uint16_t result = get_lo(a);
///	printf("%04X", result) => 0x0012
///	```
u8 get_lo(const u16 val) { return val & 0xFF; }
