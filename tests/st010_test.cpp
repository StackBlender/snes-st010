// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) StackBlender LLC
// The commands' known answers, and a saved and loaded state goes on alike.
#include "st010/St010.h"

#include <cstdio>

static int failures = 0;
static void expect(bool ok, const char* what) {
	if(!ok) std::printf("FAIL %s\n", what), failures++;
}

static void put16(st010::St010& c, unsigned a, uint16_t v) { c.writeRam(uint16_t(a), uint8_t(v)); c.writeRam(uint16_t(a + 1), uint8_t(v >> 8)); }
static uint16_t get16(const st010::St010& c, unsigned a) { return uint16_t(c.readRam(uint16_t(a)) | c.readRam(uint16_t(a + 1)) << 8); }
static void run(st010::St010& c, uint8_t command) { c.writeRam(0x20, command); c.writeRam(0x21, 0x80); }

int main() {
	using st010::St010;
	St010 chip;
	put16(chip, 0, 0x0d00); put16(chip, 2, 0x0293); run(chip, 0x06);
	expect(get16(chip, 0x10) == 0xee00 && get16(chip, 0x12) == 0x0042, "multiply: 2 a b");
	expect(chip.readRam(0x20) == 0 && !(chip.readRam(0x21) & 0x80), "done: 020h and the busy bit cleared");
	put16(chip, 0, 0x0047); put16(chip, 2, 0x01ff); put16(chip, 4, 0x0049); run(chip, 0x03);
	expect(get16(chip, 0x11) == 0x0028 && get16(chip, 0x15) == 0x0123, "scale");
	expect(St010::distance(100, 100) == 136 && St010::distance(0x47, 0x1ff) == 0x207, "distance estimate");
	expect(St010::sine(64) == 32767 && St010::sine(0) == 0 && St010::cosine(0) == 32767, "sine table");
	expect(St010::direction(128, 0) == 0xc0 && St010::direction(0, 128) == 0x80 && St010::direction(-128, -128) == 0x20, "direction");
	expect(St010::perspective(0) == 896 && St010::perspective(175) == 43, "perspective");
	put16(chip, 0, 0x4000); run(chip, 0x07);
	expect(get16(chip, 0x0f0) == 0 && get16(chip, 0x250) == 895 && int16_t(get16(chip, 0x3b0)) == -896, "raster tables at 90 degrees");
	put16(chip, 0x24, 3);
	put16(chip, 0x40, 5); put16(chip, 0x42, 9); put16(chip, 0x44, 5);
	put16(chip, 0x80, 1); put16(chip, 0x82, 2); put16(chip, 0x84, 3);
	run(chip, 0x02);
	expect(get16(chip, 0x40) == 9 && get16(chip, 0x80) == 2 && get16(chip, 0x82) == 1 && get16(chip, 0x84) == 3, "sort, largest first, equal keys in order");
	auto state = chip.saveState();
	St010 copy;
	expect(copy.loadState(state.data(), state.size()), "load a state");
	put16(chip, 0, 0x1234); put16(chip, 2, 0xf567); put16(chip, 4, 0x2345); run(chip, 0x08);
	put16(copy, 0, 0x1234); put16(copy, 2, 0xf567); put16(copy, 4, 0x2345); run(copy, 0x08);
	expect(chip.saveState() == copy.saveState(), "a loaded state goes on alike");
	uint8_t junk[3] = {1, 2, 3};
	expect(!copy.loadState(junk, 3), "a blob that isn't a state is refused");
	std::printf("%s\n", failures ? "st010: FAILED" : "st010: ok");
	return failures ? 1 : 0;
}
