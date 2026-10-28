// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) StackBlender LLC
//
// A clean-room ST010: the Seta coprocessor in F1 ROC II (an NEC uPD96050 running the chip
// maker's program), reimplemented from the public list of its commands and black-box
// measurement of what the chip does, never from its program.
//
// The SNES sees the chip's 4 KiB of RAM (battery-backed: the game keeps its records there). A
// command's inputs go in at 000h..., its number at 020h; setting bit 7 of 021h starts it, and the
// chip clears 020h and that bit when it is done. Here a command runs at once, as the write that
// starts it. Its results are left in the RAM (docs/commands.md).
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace st010 {

class St010 {
public:
	St010();

	// The RAM as the SNES maps it (address 000h-FFFh).
	uint8_t readRam(uint16_t address) const { return m_ram[address & 0xfff]; }
	void writeRam(uint16_t address, uint8_t value);

	// The RAM for the host to keep with the cartridge's save, and its whole state for save
	// states (the RAM is all of it).
	uint8_t* ram() { return m_ram.data(); }
	static constexpr size_t RamSize = 0x1000;
	std::vector<uint8_t> saveState() const;
	bool loadState(const uint8_t* data, size_t size);

	// The building blocks, exposed for tests. Angles: 256 steps a turn (the high byte of a 16-bit
	// angle).
	static int16_t sine(uint8_t step);
	static int16_t cosine(uint8_t step) { return sine(uint8_t(step + 64)); }
	static uint16_t distance(int16_t x, int16_t y);                  // 04
	static uint8_t direction(int16_t x, int16_t y);                  // 01: the angle of (-x, -y)
	static int16_t perspective(int line);                            // 07: line 0-175

private:
	void execute();
	int16_t r16(unsigned a) const { return int16_t(m_ram[a] | m_ram[a + 1] << 8); }
	void w16(unsigned a, uint16_t v) { m_ram[a] = uint8_t(v); m_ram[a + 1] = uint8_t(v >> 8); }
	void w32(unsigned a, uint32_t v) { w16(a, uint16_t(v)); w16(a + 2, uint16_t(v >> 16)); }

	std::array<uint8_t, RamSize> m_ram{};
};

} // namespace st010
